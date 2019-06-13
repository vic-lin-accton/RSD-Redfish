#include "../include/acc_bal30_api_dist_helper/acc_bal30_api_dist_helper.hpp"
#include <dlfcn.h>
#include <stdlib.h>

using namespace acc_bal30_api_dist_helper;

#ifdef __cplusplus
extern "C"
{

#include <bcmolt_api.h>
#include <bcmolt_host_api.h>
#include <bcmolt_api_model_supporting_enums.h>
#include <bal_version.h>
#include <bcmolt_api_conn_mgr.h>
#include <bcmcli_session.h>
#include <bcmcli.h>
#include <bcm_api_cli.h>
#include <bcmos_common.h>
#include <bcm_config.h>

}
#endif

bcmolt_oltid dev_id = 0;


const uint32_t tm_upstream_sched_id_start = 1020;
const uint32_t tm_downstream_sched_id_start = 1004;
const std::string upstream = "upstream";
const std::string downstream = "downstream";

//0 to 3 are default queues. Lets not use them.
const uint32_t tm_queue_id_start = 4;
// Upto 8 fixed Upstream. Queue id 0 to 3 are pre-created, lets not use them.
const uint32_t us_fixed_queue_id_list[8] = {4, 5, 6, 7, 8, 9, 10, 11};

#define CLI_HOST_PROMPT_FORMAT "BCM.%u> "
#define SERIAL_NUMBER_SIZE 12

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
bcmos_bool status_bcm_cli_quit = BCMOS_FALSE;
bcmos_task bal_cli_thread;
const char *bal_cli_thread_name = "acc_help_bal_cli_thread";

//FIXME
#define FLOWS_COUNT 100

bcmolt_flow_key* flows_keys = new bcmolt_flow_key[FLOWS_COUNT];
bcmolt_odid device_id = 0;

void init_stats() 
{
    memset(flows_keys, 0, FLOWS_COUNT * sizeof(bcmolt_flow_key));
}


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

static void Olt_itf_change(short unsigned int olt , bcmolt_msg *msg)
{
    printf("Olt_itf_change PON/NNI inf call back!!!!!!!!\r\n");

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

static void OltOmciIndication(bcmolt_devid olt, bcmolt_msg *msg)
{
    printf("OltOmciIndication call back!!!!!!!!\r\n");
    printf("NICK DEBUG : ONT ");

    switch (msg->obj_type) 
    {
        case BCMOLT_OBJ_ID_ONU:
            switch (msg->subgroup) 
            {
                bcmolt_onu_key *key = &((bcmolt_onu_omci_packet*)msg)->key;
                bcmolt_onu_omci_packet_data *data = &((bcmolt_onu_omci_packet*)msg)->data;

                printf("OMCI indication: pon_ni %d, onu_id %d\n", key->pon_ni, key->onu_id);

                int count =0;
                for(count = 1 ; count <= data->packet_size ; count++)
                {
                    printf("%02X", data->buffer.arr[count-1]);
                }
                printf("\r\n");
            }
    }
    bcmolt_msg_free(msg);
    return;
}

static void OltOperIndication (short unsigned int olt, bcmolt_msg *msg)
{
    printf("OltOperIndication call back!!!!!!!!\r\n");

    auto& rOLT = Olt_Device::Olt_Device::get_instance();

    if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_COMPLETE) 
    {
        rOLT.set_olt_status(true);
/*
        if(!rOLT.get_board_basic_info())
        {
            printf("NICK DEBUG : get_board_basic_info ERROR!!\r\n");
            return ;
        }
        bcmolt_rx_cfg cb_cfg = {};
        cb_cfg.obj_type = BCMOLT_OBJ_ID_ONU;
        cb_cfg.rx_cb = OltOmciIndication;
        cb_cfg.subgroup = bcmolt_onu_auto_subgroup_omci_packet;
        cb_cfg.module = BCMOS_MODULE_ID_OMCI_TRANSPORT;

        if(bcmolt_ind_subscribe(dev_id, &cb_cfg) != BCM_ERR_OK)
        {
            printf("Register_callback BCMOLT_OBJ_ID_ONU OmciIndication complete error!!!\r\n");
            bcmolt_msg_free(msg);
            return;
        }
        else
        {
            printf("Register_callback OmciIndication ok!!!\r\n");
        }
*/
    } 
    else if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_DISCONNECTION_COMPLETE) 
    {
        rOLT.set_olt_status(false);
        printf("NICK DEBUG : OltOperIndication call back! BCMOLT_DEVICE_AUTO_SUBGROUP_DISCONNECTION_COMPLETE\r\n");
    }
    else if (msg->subgroup == BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_FAILURE) 
    {
        rOLT.set_olt_status(false);
        printf("NICK DEBUG : OltOperIndication call back! BCMOLT_DEVICE_AUTO_SUBGROUP_CONNECTION_FAILURE\r\n");
    }
    else 
    {
        rOLT.set_olt_status(false);
        printf("NICK DEBUG : OltOperIndication no match case !!!!");
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
    printf("NICK DEBUG : OltLosIndication call back!!!!!!!!\r\n");

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
    printf("OnuActivationFailureIndication call back!!!!!!!!\r\n");

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

std::string vendor_specific_to_str(char const * const vendor_specific) {
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
    // printf("NICK DEBUG : OltOnuDiscoveryIndication call back!!!!!!!!\r\n");
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
    printf("NICK DEBUG : OltOnuOperIndication call back!!!!!!!!\r\n");
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

namespace acc_bal30_api_dist_helper
{
    static Olt_Device * g_Olt_Device = NULL;

    void Olt_Device::set_olt_status(bool status)
    {
        m_bal_status = status;
    }

    bool Olt_Device::get_olt_status()
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
                case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6862_X_: m_chip_family = "Maple"; break;
                case BCMOLT_CHIP_FAMILY_CHIP_FAMILY_6865_X_: m_chip_family = "Aspen"; break;
            }

            printf("topology nni:%d pon:%d dev:%d ppd:%d family: %s\n",
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

    bool Olt_Device::connect_bal(int argc, char *argv[]) 
    {

        bcmos_errno err;
        bcmolt_host_init_parms init_parms = {};
        init_parms.transport.type = BCM_HOST_API_CONN_LOCAL;
        bcmcli_session_parm mon_session_parm;

        if (BCM_ERR_OK != bcmolt_host_init(&init_parms)) 
        {
            printf("Failed to init bcmolt_host\r\n");
            m_bcmbal_init = false;
            return m_bcmbal_init;
        }
        else
        {
            /* Create CLI session */
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

            if(bcmolt_api_conn_mgr_is_connected(dev_id))
            {

                bcmos_errno err;
                bcmolt_odid dev;

                for (dev = 0; dev < BCM_MAX_DEVS_PER_LINE_CARD; dev++) 
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
                        printf("Enable Maple - %d/%d\n", dev + 1, BCM_MAX_DEVS_PER_LINE_CARD);
                        BCMOLT_OPER_INIT(&oper, device, connect, key);
                        BCMOLT_MSG_FIELD_SET(&oper, inni_config.mode, BCMOLT_INNI_MODE_ALL_10_G_XFI);
                        BCMOLT_MSG_FIELD_SET (&oper, system_mode, BCMOLT_SYSTEM_MODE_XGS__2_X);
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
                init_stats();

                register_callback();

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
            printf("Using dynamic loading function\r\n");
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
            printf("Register_callback BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL bcmbal_subscriber_terminal_auto_id_oper_status_change ok!!!\r\n");			 

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
            printf("Register_callback BCMOLT_OBJ_ID_ONU bcmolt_onu_auto_subgroup_onu_deactivation_completed ok!!!\r\n");

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


    bool CreateDefaultSchedQueue_(uint32_t intf_id, const std::string direction) 
    {
        bcmos_errno err;
        bcmolt_tm_sched_cfg tm_sched_cfg;
        bcmolt_tm_sched_key tm_sched_key = {.id = 1};
        tm_sched_key.id = get_default_tm_sched_id(intf_id, direction);

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

        BCMOLT_MSG_FIELD_SET(&tm_sched_cfg, num_priorities, 4);

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
            printf("Failed to create %s scheduler, id %d, intf_id %d\n", direction.c_str(), tm_sched_key.id, intf_id);
            return false; 
        }

        printf("Create %s scheduler success, id %d, intf_id %d\n", direction.c_str(), tm_sched_key.id, intf_id);

        for (int queue_id = 0; queue_id < 4; queue_id++) 
        {
            bcmolt_tm_queue_cfg tm_queue_cfg;
            bcmolt_tm_queue_key tm_queue_key = {};
            tm_queue_key.sched_id = get_default_tm_sched_id(intf_id, direction);
            tm_queue_key.id = queue_id;

            BCMOLT_CFG_INIT(&tm_queue_cfg, tm_queue, tm_queue_key);
            BCMOLT_MSG_FIELD_SET(&tm_queue_cfg, tm_sched_param.type, BCMOLT_TM_SCHED_PARAM_TYPE_PRIORITY);
            BCMOLT_MSG_FIELD_SET(&tm_queue_cfg, tm_sched_param.u.priority.priority, queue_id);

            err = bcmolt_cfg_set(dev_id, &tm_queue_cfg.hdr);
            if (err) 
            {
                printf("Failed to create %s tm queue, id %d, sched_id %d\n", direction.c_str(), tm_queue_key.id, tm_queue_key.sched_id);
                return false;
            }

            printf("Create %s tm_queue success, id %d, sched_id %d\n", direction.c_str(), tm_queue_key.id, tm_queue_key.sched_id);
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

    bool  Olt_Device::enable_pon_if_(int intf_id) 
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
            printf("Initializing tm sched creation for PON interface: %d\n", intf_id);
            //CreateDefaultSchedQueue_(intf_id, downstream);
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


    bool Olt_Device::activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific) 
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

        printf("Enabling ONU %d on PON %d : vendor id %s,  vendor specific %s\r\n", onu_id, intf_id, vendor_id, vendor_specific_to_str(vendor_specific).c_str());
        memcpy(serial_number.vendor_id.arr, vendor_id, 4);
        memcpy(serial_number.vendor_specific.arr, vendor_specific, 4);
        BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, onu_rate, BCMOLT_ONU_RATE_RATE_10G_DS_10G_US);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.serial_number, serial_number);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.auto_learning, BCMOS_TRUE);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.xgpon.ranging_burst_profile, 0);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.xgpon.data_burst_profile, 1);
        err = bcmolt_cfg_set(dev_id, &onu_cfg.hdr);

        if (err != BCM_ERR_OK) 
        {
            printf("Failed to set activate ONU %d on PON %d, err %d\n", onu_id, intf_id, err);
            return false;
        }

        //sched_add(intf_id, onu_id, (1023+onu_id) );	
        return true;
    }

    bool Olt_Device::sched_add(int intf_id, int onu_id, int agg_port_id) 
    {
        /*
           bcmbal_tm_sched_cfg cfg;
           bcmbal_tm_sched_key key = { };
           bcmbal_tm_sched_type sched_type;


           key.id =  (1023 + onu_id);
           key.dir = BCMBAL_TM_SCHED_DIR_US;

           printf("SchedAdd_ intf_id[%d] onu_id[%d] agg_port_id[%d]\r\n", intf_id, onu_id , agg_port_id );

           BCMBAL_CFG_INIT(&cfg, tm_sched, key);
           {
           bcmbal_tm_sched_owner val = { };			
           val.type = BCMBAL_TM_SCHED_OWNER_TYPE_AGG_PORT;

           val.u.agg_port.intf_id = (bcmbal_intf_id) intf_id;
           val.u.agg_port.presence_mask = val.u.agg_port.presence_mask | BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_INTF_ID;

           val.u.agg_port.sub_term_id = (bcmbal_sub_id) onu_id;
           val.u.agg_port.presence_mask = val.u.agg_port.presence_mask | BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_SUB_TERM_ID;

           val.u.agg_port.agg_port_id = (bcmbal_aggregation_port_id) agg_port_id;
           val.u.agg_port.presence_mask = val.u.agg_port.presence_mask | BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_AGG_PORT_ID;
           BCMBAL_CFG_PROP_SET(&cfg, tm_sched, owner, val);

           }

           if(d_bcmbal_cfg_set)			
           { 
           if (d_bcmbal_cfg_set(DEFAULT_ATERM_ID, &(cfg.hdr))) 
           {
           printf("ERROR: Failed to create upstream DBA sched id:[%d] intf_id[%d] onu_id[%d]\r\n", key.id,intf_id, onu_id);
           return false;
           }
           }
           printf("Create upstream DBA sched id:[%d] intf_id[%d] onu_id[%d] OK!!!\r\n", key.id,intf_id, onu_id);
           */
        return true;
    }

#define MAX_CHAR_LEN  20
#define MAX_OMCI_MSG_LEN 44
    bool Olt_Device::omci_msg_out(int intf_id, int onu_id, const std::string pkt) 
    {
        bcmolt_bin_str buf; /* A structure with a msg pointer and length value */
        bcmos_errno err = BCM_ERR_INTERNAL;

        /*
        bcmbal_dest proxy_pkt_dest;
        proxy_pkt_dest.type = BCMBAL_DEST_TYPE_ITU_OMCI_CHANNEL;
        proxy_pkt_dest.u.itu_omci_channel.sub_term_id = onu_id;
        proxy_pkt_dest.u.itu_omci_channel.intf_id = intf_id;
        */

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

        printf("Sending omci msg to ONU of length is %d\r\n", buf.len);

        for (idx1=0,idx2=0; idx1<((buf.len)*2); idx1++,idx2++) 
        {
            sprintf(str1,"%c", pkt[idx1]);
            sprintf(str2,"%c", pkt[++idx1]);
            strcat(str1,str2);
            arraySend[idx2] = strtol(str1, NULL, 16);
        }

        buf.arr = (uint8_t *)malloc((buf.len)*sizeof(uint8_t));
        memcpy(buf.arr, (uint8_t *)arraySend, buf.len);

        /*
        if(d_bcmbal_pkt_send)
        {
            err = d_bcmbal_pkt_send(0, proxy_pkt_dest, (const char *)(buf.arr), buf.len);

            if(err != BCM_ERR_OK)
                printf("ERROR: Failed to sent omci to ONU [%d] through PON intf [%d]\r\n", onu_id, intf_id);
            else
                printf("OMCI request msg of length [%d] sent to ONU [%d] through PON intf [%d] OK !!\r\n", buf.len, onu_id, intf_id);
        }
        */
        //decode_OMCI_hex_packet(buf.arr);

        printf("NICK DEBUG : omci raw data: ");
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

    bool Olt_Device::flow_add(int onu_id, int flow_id, const std::string flow_type, const std::string pkt_tag_type, int access_intf_id,
            int network_intf_id, int gemport_id, int classifier, 
            int action ,int action_cmd, struct action_val a_val, struct class_val c_val)
    {
        /*
        bcmbal_flow_cfg cfg;
        bcmbal_flow_key key = { };

        printf("NICK DEBUG : FlowAdd_ intf_id[%d] onu_id[%d] flow_id[%d] flow_type[%s] pkt_tag_type[%s] gemport_id[%d] network_intf_id[%d] classifier[0x%04X] action[0x%04X]\r\n", access_intf_id, onu_id , flow_id, flow_type.c_str(), pkt_tag_type.c_str() , gemport_id,network_intf_id, classifier, action);

        key.flow_id = flow_id;
        if (flow_type.compare("upstream") == 0 ) 
        {
            key.flow_type = BCMBAL_FLOW_TYPE_UPSTREAM;
        } 
        else if (flow_type.compare("downstream") == 0) 
        {
            key.flow_type = BCMBAL_FLOW_TYPE_DOWNSTREAM;
        } 
        else 
        {
            printf("ERROR: Invalid flow type !!!\r\n"); 
            return false;
        }

        BCMBAL_CFG_INIT(&cfg, flow, key);

        BCMBAL_CFG_PROP_SET(&cfg, flow, admin_state, BCMBAL_STATE_UP);
        BCMBAL_CFG_PROP_SET(&cfg, flow, access_int_id, access_intf_id);
        BCMBAL_CFG_PROP_SET(&cfg, flow, network_int_id, network_intf_id);
        BCMBAL_CFG_PROP_SET(&cfg, flow, sub_term_id, onu_id);
        BCMBAL_CFG_PROP_SET(&cfg, flow, svc_port_id, gemport_id);


        {
            bcmbal_classifier val = {};

            if (classifier == BCMBAL_CLASSIFIER_ID_O_TPID || c_val.o_tpid != 0) 
            {
                val.o_tpid = c_val.o_tpid;
                val.presence_mask =  val.presence_mask | BCMBAL_CLASSIFIER_ID_O_TPID;
                printf("NICK DEBUG : class val.o_tpid[%d]\r\n", val.o_tpid);	
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);			
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_O_VID || c_val.o_vid !=0) 
            {
                val.o_vid = c_val.o_vid;
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_O_VID;
                printf("NICK DEBUG : class val.o_vid[%d]\r\n", val.o_vid);
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_I_TPID || c_val.i_tpid !=0 ) 
            {
                val.i_tpid = c_val.i_tpid;
                val.presence_mask =  val.presence_mask | BCMBAL_CLASSIFIER_ID_I_TPID;
                printf("NICK DEBUG :class  val.i_tpid[%d]\r\n", val.i_tpid);
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);							
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_I_VID || c_val.i_vid != 0) 
            {
                val.i_vid = c_val.i_vid;
                val.presence_mask =   val.presence_mask |BCMBAL_CLASSIFIER_ID_I_VID;
                printf("NICK DEBUG : val.i_vid[%d]\r\n", val.i_vid);	
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_O_PBITS || c_val.o_pbits !=0 ) 
            {
                val.o_pbits = c_val.o_pbits; 
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_O_PBITS;
                printf("NICK DEBUG : val.o_pbits[%d]\r\n", val.o_pbits);
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);									
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_I_PBITS  || c_val.i_pbits != 0) 
            {
                val.i_pbits = c_val.i_pbits;
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_I_PBITS;
                printf("NICK DEBUG : val.i_pbits[%d]\r\n", val.i_pbits);
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_ETHER_TYPE || c_val.ether_type !=0) 
            {
                val.ether_type = c_val.ether_type;
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_ETHER_TYPE;
                printf("NICK DEBUG : val.ether_type [%d]\r\n", val.ether_type );	
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_IP_PROTO || c_val.ip_proto !=0) 
            {
                val.ip_proto = c_val.ip_proto; 
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_IP_PROTO;
                printf("NICK DEBUG : val.ip_proto [%d]\r\n", val.ip_proto );
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_SRC_PORT || c_val.src_port != 0) 
            {
                val.src_port = c_val.src_port; 
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_SRC_PORT;
                printf("NICK DEBUG : val.src_port [%d]\r\n", val.src_port );	
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (classifier == BCMBAL_CLASSIFIER_ID_DST_PORT || c_val.dst_port != 0 ) 
            {
                val.dst_port = c_val.dst_port; 
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_DST_PORT;
                printf("NICK DEBUG : val.dst_port [%d]\r\n", val.dst_port );
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            }

            if (pkt_tag_type.compare("untagged") == 0) 
            {
                val.pkt_tag_type  = BCMBAL_PKT_TAG_TYPE_UNTAGGED;
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_PKT_TAG_TYPE;
                printf("NICK DEBUG : classifier.pkt_tag_type untagged\r\n");	
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            } 
            else if (pkt_tag_type.compare("single_tag") == 0) 
            {
                val.pkt_tag_type  = BCMBAL_PKT_TAG_TYPE_SINGLE_TAG;
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_PKT_TAG_TYPE;
                printf("NICK DEBUG : classifier.pkt_tag_type single_tag\r\n");
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);						
            } 
            else if (pkt_tag_type.compare("double_tag") == 0) 
            {
                val.pkt_tag_type  = BCMBAL_PKT_TAG_TYPE_DOUBLE_TAG;
                val.presence_mask =  val.presence_mask |BCMBAL_CLASSIFIER_ID_PKT_TAG_TYPE;
                printf("NICK DEBUG : classifier.pkt_tag_type double_tag\r\n");
                BCMBAL_CFG_PROP_SET(&cfg, flow, classifier, val);				  
            }

        }

        {
            bcmbal_action val = { 0 };

            if (action_cmd == BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG) 
            {
                val.cmds_bitmask  |= BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG;
                val.presence_mask |= BCMBAL_ACTION_ID_CMDS_BITMASK;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);
                printf("NICK DEBUG : cmd.add_outer_tag BCMBAL_ACTION_CMD_ID_ADD_OUTER_TAG\r\n");	

            }

            if (action_cmd == BCMBAL_ACTION_CMD_ID_REMOVE_OUTER_TAG) 
            {
                val.cmds_bitmask  |= BCMBAL_ACTION_CMD_ID_REMOVE_OUTER_TAG;
                val.presence_mask |= BCMBAL_ACTION_ID_CMDS_BITMASK;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : cmd.remove_outer_tag BCMBAL_ACTION_CMD_ID_REMOVE_OUTER_TAG\r\n");				
            }

            if (action_cmd == BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST) 
            {
                val.cmds_bitmask  |= BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST;
                val.presence_mask |= BCMBAL_ACTION_ID_CMDS_BITMASK;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : cmd.trap_to_host BCMBAL_ACTION_CMD_ID_TRAP_TO_HOST\r\n");				
            }

            if (action == BCMBAL_ACTION_ID_O_VID || a_val.o_vid != 0) 
            {
                val.o_vid = a_val.o_vid;
                val.presence_mask = val.presence_mask |BCMBAL_ACTION_ID_O_VID;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : action.o_vid [%d] BCMBAL_ACTION_ID_O_VID\r\n", val.o_vid);		
            }

            if (action == BCMBAL_ACTION_ID_O_PBITS ||a_val.o_pbits !=0 ) 
            {
                val.o_pbits = a_val.o_pbits;
                val.presence_mask = val.presence_mask |BCMBAL_ACTION_ID_O_PBITS;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : action.o_pbits [%d] BCMBAL_ACTION_ID_O_PBITS\r\n", val.o_pbits);				
            }

            if (action == BCMBAL_ACTION_ID_O_TPID != a_val.o_tpid ) 
            {
                val.o_tpid = a_val.o_tpid;
                val.presence_mask = val.presence_mask |BCMBAL_ACTION_ID_O_TPID;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);
                printf("NICK DEBUG : action.o_tpid [%d] BCMBAL_ACTION_ID_O_TPID\r\n", val.o_tpid);				
            }

            if (action == BCMBAL_ACTION_ID_I_VID || a_val.i_vid !=0 ) 
            {
                val.i_vid = a_val.i_vid;
                val.presence_mask = val.presence_mask |BCMBAL_ACTION_ID_I_VID;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : action.i_vid [%d] BCMBAL_ACTION_ID_I_VID\r\n", val.i_vid);				
            }

            if (action == BCMBAL_ACTION_ID_I_PBITS || a_val.i_pbits != 0) 
            {
                val.i_pbits = a_val.i_pbits;
                val.presence_mask = val.presence_mask |BCMBAL_ACTION_ID_I_PBITS;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : action.i_pbits [%d] BCMBAL_ACTION_ID_I_PBITS\r\n", val.i_pbits);				
            }

            if (action == BCMBAL_ACTION_ID_I_TPID || a_val.i_tpid !=0 ) 
            {
                val.i_tpid = a_val.i_tpid;
                val.presence_mask = val.presence_mask |BCMBAL_ACTION_ID_I_TPID;
                BCMBAL_CFG_PROP_SET(&cfg, flow, action, val);				
                printf("NICK DEBUG : action.i_tpid [%d] BCMBAL_ACTION_ID_I_TPID\r\n", val.i_tpid);				
            }
        }

        {
            bcmbal_tm_sched_id val;
            val = (bcmbal_tm_sched_id) (1023 + onu_id) ;
            BCMBAL_CFG_PROP_SET(&cfg, flow, dba_tm_sched_id, val);
        }

        if(d_bcmbal_cfg_set)			
        {
            if (d_bcmbal_cfg_set(DEFAULT_ATERM_ID, &(cfg.hdr))) 
            {
                printf("ERROR: flow add failed !!!\r\n");
                return false; 
            }
            else
                printf("NICK DEBUG : FlowAdd_ intf_id[%d] onu_id[%d] flow_id[%d] flow_type[%s] pkt_tag_type[%s] gemport_id[%d] network_intf_id[%d] classifier[0x%04X] action[0x%04X] OK\r\n", access_intf_id, onu_id , flow_id, flow_type.c_str(), pkt_tag_type.c_str() , gemport_id,network_intf_id, classifier, action);

        }

        */
        return true;
    }
}
