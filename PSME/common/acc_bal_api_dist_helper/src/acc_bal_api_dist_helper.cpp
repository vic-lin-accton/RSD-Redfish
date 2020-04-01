#include "../include/acc_bal_api_dist_helper/acc_bal_api_dist_helper.hpp"
#include "../include/acc_bal_api_dist_helper/asgvolt64.hpp"
#include "../include/acc_bal_api_dist_helper/asxvolt16.hpp"
#include <dlfcn.h>
#include <stdlib.h>
#include <thread>
using namespace acc_bal_api_dist_helper;

#ifdef __cplusplus
extern "C" {
#include <bcmolt_api.h>
#include <bcmolt_host_api.h>
#include <bcmolt_api_model_supporting_enums.h>
#include <bcmolt_api_conn_mgr.h>
#include <bcmos_common.h>
#include <bcm_config.h>
}
#endif
#include "../include/acc_bal_api_dist_helper/com_def.hpp"

bcmolt_oltid dev_id = 0;
const std::string upstream = "upstream";
const std::string downstream = "downstream";

#define SERIAL_NUMBER_SIZE 12
#define EAP_ETHER_TYPE 34958
#define TM_UPSTREAM_SCHED_ID_START 1
#define TM_DOWNSTREAM_SCHED_ID_START 180
#define TM_Q_SET_ID (bcmolt_tm_queue_set_id)32768U

bcmos_errno (*d_bcmbal_cfg_get)(bcmolt_oltid olt, bcmolt_cfg *objinfo) = NULL;
bcmos_errno (*d_bcmbal_cfg_set)(bcmolt_oltid olt, bcmolt_cfg *objinfo) = NULL;
bcmos_errno (*d_bcmbal_cfg_clear)(bcmolt_oltid olt, bcmolt_cfg *objinfo) = NULL;
bcmos_errno (*d_bcmbal_stat_get)(bcmolt_oltid olt, bcmolt_stat *stat, bcmolt_stat_flags flags) = NULL;
bcmos_errno (*d_bcmbal_oper_submit)(bcmolt_oltid olt, bcmolt_oper *oper) = NULL;
bcmos_errno (*d_bcmbal_ind_subscribe)(bcmolt_oltid olt, bcmolt_rx_cfg *rx_cfg) = NULL;
bcmos_errno (*d_bcmbal_host_init)(const bcmolt_host_init_parms *init_parms) = NULL;
bcmos_bool (*d_bcmbal_api_conn_mgr_is_connected)(bcmolt_goid olt) = NULL;
void (*d_bcmbal_msg_free)(bcmolt_msg *msg) = NULL;
void (*d_bcmbal_api_set_prop_present)(bcmolt_msg *msg, const void *prop_ptr) = NULL;
void (*d_bcmbal_usleep)(uint32_t us) = NULL;
const char *(*d_bcmbal_strerror)(bcmos_errno err) = NULL;

static inline int get_default_tm_sched_id(int intf_id, std::string direction)
{
    if (direction.compare(upstream) == 0)
    {
        return TM_UPSTREAM_SCHED_ID_START + intf_id;
    }
    else if (direction.compare(downstream) == 0)
    {
        return TM_DOWNSTREAM_SCHED_ID_START + intf_id;
    }
    else
    {
        printf("invalid direction - %s\n", direction.c_str());
        return 0;
    }
}

static void OltBalReady(short unsigned int olt, bcmolt_msg *msg)
{
    if (msg->subgroup == BCMOLT_OLT_AUTO_SUBGROUP_BAL_READY)
    {
        //After BAL ready, enable OLT maple
        bcmolt_odid dev;
        UNUSED(olt);

        auto &rOLT = Olt_Device::Olt_Device::get_instance();
        int maple_num = rOLT.get_maple_num();

        for (dev = 0; dev < maple_num; dev++)
        {
            bcmos_errno err = BCM_ERR_INTERNAL;
            bcmolt_device_cfg dev_cfg = {};
            bcmolt_device_key dev_key = {};
            dev_key.device_id = dev;
            BCMOLT_CFG_INIT(&dev_cfg, device, dev_key);
            BCMOLT_MSG_FIELD_GET(&dev_cfg, system_mode);

            if (d_bcmbal_cfg_get)
            {
                err = d_bcmbal_cfg_get(dev_id, &dev_cfg.hdr);

                if (err == BCM_ERR_NOT_CONNECTED)
                {
                    bcmolt_device_key key = {.device_id = dev};
                    bcmolt_device_connect oper;
                    bcmolt_device_cfg cfg;
                    BCMOLT_OPER_INIT(&oper, device, connect, key);
                    BCMOLT_MSG_FIELD_SET(&oper, inni_config.mode, BCMOLT_INNI_MODE_ALL_10_G_XFI);

                    if (rOLT.get_platform() == "asxvolt16")
                    {
                        printf("asxvolt16 Enable Maple - %d/%d\n", dev + 1, maple_num);
                        BCMOLT_MSG_FIELD_SET(&oper, system_mode, BCMOLT_SYSTEM_MODE_XGS__2_X);
                    }
                    else if (rOLT.get_platform() == "asgvolt64")
                    {
                        printf("asgvolt64 Enable Maple - %d/%d\n", dev + 1, maple_num);
                        BCMOLT_MSG_FIELD_SET(&oper, inni_config.mux, BCMOLT_INNI_MUX_FOUR_TO_ONE);
                        BCMOLT_MSG_FIELD_SET(&oper, system_mode, BCMOLT_SYSTEM_MODE_GPON__16_X);
                    }
                    else
                    {
                        printf("!!!!!!!Can't identify PLATFORM TYPE!!!!!!!\r\n");
                        return;
                    }

                    if (d_bcmbal_oper_submit)
                    {
                        err = d_bcmbal_oper_submit(dev_id, &oper.hdr);
                        if (err)
                            printf("Enable Maple deivce %d failed\n", dev);
                        d_bcmbal_usleep(200000);
                    }
                    else
                        printf("!!!!!!!d_bcmbal_oper_submit ERROR!!!!!!!\r\n");
                }
                else
                {
                    printf("Maple deivce %d already connected\n", dev);
                }
            }
            else
            {
                printf("d_bcmbal_cfg_get error\r\n");
                if (d_bcmbal_msg_free)
                    d_bcmbal_msg_free(msg);
                else
                    printf("d_bcmbal_msg_free error\r\n");
                return;
            }
        }
        rOLT.register_callback();
        printf("BAL Ready !!!!!!!!\r\n");
        rOLT.set_bal_status(true);
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");
    return;
}

static void Olt_itf_change(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_PON_INTERFACE:
        switch (msg->subgroup)
        {
        case BCMOLT_PON_INTERFACE_AUTO_SUBGROUP_STATE_CHANGE_COMPLETED:
        {
            bcmolt_pon_interface_state_change_completed_data *data = &((bcmolt_pon_interface_state_change_completed *)msg)->data;
            if (INTERFACE_STATE_IF_UP(data->new_state))
                printf("PON %d is UP completed\r\n", ((bcmolt_pon_interface_state_change_completed *)msg)->key.pon_ni);
            if (INTERFACE_STATE_IF_DOWN(data->new_state))
                printf("PON %d is DOWN completed\r\n", ((bcmolt_pon_interface_state_change_completed *)msg)->key.pon_ni);
            break;
        }
        default:
            break;
        }
        break;
    case BCMOLT_OBJ_ID_NNI_INTERFACE:
        switch (msg->subgroup)
        {
        case BCMOLT_NNI_INTERFACE_AUTO_SUBGROUP_STATE_CHANGE:
        {
            bcmolt_nni_interface_state_change_data *data = &((bcmolt_nni_interface_state_change *)msg)->data;

            if (INTERFACE_STATE_IF_UP(data->new_state))
                printf("NNI %d is UP state change.\r\n", ((bcmolt_nni_interface_state_change *)msg)->key.id);
            if (INTERFACE_STATE_IF_DOWN(data->new_state))
                printf("NNI %d is DOWN state change.\r\n", ((bcmolt_nni_interface_state_change *)msg)->key.id);
            break;
        }
        default:
            break;
        }
    default:
        break;
    }

    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");

    return;
}

static void OltOmciIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_ONU:
        switch (msg->subgroup)
        {
        case BCMOLT_ONU_AUTO_SUBGROUP_OMCI_PACKET:
        {
            bcmolt_onu_key *key = &((bcmolt_onu_omci_packet *)msg)->key;
            bcmolt_onu_omci_packet_data *data = &((bcmolt_onu_omci_packet *)msg)->data;
            printf("OMCI OltOmciIndication : pon_ni %d, onu_id %d\n", key->pon_ni, key->onu_id);
            break;
        }
        default:
            break;
        }
    default:
        break;
    }

    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");
    return;
}

static void OltOperIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_COMPLETE)
    {
        static int count = 0;
        auto &rOLT = Olt_Device::Olt_Device::get_instance();
        count++;

        if (count == rOLT.get_maple_num())
        {
            bcmolt_rx_cfg cb_cfg = {};
            cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
            cb_cfg.rx_cb = OltOmciIndication;
            cb_cfg.subgroup = bcmolt_onu_auto_subgroup_omci_packet;
            cb_cfg.module = BCMOS_MODULE_ID_OMCI_TRANSPORT;
            if (d_bcmbal_ind_subscribe)
                d_bcmbal_ind_subscribe(dev_id, &cb_cfg);
            else
            {

                printf("d_bcmbal_ind_subscribe error\r\n");
                return;
            }

            printf("OLT Ready !!!!!!!!\r\n");
            rOLT.set_olt_status(true);
            count = 0;
        }
    }
    else if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_DISCONNECTION_COMPLETE)
    {
        printf("OltOperIndication call back! BCMOLT_DEVICE_AUTO_SUBGROUP_DISCONNECTION_COMPLETE\r\n");
    }
    else if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_FAILURE)
    {
        printf("OltOperIndication call back! BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_FAILURE\r\n");
    }
    else
    {
        printf("OltOperIndication no match case !!!!");
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");
    return;
}

static int interface_key_to_port_no(bcmolt_interface_id intf_id, bcmolt_interface_type intf_type)
{
    if (intf_type == BCMOLT_INTERFACE_TYPE_NNI)
    {
        return (0x1 << 16) + intf_id;
    }
    if (intf_type == BCMOLT_INTERFACE_TYPE_PON)
    {
        return (0x2 << 28) + intf_id;
    }
    return intf_id;
}

static void OltLosIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_COMPLETE)
    {
        switch (msg->obj_type)
        {
        case BCMOLT_OBJ_ID_PON_INTERFACE:
            switch (msg->subgroup)
            {
            case BCMOLT_PON_INTERFACE_AUTO_SUBGROUP_LOS:
            {
                bcmolt_pon_interface_los *bcm_los_ind = (bcmolt_pon_interface_los *)msg;
                int intf_id = interface_key_to_port_no(bcm_los_ind->key.pon_ni, BCMOLT_INTERFACE_TYPE_PON);
                printf("LOS indication : intf_id: %d port: %d \n", bcm_los_ind->key.pon_ni, intf_id);
                break;
            }
            default:
                printf("BCMOLT_OBJ_ID_PON_INTERFACE subgroup [%d]\r\n", msg->subgroup);
            }
        default:
            printf("BCMOLT_OBJ_ID_PON_INTERFACE obj type [%d]\r\n", msg->obj_type);
        }
    }

    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");

    return;
}

static void OltOnuActivationFailureIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_ONU:
        switch (msg->subgroup)
        {
        case BCMOLT_ONU_AUTO_SUBGROUP_ONU_DEACTIVATION_COMPLETED:
        {
            bcmolt_onu_key *key = &((bcmolt_onu_onu_activation_completed *)msg)->key;
            bcmolt_onu_onu_activation_completed_data *data = &((bcmolt_onu_onu_activation_completed *)msg)->data;
            printf("Got onu deactivation, intf_id %d, onu_id %d, fail_reason %d\n", key->pon_ni, key->onu_id, data->fail_reason);
        }
        default:
            break;
        }
    default:
        break;
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");

    return;
}

static void OltOnuAllIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_ONU:
        printf("BCMOLT_ONU_AUTO_SUBGROUP_ALL sub ID[%d]\r\n", msg->subgroup);
    default:
        break;
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");
    return;
}

const char *serial_number_to_str(bcmolt_serial_number *serial_number)
{
    static char buff[SERIAL_NUMBER_SIZE + 1];

    sprintf(buff, "%c%c%c%c%1X%1X%1X%1X%1X%1X%1X%1X",
            serial_number->vendor_id.arr[0],
            serial_number->vendor_id.arr[1],
            serial_number->vendor_id.arr[2],
            serial_number->vendor_id.arr[3],
            serial_number->vendor_specific.arr[0] >> 4 & 0x0f,
            serial_number->vendor_specific.arr[0] & 0x0f,
            serial_number->vendor_specific.arr[1] >> 4 & 0x0f,
            serial_number->vendor_specific.arr[1] & 0x0f,
            serial_number->vendor_specific.arr[2] >> 4 & 0x0f,
            serial_number->vendor_specific.arr[2] & 0x0f,
            serial_number->vendor_specific.arr[3] >> 4 & 0x0f,
            serial_number->vendor_specific.arr[3] & 0x0f);

    return buff;
}

std::string vendor_specific_to_str(char const *const vendor_specific)
{
    char buff[SERIAL_NUMBER_SIZE + 1];

    sprintf(buff, "%1X%1X%1X%1X%1X%1X%1X%1X",
            vendor_specific[0] >> 4 & 0x0f,
            vendor_specific[0] & 0x0f,
            vendor_specific[1] >> 4 & 0x0f,
            vendor_specific[1] & 0x0f,
            vendor_specific[2] >> 4 & 0x0f,
            vendor_specific[2] & 0x0f,
            vendor_specific[3] >> 4 & 0x0f,
            vendor_specific[3] & 0x0f);

    return buff;
}

static void OltOnuDiscoveryIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_PON_INTERFACE:
        switch (msg->subgroup)
        {
        case BCMOLT_PON_INTERFACE_AUTO_SUBGROUP_ONU_DISCOVERED:
        {
            bcmolt_pon_interface_key *key = &((bcmolt_pon_interface_onu_discovered *)msg)->key;
            bcmolt_pon_interface_onu_discovered_data *data = &((bcmolt_pon_interface_onu_discovered *)msg)->data;

            bcmolt_serial_number *in_serial_number = &(data->serial_number);
            printf("onu discover indication, pon_ni %d, serial_number %s\n", key->pon_ni, serial_number_to_str(in_serial_number));
            //printf("onu discover indication, pon_ni %d, vendor_specific %s\n", key->pon_ni, in_serial_number->vendor_specific.arr);
            break;
        }
        default:
            break;
        }
    default:
        break;
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");

    return;
}

static void OltOnuOperIndication(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_ONU:
        switch (msg->subgroup)
        {
        case BCMOLT_ONU_AUTO_SUBGROUP_STATE_CHANGE:
        {
            bcmolt_onu_key *key = &((bcmolt_onu_state_change *)msg)->key;
            bcmolt_onu_state_change_data *data = &((bcmolt_onu_state_change *)msg)->data;

            if (ONU_STATE_IF_UP(data->new_onu_state))
                printf("onu oper state indication, intf_id %d, onu_id %d UP!, \n", key->pon_ni, key->onu_id, data->new_onu_state);
            if (ONU_STATE_IF_DOWN(data->new_onu_state))
                printf("onu oper state indication, intf_id %d, onu_id %d DOWN!, \n", key->pon_ni, key->onu_id, data->new_onu_state);
        }
        default:
            break;
        }
    default:
        break;
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");

    return;
}

static void OltOnuO5(short unsigned int olt, bcmolt_msg *msg)
{
    UNUSED(olt);
    switch (msg->obj_type)
    {
    case BCMOLT_OBJ_ID_ONU:
        switch (msg->subgroup)
        {
        case BCMOLT_ONU_AUTO_SUBGROUP_RANGING_COMPLETED:
        {
            bcmolt_onu_key *key = &((bcmolt_onu_ranging_completed *)msg)->key;
            bcmolt_onu_ranging_completed_data *data = &((bcmolt_onu_ranging_completed *)msg)->data;

            if (data->status == BCMOLT_RESULT_SUCCESS)
                printf("onu indication, intf_id %d, onu_id %d UP!, \n", key->pon_ni, key->onu_id, data->status);
            else
                printf("onu indications O5 fail\r\n");
        }
        default:
            break;
        }
    default:
        break;
    }
    if (d_bcmbal_msg_free)
        d_bcmbal_msg_free(msg);
    else
        printf("d_bcmbal_msg_free error\r\n");

    return;
}

static bool OnuRssiMeasurement(int in_onu_id, int in_pon_id)
{
    bcmolt_onu_key onu_key = {.pon_ni = in_pon_id, .onu_id = in_onu_id};
    bcmolt_onu_rssi_measurement rssi_oper;

    BCMOLT_OPER_INIT(&rssi_oper, onu, rssi_measurement, onu_key);

    if (d_bcmbal_oper_submit)
    {
        if (d_bcmbal_oper_submit(dev_id, &rssi_oper.hdr) == BCM_ERR_OK)
            return true;
        else
            return false;
    }
    else
        return false;
}

namespace acc_bal_api_dist_helper
{
static Olt_Device *g_Olt_Device = NULL;

void Olt_Device::set_olt_status(bool status)
{
    m_olt_status = status;
}

void Olt_Device::set_bal_status(bool status)
{
    m_bal_status = status;
}

bool Olt_Device::get_olt_status()
{
    return m_olt_status;
}

bool Olt_Device::get_bal_status()
{
    return m_bal_status;
}

bool Olt_Device::get_connection_status()
{
    if (d_bcmbal_api_conn_mgr_is_connected)
        return d_bcmbal_api_conn_mgr_is_connected(dev_id);
    else
        return false;
}

void Olt_Device::set_pon_status(int port, int status)
{
    m_pon_port[port].set_status(status);
    return;
}

void Olt_Device::set_nni_status(int port, int status)
{
    m_nni_port[port].set_status(status);
    return;
}

void Olt_Device::set_intf_type(int port, int type)
{
}

bool Olt_Device::check_bal_ready()
{

    bcmos_errno err;
    int maxTrials = 150;
    bcmolt_olt_cfg olt_cfg = {};
    bcmolt_olt_key olt_key = {};

    BCMOLT_CFG_INIT(&olt_cfg, olt, olt_key);
    BCMOLT_MSG_FIELD_GET(&olt_cfg, bal_state);

    while (olt_cfg.data.bal_state != BCMOLT_BAL_STATE_BAL_AND_SWITCH_READY)
    {
        if (--maxTrials == 0)
            return false;
        if (d_bcmbal_cfg_get)
        {
            if (d_bcmbal_cfg_get(dev_id, &olt_cfg.hdr))
            {
                printf("////////////Wait connection\r\n");
                sleep(1);
                continue;
            }
            else
                printf("////////////Wait BAL Ready[%d] seconds\r\n", maxTrials);
        }
        else
            return false;
        sleep(1);
    }
    return true;
}

int Olt_Device::get_board_basic_info()
{
    printf("get borad basic info!\r\n");
    bcmos_errno err;
    bcmolt_device_cfg dev_cfg = {};
    bcmolt_device_key dev_key = {};
    bcmolt_olt_cfg olt_cfg = {};
    bcmolt_olt_key olt_key = {};
    bcmolt_topology_map topo_map[get_max_pon_num()] = {};
    bcmolt_topology topo = {};

    topo.topology_maps.len = get_max_pon_num();
    topo.topology_maps.arr = &topo_map[0];
    BCMOLT_CFG_INIT(&olt_cfg, olt, olt_key);
    BCMOLT_MSG_FIELD_GET(&olt_cfg, bal_state);
    BCMOLT_FIELD_SET_PRESENT(&olt_cfg.data, olt_cfg_data, topology);
    BCMOLT_CFG_LIST_BUF_SET(&olt_cfg, olt, topo.topology_maps.arr, sizeof(bcmolt_topology_map) * topo.topology_maps.len);

    if (d_bcmbal_cfg_get)
    {
        d_bcmbal_cfg_get(dev_id, &olt_cfg.hdr);

        if (olt_cfg.data.bal_state == BCMOLT_BAL_STATE_BAL_AND_SWITCH_READY)
            m_oper_state = "up";
        printf("OLT op_state:[%s]\n", m_oper_state.c_str());

        m_nni_ports_num = olt_cfg.data.topology.num_switch_ports;
        m_pon_ports_num = olt_cfg.data.topology.topology_maps.len;

        dev_key.device_id = dev_id;
        BCMOLT_CFG_INIT(&dev_cfg, device, dev_key);
        BCMOLT_MSG_FIELD_GET(&dev_cfg, firmware_sw_version);
        BCMOLT_MSG_FIELD_GET(&dev_cfg, chip_family);
        BCMOLT_MSG_FIELD_GET(&dev_cfg, system_mode);

        err = d_bcmbal_cfg_get(dev_id, &dev_cfg.hdr);

        if (err)
        {
            m_bal_version = std::to_string(dev_cfg.data.firmware_sw_version.major) + "." + std::to_string(dev_cfg.data.firmware_sw_version.minor) + "." + std::to_string(dev_cfg.data.firmware_sw_version.revision);
            m_firmware_version = "BAL." + m_bal_version + "__" + m_firmware_version;

            switch (dev_cfg.data.system_mode)
            {
            case 9 ... 12:
                m_pon_type = "GPON";
                break;
            case 13 ... 16:
                m_pon_type = "XGPON";
                break;
            case 18 ... 20:
                m_pon_type = "XGS-PON";
                break;
            default:
                break;
            }

            switch (dev_cfg.data.chip_family)
            {
            case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6862_X:
                m_chip_family = "Maple";
                break;
            case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6865_X:
                m_chip_family = "Aspen";
                break;
            default:
                break;
            }

            printf("topology nni:%d pon:%d maple dev number :%d pon number per device :%d family: %s\n",
                   m_nni_ports_num,
                   m_pon_ports_num,
                   BCM_MAX_DEVS_PER_LINE_CARD,
                   BCM_MAX_PONS_PER_DEV,
                   m_chip_family.c_str());

            printf("OLT info., [nni ports: %d] [pon ports: %d] [pon type %s]\n", m_nni_ports_num, m_pon_ports_num, m_pon_type.c_str());
            printf("OLT info., [firware_ver:%s]\n", m_firmware_version.c_str());
        }
    }
    return 1;
}

#if defined BALCLI
bool Olt_Device::enable_cli()
{
    /* Create CLI session */
    bcmcli_session_parm mon_session_parm;
    memset(&mon_session_parm, 0, sizeof(mon_session_parm));
    mon_session_parm.get_prompt = openolt_cli_get_prompt_cb;
    mon_session_parm.access_right = BCMCLI_ACCESS_ADMIN;
    bcmos_errno rc = bcmcli_session_open(&mon_session_parm, &current_session);
    bcm_openolt_api_cli_init(NULL, current_session);

    BCMCLI_MAKE_CMD_NOPARM(NULL, "quit", "Quit", bcm_cli_quit);

    if (BCM_ERR_OK != bcmolt_apiend_cli_init())
    {
        printf("Failed to add apiend init\n");
        return false;
    }
    else
        return true;
}
#endif

bool Olt_Device::connect_host()
{
    bcmos_errno err;
    bcmolt_host_init_parms init_parms = {};
    init_parms.transport.type = BCM_HOST_API_CONN_LOCAL;

    if (d_bcmbal_host_init)
    {
        if (m_bcm_host_init)
            return true;

        if (BCM_ERR_OK != d_bcmbal_host_init(&init_parms))
        {
            printf("failed to init bcmolt_host\r\n");
            return false;
        }
        else
        {
            printf("bcmolt_host_init OK!!!\r\n");
            m_bcm_host_init = true;
        }
    }
    else
        return false;
}

bool Olt_Device::connect_bal(bool wait_bal_ready)
{
    bcmos_errno err;
    if (m_bcm_host_init)
    {
        if (d_bcmbal_api_conn_mgr_is_connected)
        {
            if (d_bcmbal_api_conn_mgr_is_connected(dev_id) == true)
            {
                if (d_bcmbal_ind_subscribe)
                {
                    if (!wait_bal_ready)
                    {
                        bcmolt_msg *msg = malloc(sizeof(bcmolt_msg));
                        if (msg != NULL)
                        {
                            msg->subgroup = BCMOLT_OLT_AUTO_SUBGROUP_BAL_READY;
                            OltBalReady(dev_id, msg);
                            return true;
                        }
                        else
                            return false;
                    }
                    else
                    {
                        //Need wait bal ready then register other callback indicator//
                        bcmolt_rx_cfg cb_cfg = {};
                        cb_cfg.obj_type = BCMOLT_OBJ_ID_OLT;
                        cb_cfg.rx_cb = OltBalReady;
                        cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;
                        cb_cfg.subgroup = bcmolt_olt_auto_subgroup_bal_ready;

                        if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
                        {
                            printf("Register_callback BCMOLT_OBJ_ID_OLT bcmolt_olt_auto_subgroup_bal_ready error!!!\r\n");
                            return false;
                        }
                        else
                            printf("Register_callback BCMOLT_OBJ_ID_OLT bcmolt_olt_auto_subgroup_bal_ready ok!!!\r\n");
                    }
                }
                else
                    return false;

                return true;
            }
            else
            {
                printf("bcmolt_api_conn_mgr_is_connected Fail!!\r\n");
                return false;
            }
        }
    }
    else
        return false;
}

Olt_Device &Olt_Device::get_instance()
{
    if (NULL == g_Olt_Device)
    {
        //Check if XGS PON
        ifstream ifs("/etc/onl/platform");
        std::string s;
        getline(ifs, s, (char)ifs.eof());
        printf("Creating Olt_Device on platform [%s] size[%d]\r\n", s.c_str(), s.size());

        if (s.compare(0, s.size() - 1, "x86-64-accton-asxvolt16-r0") == 0)
        {
            printf("x86-64-accton-asxvolt16-r0\r\n");
            g_Olt_Device = new XGS_PON_Olt_Device();
        }
        else if (s.compare(0, s.size() - 1, "x86-64-accton-asgvolt64-r0") == 0)
        {
            printf("x86-64-accton-asgvolt64-r0\r\n");
            g_Olt_Device = new G_PON_Olt_Device();
        }
        else
            g_Olt_Device = new XGS_PON_Olt_Device();
    }
    return *g_Olt_Device;
}

void Olt_Device::cleanup()
{
    if (g_Olt_Device != NULL)
    {
        delete g_Olt_Device;
        g_Olt_Device = NULL;
    }
    return;
}

Olt_Device::~Olt_Device()
{
    if (fHandle)
        dlclose(fHandle);
}

Olt_Device::Olt_Device()
{
    fHandle = dlopen("/usr/local/lib/libbal_host_api.so", RTLD_LAZY);

    if (fHandle)
    {
        printf("Using dynamic loading function\r\n");
        d_bcmbal_cfg_get = (bcmos_errno(*)(bcmolt_oltid olt, bcmolt_cfg * objinfo))dlsym(fHandle, "bcmolt_cfg_get");
        d_bcmbal_cfg_set = (bcmos_errno(*)(bcmolt_oltid olt, bcmolt_cfg * objinfo))dlsym(fHandle, "bcmolt_cfg_set");
        d_bcmbal_cfg_clear = (bcmos_errno(*)(bcmolt_oltid olt, bcmolt_cfg * objinfo))dlsym(fHandle, "bcmolt_cfg_clear");
        d_bcmbal_stat_get = (bcmos_errno(*)(bcmolt_oltid olt, bcmolt_stat * stat, bcmolt_stat_flags flags))dlsym(fHandle, "bcmolt_stat_get");
        d_bcmbal_oper_submit = (bcmos_errno(*)(bcmolt_oltid olt, bcmolt_oper * oper))dlsym(fHandle, "bcmolt_oper_submit");
        d_bcmbal_msg_free = (void (*)(bcmolt_msg * msg))dlsym(fHandle, "bcmolt_msg_free");
        d_bcmbal_api_set_prop_present = (void (*)(bcmolt_msg * msg, const void *prop_ptr))dlsym(fHandle, "bcmolt_api_set_prop_present");
        d_bcmbal_host_init = (bcmos_errno(*)(const bcmolt_host_init_parms *init_parms))dlsym(fHandle, "bcmolt_host_init");
        d_bcmbal_usleep = (void (*)(uint32_t us))dlsym(fHandle, "bcmos_usleep");
        d_bcmbal_ind_subscribe = (bcmos_errno(*)(bcmolt_oltid olt, bcmolt_rx_cfg * rx_cfg))dlsym(fHandle, "bcmolt_ind_subscribe");
        d_bcmbal_api_conn_mgr_is_connected = (bcmos_bool(*)(bcmolt_goid olt))dlsym(fHandle, "bcmolt_api_conn_mgr_is_connected");
        d_bcmbal_strerror = (const char *(*)(bcmos_errno err))dlsym(fHandle, "bcmos_strerror");
        m_bal_lib_init = true;
        connect_host();
    }
    else
    {
        m_bal_lib_init = false;
        printf("Cannot find libbal_api_dist.so in /usr/local/lib/\r\n");
    }
}

void Olt_Device::register_callback()
{
    try
    {
        bcmolt_rx_cfg cb_cfg = {};
        uint16_t subgroup;

        if (m_subscribed)
        {
            return;
        }

        cb_cfg.obj_type = BCMOLT_OBJ_ID_DEVICE;
        cb_cfg.rx_cb = OltOperIndication;
        cb_cfg.subgroup = bcmolt_device_auto_subgroup_connection_complete;
        cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;

        if (d_bcmbal_ind_subscribe)
        {
            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_DEVICE complete error!!!\r\n");
                return;
            }
            else
            {
                printf("Register_callback bcmolt_device_auto_subgroup_connection_complete ok!!!\r\n");
            }

            cb_cfg.obj_type = BCMOLT_OBJ_ID_DEVICE;
            cb_cfg.subgroup = bcmolt_device_auto_subgroup_disconnection_complete;
            cb_cfg.rx_cb = OltOperIndication;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_DEVICE disconnection error!!!\r\n");
                return;
            }
            else
            {
                printf("Register_callback bcmolt_device_auto_subgroup_disconnection_complete ok!!!\r\n");
            }

            cb_cfg.obj_type = BCMOLT_OBJ_ID_DEVICE;
            cb_cfg.rx_cb = OltOperIndication;
            cb_cfg.subgroup = bcmolt_device_auto_subgroup_connection_failure;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_DEVICE connection failure error!!!\r\n");
                return;
            }
            else
            {
                printf("Register_callback bcmolt_device_auto_subgroup_disconnection_complete ok!!!\r\n");
            }

            cb_cfg.obj_type = BCMOLT_OBJ_ID_PON_INTERFACE;
            cb_cfg.subgroup = bcmolt_pon_interface_auto_subgroup_los;
            cb_cfg.rx_cb = OltLosIndication;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_PON_INTERFACE OltLosIndication error!!!\r\n");
                return;
            }
            else
            {
                printf("Register_callback bcmolt_pon_interface_auto_subgroup_los ok!!!\r\n");
            }

            cb_cfg.obj_type = BCMOLT_OBJ_ID_PON_INTERFACE;
            cb_cfg.subgroup = bcmolt_pon_interface_auto_subgroup_state_change_completed;
            cb_cfg.rx_cb = Olt_itf_change;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_PON_INTERFACE Olt_itf_change error!!!\r\n");
                return;
            }
            else
                printf("Register_callback bcmolt_pon_interface_auto_subgroup_state_change_completed ok!!!\r\n");

            cb_cfg.obj_type = BCMOLT_OBJ_ID_PON_INTERFACE;
            cb_cfg.rx_cb = OltOnuDiscoveryIndication;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;
            cb_cfg.subgroup = bcmolt_pon_interface_auto_subgroup_onu_discovered;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_PON_INTERFACE OltOnuDiscoveryIndication error!!!\r\n");
                return;
            }
            else
                printf("Register_callback bcmolt_pon_interface_auto_subgroup_onu_discovered ok!!!\r\n");

            cb_cfg.obj_type = BCMOLT_OBJ_ID_NNI_INTERFACE;
            cb_cfg.subgroup = bcmolt_nni_interface_auto_subgroup_state_change;
            cb_cfg.rx_cb = Olt_itf_change;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_NNI_INTERFACE error!!!\r\n");
                return;
            }
            else
                printf("Register_callback bcmolt_nni_interface_auto_subgroup_state_change ok!!!\r\n");

            cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
            cb_cfg.rx_cb = OltOnuOperIndication;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;
            cb_cfg.subgroup = bcmolt_onu_auto_subgroup_state_change;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL error!!!\r\n");
                return;
            }
            else
                printf("Register_callback bcmolt_onu_auto_subgroup_state_change ok!!!\r\n");

            cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
            cb_cfg.rx_cb = OltOnuActivationFailureIndication;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;
            cb_cfg.subgroup = bcmolt_onu_auto_subgroup_onu_deactivation_completed;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_ONU OltOnuActivationFailureIndication error!!!\r\n");
                return;
            }
            else
                printf("Register_callback bcmolt_onu_auto_subgroup_onu_deactivation_completed ok!!!\r\n");

            cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
            cb_cfg.rx_cb = OltOnuAllIndication;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;
            cb_cfg.subgroup = BCMOLT_ONU_AUTO_SUBGROUP_ALL;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_ONU bcmolt_onu_auto_subgroup_all error!!!\r\n");
                return;
            }
            else
                printf("Register_callback BCMOLT_OBJ_ID_ONU bcmolt_onu_auto_subgroup_all ok!!!\r\n");

            cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
            cb_cfg.rx_cb = OltOnuO5;
            cb_cfg.flags = BCMOLT_AUTO_FLAGS_NONE;
            cb_cfg.subgroup = bcmolt_onu_auto_subgroup_ranging_completed;

            if (d_bcmbal_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
            {
                printf("Register_callback BCMOLT_OBJ_ID_ONU error!!!\r\n");
                return;
            }
            else
                printf("Register_callback bcmolt_onu_auto_subgroup_ranging_completed ok!!!\r\n");

            m_subscribed = true;
        }
        else
            return;
    }
    catch (const exception &e)
    {
        printf("register_callback error\r\n");
        return;
    }
}

bool Olt_Device::rssi_measurement(int in_onu_id, int in_pon_id)
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    return OnuRssiMeasurement(in_onu_id, in_pon_id);
}

json::Value Olt_Device::get_port_statistic(int port)
{
    json::Value status(json::Value::Type::OBJECT);
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        if (((port - 1) >= 0) && ((port - 1) < get_total_port_num()))
        {
            if ((port - 1) < get_max_pon_num())
                return get_pon_statistic(port - 1);
            else
                return get_nni_statistic(port - get_max_pon_num() - 1);
        }
        return status;
    }
    catch (const exception &e)
    {
        printf("get_port_statistic error\r\n");
        return status;
    }
}

json::Value Olt_Device::get_pon_statistic(int port)
{
    json::Value status(json::Value::Type::OBJECT);
    try
    {
        bcmolt_stat_flags clear_on_read = BCMOLT_STAT_FLAGS_NONE;
        bcmolt_pon_interface_itu_pon_stats itu_pon_stats;
        bcmos_errno err = BCM_ERR_INTERNAL;

        bcmolt_pon_interface_key key;
        key.pon_ni = (bcmolt_interface)port;

        BCMOLT_STAT_INIT(&itu_pon_stats, pon_interface, itu_pon_stats, key);
        BCMOLT_MSG_FIELD_GET(&itu_pon_stats, tx_packets);
        BCMOLT_MSG_FIELD_GET(&itu_pon_stats, bip_errors);
        BCMOLT_MSG_FIELD_GET(&itu_pon_stats, rx_crc_error);

        if (d_bcmbal_stat_get)
        {
            err = d_bcmbal_stat_get((bcmolt_oltid)dev_id, &itu_pon_stats.hdr, clear_on_read);

            if (err == BCM_ERR_OK)
            {
                status["tx_packets"] = m_pon_port[port].m_port_statistic.tx_packets = itu_pon_stats.data.tx_packets;
                status["bip_errors"] = m_pon_port[port].m_port_statistic.bip_errors = itu_pon_stats.data.bip_errors;
                status["rx_crc_errors"] = m_pon_port[port].m_port_statistic.rx_crc_errors = itu_pon_stats.data.rx_crc_error;
                printf("pon port_id[%d] \
                    tx_packets[%llu] \
                    bip_errors[%llu] \
                    rx_crc_errors[%llu]\r\n",
                       m_pon_port[port].m_port_id,
                       (unsigned long long)m_pon_port[port].m_port_statistic.tx_packets,
                       (unsigned long long)m_pon_port[port].m_port_statistic.bip_errors,
                       (unsigned long long)m_pon_port[port].m_port_statistic.rx_crc_errors);
            }
            else
            {
                printf("Failed to get pon port statistics, port_id %d\n", (int)port);
            }

            bcmolt_onu_key onu_key;
            onu_key.pon_ni = (bcmolt_interface)port;
            bcmolt_onu_itu_pon_stats pon_stats;
            BCMOLT_STAT_INIT(&pon_stats, onu, itu_pon_stats, onu_key);
            BCMOLT_MSG_FIELD_GET(&pon_stats, rx_bytes);
            BCMOLT_MSG_FIELD_GET(&pon_stats, rx_packets);
            BCMOLT_MSG_FIELD_GET(&pon_stats, tx_bytes);

            err = d_bcmbal_stat_get((bcmolt_oltid)dev_id, &pon_stats.hdr, clear_on_read);

            if (err == BCM_ERR_OK)
            {
                status["rx_bytes"] = m_pon_port[port].m_port_statistic.rx_bytes = pon_stats.data.rx_bytes;
                status["rx_packets"] = m_pon_port[port].m_port_statistic.rx_packets = pon_stats.data.rx_packets;
                status["tx_bytes"] = m_pon_port[port].m_port_statistic.tx_bytes = pon_stats.data.tx_bytes;
                printf("pon port_id[%d] \
                    rx_bytes[%llu] \
                    rx_packets[%llu] \
                    tx_bytes[%llu]\r\n",
                       port,
                       (unsigned long long)m_pon_port[port].m_port_statistic.rx_bytes,
                       (unsigned long long)m_pon_port[port].m_port_statistic.rx_packets,
                       (unsigned long long)m_pon_port[port].m_port_statistic.tx_bytes);
            }
            else
            {
                printf("Failed to get pon port statistics, port_id %d\n", (int)port);
            }
            return status;
        }
        else
            return status;
    }
    catch (const exception &e)
    {
        printf("get_pon_statistic error\r\n");
        return status;
    }
}

json::Value Olt_Device::get_nni_statistic(int port)
{
    json::Value status(json::Value::Type::OBJECT);
    try
    {
        bcmos_errno err;
        bcmolt_stat_flags clear_on_read = BCMOLT_STAT_FLAGS_NONE;
        bcmolt_nni_interface_stats nni_stats;
        bcmolt_nni_interface_key nni_intf_key;
        nni_intf_key.id = port;
        BCMOLT_STAT_INIT(&nni_stats, nni_interface, stats, nni_intf_key);
        BCMOLT_MSG_FIELD_GET(&nni_stats, rx_bytes);
        BCMOLT_MSG_FIELD_GET(&nni_stats, rx_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, rx_ucast_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, rx_mcast_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, rx_bcast_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, rx_error_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, tx_bytes);
        BCMOLT_MSG_FIELD_GET(&nni_stats, tx_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, tx_ucast_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, tx_mcast_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, tx_bcast_packets);
        BCMOLT_MSG_FIELD_GET(&nni_stats, tx_error_packets);

        if (d_bcmbal_stat_get)
        {
            err = d_bcmbal_stat_get((bcmolt_oltid)dev_id, &nni_stats.hdr, clear_on_read);

            if (err == BCM_ERR_OK)
            {
                status["rx_bytes"] = m_nni_port[port].m_port_statistic.rx_bytes = nni_stats.data.rx_bytes;
                status["rx_packets"] = m_nni_port[port].m_port_statistic.rx_packets = nni_stats.data.rx_packets;
                status["rx_ucast_packets"] = m_nni_port[port].m_port_statistic.rx_ucast_packets = nni_stats.data.rx_ucast_packets;
                status["rx_mcast_packets"] = m_nni_port[port].m_port_statistic.rx_mcast_packets = nni_stats.data.rx_mcast_packets;
                status["rx_bcast_packets"] = m_nni_port[port].m_port_statistic.rx_bcast_packets = nni_stats.data.rx_bcast_packets;
                status["rx_error_packets"] = m_nni_port[port].m_port_statistic.rx_error_packets = nni_stats.data.rx_error_packets;

                status["tx_bytes"] = m_nni_port[port].m_port_statistic.tx_bytes = nni_stats.data.tx_bytes;
                status["tx_packets"] = m_nni_port[port].m_port_statistic.tx_packets = nni_stats.data.tx_packets;
                status["tx_ucast_packets"] = m_nni_port[port].m_port_statistic.tx_ucast_packets = nni_stats.data.tx_ucast_packets;
                status["tx_mcast_packets"] = m_nni_port[port].m_port_statistic.tx_mcast_packets = nni_stats.data.tx_mcast_packets;
                status["tx_bcast_packets"] = m_nni_port[port].m_port_statistic.tx_bcast_packets = nni_stats.data.tx_bcast_packets;
                status["tx_error_packets"] = m_nni_port[port].m_port_statistic.tx_error_packets = nni_stats.data.tx_error_packets;

                printf(
                    "NNI port_id[%d]\r\n\
                rx_bytes[%llu]\r\n\
                rx_packets[%llu]\r\n\
                rx_ucast_packets[%llu]\r\n\
                rx_mcast_packets[%llu]\r\n\
                rx_bcast_packets[%llu]\r\n\
                rx_error_packets[%llu]\r\n\
                tx_bytes[%llu]\r\n\
                tx_packets[%llu]\r\n\
                tx_ucast_packets[%llu]\r\n\
                tx_mcast_packets[%llu]\r\n\
                tx_bcast_packets[%llu]\r\n\
                    tx_error_packets[%llu]\r\n",
                    port,
                    (unsigned long long)m_nni_port[port].m_port_statistic.rx_bytes,
                    (unsigned long long)m_nni_port[port].m_port_statistic.rx_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.rx_ucast_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.rx_mcast_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.rx_bcast_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.rx_error_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.tx_bytes,
                    (unsigned long long)m_nni_port[port].m_port_statistic.tx_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.tx_ucast_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.tx_mcast_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.tx_bcast_packets,
                    (unsigned long long)m_nni_port[port].m_port_statistic.tx_error_packets);
            }
            else
            {
                status["rx_bytes"] = 0;
                status["rx_packets"] = 0;
                status["rx_ucast_packets"] = 0;
                status["rx_mcast_packets"] = 0;
                status["rx_bcast_packets"] = 0;
                status["rx_error_packets"] = 0;

                status["tx_bytes"] = 0;
                status["tx_packets"] = 0;
                status["tx_ucast_packets"] = 0;
                status["tx_mcast_packets"] = 0;
                status["tx_bcast_packets"] = 0;
                status["tx_error_packets"] = 0;

                printf("Failed to get nni port statistics, port_id %d, intf_type %d\n", (int)port);
            }
            return status;
        }
        else
            return status;
    }
    catch (const exception &e)
    {
        printf("get_nni_statistic error\r\n");
        return status;
    }
}

json::Value Olt_Device::get_flow_info(int flow_id, std::string flowtype)
{
    json::Value status(json::Value::Type::OBJECT);
    try
    {
        int iflowtype=0;
        if (flowtype == "upstream")
            iflowtype = 0;
        else
            iflowtype = 1;

        status["FlowId"] = flow_id;
        status["FlowType"] = flowtype;
        status["GemPortId"] = get_flow_status(flow_id, iflowtype, SVC_PORT_ID);
        status["Priority"] = get_flow_status(flow_id, iflowtype, PRIORITY);
        status["Cookie"] = get_flow_status(flow_id, iflowtype, COOKIE);

        if (get_flow_status(flow_id, iflowtype, INGRESS_INTF_TYPE) == BCMOLT_FLOW_INTERFACE_TYPE_PON)
            status["IngressIntfType"] = "Pon";
        else
            status["IngressIntfType"] = "Nni";

        if (get_flow_status(flow_id, iflowtype, EGRESS_INTF_TYPE) == BCMOLT_FLOW_INTERFACE_TYPE_PON)
            status["EgressIntfType"] = "Pon";
        else
            status["EgressIntfType"] = "Nni";

        status["IngressIntfId"] = get_flow_status(flow_id, iflowtype, INGRESS_INTF_ID);
        status["EgressIntfid"] = get_flow_status(flow_id, iflowtype, EGRESS_INTF_ID);
        status["ClassifierOVid"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_O_VID);
        status["ClassifierOPbits"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_O_PBITS);
        status["ClassifierIVid"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_I_VID);
        status["ClassifierIPBits"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_I_PBITS);
        status["ClassifierEtherType"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_ETHER_TYPE);
        status["ClassifierIpProto"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_IP_PROTO);
        status["ClassifierSrcPort"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_SRC_PORT);
        status["ClassifierDstPort"] = get_flow_status(flow_id, iflowtype, CLASSIFIER_DST_PORT);

        if (get_flow_status(flow_id, iflowtype, CLASSIFIER_PKT_TAG_TYPE) == BCMOLT_PKT_TAG_TYPE_UNTAGGED)
            status["ClassifierPktTagType"] = "Untagged";
        else if (get_flow_status(flow_id, iflowtype, CLASSIFIER_PKT_TAG_TYPE) == BCMOLT_PKT_TAG_TYPE_SINGLE_TAG)
            status["ClassifierPktTagType"] = "Singletagged";
        else if (get_flow_status(flow_id, iflowtype, CLASSIFIER_PKT_TAG_TYPE) == BCMOLT_PKT_TAG_TYPE_DOUBLE_TAG)
            status["ClassifierPktTagType"] = "Doubletagged";
        else
            status["ClassifierPktTagType"] = "na";

        status["EgressQosType"] = get_flow_status(flow_id, iflowtype, EGRESS_QOS_TYPE);
        status["EgressQosQueueId"] = get_flow_status(flow_id, iflowtype, EGRESS_QOS_QUEUE_ID);
        status["EgressQosTmSchedId"] = get_flow_status(flow_id, iflowtype, EGRESS_QOS_TM_SCHED_ID);

        if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) == BCMOLT_ACTION_CMD_ID_ADD_OUTER_TAG)
            status["ActionCmdsBitMask"] = "AddOuterTag";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) == BCMOLT_ACTION_CMD_ID_REMOVE_OUTER_TAG)
            status["ActionCmdsBitMask"] = "RemoveOuterTag";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) == BCMOLT_ACTION_CMD_ID_XLATE_OUTER_TAG)
            status["ActionCmdsBitMask"] = "XlateOuterTag";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) == BCMOLT_ACTION_CMD_ID_ADD_INNER_TAG)
            status["ActionCmdsBitMask"] = "AddInnerTag";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) ==BCMOLT_ACTION_CMD_ID_REMOVE_INNER_TAG)
            status["ActionCmdsBitMask"] = "RemoveInnerTag";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) ==BCMOLT_ACTION_CMD_ID_XLATE_INNER_TAG)
            status["ActionCmdsBitMask"] = "XlateInnerTag";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) ==BCMOLT_ACTION_CMD_ID_REMARK_OUTER_PBITS)
            status["ActionCmdsBitMask"] = "RemarkOuterPbits";
        else if (get_flow_status(flow_id, iflowtype, ACTION_CMDS_BITMASK) ==BCMOLT_ACTION_CMD_ID_REMARK_INNER_PBITS)
            status["ActionCmdsBitMask"] = "RemarkInnerPbits";
        else
            status["ActionCmdsBitMask"] = "na";
        status["ActionOVid"] = get_flow_status(flow_id, iflowtype, ACTION_O_VID);
        status["ActionIVid"] = get_flow_status(flow_id, iflowtype, ACTION_I_VID);
        status["ActionOPbits"] = get_flow_status(flow_id, iflowtype, ACTION_O_PBITS);
        status["ActionIPbits"] = get_flow_status(flow_id, iflowtype, ACTION_I_PBITS);
        status["State"] = get_flow_status(flow_id, iflowtype, STATE);
        status["GroupId"] = get_flow_status(flow_id, iflowtype, GROUP_ID);
        return status;
    }
    catch (const exception &e)
    {
        printf("get_flow_info error\r\n");
        return status;
    }
}

bool CreateDefaultSchedQueue(uint32_t intf_id, const std::string direction)
{
    try
    {
        bcmos_errno err;
        bcmolt_tm_sched_cfg tm_sched_cfg;
        bcmolt_tm_sched_key tm_sched_key = {.id = 1};
        tm_sched_key.id = get_default_tm_sched_id(intf_id, direction);

        printf("CreateDefaultSchedQueue for %s tm_sched id %d intf_id %d ", direction.c_str(), tm_sched_key.id, intf_id);
        d_bcmbal_usleep(200000);

        BCMOLT_CFG_INIT(&tm_sched_cfg, tm_sched, tm_sched_key);
        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, attachment_point.type, BCMOLT_TM_SCHED_OUTPUT_TYPE_INTERFACE);

        if (direction.compare(upstream) == 0)
        {
            BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, attachment_point.u.interface.interface_ref.intf_type, BCMOLT_INTERFACE_TYPE_NNI);
        }
        else if (direction.compare(downstream) == 0)
        {
            BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, attachment_point.u.interface.interface_ref.intf_type, BCMOLT_INTERFACE_TYPE_PON);
        }

        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, attachment_point.u.interface.interface_ref.intf_id, intf_id);
        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, sched_type, BCMOLT_TM_SCHED_TYPE_SP);
        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, num_priorities, 8);

        uint32_t cir = 1000000;
        uint32_t pir = 1000000;
        uint32_t burst = 65536;

        printf("applying traffic shaping in %s pir=%u, burst=%u\n", direction.c_str(), pir, burst);

        BCMOLT_FIELD_SET_PRESENT(&tm_sched_cfg.data.rate, tm_shaping, pir);
        BCMOLT_FIELD_SET_PRESENT(&tm_sched_cfg.data.rate, tm_shaping, burst);
        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, rate.pir, pir);
        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, rate.burst, burst);

        if (d_bcmbal_cfg_set)
        {
            err = d_bcmbal_cfg_set(dev_id, &tm_sched_cfg.hdr);
            if (err)
            {
                printf("ERROR!!\r\n");
                return false;
            }
            else
                printf("OK!!\r\n");

            for (int queue_id = 1; queue_id < 2; queue_id++)
            {
                bcmolt_tm_queue_cfg tm_queue_cfg;
                bcmolt_tm_queue_key tm_queue_key = {};
                tm_queue_key.id = queue_id;
                tm_queue_key.sched_id = get_default_tm_sched_id(intf_id, direction);
                tm_queue_key.tm_q_set_id = TM_Q_SET_ID;
                BCMOLT_CFG_INIT(&tm_queue_cfg, tm_queue, tm_queue_key);
                BCMOLT_MSG_FIELD_SET(&tm_queue_cfg, tm_sched_param.type, BCMOLT_TM_SCHED_PARAM_TYPE_PRIORITY);
                BCMOLT_MSG_FIELD_SET(&tm_queue_cfg, tm_sched_param.u.priority.priority, queue_id);

                if (d_bcmbal_cfg_set)
                {
                    err = d_bcmbal_cfg_set(dev_id, &tm_queue_cfg.hdr);
                    printf("CreateDefaultSchedQueue for %s tm_queue id %d, sched_id %d tm_queue_key.id \n", direction.c_str(), tm_queue_key.id, tm_queue_key.sched_id, tm_queue_key.id);
                    if (err)
                    {
                        printf(" ERROR!!\n");
                        return false;
                    }
                    else
                        printf(" OK!!\n");
                }
                else
                    return false;
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("CreateDefaultSchedQueue error\r\n");
        return false;
    }
}

bcmos_errno get_nni_interface_status(bcmolt_interface id, bcmolt_interface_state *state)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_nni_interface_key nni_key;
        bcmolt_nni_interface_cfg nni_cfg;
        nni_key.id = id;

        BCMOLT_CFG_INIT(&nni_cfg, nni_interface, nni_key);
        BCMOLT_FIELD_SET_PRESENT(&nni_cfg.data, nni_interface_cfg_data, state);

        if (d_bcmbal_cfg_get)
        {
            err = d_bcmbal_cfg_get(dev_id, &nni_cfg.hdr);
            *state = nni_cfg.data.state;
            return err;
        }
        else
            return err;
    }
    catch (const exception &e)
    {
        printf("get_nni_interface_status error\r\n");
        return BCM_ERR_INTERNAL;
    }
}

bool Olt_Device::get_nni_status(int port)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_nni_interface_key intf_key = {.id = (bcmolt_interface)port};
        bcmolt_nni_interface_set_nni_state nni_interface_set_state;
        bcmolt_interface_state state;

        err = get_nni_interface_status((bcmolt_interface)port, &state);
        if (err == BCM_ERR_OK)
        {
            if (state == BCMOLT_INTERFACE_STATE_ACTIVE_WORKING)
                return true;
            else
                return false;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        return false;
    }
}

bool Olt_Device::is_onu_active(int intf_id, int onu_id)
{
    bcmos_errno err = BCM_ERR_INTERNAL;
    bcmolt_onu_key onu_key;
    bcmolt_bin_str_36 registration_id;
    bcmolt_onu_cfg onu_cfg;
    onu_key.onu_id = onu_id;
    onu_key.pon_ni = intf_id;
    BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
    BCMOLT_FIELD_SET_PRESENT(&onu_cfg.data, onu_cfg_data, onu_state);

    if (d_bcmbal_cfg_get)
        err = d_bcmbal_cfg_get(dev_id, &onu_cfg.hdr);

    if (err == BCM_ERR_OK)
    {
        if ((onu_cfg.data.onu_state == BCMOLT_ONU_STATE_PROCESSING ||
             onu_cfg.data.onu_state == BCMOLT_ONU_STATE_ACTIVE) ||
            (onu_cfg.data.onu_state == BCMOLT_ONU_STATE_INACTIVE &&
             onu_cfg.data.onu_old_state == BCMOLT_ONU_STATE_NOT_CONFIGURED))
            return true;
    }
    else
    {
        printf("is_onu_active::bcmbal_cfg_get error\r\n");
        return false;
    }
}

bool Olt_Device::enable_nni_if(int intf_id)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_nni_interface_key intf_key = {.id = (bcmolt_interface)intf_id};
        bcmolt_nni_interface_set_nni_state nni_interface_set_state;
        bcmolt_interface_state state;

        err = get_nni_interface_status((bcmolt_interface)intf_id, &state);
        if (err == BCM_ERR_OK)
        {
            if (state == BCMOLT_INTERFACE_STATE_ACTIVE_WORKING)
            {
                printf("NNI interface: %d already enabled\n", intf_id);
                CreateDefaultSchedQueue(intf_id, upstream);
                return true;
            }
        }

        BCMOLT_OPER_INIT(&nni_interface_set_state, nni_interface, set_nni_state, intf_key);
        BCMOLT_FIELD_SET(&nni_interface_set_state.data, nni_interface_set_nni_state_data, nni_state, BCMOLT_INTERFACE_OPERATION_ACTIVE_WORKING);

        if (d_bcmbal_oper_submit)
        {
            err = d_bcmbal_oper_submit(dev_id, &nni_interface_set_state.hdr);
            if (err != BCM_ERR_OK)
            {
                printf("Failed to enable NNI interface: %d, err %d\n", intf_id, err);
                return false;
            }
            else
            {
                printf("Successfully enabled NNI interface: %d\n", intf_id);
                CreateDefaultSchedQueue(intf_id, upstream);
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("enable_nni_if error\r\n");
        return false;
    }
}

bcmos_errno get_pon_interface_status(bcmolt_interface pon_ni, bcmolt_interface_state *state)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_pon_interface_key pon_key;
        bcmolt_pon_interface_cfg pon_cfg;
        pon_key.pon_ni = pon_ni;

        BCMOLT_CFG_INIT(&pon_cfg, pon_interface, pon_key);
        BCMOLT_FIELD_SET_PRESENT(&pon_cfg.data, pon_interface_cfg_data, state);
        BCMOLT_FIELD_SET_PRESENT(&pon_cfg.data, pon_interface_cfg_data, itu);

        if (d_bcmbal_cfg_get)
        {
            err = d_bcmbal_cfg_get(dev_id, &pon_cfg.hdr);
            *state = pon_cfg.data.state;
            return err;
        }
        else
            return err;
    }
    catch (const exception &e)
    {
        printf("get_pon_interface_status error\r\n");
        return BCM_ERR_INTERNAL;
    }
}

bool Olt_Device::get_pon_status(int port)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_pon_interface_key intf_key = {.pon_ni = (bcmolt_interface)port};
        bcmolt_pon_interface_set_pon_interface_state pon_interface_set_state;
        bcmolt_interface_state state;

        err = get_pon_interface_status((bcmolt_interface)port, &state);

        if (err == BCM_ERR_OK)
        {
            if (state == BCMOLT_INTERFACE_STATE_ACTIVE_WORKING)
                return true;
            else
                return false;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        return false;
    }
}

bool Olt_Device::enable_pon_if(int intf_id)
{
    try
    {
        bcmos_errno err = BCM_ERR_OK;
        bcmolt_pon_interface_cfg interface_obj;
        bcmolt_pon_interface_key intf_key = {.pon_ni = (bcmolt_interface)intf_id};
        bcmolt_pon_interface_set_pon_interface_state pon_interface_set_state;
        bcmolt_interface_state state;

        err = get_pon_interface_status((bcmolt_interface)intf_id, &state);

        if (err == BCM_ERR_OK)
        {
            if (state == BCMOLT_INTERFACE_STATE_ACTIVE_WORKING)
            {
                printf("PON interface: %d already enabled\n", intf_id);
                return true;
            }
        }

        BCMOLT_CFG_INIT(&interface_obj, pon_interface, intf_key);
        BCMOLT_OPER_INIT(&pon_interface_set_state, pon_interface, set_pon_interface_state, intf_key);
        BCMOLT_MSG_FIELD_SET(&interface_obj, discovery.control, BCMOLT_CONTROL_STATE_ENABLE);
        BCMOLT_MSG_FIELD_SET(&interface_obj, discovery.interval, 5000);
        BCMOLT_MSG_FIELD_SET(&interface_obj, discovery.onu_post_discovery_mode, BCMOLT_ONU_POST_DISCOVERY_MODE_ACTIVATE);

        BCMOLT_MSG_FIELD_SET(&interface_obj, itu.automatic_onu_deactivation.los, true);
        BCMOLT_MSG_FIELD_SET(&interface_obj, itu.automatic_onu_deactivation.onu_alarms, true);
        BCMOLT_MSG_FIELD_SET(&interface_obj, itu.automatic_onu_deactivation.tiwi, true);
        BCMOLT_MSG_FIELD_SET(&interface_obj, itu.automatic_onu_deactivation.ack_timeout, true);
        BCMOLT_MSG_FIELD_SET(&interface_obj, itu.automatic_onu_deactivation.sfi, true);
        BCMOLT_MSG_FIELD_SET(&interface_obj, itu.automatic_onu_deactivation.loki, true);

        BCMOLT_FIELD_SET(&pon_interface_set_state.data, pon_interface_set_pon_interface_state_data, operation, BCMOLT_INTERFACE_OPERATION_ACTIVE_WORKING);

        if (d_bcmbal_cfg_set)
        {
            err = d_bcmbal_cfg_set(dev_id, &interface_obj.hdr);
            if (err != BCM_ERR_OK)
            {
                printf("Failed to enable discovery onu: %d\n", intf_id);
                return false;
            }
        }
        else
            return false;

        if (d_bcmbal_oper_submit)
        {
            err = d_bcmbal_oper_submit(dev_id, &pon_interface_set_state.hdr);
            if (err != BCM_ERR_OK)
            {
                printf("Failed to enable PON interface: %d error[%d]\n", intf_id, err);
                return false;
            }
            else
            {
                printf("Successfully enabled PON interface: %d\n", intf_id);
                CreateDefaultSchedQueue(intf_id, downstream);
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("enable_pon_if error\r\n");
        return false;
    }
}

bool Olt_Device::disable_pon_if(int intf_id)
{
    try
    {
        bcmos_errno err = BCM_ERR_OK;
        bcmolt_pon_interface_cfg interface_obj;
        bcmolt_pon_interface_key intf_key = {.pon_ni = (bcmolt_interface)intf_id};
        bcmolt_pon_interface_set_pon_interface_state pon_interface_set_state;

        BCMOLT_CFG_INIT(&interface_obj, pon_interface, intf_key);
        BCMOLT_OPER_INIT(&pon_interface_set_state, pon_interface, set_pon_interface_state, intf_key);
        BCMOLT_MSG_FIELD_SET(&interface_obj, discovery.control, BCMOLT_CONTROL_STATE_DISABLE);
        BCMOLT_FIELD_SET(&pon_interface_set_state.data, pon_interface_set_pon_interface_state_data, operation, BCMOLT_INTERFACE_OPERATION_INACTIVE);

        if (d_bcmbal_cfg_set)
        {
            err = d_bcmbal_cfg_set(dev_id, &interface_obj.hdr);
            if (err != BCM_ERR_OK)
            {
                printf("Failed to disable pon intf: %d\n", intf_id);
                return false;
            }
            else
            {
                if (d_bcmbal_oper_submit)
                {
                    err = d_bcmbal_oper_submit(dev_id, &pon_interface_set_state.hdr);
                    if (err != BCM_ERR_OK)
                    {
                        printf("Failed to disable PON interface: %d error[%d]\n", intf_id, err);
                        return false;
                    }
                    else
                    {
                        printf("Successfully disable PON interface: %d\n", intf_id);
                    }
                    return true;
                }
                else
                    return false;
            }
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("disable_pon_if error\r\n");
        return false;
    }
}

bool Olt_Device::disable_nni_if(int intf_id)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_nni_interface_key intf_key = {.id = (bcmolt_interface)intf_id};
        bcmolt_nni_interface_set_nni_state nni_interface_set_state;
        bcmolt_interface_state state;

        BCMOLT_OPER_INIT(&nni_interface_set_state, nni_interface, set_nni_state, intf_key);
        BCMOLT_FIELD_SET(&nni_interface_set_state.data, nni_interface_set_nni_state_data, nni_state, BCMOLT_INTERFACE_OPERATION_INACTIVE);

        if (d_bcmbal_oper_submit)
        {
            err = d_bcmbal_oper_submit(dev_id, &nni_interface_set_state.hdr);
            if (err != BCM_ERR_OK)
            {
                printf("Failed to disable NNI interface: %d, err %d\n", intf_id, err);
                return false;
            }
            else
            {
                printf("Successfully disable NNI interface: %d\n", intf_id);
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("disable_nni_if error\r\n");
        return false;
    }
}

long Olt_Device::get_flow_status(uint16_t flow_id, uint16_t flow_type, uint16_t data_id)
{
    if (d_bcmbal_cfg_get == NULL || d_bcmbal_strerror == NULL)
        return -1;
    bcmos_errno err;
    bcmolt_flow_key flow_key;
    bcmolt_flow_cfg flow_cfg;

    flow_key.flow_id = flow_id;
    flow_key.flow_type = (bcmolt_flow_type)flow_type;

    BCMOLT_CFG_INIT(&flow_cfg, flow, flow_key);

    switch (data_id)
    {
    case ONU_ID: //onu_id
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, onu_id);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get onu_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.onu_id;
    case FLOW_TYPE:
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get flow_type, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.key.flow_type;
    case SVC_PORT_ID: //svc_port_id
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, svc_port_id);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get svc_port_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.svc_port_id;
    case PRIORITY:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, priority);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get priority, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.priority;
    case COOKIE: //cookie
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, cookie);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get cookie, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.cookie;
    case INGRESS_INTF_TYPE: //ingress intf_type
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, ingress_intf);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get ingress intf_type, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.ingress_intf.intf_type;
    case EGRESS_INTF_TYPE: //egress intf_type
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, egress_intf);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get egress intf_type, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.egress_intf.intf_type;
    case INGRESS_INTF_ID: //ingress intf_id
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, ingress_intf);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get ingress intf_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.ingress_intf.intf_id;
    case EGRESS_INTF_ID: //egress intf_id
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, egress_intf);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get egress intf_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.egress_intf.intf_id;
    case CLASSIFIER_O_VID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier o_vid, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.o_vid;
    case CLASSIFIER_O_PBITS:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier o_pbits, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.o_pbits;
    case CLASSIFIER_I_VID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier i_vid, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.i_vid;
    case CLASSIFIER_I_PBITS:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier i_pbits, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.i_pbits;
    case CLASSIFIER_ETHER_TYPE:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier ether_type, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.ether_type;
    case CLASSIFIER_IP_PROTO:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier ip_proto, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.ip_proto;
    case CLASSIFIER_SRC_PORT:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier src_port, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.src_port;
    case CLASSIFIER_DST_PORT:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier dst_port, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.dst_port;
    case CLASSIFIER_PKT_TAG_TYPE:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, classifier);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get classifier pkt_tag_type, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.classifier.pkt_tag_type;
    case EGRESS_QOS_TYPE:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, egress_qos);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get egress_qos type, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.egress_qos.type;
    case EGRESS_QOS_QUEUE_ID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, egress_qos);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get egress_qos queue_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        switch (flow_cfg.data.egress_qos.type)
        {
        case BCMOLT_EGRESS_QOS_TYPE_FIXED_QUEUE:
            return flow_cfg.data.egress_qos.u.fixed_queue.queue_id;
        case BCMOLT_EGRESS_QOS_TYPE_TC_TO_QUEUE:
            return flow_cfg.data.egress_qos.u.tc_to_queue.tc_to_queue_id;
        case BCMOLT_EGRESS_QOS_TYPE_PBIT_TO_TC:
            return flow_cfg.data.egress_qos.u.pbit_to_tc.tc_to_queue_id;
        case BCMOLT_EGRESS_QOS_TYPE_PRIORITY_TO_QUEUE:
            return flow_cfg.data.egress_qos.u.priority_to_queue.tm_q_set_id;
        case BCMOLT_EGRESS_QOS_TYPE_NONE:
        default:
            return -1;
        }
    case EGRESS_QOS_TM_SCHED_ID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, egress_qos);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get egress_qos tm_sched_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.egress_qos.tm_sched.id;
    case ACTION_CMDS_BITMASK:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, action);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get action cmds_bitmask, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.action.cmds_bitmask;
    case ACTION_O_VID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, action);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get action o_vid, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.action.o_vid;
    case ACTION_O_PBITS:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, action);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get action o_pbits, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.action.o_pbits;
    case ACTION_I_VID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, action);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get action i_vid, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.action.i_vid;
    case ACTION_I_PBITS:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, action);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get action i_pbits, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.action.i_pbits;
    case STATE:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, state);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get state, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.state;
    case GROUP_ID:
        BCMOLT_FIELD_SET_PRESENT(&flow_cfg.data, flow_cfg_data, group_id);
        err = d_bcmbal_cfg_get(dev_id, &flow_cfg.hdr);
        if (err)
        {
            printf("Failed to get group_id, err = %s\n", d_bcmbal_strerror(err));
            return err;
        }
        return flow_cfg.data.group_id;
    default:
        return BCM_ERR_INTERNAL;
    }
    return err;
}

bool Olt_Device::delete_onu(int intf_id, int onu_id)
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        if (deactivate_onu(intf_id, onu_id))
        {
            //Todo::Need a wait function to check if onu in inactive state//
            std::this_thread::sleep_for(std::chrono::milliseconds(500)	);
            if (d_bcmbal_cfg_clear)
            {
                bcmos_errno err;
                bcmolt_onu_cfg cfg_obj;
                bcmolt_onu_key key;
                key.onu_id = onu_id;
                key.pon_ni = intf_id;
                BCMOLT_CFG_INIT(&cfg_obj, onu, key);
                err = d_bcmbal_cfg_clear(dev_id, &cfg_obj.hdr);
                if (err != BCM_ERR_OK)
                {
                    printf("delete_onu error!!\r\n");
                    return false;
                }
                else
                    return true;
            }
            else
                return false;
        }
        else
        {
            printf("deactive_onu error!!\r\n");
            return false;
        }
    }
    catch (const exception &e)
    {
        printf("deactivate_onu error\r\n");
        return false;
    }
}

bool Olt_Device::deactivate_onu(int intf_id, int onu_id)
{
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_onu_set_onu_state onu_oper;
        bcmolt_onu_cfg onu_cfg;
        bcmolt_onu_key onu_key;
        bcmolt_onu_state onu_state;

        onu_key.onu_id = onu_id;
        onu_key.pon_ni = intf_id;
        BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
        BCMOLT_FIELD_SET_PRESENT(&onu_cfg.data, onu_cfg_data, onu_state);

        if (d_bcmbal_cfg_get)
        {
            err = d_bcmbal_cfg_get(dev_id, &onu_cfg.hdr);
            if (err == BCM_ERR_OK)
            {
                onu_state = onu_cfg.data.onu_state;
                switch (onu_state)
                {
                case BCMOLT_ONU_STATE_ACTIVE:
                    BCMOLT_OPER_INIT(&onu_oper, onu, set_onu_state, onu_key);
                    BCMOLT_FIELD_SET(&onu_oper.data, onu_set_onu_state_data, onu_state, BCMOLT_ONU_OPERATION_INACTIVE);

                    if (d_bcmbal_oper_submit)
                    {
                        err = d_bcmbal_oper_submit(dev_id, &onu_oper.hdr);

                        if (err != BCM_ERR_OK)
                        {
                            printf("Failed to deactivate ONU %d on PON %d, err %d\n", onu_id, intf_id, err);
                            return false;
                        }
                        else
                            printf("Inactive ok!!\r\n");
                    }
                    break;
                default:
                    printf("Not Active!!\r\n");
                    break;
                }
            }
            else
            {
                printf("deactivate_onu::failed to bcmolt_cfg_get\r\n");
                return false;
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("deactivate_onu error\r\n");
        return false;
    }
}

bool Olt_Device::omci_msg_out(int intf_id, int onu_id, const std::string pkt)
{
#define MAX_CHAR_LEN 20
#define MAX_OMCI_MSG_LEN 44
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        bcmolt_bin_str buf; /* A structure with a msg pointer and length value */
        bcmolt_onu_cpu_packets omci_cpu_packets;
        bcmolt_onu_key key;

        key.pon_ni = intf_id;
        key.onu_id = onu_id;

        BCMOLT_OPER_INIT(&omci_cpu_packets, onu, cpu_packets, key);
        BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, packet_type, BCMOLT_PACKET_TYPE_OMCI);
        BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, calc_crc, BCMOS_TRUE);

        if ((pkt.size() / 2) > MAX_OMCI_MSG_LEN)
        {
            buf.len = MAX_OMCI_MSG_LEN;
        }
        else
        {
            buf.len = pkt.size() / 2;
        }

        uint16_t idx1 = 0;
        uint16_t idx2 = 0;
        uint8_t arraySend[buf.len];
        char str1[MAX_CHAR_LEN];
        char str2[MAX_CHAR_LEN];
        memset(&arraySend, 0, buf.len);

        printf("Sending omci msg to ONU %d to PON %d of length is %d\r\n", onu_id, intf_id, buf.len);

        for (idx1 = 0, idx2 = 0; idx1 < ((buf.len) * 2); idx1++, idx2++)
        {
            sprintf(str1, "%c", pkt[idx1]);
            sprintf(str2, "%c", pkt[++idx1]);
            strcat(str1, str2);
            arraySend[idx2] = strtol(str1, NULL, 16);
        }

        buf.arr = (uint8_t *)malloc((buf.len) * sizeof(uint8_t));
        memcpy(buf.arr, (uint8_t *)arraySend, buf.len);

        BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, number_of_packets, 1);
        BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, packet_size, buf.len);
        BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, buffer, buf);

        if (d_bcmbal_oper_submit)
        {
            bcmos_errno err = d_bcmbal_oper_submit(dev_id, &omci_cpu_packets.hdr);
            if (err)
            {
                printf("Error sending OMCI message to ONU %d on PON %d Err:%d\n", onu_id, intf_id, err);
            }
            printf("OMCI raw data: ");
            int count = 0;
            int buf_len = buf.len;

            for (count = 1; count <= buf_len; count++)
            {
                printf("%02X", buf.arr[count - 1]);
            }
            printf("\r\n");
            free(buf.arr);
            return true;
        }
        else
        {
            printf("d_bcmbal_oper_submit error\r\n");
            return false;
        }
    }
    catch (const exception &e)
    {
        printf("omci_msg_out error\r\n");
        return false;
    }
}

std::map<flow_pair, int32_t> Olt_Device::get_flow_map()
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        printf("get_flow_map\r\n");
        return flow_map;
    }
    catch (const exception &e)
    {
        printf("get_flow_map error\r\n");
    }
}

int Olt_Device::get_flow_size()
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        printf("get_flow_size [%d]\r\n", flow_map.size());
        return flow_map.size();
    }
    catch (const exception &e)
    {
        printf("get_flow_size error\r\n");
        return -1;
    }
}

bool Olt_Device::flow_remove(uint32_t flow_id, const std::string flow_type)
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        bcmolt_flow_cfg cfg;
        bcmolt_flow_key key = {};

        printf("flow_remove flow_id[%d] flow_type[%s]", flow_id, flow_type.c_str());

        key.flow_id = (bcmolt_flow_id)flow_id;
        key.flow_id = flow_id;

        if (flow_type.compare(upstream) == 0)
        {
            key.flow_type = BCMOLT_FLOW_TYPE_UPSTREAM;
        }
        else if (flow_type.compare(downstream) == 0)
        {
            key.flow_type = BCMOLT_FLOW_TYPE_DOWNSTREAM;
        }
        else
        {
            printf("Invalid flow type %s\n");
            return false;
        }

        BCMOLT_CFG_INIT(&cfg, flow, key);

        if (d_bcmbal_cfg_clear)
        {
            bcmos_errno err = d_bcmbal_cfg_clear(dev_id, &cfg.hdr);

            if (err)
            {
                printf(" Error\n");
                return false;
            }
            else
            {
                if (flow_map.size() != 0)
                {
                    std::map<flow_pair, int>::iterator it;
                    for (it = flow_map.begin(); it != flow_map.end(); it++)
                    {
                        if (it->first.first == flow_id && it->first.second == key.flow_type)
                        {
                            flow_map.erase(it);
                        }
                    }
                }
                printf(" OK\n");
            }

            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("flow_remove error\r\n");
        return false;
    }
}

// access_intf_id  : PON ID
// network_intf_id : NNI ID
bool Olt_Device::flow_add(int onu_id, int flow_id, const std::string flow_type, const std::string pkt_tag_type, int access_intf_id,
                          int network_intf_id, int gemport_id, int classifier,
                          int action, int action_cmd, struct action_val a_val, struct class_val c_val)
{

    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        bool single_tag = false;
        bcmolt_flow_cfg cfg;
        bcmolt_flow_key key = {}; /**< Object key. */

        printf("flow_add PON[%d] onu_id[%d] flow_id[%d] flow_type[%s] pkt_tag_type[%s] gemport_id[%d] NNI[%d] classifier[0x%04X] action[0x%04X]\r\n", access_intf_id, onu_id, flow_id, flow_type.c_str(), pkt_tag_type.c_str(), gemport_id, network_intf_id, classifier, action);

        key.flow_id = flow_id;

        if (flow_type.compare("upstream") == 0)
        {
            key.flow_type = BCMOLT_FLOW_TYPE_UPSTREAM;
        }
        else if (flow_type.compare("downstream") == 0)
        {
            key.flow_type = BCMOLT_FLOW_TYPE_DOWNSTREAM;
        }
        else
        {
            printf("ERROR: Invalid flow type !!!\r\n");
            return false;
        }
        BCMOLT_CFG_INIT(&cfg, flow, key);

        if (access_intf_id >= 0 && network_intf_id >= 0)
        {
            if (key.flow_type == BCMOLT_FLOW_TYPE_UPSTREAM)
            {
                BCMOLT_MSG_FIELD_SET(&cfg, ingress_intf.intf_type, BCMOLT_FLOW_INTERFACE_TYPE_PON);
                BCMOLT_MSG_FIELD_SET(&cfg, ingress_intf.intf_id, access_intf_id);
                BCMOLT_MSG_FIELD_SET(&cfg, egress_intf.intf_type, BCMOLT_FLOW_INTERFACE_TYPE_NNI);
                BCMOLT_MSG_FIELD_SET(&cfg, egress_intf.intf_id, network_intf_id);
                printf(" flow_type BCMOLT_FLOW_TYPE_UPSTREAM\r\n");
            }
            else if (key.flow_type == BCMOLT_FLOW_TYPE_DOWNSTREAM)
            {
                BCMOLT_MSG_FIELD_SET(&cfg, ingress_intf.intf_type, BCMOLT_FLOW_INTERFACE_TYPE_NNI);
                BCMOLT_MSG_FIELD_SET(&cfg, ingress_intf.intf_id, network_intf_id);
                BCMOLT_MSG_FIELD_SET(&cfg, egress_intf.intf_type, BCMOLT_FLOW_INTERFACE_TYPE_PON);
                BCMOLT_MSG_FIELD_SET(&cfg, egress_intf.intf_id, access_intf_id);
                printf(" flow_type BCMOLT_FLOW_TYPE_DOWNSTREAM\r\n");
            }
        }

        if (onu_id >= 0)
        {
            BCMOLT_MSG_FIELD_SET(&cfg, onu_id, onu_id);
        }

        if (gemport_id >= 0)
        {
            BCMOLT_MSG_FIELD_SET(&cfg, svc_port_id, gemport_id);
        }

        {
            bcmolt_classifier val = {};

            if (classifier == BCMOLT_CLASSIFIER_ID_O_VID || c_val.o_vid != 0)
            {
                val.o_vid = c_val.o_vid;
                BCMOLT_FIELD_SET(&val, classifier, o_vid, val.o_vid);
                printf(" class val.o_vid[%d]\r\n", val.o_vid);
            }

            if (classifier == BCMOLT_CLASSIFIER_ID_I_VID || c_val.i_vid != 0)
            {
                val.i_vid = c_val.i_vid;
                BCMOLT_FIELD_SET(&val, classifier, i_vid, val.i_vid);
                printf(" val.i_vid[%d]\r\n", val.i_vid);
            }

            if (classifier == BCMOLT_CLASSIFIER_ID_ETHER_TYPE || c_val.ether_type != 0)
            {
                val.ether_type = c_val.ether_type;
                BCMOLT_FIELD_SET(&val, classifier, ether_type, val.ether_type);
                printf(" val.ether_type [%d]\r\n", val.ether_type);
            }

            if (classifier == BCMOLT_CLASSIFIER_ID_IP_PROTO || c_val.ip_proto != 0)
            {
                val.ip_proto = c_val.ip_proto;
                BCMOLT_FIELD_SET(&val, classifier, ip_proto, val.ip_proto);
                printf(" val.ip_proto [%d]\r\n", val.ip_proto);
            }

            if (classifier == BCMOLT_CLASSIFIER_ID_SRC_PORT || c_val.src_port != 0)
            {
                val.src_port = c_val.src_port;
                BCMOLT_FIELD_SET(&val, classifier, src_port, val.src_port);
                printf(" val.src_port [%d]\r\n", val.src_port);
            }

            if (classifier == BCMOLT_CLASSIFIER_ID_DST_PORT || c_val.dst_port != 0)
            {
                val.dst_port = c_val.dst_port;
                BCMOLT_FIELD_SET(&val, classifier, dst_port, val.dst_port);
                printf(" val.dst_port [%d]\r\n", val.dst_port);
            }

            if (pkt_tag_type.compare("untagged") == 0)
            {
                BCMOLT_FIELD_SET(&val, classifier, pkt_tag_type, BCMOLT_PKT_TAG_TYPE_UNTAGGED);
                printf(" classifier.pkt_tag_type untagged\r\n");
            }
            else if (pkt_tag_type.compare("single_tag") == 0)
            {
                val.o_vid = c_val.o_vid;
                BCMOLT_FIELD_SET(&val, classifier, pkt_tag_type, BCMOLT_PKT_TAG_TYPE_SINGLE_TAG);
                BCMOLT_FIELD_SET(&val, classifier, o_vid, val.o_vid);
                single_tag = true;
                printf(" classifier.pkt_tag_type single_tag o_vid[%d]\r\n", val.o_vid);
            }
            else if (pkt_tag_type.compare("double_tag") == 0)
            {
                val.o_vid = c_val.o_vid;
                BCMOLT_FIELD_SET(&val, classifier, pkt_tag_type, BCMOLT_PKT_TAG_TYPE_DOUBLE_TAG);
                BCMOLT_FIELD_SET(&val, classifier, o_vid, val.o_vid);
                BCMOLT_FIELD_SET(&val, classifier, i_vid, val.i_vid);
                printf(" classifier.pkt_tag_type double_tag o_vid[%d] i_vid[%d]\r\n", val.o_vid, val.i_vid);
            }

            BCMOLT_MSG_FIELD_SET(&cfg, classifier, val);
        }

        {
            bcmolt_action val = {};

            if (action_cmd == BCMOLT_ACTION_CMD_ID_ADD_OUTER_TAG)
            {
                BCMOLT_FIELD_SET(&val, action, cmds_bitmask, BCMOLT_ACTION_CMD_ID_ADD_OUTER_TAG);
                printf(" cmd.add_outer_tag BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG\r\n");
            }

            if (action_cmd == BCMOLT_ACTION_CMD_ID_REMOVE_OUTER_TAG)
            {
                BCMOLT_FIELD_SET(&val, action, cmds_bitmask, BCMOLT_ACTION_CMD_ID_REMOVE_OUTER_TAG);
                printf(" cmd.remove_outer_tag BCMBAL_ACTION_CMD_ID_REMOVE_OUTER_TAG\r\n");
            }

            if (action == BCMOLT_ACTION_ID_O_VID || a_val.o_vid != 0)
            {
                val.o_vid = a_val.o_vid;
                BCMOLT_FIELD_SET(&val, action, o_vid, val.o_vid);
                printf(" action.o_vid [%d] BCMBAL_ACTION_ID_O_VID\r\n", val.o_vid);
            }

            BCMOLT_MSG_FIELD_SET(&cfg, action, val);
        }

        if ((access_intf_id >= 0) && (onu_id >= 0))
        {
            bcmolt_tm_queue_ref val = {};

            if (key.flow_type == BCMOLT_FLOW_TYPE_DOWNSTREAM)
            {
                if (single_tag && c_val.ether_type == EAP_ETHER_TYPE)
                {
                    val.sched_id = get_default_tm_sched_id(access_intf_id, downstream);
                    val.queue_id = 1;
                    printf(" SINGLE TAG EAP_ETHER_TYPE[0x%04X] sched_id[%d] queue_id[%d] \r\n", EAP_ETHER_TYPE, val.sched_id, val.queue_id);
                }
                else
                {
                    val.sched_id = get_default_tm_sched_id(access_intf_id, downstream);
                    val.queue_id = 1;
                    printf(" NOT SINGLE TAG sched_id[%d] queue_id[%d] \r\n", val.sched_id, val.queue_id);
                }

                BCMOLT_MSG_FIELD_SET(&cfg, egress_qos.type, BCMOLT_EGRESS_QOS_TYPE_FIXED_QUEUE);
                BCMOLT_MSG_FIELD_SET(&cfg, egress_qos.tm_sched.id, val.sched_id);
                BCMOLT_MSG_FIELD_SET(&cfg, egress_qos.u.fixed_queue.queue_id, val.queue_id);
            }
            else if (key.flow_type == BCMOLT_FLOW_TYPE_UPSTREAM)
            {
                val.sched_id = get_default_tm_sched_id(network_intf_id, upstream);
                val.queue_id = 1;
            }

            BCMOLT_MSG_FIELD_SET(&cfg, egress_qos.type, BCMOLT_EGRESS_QOS_TYPE_FIXED_QUEUE);
            BCMOLT_MSG_FIELD_SET(&cfg, egress_qos.tm_sched.id, val.sched_id);
            BCMOLT_MSG_FIELD_SET(&cfg, egress_qos.u.fixed_queue.queue_id, val.queue_id);
        }

        BCMOLT_MSG_FIELD_SET(&cfg, state, BCMOLT_FLOW_STATE_ENABLE);

        if (d_bcmbal_cfg_set)
        {
            if (d_bcmbal_cfg_set(dev_id, &cfg.hdr))
            {
                printf("Flow add failed\n");
                return false;
            }
            else
            {
                printf("Org Flow size[%d]\n", flow_map.size());
                flow_map[std::pair<int, int>(key.flow_id, key.flow_type)] = flow_map.size();
                printf("Flow add OK!! Flow size[%d]\n", flow_map.size());
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("flow_add error\r\n");
        return false;
    }
}

bool Olt_Device::get_inf_active_state(int port)
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        if (port < get_max_pon_num())
            return get_pon_status(port);
        else
            return get_nni_status(port - get_max_pon_num());
    }
    catch (const exception &e)
    {
        printf("get_inf_active_state error\r\n");
        return false;
    }
}

bool Olt_Device::set_inf_active_state(int port, bool status)
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        if (port < get_max_pon_num())
        {
            if (status)
                return enable_pon_if(port);
            else
                return disable_pon_if(port);
        }
        else
        {
            if (status)
                return enable_nni_if(port - get_max_pon_num());
            else
                return disable_nni_if(port - get_max_pon_num());
        }
    }
    catch (const exception &e)
    {
        printf("set_inf_active_state error\r\n");
        return false;
    }
}

} // namespace acc_bal_api_dist_helper
