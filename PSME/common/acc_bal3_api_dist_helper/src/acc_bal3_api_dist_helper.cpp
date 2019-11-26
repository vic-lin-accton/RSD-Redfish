#include "../include/acc_bal3_api_dist_helper/acc_bal3_api_dist_helper.hpp"
#include <dlfcn.h>
#include <stdlib.h>

using namespace acc_bal3_api_dist_helper;

#ifdef __cplusplus
extern "C"
{

#include <bcmolt_api.h>
#include <bcmolt_host_api.h>
#include <bcmolt_api_model_supporting_enums.h>
//#include <bal_version.h>
#include <bcmolt_api_conn_mgr.h>
#include <bcmcli_session.h>
#include <bcmcli.h>
#include <bcm_api_cli.h>
#include <bcmos_common.h>
#include <bcm_config.h>

}
#endif

bcmolt_oltid dev_id = 0;

const uint32_t tm_upstream_sched_id_start   = 600;
const uint32_t tm_downstream_sched_id_start = 800;
#if defined BAL31 || defined BAL32 
#define TM_Q_SET_ID (bcmolt_tm_queue_set_id)32768U
#endif
const std::string upstream     = "upstream";
const std::string downstream   = "downstream";

#define CLI_HOST_PROMPT_FORMAT "BCM.%u> "
#define SERIAL_NUMBER_SIZE     12
#define EAP_ETHER_TYPE         34958

#define INTERFACE_STATE_IF_DOWN(state) \
    ((state == BCMOLT_INTERFACE_STATE_INACTIVE || \
      state == BCMOLT_INTERFACE_STATE_PROCESSING || \
      state == BCMOLT_INTERFACE_STATE_ACTIVE_STANDBY) ? BCMOS_TRUE : BCMOS_FALSE)
#define INTERFACE_STATE_IF_UP(state) \
    ((state == BCMOLT_INTERFACE_STATE_ACTIVE_WORKING) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_STATE_IF_DOWN(state) \
    ((state == BCMOLT_ONU_OPERATION_INACTIVE || \
      state == BCMOLT_ONU_OPERATION_DISABLE || \
      state == BCMOLT_ONU_OPERATION_ACTIVE_STANDBY) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_STATE_IF_UP(state) \
    ((state == BCMOLT_ONU_OPERATION_ACTIVE) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_RANGING_STATE_IF_UP(state) \
    ((state == BCMOLT_RESULT_SUCCESS) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_RANGING_STATE_IF_DOWN(state) \
    ((state != BCMOLT_RESULT_SUCCESS) ? BCMOS_TRUE : BCMOS_FALSE)

static bcmcli_session *current_session;
static bcmcli_entry *api_parent_dir;
const char *bal_cli_thread_name = "acc_help_bal_cli_thread";
bcmos_bool status_bcm_cli_quit = BCMOS_FALSE;
bcmos_task bal_cli_thread;


bcmolt_odid device_id = 0;

static int bal_apiend_cli_thread_handler(long data)
{
    char init_string[]="\n";
    bcmcli_session *sess = current_session;
    bcmos_task_parm bal_cli_task_p_dummy;

    if (!bcmcli_is_stopped(sess))
    {
        bcmcli_parse(sess, init_string);

        bcmcli_driver(sess);
    };

    printf("BAL API End CLI terminated\n");
    bcmcli_session_close(current_session);
    bcmcli_token_destroy(NULL);
    return 0;
}

bcmos_errno bcmolt_apiend_cli_init() 
{
    bcmos_errno ret;
    bcmos_task_parm bal_cli_task_p = {};
    bcmos_task_parm bal_cli_task_p_dummy;

    if (BCM_ERR_OK != bcmos_task_query(&bal_cli_thread, &bal_cli_task_p_dummy))
    {
        bal_cli_task_p.name = bal_cli_thread_name;
        bal_cli_task_p.handler = bal_apiend_cli_thread_handler;
        bal_cli_task_p.priority = TASK_PRIORITY_CLI;

        ret = bcmos_task_create(&bal_cli_thread, &bal_cli_task_p);
        if (BCM_ERR_OK != ret)
        {
            bcmos_printf("Couldn't create BAL API end CLI thread\n");
            return ret;
        }
    }
}

static void openolt_cli_get_prompt_cb(bcmcli_session *session, char *buf, uint32_t max_len)
{
    snprintf(buf, max_len, CLI_HOST_PROMPT_FORMAT, dev_id);
}

bcmos_errno bcm_openolt_api_cli_init(bcmcli_entry *parent_dir, bcmcli_session *session)
{
    bcmos_errno rc;
    api_parent_dir = parent_dir;
    rc = bcm_api_cli_set_commands(session);
    return rc;
}

static bcmos_errno bcm_cli_quit(bcmcli_session *session, const bcmcli_cmd_parm parm[], uint16_t n_parms)
{
    bcmcli_stop(session);
    bcmcli_session_print(session, "CLI terminated by 'Quit' command\n");
    status_bcm_cli_quit = BCMOS_TRUE;

    return BCM_ERR_OK;
}

int get_status_bcm_cli_quit(void) 
{
    return status_bcm_cli_quit;
}

static inline int get_default_tm_sched_id(int intf_id, std::string direction) 
{
    if (direction.compare(upstream) == 0) 
    {
        return tm_upstream_sched_id_start + intf_id;
    } 
    else if (direction.compare(downstream) == 0) 
    {
        return tm_downstream_sched_id_start + intf_id;
    }
    else 
    {
        printf("invalid direction - %s\n", direction.c_str());
        return 0;
    }
}

static void OltBalReady (short unsigned int olt, bcmolt_msg *msg)
{
    if (msg->subgroup == BCMOLT_OLT_AUTO_SUBGROUP_BAL_READY) 
    {
        //After BAL ready, enable OLT maple
        bcmos_errno err;
        bcmolt_odid dev;

        auto& rOLT = Olt_Device::Olt_Device::get_instance();
        int maple_num = rOLT.get_maple_num();

        for (dev = 0; dev < maple_num; dev++) 
        {
            bcmolt_device_cfg dev_cfg = {};
            bcmolt_device_key dev_key = {};
            dev_key.device_id = dev;
            BCMOLT_CFG_INIT(&dev_cfg, device, dev_key);
            BCMOLT_MSG_FIELD_GET(&dev_cfg, system_mode);
            err = bcmolt_cfg_get(dev_id, &dev_cfg.hdr);

            if (err == BCM_ERR_NOT_CONNECTED) 
            {
                bcmolt_device_key key = {.device_id = dev};
                bcmolt_device_connect oper;
                bcmolt_device_cfg cfg;
                BCMOLT_OPER_INIT(&oper, device, connect, key);
                BCMOLT_MSG_FIELD_SET(&oper, inni_config.mode, BCMOLT_INNI_MODE_ALL_10_G_XFI);
				
                if(rOLT.get_platform() == "asxvolt16")
                {
                    printf("asxvolt16 Enable Maple - %d/%d\n", dev + 1, maple_num);            
                    BCMOLT_MSG_FIELD_SET (&oper, system_mode, BCMOLT_SYSTEM_MODE_XGS__2_X);
                }
                else if(rOLT.get_platform() == "asgvolt64")
                {
                    printf("asgvolt64 Enable Maple - %d/%d\n", dev + 1, maple_num);            
                    BCMOLT_MSG_FIELD_SET(&oper, inni_config.mux, BCMOLT_INNI_MUX_FOUR_TO_ONE);            
                    BCMOLT_MSG_FIELD_SET (&oper, system_mode, BCMOLT_SYSTEM_MODE_GPON__16_X);
                }
                else
                {
                    printf("!!!!!!!Can't identify PLATFORM TYPE!!!!!!!\r\n");     
                    return;
                }
				
                err = bcmolt_oper_submit(dev_id, &oper.hdr);
                if (err)
                    printf("Enable Maple deivce %d failed\n", dev);
                bcmos_usleep(200000);
            }
            else 
            {
                printf("Maple deivce %d already connected\n", dev);
            }
        }
        rOLT.register_callback();
        printf("BAL Ready !!!!!!!!\r\n");
        rOLT.set_bal_status(true);
    }
    bcmolt_msg_free(msg);
}

static void Olt_itf_change(short unsigned int olt , bcmolt_msg *msg)
{
    switch (msg->obj_type) {
        case BCMOLT_OBJ_ID_PON_INTERFACE:
            switch (msg->subgroup) 
            {
                case BCMOLT_PON_INTERFACE_AUTO_SUBGROUP_STATE_CHANGE_COMPLETED:
                    {
                        bcmolt_pon_interface_key *key = &((bcmolt_pon_interface_state_change_completed*)msg)->key;
                        bcmolt_pon_interface_state_change_completed_data *data = &((bcmolt_pon_interface_state_change_completed*)msg)->data;
                        if (INTERFACE_STATE_IF_UP(data->new_state))
                            printf("PON %d is UP completed\r\n",   ((bcmolt_pon_interface_state_change_completed*)msg)->key.pon_ni);
                        if (INTERFACE_STATE_IF_DOWN(data->new_state))
                            printf("PON %d is DOWN completed\r\n", ((bcmolt_pon_interface_state_change_completed*)msg)->key.pon_ni);
                        break;
                    }
            }
            break;
        case BCMOLT_OBJ_ID_NNI_INTERFACE:
            switch (msg->subgroup) 
            {
                case BCMOLT_NNI_INTERFACE_AUTO_SUBGROUP_STATE_CHANGE:
                    {
                        bcmolt_nni_interface_key *key = &((bcmolt_nni_interface_state_change *)msg)->key;
                        bcmolt_nni_interface_state_change_data *data = &((bcmolt_nni_interface_state_change *)msg)->data;

                        if (INTERFACE_STATE_IF_UP(data->new_state))
                            printf("NNI %d is UP state change.\r\n",   ((bcmolt_nni_interface_state_change *)msg)->key.id );
                        if (INTERFACE_STATE_IF_DOWN(data->new_state))
                            printf("NNI %d is DOWN state change.\r\n", ((bcmolt_nni_interface_state_change *)msg)->key.id);
                        break;
                    }
            }
    }
    bcmolt_msg_free(msg);
    return ;
}

static void OltOmciIndication(short unsigned int olt, bcmolt_msg *msg) 
{
    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_ONU:
            switch (msg->subgroup) 
            {
                case BCMOLT_ONU_AUTO_SUBGROUP_OMCI_PACKET:
                    {
                        bcmolt_onu_key *key = &((bcmolt_onu_omci_packet*)msg)->key;
                        bcmolt_onu_omci_packet_data *data = &((bcmolt_onu_omci_packet*)msg)->data;
                        printf("OMCI OltOmciIndication : pon_ni %d, onu_id %d\n", key->pon_ni, key->onu_id);
                        break;
                    }
            }
    }
    bcmolt_msg_free(msg);
}



static void OltOperIndication (short unsigned int olt, bcmolt_msg *msg)
{
    if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_COMPLETE) 
    {
        static int count=0;
        auto& rOLT = Olt_Device::Olt_Device::get_instance();
        count ++;

        if(count == rOLT.get_maple_num()) 
        {
        bcmolt_rx_cfg cb_cfg = {};
        cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
        cb_cfg.rx_cb = OltOmciIndication;
        cb_cfg.subgroup = bcmolt_onu_auto_subgroup_omci_packet;
        cb_cfg.module = BCMOS_MODULE_ID_OMCI_TRANSPORT;
            bcmolt_ind_subscribe(dev_id, &cb_cfg);
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
    bcmolt_msg_free(msg);
    return;
}

static int interface_key_to_port_no(bcmolt_interface_id intf_id, bcmolt_interface_type intf_type) 
{
    if (intf_type == BCMOLT_INTERFACE_TYPE_NNI) {
        return (0x1 << 16) + intf_id;
    }
    if (intf_type == BCMOLT_INTERFACE_TYPE_PON) {
        return (0x2 << 28) + intf_id;
    }
    return intf_id;
}

static void OltLosIndication(short unsigned int olt , bcmolt_msg *msg)
{
    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_PON_INTERFACE:
            switch (msg->subgroup) {
                case BCMOLT_PON_INTERFACE_AUTO_SUBGROUP_LOS:
                    {
                        bcmolt_pon_interface_los* bcm_los_ind = (bcmolt_pon_interface_los *) msg;
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
    bcmolt_msg_free(msg);
    return ;
}

static void OltOnuActivationFailureIndication(short unsigned int olt , bcmolt_msg *msg)
{
    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_ONU:
            switch (msg->subgroup) 
            {
                case BCMOLT_ONU_AUTO_SUBGROUP_ONU_DEACTIVATION_COMPLETED:
                    {
                        bcmolt_onu_key *key = &((bcmolt_onu_onu_activation_completed*)msg)->key;
                        bcmolt_onu_onu_activation_completed_data *data = &((bcmolt_onu_onu_activation_completed*)msg)->data;
                        printf("Got onu deactivation, intf_id %d, onu_id %d, fail_reason %d\n", key->pon_ni, key->onu_id, data->fail_reason);
                    }
            }
    }
    bcmolt_msg_free(msg);
    return ;
}

static void OltOnuAllIndication(short unsigned int olt , bcmolt_msg *msg)
{
    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_ONU:
            printf("BCMOLT_ONU_AUTO_SUBGROUP_ALL sub ID[%d]\r\n", msg->subgroup);
    }
    bcmolt_msg_free(msg);
    return ;
}

const char* serial_number_to_str(bcmolt_serial_number* serial_number) 
{
    static char buff[SERIAL_NUMBER_SIZE+1];

    sprintf(buff, "%c%c%c%c%1X%1X%1X%1X%1X%1X%1X%1X",
            serial_number->vendor_id.arr[0],
            serial_number->vendor_id.arr[1],
            serial_number->vendor_id.arr[2],
            serial_number->vendor_id.arr[3],
            serial_number->vendor_specific.arr[0]>>4 & 0x0f,
            serial_number->vendor_specific.arr[0] & 0x0f,
            serial_number->vendor_specific.arr[1]>>4 & 0x0f,
            serial_number->vendor_specific.arr[1] & 0x0f,
            serial_number->vendor_specific.arr[2]>>4 & 0x0f,
            serial_number->vendor_specific.arr[2] & 0x0f,
            serial_number->vendor_specific.arr[3]>>4 & 0x0f,
            serial_number->vendor_specific.arr[3] & 0x0f);

    return buff;
}

std::string vendor_specific_to_str(char const * const vendor_specific) 
{
    char buff[SERIAL_NUMBER_SIZE+1];

    sprintf(buff, "%1X%1X%1X%1X%1X%1X%1X%1X",
            vendor_specific[0]>>4 & 0x0f,
            vendor_specific[0] & 0x0f,
            vendor_specific[1]>>4 & 0x0f,
            vendor_specific[1] & 0x0f,
            vendor_specific[2]>>4 & 0x0f,
            vendor_specific[2] & 0x0f,
            vendor_specific[3]>>4 & 0x0f,
            vendor_specific[3] & 0x0f);

    return buff;
}

static void OltOnuDiscoveryIndication(short unsigned int olt , bcmolt_msg *msg)
{
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
                        //printf("onu discover indication, pon_ni %d, serial_number %s\n", key->pon_ni, serial_number_to_str(in_serial_number));
                        //printf("onu discover indication, pon_ni %d, vendor_specific %s\n", key->pon_ni, in_serial_number->vendor_specific.arr);
                        break;
                    }
            }
    }
    bcmolt_msg_free(msg);
    return ;
}

static void OltOnuOperIndication(short unsigned int olt , bcmolt_msg *msg) 
{
    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_ONU:
            switch (msg->subgroup) 
            {
                case BCMOLT_ONU_AUTO_SUBGROUP_STATE_CHANGE:
                    {
                        bcmolt_onu_key *key = &((bcmolt_onu_state_change*)msg)->key;
                        bcmolt_onu_state_change_data *data = &((bcmolt_onu_state_change*)msg)->data;

                        if (ONU_STATE_IF_UP(data->new_onu_state))
                            printf("onu oper state indication, intf_id %d, onu_id %d UP!, \n", key->pon_ni, key->onu_id, data->new_onu_state);
                        if (ONU_STATE_IF_DOWN(data->new_onu_state))
                            printf("onu oper state indication, intf_id %d, onu_id %d DOWN!, \n", key->pon_ni, key->onu_id, data->new_onu_state);
                    }
            }
    }
    bcmolt_msg_free(msg);
    return ;
}


static void OltOnuO5(short unsigned int olt , bcmolt_msg *msg) 
{
    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_ONU:
            switch (msg->subgroup) 
            {
                case BCMOLT_ONU_AUTO_SUBGROUP_RANGING_COMPLETED:
                    {
                        bcmolt_onu_key *key = &((bcmolt_onu_ranging_completed*)msg)->key;
                        bcmolt_onu_ranging_completed_data *data = &((bcmolt_onu_ranging_completed*)msg)->data;

                        if (data->status == BCMOLT_RESULT_SUCCESS)
                            printf("onu indication, intf_id %d, onu_id %d UP!, \n", key->pon_ni, key->onu_id, data->status);
                        else
                            printf("onu indications O5 fail\r\n");
                    }
            }
    }
    bcmolt_msg_free(msg);
    return ;
}


namespace acc_bal3_api_dist_helper
{
    static Olt_Device * g_Olt_Device = NULL;

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

    void XGS_PON_Olt_Device::set_pon_status(int port,int status)
    {
        m_pon_port[port].set_status(status);
        return;
    }

    void G_PON_Olt_Device::set_pon_status(int port,int status)
    {
        m_pon_port[port].set_status(status);
        return;
    }

    void XGS_PON_Olt_Device::set_nni_status(int port,int status)
    {
        m_nni_port[port].set_status(status);
        return;
    }

    void G_PON_Olt_Device::set_nni_status(int port,int status)
    {
        m_nni_port[port].set_status(status);
        return;
    }

    void Olt_Device::set_intf_type(int port,int type)
    {
    }

    int Olt_Device::get_board_basic_info()
    {
        bcmos_errno err;
        bcmolt_device_cfg dev_cfg = { };
        bcmolt_device_key dev_key = { };
        bcmolt_olt_cfg olt_cfg = { };
        bcmolt_olt_key olt_key = { };

        dev_key.device_id = dev_id;
        BCMOLT_CFG_INIT(&dev_cfg, device, dev_key);
        BCMOLT_MSG_FIELD_GET(&dev_cfg, firmware_sw_version);
        BCMOLT_MSG_FIELD_GET(&dev_cfg, chip_family);
        BCMOLT_MSG_FIELD_GET(&dev_cfg, system_mode);

        err = bcmolt_cfg_get(dev_id, &dev_cfg.hdr);

        if (err) 
        {
            printf("device: Failed to query OLT\n");
            return 0;
        }
        else
        {
            bcmolt_topology_map topo_map[BCM_MAX_PONS_PER_OLT] = { };
            bcmolt_topology topo = { };
            topo.topology_maps.len = BCM_MAX_PONS_PER_OLT;
            topo.topology_maps.arr = &topo_map[0];
            BCMOLT_CFG_INIT(&olt_cfg, olt, olt_key);
            BCMOLT_MSG_FIELD_GET(&olt_cfg, bal_state);
            BCMOLT_FIELD_SET_PRESENT(&olt_cfg.data, olt_cfg_data, topology);
            BCMOLT_CFG_LIST_BUF_SET(&olt_cfg, olt, topo.topology_maps.arr, sizeof(bcmolt_topology_map) * topo.topology_maps.len);

            err = bcmolt_cfg_get(dev_id, &olt_cfg.hdr);

            if (err) 
            {
                printf("cfg: Failed to query OLT\n");
                return 0; 
            }

            printf("OLT  oper_state:[%s]\n", olt_cfg.data.bal_state == BCMOLT_BAL_STATE_BAL_AND_SWITCH_READY ? "up" : "down");

            m_nni_ports_num = olt_cfg.data.topology.num_switch_ports; 
            m_pon_ports_num = olt_cfg.data.topology.topology_maps.len; 

            m_bal_version = std::to_string(dev_cfg.data.firmware_sw_version.major)
                + "." + std::to_string(dev_cfg.data.firmware_sw_version.minor)
                + "." + std::to_string(dev_cfg.data.firmware_sw_version.revision);

            m_firmware_version = "BAL." + m_bal_version + "__" + m_firmware_version;

            switch(dev_cfg.data.system_mode) 
            {
                case 9 ... 12:  m_pon_type = "GPON"; break;
                case 13 ... 16: m_pon_type = "XGPON"; break;
                case 18 ... 20: m_pon_type = "XGS-PON"; break;
            }

            switch(dev_cfg.data.chip_family) 
            {
//For BAL3.2
#ifdef BAL32     
                case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6862_X: m_chip_family = "Maple"; break;
                case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6865_X: m_chip_family = "Aspen"; break;
#else
                case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6862_X_: m_chip_family = "Maple"; break;
                case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6865_X_: m_chip_family = "Aspen"; break;

#endif
            }

            printf("topology nni:%d pon:%d maple dev number :%d pon number per device :%d family: %s\n",
                    m_nni_ports_num,
                    m_pon_ports_num,
                    BCM_MAX_DEVS_PER_LINE_CARD,
                    BCM_MAX_PONS_PER_DEV,
                    m_chip_family.c_str());

            printf("OLT info., [nni ports: %d] [pon ports: %d] [pon type %s]\n", m_nni_ports_num , m_pon_ports_num, m_pon_type.c_str());
            printf("OLT info., [firware_ver:%s]\n", m_firmware_version.c_str());
        }

        return 1;
    }

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

        if (BCM_ERR_OK !=bcmolt_apiend_cli_init()) 
        {
            printf("Failed to add apiend init\n");
            return false; 
        }
        else
            return true;
    }

    bool Olt_Device::connect_bal(int argc, char *argv[]) 
    {

        bcmos_errno err;
        bcmolt_host_init_parms init_parms = {};
        init_parms.transport.type = BCM_HOST_API_CONN_LOCAL;

        if (BCM_ERR_OK != bcmolt_host_init(&init_parms)) 
        {
            printf("Failed to init bcmolt_host\r\n");
            m_bcmbal_init = false;
            return m_bcmbal_init;
        }
        else
        {
            if(bcmolt_api_conn_mgr_is_connected(dev_id))
            {
                //Need wait bal ready then register other callback indicator//
                bcmolt_rx_cfg cb_cfg = {};
                cb_cfg.obj_type = BCMOLT_OBJ_ID_OLT;
                cb_cfg.rx_cb    = OltBalReady ;
                cb_cfg.flags    = BCMOLT_AUTO_FLAGS_NONE;
                cb_cfg.subgroup = bcmolt_olt_auto_subgroup_bal_ready;

                if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
                {
                    printf("Register_callback BCMOLT_OBJ_ID_OLT bcmolt_olt_auto_subgroup_bal_ready error!!!\r\n");
                    return false;
                }
                else
                    printf("Register_callback BCMOLT_OBJ_ID_OLT bcmolt_olt_auto_subgroup_bal_ready ok!!!\r\n");

                m_bcmbal_init = true;
                return m_bcmbal_init;
            }
            else
            {
                m_bcmbal_init = true;
                return m_bcmbal_init;
            }
        }
        return false;
    }

    Olt_Device& Olt_Device::get_instance()
    {
        static const char * ARGV[1] = 
        {
            ""
        };

        if (NULL == g_Olt_Device) 
        {
            //Check if XGS PON
            ifstream ifs ("/etc/onl/platform");
            std::string s;
            getline (ifs, s, (char) ifs.eof());
            printf("Creating Olt_Device on platform [%s] size[%d]\r\n", s.c_str(), s.size());	    

            if(s.compare(0,s.size()-1,"x86-64-accton-asxvolt16-r0") == 0)
            {           
                printf("x86-64-accton-asxvolt16-r0\r\n");	    
                g_Olt_Device = new XGS_PON_Olt_Device(sizeof(ARGV)/sizeof(char *),(char **) ARGV);
            }
            else if (s.compare(0,s.size()-1,"x86-64-accton-asgvolt64-r0") == 0)
            {
                printf("x86-64-accton-asgvolt64-r0\r\n");	    
                g_Olt_Device = new G_PON_Olt_Device(sizeof(ARGV)/sizeof(char *),(char **) ARGV);
            }
            else
                g_Olt_Device = NULL;

        }
        return *g_Olt_Device;
    }

    void Olt_Device::cleanup() 
    {
        if(g_Olt_Device!=NULL)
        {
            delete g_Olt_Device;
            g_Olt_Device = NULL;
        }
        return;
    }

    Olt_Device::~Olt_Device() 
    {
        if(fHandle)
            dlclose(fHandle);
    }

    Olt_Device::Olt_Device(int argc, char** argv)
    {

        fHandle = dlopen("/lib/x86_64-linux-gnu/libbal_host_api.so",RTLD_LAZY);

        if(fHandle)
        {
            m_bal_lib_init = true;
        }
        else
        {
            m_bal_lib_init = false;
            printf("Cannot find libbal_api_dist.so and using loading function\r\n");
        }

        connect_bal(argc, argv);

    }

    void Olt_Device::register_callback()
    {
        bcmolt_rx_cfg cb_cfg = {};
        bcmos_errno rc;

        uint16_t subgroup;

        if (m_subscribed) 
        {
            return ;
        }

        cb_cfg.obj_type    = BCMOLT_OBJ_ID_DEVICE;
        cb_cfg.rx_cb       = OltOperIndication;
        cb_cfg.subgroup    = bcmolt_device_auto_subgroup_connection_complete; 
        cb_cfg.flags       = BCMOLT_AUTO_FLAGS_NONE; 

        if(bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)  
        {
            printf("Register_callback BCMOLT_OBJ_ID_DEVICE complete error!!!\r\n");
            return; 
        }
        else
        {
            printf("Register_callback bcmolt_device_auto_subgroup_connection_complete ok!!!\r\n");
        }

        cb_cfg.obj_type    = BCMOLT_OBJ_ID_DEVICE;
        cb_cfg.subgroup    = bcmolt_device_auto_subgroup_disconnection_complete; 
        cb_cfg.rx_cb       = OltOperIndication;
        cb_cfg.flags       = BCMOLT_AUTO_FLAGS_NONE; 

        if(bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)  
        {
            printf("Register_callback BCMOLT_OBJ_ID_DEVICE disconnection error!!!\r\n");
            return; 
        }
        else
        {
            printf("Register_callback bcmolt_device_auto_subgroup_disconnection_complete ok!!!\r\n");
        }

        cb_cfg.obj_type    = BCMOLT_OBJ_ID_DEVICE;
        cb_cfg.rx_cb       = OltOperIndication;
        cb_cfg.subgroup    = bcmolt_device_auto_subgroup_connection_failure; 
        cb_cfg.flags       = BCMOLT_AUTO_FLAGS_NONE; 

        if(bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)  
        {
            printf("Register_callback BCMOLT_OBJ_ID_DEVICE connection failure error!!!\r\n");
            return; 
        }
        else
        {
            printf("Register_callback bcmolt_device_auto_subgroup_disconnection_complete ok!!!\r\n");
        }

        cb_cfg.obj_type    = BCMOLT_OBJ_ID_PON_INTERFACE;
        cb_cfg.subgroup    = bcmolt_pon_interface_auto_subgroup_los; 
        cb_cfg.rx_cb       = OltLosIndication; 
        cb_cfg.flags       = BCMOLT_AUTO_FLAGS_NONE; 

        if(bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)  
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

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK) 
        {
            printf("Register_callback BCMOLT_OBJ_ID_PON_INTERFACE Olt_itf_change error!!!\r\n");
            return;
        }
        else
            printf("Register_callback bcmolt_pon_interface_auto_subgroup_state_change_completed ok!!!\r\n");			 

        cb_cfg.obj_type = BCMOLT_OBJ_ID_PON_INTERFACE;
        cb_cfg.rx_cb    = OltOnuDiscoveryIndication;
        cb_cfg.flags    = BCMOLT_AUTO_FLAGS_NONE;
        cb_cfg.subgroup = bcmolt_pon_interface_auto_subgroup_onu_discovered;

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK) 
        {
            printf("Register_callback BCMOLT_OBJ_ID_PON_INTERFACE OltOnuDiscoveryIndication error!!!\r\n");
            return;
        }
        else
            printf("Register_callback bcmolt_pon_interface_auto_subgroup_onu_discovered ok!!!\r\n");			 

        cb_cfg.obj_type = BCMOLT_OBJ_ID_NNI_INTERFACE;
        cb_cfg.subgroup = bcmolt_nni_interface_auto_subgroup_state_change;
        cb_cfg.rx_cb    = Olt_itf_change;
        cb_cfg.flags    = BCMOLT_AUTO_FLAGS_NONE;

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK) 
        {
            printf("Register_callback BCMOLT_OBJ_ID_NNI_INTERFACE error!!!\r\n");
            return;
        }
        else
            printf("Register_callback bcmolt_nni_interface_auto_subgroup_state_change ok!!!\r\n");			 

        cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
        cb_cfg.rx_cb    = OltOnuOperIndication;
        cb_cfg.flags    = BCMOLT_AUTO_FLAGS_NONE;
        cb_cfg.subgroup = bcmolt_onu_auto_subgroup_state_change;

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK) 
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

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
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

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
        {
            printf("Register_callback BCMOLT_OBJ_ID_ONU bcmolt_onu_auto_subgroup_all error!!!\r\n");
            return;
        }
        else
            printf("Register_callback BCMOLT_OBJ_ID_ONU bcmolt_onu_auto_subgroup_all ok!!!\r\n");

        cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
        cb_cfg.rx_cb    = OltOnuO5;
        cb_cfg.flags    = BCMOLT_AUTO_FLAGS_NONE;
        cb_cfg.subgroup = bcmolt_onu_auto_subgroup_ranging_completed;

        if (bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK) 
        {
            printf("Register_callback BCMOLT_OBJ_ID_ONU error!!!\r\n");
            return;
        }
        else
            printf("Register_callback bcmolt_onu_auto_subgroup_ranging_completed ok!!!\r\n");			 

        m_subscribed = true;
        return ; 
    }

    void XGS_PON_Olt_Device::get_pon_port_type()
    {
        /*
           for (int port_id = 0; port_id < m_pon_ports_num; ++port_id) 
           {
           bcmbal_interface_cfg interface_obj;
           bcmbal_interface_key interface_key;

           interface_key.intf_id = port_id;
           interface_key.intf_type = BCMBAL_INTF_TYPE_PON;

           BCMBAL_CFG_INIT(&interface_obj, interface, interface_key);
           BCMBAL_CFG_PROP_GET(&interface_obj, interface, admin_state);
           BCMBAL_CFG_PROP_GET(&interface_obj, interface, transceiver_type);

           bcmos_errno err = BCM_ERR_INTERNAL; 

           if(d_bcmbal_cfg_get)
           err = d_bcmbal_cfg_get(DEFAULT_ATERM_ID, &(interface_obj.hdr));

           if (err != BCM_ERR_OK) 
           {

           m_pon_port[port_id].m_pon_port_type[port_id] = "UNKNOWN"; 
           if(err != BCM_ERR_RANGE) 
           printf("ERROR get PON port %d type \n", port_id);
           }
           else 
           {
           switch(interface_obj.data.transceiver_type) 
           {
           case BCMBAL_TRX_TYPE_GPON_LTE_3680_P:
           case BCMBAL_TRX_TYPE_GPON_SPS_43_48:
           case BCMBAL_TRX_TYPE_GPON_LTE_3680_M:
           case BCMBAL_TRX_TYPE_GPON_SPS_SOG_4321:
           case BCMBAL_TRX_TYPE_GPON_SOURCE_PHOTONICS:

           m_pon_port[port_id].m_pon_port_type[port_id] = "GPON";
           break;

           default:

           m_pon_port[port_id].m_pon_port_type[port_id] = "XGSPON";
           break;
           }
           printf("PON port_id:%d type:%d:%s\n", port_id, interface_obj.data.transceiver_type, m_pon_port[port_id].m_pon_port_type[port_id].c_str());
           }
           }
           */
    }

    void G_PON_Olt_Device::get_pon_port_type()
    {
        /*
           for (int port_id = 0; port_id < m_pon_ports_num; ++port_id) 
           {
           bcmbal_interface_cfg interface_obj;
           bcmbal_interface_key interface_key;

           interface_key.intf_id = port_id;
           interface_key.intf_type = BCMBAL_INTF_TYPE_PON;

           BCMBAL_CFG_INIT(&interface_obj, interface, interface_key);
           BCMBAL_CFG_PROP_GET(&interface_obj, interface, admin_state);
           BCMBAL_CFG_PROP_GET(&interface_obj, interface, transceiver_type);

           bcmos_errno err = BCM_ERR_INTERNAL; 

           if(d_bcmbal_cfg_get)
           err = d_bcmbal_cfg_get(DEFAULT_ATERM_ID, &(interface_obj.hdr));

           if (err != BCM_ERR_OK) 
           {

           m_pon_port[port_id].m_pon_port_type[port_id] = "UNKNOWN"; 
           if(err != BCM_ERR_RANGE) 
           printf("ERROR get PON port %d type \n", port_id);
           }
           else 
           {
           switch(interface_obj.data.transceiver_type) 
           {
           case BCMBAL_TRX_TYPE_GPON_LTE_3680_P:
           case BCMBAL_TRX_TYPE_GPON_SPS_43_48:
           case BCMBAL_TRX_TYPE_GPON_LTE_3680_M:
           case BCMBAL_TRX_TYPE_GPON_SPS_SOG_4321:
           case BCMBAL_TRX_TYPE_GPON_SOURCE_PHOTONICS:

           m_pon_port[port_id].m_pon_port_type[port_id] = "GPON";
           break;

           default:

           m_pon_port[port_id].m_pon_port_type[port_id] = "XGSPON";
           break;
           }
           printf("PON port_id:%d type:%d:%s\n", port_id, interface_obj.data.transceiver_type, m_pon_port[port_id].m_pon_port_type[port_id].c_str());
           }
           }
           */
    }

    /*
       json::Value XGS_PON_Olt_Device::get_port_statistic(int port)
       {
       json::Value status(json::Value::Type::OBJECT);

       if( ((port -1) >= 0) && ((port - 1) < XGS_PON_TOTAL_INTF_NUM) )
       {
       if((port-1) < XGS_PON_MAX_PON_PORT_NUM)
       return get_pon_statistic( port -1);
       else
       return get_nni_statistic( port - XGS_PON_MAX_PON_PORT_NUM -1);
       }
       return status;
       }


       json::Value G_PON_Olt_Device::get_port_statistic(int port)
       {
       json::Value status(json::Value::Type::OBJECT);

       if( ((port -1) >= 0) && ((port - 1) < G_PON_TOTAL_INTF_NUM) )
       {
       if((port-1) < G_PON_MAX_PON_PORT_NUM)
       return get_pon_statistic( port -1);
       else
       return get_nni_statistic( port - G_PON_MAX_PON_PORT_NUM -1);
       }
       return status;
       }

       json::Value XGS_PON_Olt_Device::get_pon_statistic(int port)
       {
       json::Value status(json::Value::Type::OBJECT);

       bcmos_errno err  =BCM_ERR_INTERNAL;
       bcmbal_interface_stat stat;    
       bcmos_bool clear_on_read = false;

       bcmbal_interface_key pon_interface;
       pon_interface.intf_type = BCMBAL_INTF_TYPE_PON;
       pon_interface.intf_id = port;

       BCMBAL_STAT_INIT(&stat, interface, pon_interface);
       BCMBAL_STAT_PROP_GET(&stat, interface, all_properties);

       if (d_bcmbal_stat_get) 
       {
       err =d_bcmbal_stat_get(DEFAULT_ATERM_ID, &stat.hdr, clear_on_read);
       }

       if (err == BCM_ERR_OK)
       {
       status["rx_bytes"]         = m_pon_port[port].m_port_statistic.rx_bytes         = stat.data.rx_bytes; 
       status["rx_packets"]       = m_pon_port[port].m_port_statistic.rx_packets       = stat.data.rx_packets;
       status["rx_ucast_packets"] = m_pon_port[port].m_port_statistic.rx_ucast_packets = stat.data.rx_ucast_packets;
       status["rx_mcast_packets"] = m_pon_port[port].m_port_statistic.rx_mcast_packets = stat.data.rx_mcast_packets;
       status["rx_bcast_packets"] = m_pon_port[port].m_port_statistic.rx_bcast_packets = stat.data.rx_bcast_packets;
       status["rx_error_packets"] = m_pon_port[port].m_port_statistic.rx_error_packets = stat.data.rx_error_packets;

       status["tx_bytes"]         = m_pon_port[port].m_port_statistic.tx_bytes         = stat.data.tx_bytes; 
       status["tx_packets"]       = m_pon_port[port].m_port_statistic.tx_packets       = stat.data.tx_packets;
       status["tx_ucast_packets"] = m_pon_port[port].m_port_statistic.tx_ucast_packets = stat.data.tx_ucast_packets;
       status["tx_mcast_packets"] = m_pon_port[port].m_port_statistic.tx_mcast_packets = stat.data.tx_mcast_packets;
       status["tx_bcast_packets"] = m_pon_port[port].m_port_statistic.tx_bcast_packets = stat.data.tx_bcast_packets;
       status["tx_error_packets"] = m_pon_port[port].m_port_statistic.tx_error_packets = stat.data.tx_error_packets;

       printf( 
       "pon port_id[%d]\r\n\
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
        tx_error_packets[%llu]\r\n"\
        ,
           m_pon_port[port].m_port_id,\
               (unsigned long long)m_pon_port[port].m_port_statistic.rx_bytes,\
               (unsigned long long)m_pon_port[port].m_port_statistic.rx_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.rx_ucast_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.rx_mcast_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.rx_bcast_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.rx_error_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.tx_bytes,\
               (unsigned long long)m_pon_port[port].m_port_statistic.tx_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.tx_ucast_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.tx_mcast_packets, \
               (unsigned long long)m_pon_port[port].m_port_statistic.tx_bcast_packets,\
               (unsigned long long)m_pon_port[port].m_port_statistic.tx_error_packets\
               );
} 
else 
{
    printf("Failed to get pon port statistics, port_id %d, intf_type %d\n", (int)pon_interface.intf_id, (int)pon_interface.intf_type);
}
return status;
}


json::Value G_PON_Olt_Device::get_pon_statistic(int port)
{
    json::Value status(json::Value::Type::OBJECT);

    bcmos_errno err = BCM_ERR_INTERNAL;
    bcmbal_interface_stat stat;    
    bcmos_bool clear_on_read = false;

    bcmbal_interface_key pon_interface;
    pon_interface.intf_type = BCMBAL_INTF_TYPE_PON;
    pon_interface.intf_id = port;

    BCMBAL_STAT_INIT(&stat, interface, pon_interface);
    BCMBAL_STAT_PROP_GET(&stat, interface, all_properties);

    if (d_bcmbal_stat_get) 
    {
        err =d_bcmbal_stat_get(DEFAULT_ATERM_ID, &stat.hdr, clear_on_read);
    }

    if (err == BCM_ERR_OK)
    {
        status["rx_bytes"]         = m_pon_port[port].m_port_statistic.rx_bytes         = stat.data.rx_bytes; 
        status["rx_packets"]       = m_pon_port[port].m_port_statistic.rx_packets       = stat.data.rx_packets;
        status["rx_ucast_packets"] = m_pon_port[port].m_port_statistic.rx_ucast_packets = stat.data.rx_ucast_packets;
        status["rx_mcast_packets"] = m_pon_port[port].m_port_statistic.rx_mcast_packets = stat.data.rx_mcast_packets;
        status["rx_bcast_packets"] = m_pon_port[port].m_port_statistic.rx_bcast_packets = stat.data.rx_bcast_packets;
        status["rx_error_packets"] = m_pon_port[port].m_port_statistic.rx_error_packets = stat.data.rx_error_packets;

        status["tx_bytes"]         = m_pon_port[port].m_port_statistic.tx_bytes         = stat.data.tx_bytes; 
        status["tx_packets"]       = m_pon_port[port].m_port_statistic.tx_packets       = stat.data.tx_packets;
        status["tx_ucast_packets"] = m_pon_port[port].m_port_statistic.tx_ucast_packets = stat.data.tx_ucast_packets;
        status["tx_mcast_packets"] = m_pon_port[port].m_port_statistic.tx_mcast_packets = stat.data.tx_mcast_packets;
        status["tx_bcast_packets"] = m_pon_port[port].m_port_statistic.tx_bcast_packets = stat.data.tx_bcast_packets;
        status["tx_error_packets"] = m_pon_port[port].m_port_statistic.tx_error_packets = stat.data.tx_error_packets;

        printf( 
                "pon port_id[%d]\r\n\
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
                tx_error_packets[%llu]\r\n"\
                ,
                m_pon_port[port].m_port_id,\
                (unsigned long long)m_pon_port[port].m_port_statistic.rx_bytes,\
                (unsigned long long)m_pon_port[port].m_port_statistic.rx_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.rx_ucast_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.rx_mcast_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.rx_bcast_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.rx_error_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.tx_bytes,\
                (unsigned long long)m_pon_port[port].m_port_statistic.tx_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.tx_ucast_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.tx_mcast_packets, \
                (unsigned long long)m_pon_port[port].m_port_statistic.tx_bcast_packets,\
                (unsigned long long)m_pon_port[port].m_port_statistic.tx_error_packets\
                );
    } 
    else 
    {
        printf("Failed to get pon port statistics, port_id %d, intf_type %d\n", (int)pon_interface.intf_id, (int)pon_interface.intf_type);
    }
    return status;
}

json::Value XGS_PON_Olt_Device::get_nni_statistic(int port)
{
    json::Value status(json::Value::Type::OBJECT);

    bcmos_errno err = BCM_ERR_INTERNAL;
    bcmbal_interface_stat stat;    
    bcmos_bool clear_on_read = false;

    bcmbal_interface_key nni_interface;
    nni_interface.intf_type = BCMBAL_INTF_TYPE_NNI;
    nni_interface.intf_id = port;

    BCMBAL_STAT_INIT(&stat, interface, nni_interface);
    BCMBAL_STAT_PROP_GET(&stat, interface, all_properties);

    if(d_bcmbal_stat_get)
        err = d_bcmbal_stat_get(DEFAULT_ATERM_ID, &stat.hdr, clear_on_read);

    if (err == BCM_ERR_OK)
    {
        status["rx_bytes"]         = m_nni_port[port].m_port_statistic.rx_bytes         = stat.data.rx_bytes; 
        status["rx_packets"]       = m_nni_port[port].m_port_statistic.rx_packets       = stat.data.rx_packets;
        status["rx_ucast_packets"] = m_nni_port[port].m_port_statistic.rx_ucast_packets = stat.data.rx_ucast_packets;
        status["rx_mcast_packets"] = m_nni_port[port].m_port_statistic.rx_mcast_packets = stat.data.rx_mcast_packets;
        status["rx_bcast_packets"] = m_nni_port[port].m_port_statistic.rx_bcast_packets = stat.data.rx_bcast_packets;
        status["rx_error_packets"] = m_nni_port[port].m_port_statistic.rx_error_packets = stat.data.rx_error_packets;

        status["tx_bytes"]         = m_nni_port[port].m_port_statistic.tx_bytes         = stat.data.tx_bytes; 
        status["tx_packets"]       = m_nni_port[port].m_port_statistic.tx_packets       = stat.data.tx_packets;
        status["tx_ucast_packets"] = m_nni_port[port].m_port_statistic.tx_ucast_packets = stat.data.tx_ucast_packets;
        status["tx_mcast_packets"] = m_nni_port[port].m_port_statistic.tx_mcast_packets = stat.data.tx_mcast_packets;
        status["tx_bcast_packets"] = m_nni_port[port].m_port_statistic.tx_bcast_packets = stat.data.tx_bcast_packets;
        status["tx_error_packets"] = m_nni_port[port].m_port_statistic.tx_error_packets = stat.data.tx_error_packets;

        printf( 
                "pon port_id[%d]\r\n\
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
                tx_error_packets[%llu]\r\n"\
                ,
                m_nni_port[port].m_port_id,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_bytes,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_ucast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_mcast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_bcast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_error_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_bytes,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_ucast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_mcast_packets, \
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_bcast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_error_packets\
                );
    } 
    else 
    {
        status["rx_bytes"]         = 0; 
        status["rx_packets"]       = 0; 
        status["rx_ucast_packets"] = 0; 
        status["rx_mcast_packets"] = 0; 
        status["rx_bcast_packets"] = 0; 
        status["rx_error_packets"] = 0; 

        status["tx_bytes"]         = 0; 
        status["tx_packets"]       = 0; 
        status["tx_ucast_packets"] = 0; 
        status["tx_mcast_packets"] = 0;
        status["tx_bcast_packets"] = 0; 
        status["tx_error_packets"] = 0; 

        printf("Failed to get pon port statistics, port_id %d, intf_type %d\n", (int)nni_interface.intf_id, (int)nni_interface.intf_type);
    }
    return status;
}

json::Value G_PON_Olt_Device::get_nni_statistic(int port)
{
    json::Value status(json::Value::Type::OBJECT);

    bcmos_errno err = BCM_ERR_INTERNAL;
    bcmbal_interface_stat stat;    
    bcmos_bool clear_on_read = false;

    bcmbal_interface_key nni_interface;
    nni_interface.intf_type = BCMBAL_INTF_TYPE_NNI;
    nni_interface.intf_id = port;

    BCMBAL_STAT_INIT(&stat, interface, nni_interface);
    BCMBAL_STAT_PROP_GET(&stat, interface, all_properties);

    if(d_bcmbal_stat_get)
        err = d_bcmbal_stat_get(DEFAULT_ATERM_ID, &stat.hdr, clear_on_read);

    if (err == BCM_ERR_OK)
    {
        status["rx_bytes"]         = m_nni_port[port].m_port_statistic.rx_bytes         = stat.data.rx_bytes; 
        status["rx_packets"]       = m_nni_port[port].m_port_statistic.rx_packets       = stat.data.rx_packets;
        status["rx_ucast_packets"] = m_nni_port[port].m_port_statistic.rx_ucast_packets = stat.data.rx_ucast_packets;
        status["rx_mcast_packets"] = m_nni_port[port].m_port_statistic.rx_mcast_packets = stat.data.rx_mcast_packets;
        status["rx_bcast_packets"] = m_nni_port[port].m_port_statistic.rx_bcast_packets = stat.data.rx_bcast_packets;
        status["rx_error_packets"] = m_nni_port[port].m_port_statistic.rx_error_packets = stat.data.rx_error_packets;

        status["tx_bytes"]         = m_nni_port[port].m_port_statistic.tx_bytes         = stat.data.tx_bytes; 
        status["tx_packets"]       = m_nni_port[port].m_port_statistic.tx_packets       = stat.data.tx_packets;
        status["tx_ucast_packets"] = m_nni_port[port].m_port_statistic.tx_ucast_packets = stat.data.tx_ucast_packets;
        status["tx_mcast_packets"] = m_nni_port[port].m_port_statistic.tx_mcast_packets = stat.data.tx_mcast_packets;
        status["tx_bcast_packets"] = m_nni_port[port].m_port_statistic.tx_bcast_packets = stat.data.tx_bcast_packets;
        status["tx_error_packets"] = m_nni_port[port].m_port_statistic.tx_error_packets = stat.data.tx_error_packets;

        printf( 
                "pon port_id[%d]\r\n\
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
                tx_error_packets[%llu]\r\n"\
                ,
                m_nni_port[port].m_port_id,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_bytes,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_ucast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_mcast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_bcast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.rx_error_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_bytes,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_ucast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_mcast_packets, \
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_bcast_packets,\
                (unsigned long long)m_nni_port[port].m_port_statistic.tx_error_packets\
                );
    } 
    else 
    {
        status["rx_bytes"]         = 0; 
        status["rx_packets"]       = 0; 
        status["rx_ucast_packets"] = 0; 
        status["rx_mcast_packets"] = 0; 
        status["rx_bcast_packets"] = 0; 
        status["rx_error_packets"] = 0; 

        status["tx_bytes"]         = 0; 
        status["tx_packets"]       = 0; 
        status["tx_ucast_packets"] = 0; 
        status["tx_mcast_packets"] = 0;
        status["tx_bcast_packets"] = 0; 
        status["tx_error_packets"] = 0; 

        printf("Failed to get pon port statistics, port_id %d, intf_type %d\n", (int)nni_interface.intf_id, (int)nni_interface.intf_type);
    }
    return status;
}
*/


bool CreateDefaultSchedQueue(uint32_t intf_id, const std::string direction) 
{
    bcmos_errno err;
    bcmolt_tm_sched_cfg tm_sched_cfg;
    bcmolt_tm_sched_key tm_sched_key = {.id = 1};
    tm_sched_key.id = get_default_tm_sched_id(intf_id, direction);

    printf("CreateDefaultSchedQueue for %s tm_sched id %d intf_id %d ", direction.c_str(), tm_sched_key.id, intf_id);
    bcmos_usleep(200000);

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

    err = bcmolt_cfg_set(dev_id, &tm_sched_cfg.hdr);
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
#if defined(BAL31) || defined(BAL32)
        tm_queue_key.tm_q_set_id = TM_Q_SET_ID;
#endif
        BCMOLT_CFG_INIT(&tm_queue_cfg, tm_queue, tm_queue_key);
        BCMOLT_MSG_FIELD_SET(&tm_queue_cfg, tm_sched_param.type, BCMOLT_TM_SCHED_PARAM_TYPE_PRIORITY);
        BCMOLT_MSG_FIELD_SET(&tm_queue_cfg, tm_sched_param.u.priority.priority, queue_id);

        err = bcmolt_cfg_set(dev_id, &tm_queue_cfg.hdr);

        printf("CreateDefaultSchedQueue for %s tm_queue id %d, sched_id %d tm_queue_key.id \n", direction.c_str(), tm_queue_key.id, tm_queue_key.sched_id, tm_queue_key.id);
        if (err) 
        {
            printf(" ERROR!!\n");
            return false;
        }
        else
            printf(" OK!!\n");
    }
    return true;
}

bcmos_errno get_nni_interface_status(bcmolt_interface id, bcmolt_interface_state *state) 
{
    bcmos_errno err;
    bcmolt_nni_interface_key nni_key;
    bcmolt_nni_interface_cfg nni_cfg;
    nni_key.id = id;

    BCMOLT_CFG_INIT(&nni_cfg, nni_interface, nni_key);
    BCMOLT_FIELD_SET_PRESENT(&nni_cfg.data, nni_interface_cfg_data, state);
    err = bcmolt_cfg_get(dev_id, &nni_cfg.hdr);
    *state = nni_cfg.data.state;
    return err;
}

bool Olt_Device::enable_nni_if(int intf_id) 
{
    bcmos_errno err = BCM_ERR_OK;
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
    err = bcmolt_oper_submit(dev_id, &nni_interface_set_state.hdr);

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

bcmos_errno get_pon_interface_status(bcmolt_interface pon_ni, bcmolt_interface_state *state) 
{
    bcmos_errno err;
    bcmolt_pon_interface_key pon_key;
    bcmolt_pon_interface_cfg pon_cfg;
    pon_key.pon_ni = pon_ni;

    BCMOLT_CFG_INIT(&pon_cfg, pon_interface, pon_key);
    BCMOLT_FIELD_SET_PRESENT(&pon_cfg.data, pon_interface_cfg_data, state);
    BCMOLT_FIELD_SET_PRESENT(&pon_cfg.data, pon_interface_cfg_data, itu);
    err = bcmolt_cfg_get(dev_id, &pon_cfg.hdr);
    *state = pon_cfg.data.state;
    return err;
}

bool  Olt_Device::enable_pon_if(int intf_id) 
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

    err = bcmolt_cfg_set(dev_id, &interface_obj.hdr);
    if (err != BCM_ERR_OK) 
    {
        printf("Failed to enable discovery onu: %d\n", intf_id);
        return false; 
    }

    err = bcmolt_oper_submit(dev_id, &pon_interface_set_state.hdr);
    if (err != BCM_ERR_OK) 
    {
        printf("Failed to enable PON interface: %d\n", intf_id);
        return false; 
    }
    else 
    {
        printf("Successfully enabled PON interface: %d\n", intf_id);
        CreateDefaultSchedQueue(intf_id, downstream);
    }

    return true;
}

bool Olt_Device::deactivate_onu(int intf_id, int onu_id)
{
    bcmos_errno err = BCM_ERR_OK;
    bcmolt_onu_set_onu_state onu_oper; 
    bcmolt_onu_cfg onu_cfg;
    bcmolt_onu_key onu_key; 
    bcmolt_onu_state onu_state;

    onu_key.onu_id = onu_id;
    onu_key.pon_ni = intf_id;
    BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
    BCMOLT_FIELD_SET_PRESENT(&onu_cfg.data, onu_cfg_data, onu_state);

    err = bcmolt_cfg_get(dev_id, &onu_cfg.hdr);

    if (err == BCM_ERR_OK) 
    {
        switch (onu_state) 
        {
            case BCMOLT_ONU_OPERATION_ACTIVE:
                BCMOLT_OPER_INIT(&onu_oper, onu, set_onu_state, onu_key);
                BCMOLT_FIELD_SET(&onu_oper.data, onu_set_onu_state_data, onu_state, BCMOLT_ONU_OPERATION_INACTIVE);

                err = bcmolt_oper_submit(dev_id, &onu_oper.hdr);

                if (err != BCM_ERR_OK) 
                {
                    printf("Failed to deactivate ONU %d on PON %d, err %d\n", onu_id, intf_id, err);
                    return false; 
                }
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

bool XGS_PON_Olt_Device::activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific) 
{
    bcmos_errno err = BCM_ERR_OK;
    bcmolt_onu_cfg onu_cfg;
    bcmolt_onu_key onu_key;
    bcmolt_serial_number serial_number; 
    bcmolt_bin_str_36 registration_id; 

    onu_key.onu_id = onu_id;
    onu_key.pon_ni = intf_id;
    BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
    BCMOLT_FIELD_SET_PRESENT(&onu_cfg.data, onu_cfg_data, onu_state);
    err = bcmolt_cfg_get(dev_id, &onu_cfg.hdr);

    if (err == BCM_ERR_OK) 
    {
        if ((onu_cfg.data.onu_state == BCMOLT_ONU_STATE_PROCESSING ||
                    onu_cfg.data.onu_state == BCMOLT_ONU_STATE_ACTIVE) ||
                (onu_cfg.data.onu_state == BCMOLT_ONU_STATE_INACTIVE &&
                 onu_cfg.data.onu_old_state == BCMOLT_ONU_STATE_NOT_CONFIGURED))
            return true; 
    }

    printf("XGSPON Enabling ONU %d on PON %d : vendor id %s,  vendor specific %s", onu_id, intf_id, vendor_id, vendor_specific_to_str(vendor_specific).c_str());
    memcpy(serial_number.vendor_id.arr, vendor_id, 4);
    memcpy(serial_number.vendor_specific.arr, vendor_specific, 4);
    BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
#if !defined(BAL31) && !defined(BAL32)
    BCMOLT_MSG_FIELD_SET(&onu_cfg, onu_rate, BCMOLT_ONU_RATE_RATE_10G_DS_10G_US);
#endif
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.serial_number, serial_number);
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.auto_learning, BCMOS_TRUE);
#if defined(BAL31)|| defined(BAL32)
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.xgpon.ranging_burst_profile, 2);
#else
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.xgpon.ranging_burst_profile, 0);
#endif    
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.xgpon.data_burst_profile, 1);
    err = bcmolt_cfg_set(dev_id, &onu_cfg.hdr);

    if (err != BCM_ERR_OK) 
    {
        printf(" ERROR [%d]\n", err);
        return false;
    }
    printf(" OK!!\r\n");

    alloc_id_add(intf_id, onu_id, (1023+onu_id) );	
    return true;
}

bool G_PON_Olt_Device::activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific) 
{
    bcmos_errno err = BCM_ERR_OK;
    bcmolt_onu_cfg onu_cfg;
    bcmolt_onu_key onu_key;
    bcmolt_serial_number serial_number; 
    bcmolt_bin_str_36 registration_id; 

    onu_key.onu_id = onu_id;
    onu_key.pon_ni = intf_id;
    BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
    BCMOLT_FIELD_SET_PRESENT(&onu_cfg.data, onu_cfg_data, onu_state);
    err = bcmolt_cfg_get(dev_id, &onu_cfg.hdr);

    if (err == BCM_ERR_OK) 
    {
        if ((onu_cfg.data.onu_state == BCMOLT_ONU_STATE_PROCESSING ||
                    onu_cfg.data.onu_state == BCMOLT_ONU_STATE_ACTIVE) ||
                (onu_cfg.data.onu_state == BCMOLT_ONU_STATE_INACTIVE &&
                 onu_cfg.data.onu_old_state == BCMOLT_ONU_STATE_NOT_CONFIGURED))
            return true; 
    }

    printf("GPON Enabling ONU %d on PON %d : vendor id %s,  vendor specific %s", onu_id, intf_id, vendor_id, vendor_specific_to_str(vendor_specific).c_str());
    memcpy(serial_number.vendor_id.arr, vendor_id, 4);
    memcpy(serial_number.vendor_specific.arr, vendor_specific, 4);
    BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.serial_number, serial_number);
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.auto_learning, BCMOS_TRUE);
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.gpon.ds_ber_reporting_interval, 1000000);
    BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.gpon.omci_port_id, onu_id);
	
    err = bcmolt_cfg_set(dev_id, &onu_cfg.hdr);

    if (err != BCM_ERR_OK) 
    {
        printf(" ERROR [%d]\n", err);
        return false;
    }
    printf(" OK!!\r\n");

    alloc_id_add(intf_id, onu_id, (1023+onu_id) );	
    return true;
}

bool XGS_PON_Olt_Device::alloc_id_add(int intf_id, int in_onu_id, int alloc_id) 
{
    bcmos_errno err = BCM_ERR_OK;
    bcmolt_itupon_alloc_cfg cfg; /* declare main API struct */
    bcmolt_itupon_alloc_key key = {}; /* declare key */

    key.pon_ni   = intf_id;
    key.alloc_id = alloc_id;

    printf("XGSPON alloc_id_add pon_id[%d] onu_id[%d] alloc_id[%d]", intf_id, in_onu_id , alloc_id );

    /* Initialize the API struct. */
    BCMOLT_CFG_INIT(&cfg, itupon_alloc, key);

    bcmolt_pon_alloc_sla sla = {};
    uint32_t sla_cbr_rt_bw;
    sla_cbr_rt_bw = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_rt_bw, sla_cbr_rt_bw);

    uint32_t sla_cbr_nrt_bw;
    sla_cbr_nrt_bw = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_nrt_bw, sla_cbr_nrt_bw);

    uint32_t sla_guaranteed_bw;
    sla_guaranteed_bw = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, guaranteed_bw, sla_guaranteed_bw);

    uint32_t sla_maximum_bw;
    sla_maximum_bw = 155520000; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, maximum_bw, sla_maximum_bw);

    bcmolt_additional_bw_eligibility sla_additional_bw_eligibility;
    sla_additional_bw_eligibility = BCMOLT_ADDITIONAL_BW_ELIGIBILITY_BEST_EFFORT; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, additional_bw_eligibility, sla_additional_bw_eligibility);

    bcmos_bool sla_cbr_rt_compensation;
    sla_cbr_rt_compensation = BCMOS_FALSE; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_rt_compensation, sla_cbr_rt_compensation);

    uint8_t sla_cbr_rt_ap_index;
    sla_cbr_rt_ap_index = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_rt_ap_index, sla_cbr_rt_ap_index);

    uint8_t sla_cbr_nrt_ap_index;
    sla_cbr_nrt_ap_index = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_nrt_ap_index, sla_cbr_nrt_ap_index);

    bcmolt_alloc_type sla_alloc_type;
    sla_alloc_type = BCMOLT_ALLOC_TYPE_NSR; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, alloc_type, sla_alloc_type);

    uint8_t sla_weight;
    sla_weight = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, weight, sla_weight);

    uint8_t sla_priority;
    sla_priority = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, priority, sla_priority);

    BCMOLT_FIELD_SET(&cfg.data, itupon_alloc_cfg_data, sla, sla);

    bcmolt_onu_id onu_id;
    onu_id = in_onu_id; 
    BCMOLT_FIELD_SET(&cfg.data, itupon_alloc_cfg_data, onu_id, onu_id);

    err = bcmolt_cfg_set(dev_id, &cfg.hdr);

    if (err != BCM_ERR_OK) 
    {
        printf(" ERROR!! \n");
        return false;
    }
    printf(" OK!!!\r\n");
    return true;
}

bool G_PON_Olt_Device::alloc_id_add(int intf_id, int in_onu_id, int alloc_id) 
{
    bcmos_errno err = BCM_ERR_OK;
    bcmolt_itupon_alloc_cfg cfg; /* declare main API struct */
    bcmolt_itupon_alloc_key key = {}; /* declare key */

    key.pon_ni   = intf_id;
    key.alloc_id = alloc_id;

    printf("GPON alloc_id_add pon_id[%d] onu_id[%d] alloc_id[%d]", intf_id, in_onu_id , alloc_id );

    /* Initialize the API struct. */
    BCMOLT_CFG_INIT(&cfg, itupon_alloc, key);

    bcmolt_pon_alloc_sla sla = {};
    uint32_t cbr_rt_bw;
    cbr_rt_bw = 5120000; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_rt_bw, cbr_rt_bw);

    uint32_t sla_cbr_nrt_bw;
    sla_cbr_nrt_bw = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_nrt_bw, sla_cbr_nrt_bw);

    bcmolt_alloc_type sla_alloc_type;
    sla_alloc_type = BCMOLT_ALLOC_TYPE_NONE; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, alloc_type, sla_alloc_type);

    uint32_t sla_guaranteed_bw;
    sla_guaranteed_bw = 5120000; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, guaranteed_bw, sla_guaranteed_bw);

    uint32_t sla_maximum_bw;
    sla_maximum_bw = 5120000; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, maximum_bw, sla_maximum_bw);

    bcmolt_additional_bw_eligibility sla_additional_bw_eligibility;
    sla_additional_bw_eligibility = BCMOLT_ADDITIONAL_BW_ELIGIBILITY_NONE; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, additional_bw_eligibility, sla_additional_bw_eligibility);

    bcmos_bool sla_cbr_rt_compensation;
    sla_cbr_rt_compensation = BCMOS_FALSE; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_rt_compensation, sla_cbr_rt_compensation);

    uint8_t sla_cbr_rt_ap_index;
    sla_cbr_rt_ap_index = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_rt_ap_index, sla_cbr_rt_ap_index);

    uint8_t sla_cbr_nrt_ap_index;
    sla_cbr_nrt_ap_index = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, cbr_nrt_ap_index, sla_cbr_nrt_ap_index);

    uint8_t sla_weight;
    sla_weight = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, weight, sla_weight);

    uint8_t sla_priority;
    sla_priority = 0; 
    BCMOLT_FIELD_SET(&sla, pon_alloc_sla, priority, sla_priority);

    BCMOLT_FIELD_SET(&cfg.data, itupon_alloc_cfg_data, sla, sla);

    bcmolt_onu_id onu_id;
    onu_id = in_onu_id; 
    BCMOLT_FIELD_SET(&cfg.data, itupon_alloc_cfg_data, onu_id, onu_id);

    err = bcmolt_cfg_set(dev_id, &cfg.hdr);

    if (err != BCM_ERR_OK) 
    {
        printf(" ERROR!! \n");
        return false;
    }
    printf(" OK!!!\r\n");
    return true;
}

#define MAX_CHAR_LEN  20
#define MAX_OMCI_MSG_LEN 44
bool Olt_Device::omci_msg_out(int intf_id, int onu_id, const std::string pkt) 
{
    bcmolt_bin_str buf; /* A structure with a msg pointer and length value */
    bcmolt_onu_cpu_packets omci_cpu_packets;
    bcmolt_onu_key key;

    key.pon_ni = intf_id;
    key.onu_id = onu_id;

    BCMOLT_OPER_INIT(&omci_cpu_packets, onu, cpu_packets, key);
    BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, packet_type, BCMOLT_PACKET_TYPE_OMCI);
    BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, calc_crc, BCMOS_TRUE);

    if ((pkt.size()/2) > MAX_OMCI_MSG_LEN) 
    {
        buf.len = MAX_OMCI_MSG_LEN;
    } 
    else 
    {
        buf.len = pkt.size()/2;
    }

    uint16_t idx1 = 0;
    uint16_t idx2 = 0;
    uint8_t arraySend[buf.len];
    char str1[MAX_CHAR_LEN];
    char str2[MAX_CHAR_LEN];
    memset(&arraySend, 0, buf.len);

    printf("Sending omci msg to ONU %d to PON %d of length is %d\r\n", onu_id, intf_id, buf.len);

    for (idx1=0,idx2=0; idx1<((buf.len)*2); idx1++,idx2++) 
    {
        sprintf(str1,"%c", pkt[idx1]);
        sprintf(str2,"%c", pkt[++idx1]);
        strcat(str1,str2);
        arraySend[idx2] = strtol(str1, NULL, 16);
    }

    buf.arr = (uint8_t *)malloc((buf.len)*sizeof(uint8_t));
    memcpy(buf.arr, (uint8_t *)arraySend, buf.len);

    BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, number_of_packets, 1);
    BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, packet_size, buf.len);
    BCMOLT_MSG_FIELD_SET(&omci_cpu_packets, buffer, buf);

    bcmos_errno err = bcmolt_oper_submit(dev_id, &omci_cpu_packets.hdr);
    if (err) 
    {
        printf("Error sending OMCI message to ONU %d on PON %d Err:%d\n", onu_id, intf_id, err);
    } 

    printf("OMCI raw data: ");
    int count =0;
    int buf_len = buf.len ;

    for(count = 1 ; count <= buf_len ; count++)
    {
        printf("%02X", buf.arr[count-1]);
    }

    printf("\r\n");

    free(buf.arr);

    return true;
}


bool Olt_Device::flow_remove(uint32_t flow_id, const std::string flow_type) 
{
    bcmolt_flow_cfg cfg;
    bcmolt_flow_key key = {};

    printf("flow_remove flow_id[%d] flow_type[%s]", flow_id, flow_type.c_str());

    key.flow_id = (bcmolt_flow_id) flow_id;
    key.flow_id = flow_id;
    if (flow_type.compare(upstream) == 0 ) 
    {
        key.flow_type = BCMOLT_FLOW_TYPE_UPSTREAM;
    } 
    else if (flow_type.compare(downstream) == 0) 
    {
        key.flow_type = BCMOLT_FLOW_TYPE_DOWNSTREAM;
    } else 
    {
        printf("Invalid flow type %s\n");
        return false; 
    }

    BCMOLT_CFG_INIT(&cfg, flow, key);

    bcmos_errno err = bcmolt_cfg_clear(dev_id, &cfg.hdr);

    if (err) 
    {
        printf( " Error\n");
        return false;
    }
    else
        printf( " OK\n");

    return true; 
}


// access_intf_id  : PON ID
// network_intf_id : NNI ID
bool Olt_Device::flow_add(int onu_id, int flow_id, const std::string flow_type, const std::string pkt_tag_type, int access_intf_id,
        int network_intf_id, int gemport_id, int classifier, 
        int action ,int action_cmd, struct action_val a_val, struct class_val c_val)
{
    bool single_tag = false;
    bcmolt_flow_cfg cfg;
    bcmolt_flow_key key = { }; /**< Object key. */

    printf("flow_add PON[%d] onu_id[%d] flow_id[%d] flow_type[%s] pkt_tag_type[%s] gemport_id[%d] NNI[%d] classifier[0x%04X] action[0x%04X]\r\n", access_intf_id, onu_id , flow_id, flow_type.c_str(), pkt_tag_type.c_str() , gemport_id,network_intf_id, classifier, action);

    key.flow_id = flow_id;

    if (flow_type.compare("upstream") == 0 ) 
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

        if (classifier == BCMOLT_CLASSIFIER_ID_O_VID || c_val.o_vid !=0) 
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

        if (classifier == BCMOLT_CLASSIFIER_ID_ETHER_TYPE || c_val.ether_type !=0) 
        {
            val.ether_type = c_val.ether_type;
            BCMOLT_FIELD_SET(&val, classifier, ether_type, val.ether_type);
            printf(" val.ether_type [%d]\r\n", val.ether_type );	
        }

        if (classifier == BCMOLT_CLASSIFIER_ID_IP_PROTO || c_val.ip_proto !=0) 
        {
            val.ip_proto = c_val.ip_proto; 
            BCMOLT_FIELD_SET(&val, classifier, ip_proto, val.ip_proto);
            printf(" val.ip_proto [%d]\r\n", val.ip_proto );
        }

        if (classifier == BCMOLT_CLASSIFIER_ID_SRC_PORT || c_val.src_port != 0) 
        {
            val.src_port = c_val.src_port; 
            BCMOLT_FIELD_SET(&val, classifier, src_port, val.src_port);
            printf(" val.src_port [%d]\r\n", val.src_port );	
        }

        if (classifier == BCMOLT_CLASSIFIER_ID_DST_PORT || c_val.dst_port != 0 ) 
        {
            val.dst_port = c_val.dst_port; 
            BCMOLT_FIELD_SET(&val, classifier, dst_port, val.dst_port);
            printf(" val.dst_port [%d]\r\n", val.dst_port );
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
            printf(" classifier.pkt_tag_type double_tag o_vid[%d] i_vid[%d]\r\n",val.o_vid, val.i_vid );
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
                printf(" SINGLE TAG EAP_ETHER_TYPE[0x%04X] sched_id[%d] queue_id[%d] \r\n",EAP_ETHER_TYPE, val.sched_id , val.queue_id);
            } 
            else 
            {
                val.sched_id =  get_default_tm_sched_id(access_intf_id, downstream);
                val.queue_id = 1; 
                printf(" NOT SINGLE TAG sched_id[%d] queue_id[%d] \r\n", val.sched_id , val.queue_id);		
            }

            BCMOLT_MSG_FIELD_SET(&cfg , egress_qos.type, BCMOLT_EGRESS_QOS_TYPE_FIXED_QUEUE);
            BCMOLT_MSG_FIELD_SET(&cfg , egress_qos.tm_sched.id, val.sched_id);
            BCMOLT_MSG_FIELD_SET(&cfg , egress_qos.u.fixed_queue.queue_id, val.queue_id);
        } 
        else if (key.flow_type == BCMOLT_FLOW_TYPE_UPSTREAM) 
        {
            val.sched_id = get_default_tm_sched_id(network_intf_id, upstream); 
            val.queue_id = 1; 
        }

        BCMOLT_MSG_FIELD_SET(&cfg , egress_qos.type, BCMOLT_EGRESS_QOS_TYPE_FIXED_QUEUE);
        BCMOLT_MSG_FIELD_SET(&cfg , egress_qos.tm_sched.id, val.sched_id);
        BCMOLT_MSG_FIELD_SET(&cfg , egress_qos.u.fixed_queue.queue_id, val.queue_id);
    }

    BCMOLT_MSG_FIELD_SET(&cfg, state, BCMOLT_FLOW_STATE_ENABLE);

    if (bcmolt_cfg_set(dev_id, &cfg.hdr)) 
    {
        printf("Flow add failed\n");
        return false; 
    }
    else
        printf("Flow add OK!!\n");

    return true;
}
}
