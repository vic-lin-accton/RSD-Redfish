#include "acc_bal_api_dist_helper/acc_bal_api_dist_helper.hpp"
#include <dlfcn.h>
#include <stdlib.h>

using namespace acc_bal_api_dist_helper;

#ifdef __cplusplus
extern "C"
{
#include <bcmos_system.h>
#include <bal_api.h>
#include <bal_api_end.h>
}
#endif

bcmos_errno (* d_bcmbal_stat_get)(bcmbal_access_term_id access_term_id, bcmbal_stat *objinfo, bcmos_bool clear_on_read);
bcmos_errno (* d_bcmbal_cfg_get)(bcmbal_access_term_id access_term_id, bcmbal_cfg *objinfo);
bcmos_errno (* d_bcmbal_subscribe_ind)(bcmbal_access_term_id access_term_id, bcmbal_cb_cfg *cb_cfg);
bcmos_errno (* d_bcmbal_apiend_init_all)(int argc, char *argv[], bcmbal_exit_cb exit_cb, bcmbal_bal_mode mode);

bcmos_errno Olt_itf_change(bcmbal_obj *obj) 
{
    bcmbal_interface_oper_status_change* bcm_if_oper_ind = (bcmbal_interface_oper_status_change *) obj;

    auto& rOLT = Olt_Device::Olt_Device::get_instance();

    if (bcm_if_oper_ind->data.new_oper_status == BCMBAL_STATUS_UP) 
    {
        if(bcm_if_oper_ind->key.intf_type == BCMBAL_INTF_TYPE_PON )
        {
            rOLT.set_pon_status(bcm_if_oper_ind->key.intf_id, true);
            if(bcm_if_oper_ind->key.intf_id == (MAX_PON_PORT_NUM-1))
                rOLT.get_pon_port_type();
        }
        else if (bcm_if_oper_ind->key.intf_type == BCMBAL_INTF_TYPE_NNI)
        {
            rOLT.set_nni_status(bcm_if_oper_ind->key.intf_id, true);
        }
        else
            printf("set pon unknown interface type !!!\r\n");

        printf("Olt_itf_change call back! BCMBAL_STATE_UP\r\n");
    }
    else 
    {
        if(bcm_if_oper_ind->key.intf_type == BCMBAL_INTF_TYPE_PON )
        {
            rOLT.set_pon_status(bcm_if_oper_ind->key.intf_id, false);
        }
        else if (bcm_if_oper_ind->key.intf_type == BCMBAL_INTF_TYPE_NNI)
        {
            rOLT.set_nni_status(bcm_if_oper_ind->key.intf_id, false);
        }
        else
            printf("set nni unknown interface type !!!\r\n");

        printf("Olt_itf_change call back! BCMBAL_STATE_DOWN\r\n");
    }

    printf("intf oper state indication, intf_type %d, intf_id %d, oper_state %d, admin_state %d\n",
            bcm_if_oper_ind->key.intf_type,
            bcm_if_oper_ind->key.intf_id,
            bcm_if_oper_ind->data.new_oper_status,
            bcm_if_oper_ind->data.admin_state);

    return BCM_ERR_OK;
}

bcmos_errno OltOperIndication(bcmbal_obj *obj) 
{
    printf("OltOperIndication call back!!!!!!!!\r\n");

    auto& rOLT = Olt_Device::Olt_Device::get_instance();

    bcmbal_access_terminal_oper_status_change *acc_term_ind = (bcmbal_access_terminal_oper_status_change *)obj;

    if (acc_term_ind->data.admin_state == BCMBAL_STATE_UP) 
    {
        rOLT.set_olt_state(true);
        printf("OltOperIndication call back! BCMBAL_STATE_UP\r\n");
    } 
    else 
    {
        rOLT.set_olt_state(false);
        printf("OltOperIndication call back! BCMBAL_STATE_DOWN\r\n");
    }

    if (acc_term_ind->data.new_oper_status == BCMBAL_STATUS_UP) 
    {
        rOLT.set_olt_status(true);
        rOLT.get_board_basic_info();
        printf("OltOperIndication call back! BCMBAL_STATUS_UP\r\n");
    } 
    else 
    {
        rOLT.set_olt_status(false);
        printf("OltOperIndication call back! BCMBAL_STATUS_DOWN\r\n");
    }
    return BCM_ERR_OK;
}

namespace acc_bal_api_dist_helper
{
    static Olt_Device * g_Olt_Device = NULL;

    void Olt_Device::set_olt_state(bool state)
    {
        m_bal_state = state;
    }

    void Olt_Device::set_olt_status(bool status)
    {
        m_bal_status = status;
    }

    void Olt_Device::set_pon_status(int port,int status)
    {
        m_pon_port[port].set_status(status);
        return;
    }

    void Olt_Device::set_nni_status(int port,int status)
    {
        m_nni_port[port].set_status(status);
        return;
    }

    void Olt_Device::set_intf_type(int port,int type)
    {
    }

    int Olt_Device::get_board_basic_info()
    {
        bcmbal_access_terminal_cfg acc_term_obj;
        bcmbal_access_terminal_key key = {0};

        key.access_term_id = DEFAULT_ATERM_ID;
        BCMBAL_CFG_INIT(&acc_term_obj, access_terminal, key);
        BCMBAL_CFG_PROP_GET(&acc_term_obj, access_terminal, admin_state);
        BCMBAL_CFG_PROP_GET(&acc_term_obj, access_terminal, oper_status);
        BCMBAL_CFG_PROP_GET(&acc_term_obj, access_terminal, topology);
        BCMBAL_CFG_PROP_GET(&acc_term_obj, access_terminal, sw_version);
        BCMBAL_CFG_PROP_GET(&acc_term_obj, access_terminal, conn_id);

        bcmos_errno err = BCM_ERR_INTERNAL;  

        if(d_bcmbal_cfg_get)
            err    = d_bcmbal_cfg_get(DEFAULT_ATERM_ID, &(acc_term_obj.hdr));

        if (err) 
        {
            printf("Failed to query get_board_basic_info() \n");
            return 0;
        }
        else
        {
            printf("OLT  admin_state:[%s] oper_state:[%s]\n",
                    acc_term_obj.data.admin_state == BCMBAL_STATE_UP ? "up" : "down",
                    acc_term_obj.data.oper_status == BCMBAL_STATUS_UP ? "up" : "down");

            m_nni_ports_num = acc_term_obj.data.topology.num_of_nni_ports;
            m_pon_ports_num = acc_term_obj.data.topology.num_of_pon_ports;
            m_mac_devs_num = acc_term_obj.data.topology.num_of_mac_devs; 
            m_olt_state   = acc_term_obj.data.admin_state == BCMBAL_STATE_UP?ACC_BCMBAL_STATE_UP:ACC_BCMBAL_STATE_DOWN;
            m_olt_status  = acc_term_obj.data.oper_status == BCMBAL_STATUS_UP?ACC_BCMBAL_STATUS_UP:ACC_BCMBAL_STATUS_DOWN;

            switch(acc_term_obj.data.topology.pon_sub_family)
            {
                case BCMBAL_PON_SUB_FAMILY_GPON:  
                    m_pon_type = "gpon"; 
                    break;
                case BCMBAL_PON_SUB_FAMILY_XGS:   
                    m_pon_type = "xgspon"; 
                    break;

                case BCMBAL_PON_SUB_FAMILY_XGPON:   
                    m_pon_type = "xgpon"; 
                    break;
                default:   
                    m_pon_type = "unknown"; 
                    break;
            }

            m_bal_version += std::to_string(acc_term_obj.data.sw_version.major_rev) + "." + std::to_string(acc_term_obj.data.sw_version.minor_rev) + "." + std::to_string(acc_term_obj.data.sw_version.release_rev);
            m_firmware_version = "BAL." + m_bal_version; 

            printf("OLT info., [nni ports: %d] [pon ports: %d] [pon type %s]\n", m_nni_ports_num , m_pon_ports_num, m_pon_type.c_str());
            printf("OLT info., [firware_ver:%s]\n", m_firmware_version.c_str());
        }
        return 0;
    }

    bool Olt_Device::connect_bal(int argc, char *argv[]) 
    {
        if(d_bcmbal_apiend_init_all)
        {
            if(BCM_ERR_OK == d_bcmbal_apiend_init_all(argc, argv, NULL, BAL_MODE_DIST_API))
        {
            m_bcmbal_init = true;
            return m_bcmbal_init;
        }
        else
        {
            m_bcmbal_init = false;
            return m_bcmbal_init;
        }
    }
    }

    Olt_Device& Olt_Device::get_instance()
    {
        static  char * ARGV[3] = 
        {
            "olt_object",
            "-C",
            "127.0.0.1:55001"
        };

        if (NULL == g_Olt_Device) 
        {
            printf("Creating Olt_Device \r\n");
            g_Olt_Device = new Olt_Device(sizeof(ARGV)/sizeof(char *),(char **) ARGV);
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

        fHandle = dlopen("/usr/local/lib/libbal_api_dist.so",RTLD_LAZY);

        if(fHandle)
        {
            printf("Using dynamic loading function\r\n");
            d_bcmbal_stat_get= (bcmos_errno(*)(bcmbal_access_term_id access_term_id, bcmbal_stat *objinfo, bcmos_bool clear_on_read)) dlsym(fHandle,"bcmbal_stat_get");
            d_bcmbal_cfg_get = (bcmos_errno (*)(bcmbal_access_term_id access_term_id, bcmbal_cfg *objinfo)) dlsym(fHandle,"bcmbal_cfg_get");

            d_bcmbal_subscribe_ind = (bcmos_errno (*)(bcmbal_access_term_id access_term_id, bcmbal_cb_cfg *cb_cfg)) dlsym(fHandle,"bcmbal_subscribe_ind");

            d_bcmbal_apiend_init_all = (bcmos_errno (*)(int argc, char *argv[], bcmbal_exit_cb exit_cb, bcmbal_bal_mode mode)) dlsym(fHandle,"bcmbal_apiend_init_all");
            m_bal_lib_init = true;
        }
        else
        {
            m_bal_lib_init = false;
            printf("Cannot find libbal_api_dist.so and using loading function\r\n");
        }

        if(connect_bal(argc, argv))
        register_callback();
    }

    void Olt_Device::register_callback()
    {
        bcmbal_cb_cfg cb_cfg = {};
        uint16_t subgroup;

        if (m_subscribed) 
        {
            return ;
        }

        cb_cfg.obj_type = BCMBAL_OBJ_ID_ACCESS_TERMINAL;
        subgroup = bcmbal_access_terminal_auto_id_oper_status_change;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltOperIndication;

        if(d_bcmbal_subscribe_ind)
        {
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
        {
            printf("Register_callback BCMBAL_OBJ_ID_ACCESS_TERMINAL error!!!\r\n");
            return; 
        }
        else
            printf("Register_callback BCMBAL_OBJ_ID_ACCESS_TERMINAL ok!!!\r\n");
        }

        cb_cfg.obj_type = BCMBAL_OBJ_ID_INTERFACE;
        subgroup = bcmbal_interface_auto_id_oper_status_change;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)Olt_itf_change;

        if(d_bcmbal_subscribe_ind)
        {
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
        {
            printf("Register_callback BCMBAL_OBJ_ID_INTERFACE error!!!\r\n");
            return;
        }
        else
            printf("Register_callback BCMBAL_OBJ_ID_INTERFACE ok!!!\r\n");
        }

        m_subscribed = true;
        return ; 
    }

    void Olt_Device::get_pon_port_type()
    {
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
    }

    json::Value Olt_Device::get_port_statistic(int port)
    {
        json::Value status(json::Value::Type::OBJECT);

        if( ((port -1) >= 0) && ((port - 1) < TOTAL_INTF_NUM) )
        {
            if((port-1) < MAX_PON_PORT_NUM)
                return get_pon_statistic( port -1);
            else
                return get_nni_statistic( port - MAX_PON_PORT_NUM -1);
        }
            return status;
        }

    json::Value Olt_Device::get_pon_statistic(int port)
    {
        json::Value status(json::Value::Type::OBJECT);

        bcmos_errno err;
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

    json::Value Olt_Device::get_nni_statistic(int port)
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
}
