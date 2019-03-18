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
bcmos_errno (* d_bcmbal_cfg_set) (bcmbal_access_term_id access_term_id, bcmbal_cfg *objinfo);
bcmos_errno (* d_bcmbal_pkt_send)(bcmbal_access_term_id access_term_id, bcmbal_dest dest, const char *packet_to_send, uint16_t packet_len);

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
            if(bcm_if_oper_ind->key.intf_id == (rOLT.get_max_pon_num()-1))
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

    printf("NICK DEBUG :  intf oper state indication, intf_type %d, intf_id %d, oper_state %d, admin_state %d\n",
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
        printf("NICK DEBUG : OltOperIndication call back! BCMBAL_STATE_UP\r\n");
    } 
    else 
    {
        rOLT.set_olt_state(false);
        printf("NICK DEBUG : OltOperIndication call back! BCMBAL_STATE_DOWN\r\n");
    }

    if (acc_term_ind->data.new_oper_status == BCMBAL_STATUS_UP) 
    {
        rOLT.set_olt_status(true);
        rOLT.get_board_basic_info();
        printf("NICK DEBUG : OltOperIndication call back! BCMBAL_STATUS_UP\r\n");
    } 
    else 
    {
        rOLT.set_olt_status(false);
        printf("NICK DEBUG : OltOperIndication call back! BCMBAL_STATUS_DOWN\r\n");
    }

    return BCM_ERR_OK;
}


bcmos_errno  OltOnuActivationFailureIndication(bcmbal_obj *obj) 
{
    printf("OnuActivationFailureIndication call back!!!!!!!!\r\n");

    bcmbal_subscriber_terminal_key *key =
        &(((bcmbal_subscriber_terminal_sub_term_act_fail*)obj)->key);

    printf("NICK DEBUG : onu activation failure indication, intf_id:%d onu_id %d" ,key->intf_id ,key->sub_term_id);

    return BCM_ERR_OK;
}

int interface_key_to_port_no(bcmbal_interface_key key)
{
    if (key.intf_type == BCMBAL_INTF_TYPE_NNI) 
    {
        return 128 + key.intf_id;
    }

    if (key.intf_type == BCMBAL_INTF_TYPE_PON) 
    {
        return (0x2 << 28) + 1;
    }

    return key.intf_id;
}


const char* serial_number_to_str(bcmbal_serial_number* serial_number) 
{
#define SERIAL_NUMBER_SIZE 12
    static char buff[SERIAL_NUMBER_SIZE+1];

    sprintf(buff, "%c%c%c%c%1X%1X%1X%1X%1X%1X%1X%1X",
            serial_number->vendor_id[0],
            serial_number->vendor_id[1],
            serial_number->vendor_id[2],
            serial_number->vendor_id[3],
            serial_number->vendor_specific[0]>>4 & 0x0f,
            serial_number->vendor_specific[0] & 0x0f,
            serial_number->vendor_specific[1]>>4 & 0x0f,
            serial_number->vendor_specific[1] & 0x0f,
            serial_number->vendor_specific[2]>>4 & 0x0f,
            serial_number->vendor_specific[2] & 0x0f,
            serial_number->vendor_specific[3]>>4 & 0x0f,
            serial_number->vendor_specific[3] & 0x0f);

    return buff;
}

bcmos_errno OltLosIndication(bcmbal_obj *obj) 
{
    printf("NICK DEBUG : OltLosIndication call back!!!!!!!!\r\n");

    bcmbal_interface_los* bcm_los_ind = (bcmbal_interface_los *) obj;
    int intf_id = interface_key_to_port_no(bcm_los_ind->key);
    printf("LOS indication : [%d] \r\n",intf_id );
    return BCM_ERR_OK;
}

bcmos_errno OltOnuDiscoveryIndication(bcmbal_obj *obj) 
{
    printf("NICK DEBUG : OltOnuDiscoveryIndication call back!!!!!!!!\r\n");

    bcmbal_subscriber_terminal_key  *key  =  &(((bcmbal_subscriber_terminal_sub_term_disc*)obj)->key);
    bcmbal_subscriber_terminal_sub_term_disc_data *data = &(((bcmbal_subscriber_terminal_sub_term_disc*)obj)->data);
    bcmbal_serial_number *in_serial_number = &(data->serial_number);	

    printf("NICK DEBUG : onu discover indication, intf_id: [%d] serial_number: [%s] \r\n", 
    key->intf_id, serial_number_to_str(in_serial_number));

    return BCM_ERR_OK;
}


bcmos_errno OltOnuOperIndication(bcmbal_obj *obj) 
{
    printf("NICK DEBUG : OltOnuOperIndication call back!!!!!!!!\r\n");

    bcmbal_subscriber_terminal_key *key =
        &(((bcmbal_subscriber_terminal_oper_status_change*)obj)->key);

    bcmbal_subscriber_terminal_oper_status_change_data *data =
        &(((bcmbal_subscriber_terminal_oper_status_change*)obj)->data);

    printf("NICK DEBUG : onu oper state indication, intf_id:[%d]  onu_id:[%d]  old oper state: [%d] new oper state:[%d]\r\n" ,
    key->intf_id , key->sub_term_id, data->old_oper_status, data->new_oper_status);

    return BCM_ERR_OK;
}


bcmos_errno OltOnuIndication(bcmbal_obj *obj) 
{
    printf("NICK DEBUG : OltOnuIndication call back!!!!!!!!\r\n");

    bcmbal_subscriber_terminal_key *key =
        &(((bcmbal_subscriber_terminal_oper_status_change*)obj)->key);

    bcmbal_subscriber_terminal_oper_status_change_data *data =
        &(((bcmbal_subscriber_terminal_oper_status_change*)obj)->data);

    printf("NICK DEBUG : onu indication, intf_id: [%d] oper_state: [%d]  admin_state: [%d] onu_id:[%d] \r\n", 
    key->intf_id,
    data->new_oper_status,
    data->admin_state,
    key->sub_term_id);

    return BCM_ERR_OK;
}


bcmos_errno OltIfOperIndication(bcmbal_obj *obj) 
{
    printf("NICK DEBUG : intf oper state indication, intf_id:[%d] type:[%d] oper_state:[%d] admin_state:[%d]\r\n",
    ((bcmbal_interface_oper_status_change *)obj)->key.intf_id,
    ((bcmbal_interface_oper_status_change *)obj)->key.intf_type,
    ((bcmbal_interface_oper_status_change *)obj)->data.new_oper_status,
    ((bcmbal_interface_oper_status_change *)obj)->data.admin_state);

    return BCM_ERR_OK;
}


bcmos_errno OltOmciIndication(bcmbal_obj *obj) 
{
    printf("OltOmciIndication call back!!!!!!!!\r\n");

    bcmbal_packet_itu_omci_channel_rx *in =
    (bcmbal_packet_itu_omci_channel_rx *)obj;
       
    {
        printf("NICK DEBUG : ONT ");
        int count =0;
        for(count = 1 ; count <= in->data.pkt.len ; count++)
        {
        printf("%02X", in->data.pkt.val[count-1]);
        }
        printf("\r\n");
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
        return false;
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

        fHandle = dlopen("/usr/local/lib/libbal_api_dist.so",RTLD_LAZY);

        if(fHandle)
        {
            printf("Using dynamic loading function\r\n");
            d_bcmbal_stat_get= (bcmos_errno(*)(bcmbal_access_term_id access_term_id, bcmbal_stat *objinfo, bcmos_bool clear_on_read)) dlsym(fHandle,"bcmbal_stat_get");
            d_bcmbal_cfg_get = (bcmos_errno (*)(bcmbal_access_term_id access_term_id, bcmbal_cfg *objinfo)) dlsym(fHandle,"bcmbal_cfg_get");
            d_bcmbal_cfg_set     = (bcmos_errno (*)(bcmbal_access_term_id access_term_id, bcmbal_cfg *objinfo)) dlsym(fHandle,"bcmbal_cfg_set");
            d_bcmbal_pkt_send    = (bcmos_errno (*)(bcmbal_access_term_id access_term_id, bcmbal_dest dest, const char *packet_to_send, uint16_t packet_len)) dlsym(fHandle,"bcmbal_pkt_send");
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
        cb_cfg.module = BCMOS_MODULE_ID_NONE;		

        /* OLT device indication */
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
                printf("Register_callback OltOperIndication ok!!!\r\n");
        }

        /* Interface LOS indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_INTERFACE;
        subgroup = bcmbal_interface_auto_id_los;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltLosIndication;

        if(d_bcmbal_subscribe_ind)
        {
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                printf("Register_callback BCMBAL_OBJ_ID_INTERFACE error!!!\r\n");
                return;
            }
            else
                printf("Register_callback OltLosIndication ok!!!\r\n");
        }

        /* Interface indication */
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
                printf("Register_callback Olt_itf_change ok!!!\r\n");			 
        }

        /* Interface operational state change indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_INTERFACE;
        subgroup = bcmbal_interface_auto_id_oper_status_change;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltIfOperIndication;

        if(d_bcmbal_subscribe_ind)
        {	  
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                return;
            }
            else
                printf("Register_callback OltIfOperIndication ok!!!\r\n");		  
        }

        /* onu discovery indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL;
        subgroup = bcmbal_subscriber_terminal_auto_id_sub_term_disc;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltOnuDiscoveryIndication;

        if(d_bcmbal_subscribe_ind)
        {	  
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                printf("Register_callback BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL error!!!\r\n");
                return;
            }
            else
                printf("Register_callback OltOnuDiscoveryIndication ok!!!\r\n");			  
        }


        /* onu operational state change indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL;
        subgroup = bcmbal_subscriber_terminal_auto_id_oper_status_change;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltOnuOperIndication;

        if(d_bcmbal_subscribe_ind)
        {	  
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                printf("Register_callback BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL error!!!\r\n");

                return;
            }
            else
                printf("Register_callback OltOnuOperIndication ok!!!\r\n");		  
        }

        /* onu indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL;
        subgroup = bcmbal_subscriber_terminal_auto_id_oper_status_change;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltOnuIndication;

        if(d_bcmbal_subscribe_ind)
        {	  
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                printf("Register_callback BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL error!!!\r\n");
                return ;
            }
            else
                printf("Register_callback OltOnuIndication ok!!!\r\n");			  
        }

        /* onu activation failure indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL;
        subgroup = bcmbal_subscriber_terminal_auto_id_sub_term_act_fail;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltOnuActivationFailureIndication;

        if(d_bcmbal_subscribe_ind)
        {	
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                printf("Register_callback BCMBAL_OBJ_ID_SUBSCRIBER_TERMINAL error!!!\r\n");
                return;
            }
            else
                printf("Register_callback OltOnuActivationFailureIndication ok!!!\r\n");			  
        }

        /*  omci indication */
        cb_cfg.obj_type = BCMBAL_OBJ_ID_PACKET;
        subgroup = BCMBAL_IND_SUBGROUP(packet, itu_omci_channel_rx);
        cb_cfg.p_object_key_info = NULL;
        cb_cfg.p_subgroup = &subgroup;
        cb_cfg.ind_cb_hdlr = (f_bcmbal_ind_handler)OltOmciIndication;
  
        if(d_bcmbal_subscribe_ind)
        {	
            if (BCM_ERR_OK != d_bcmbal_subscribe_ind(DEFAULT_ATERM_ID, &cb_cfg)) 
            {
                printf("Register_callback BCMBAL_OBJ_ID_PACKET error!!!\r\n");
                return;
            }
            else
                printf("Register_callback omci indication ok!!!\r\n");			  
        }
	  
        m_subscribed = true;
        return ; 
    }

    void XGS_PON_Olt_Device::get_pon_port_type()
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

    void G_PON_Olt_Device::get_pon_port_type()
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
	
    bool Olt_Device::enable_bal()
    {
        bcmbal_access_terminal_cfg acc_term_obj;
        bcmbal_access_terminal_key key = { };

        if (!m_bal_enable) 
        {
            printf("....Enable OLT... \r\n");
            key.access_term_id = DEFAULT_ATERM_ID;
            BCMBAL_CFG_INIT(&acc_term_obj, access_terminal, key);
            BCMBAL_CFG_PROP_SET(&acc_term_obj, access_terminal, admin_state, BCMBAL_STATE_UP);

            if(d_bcmbal_cfg_set)			
            {
                if (d_bcmbal_cfg_set(DEFAULT_ATERM_ID, &(acc_term_obj.hdr))) 
                {
                    std::cout << "ERROR: Failed to enable OLT" << std::endl;
                    return false;
                }
                m_bal_enable = true;
            }
        }
        return true;
    }

    bool  Olt_Device::enable_pon_if_(int intf_id) 
    {
        bcmbal_interface_cfg interface_obj;
        bcmbal_interface_key interface_key;

        interface_key.intf_id = intf_id;
        interface_key.intf_type = BCMBAL_INTF_TYPE_PON;

        BCMBAL_CFG_INIT(&interface_obj, interface, interface_key);
        BCMBAL_CFG_PROP_SET(&interface_obj, interface, admin_state, BCMBAL_STATE_UP);

        if(d_bcmbal_cfg_set)			
        {    
            if (d_bcmbal_cfg_set(DEFAULT_ATERM_ID, &(interface_obj.hdr))) 
            {
                printf("ERROR: Failed to enable PON interface: %d !!!!", intf_id);
                return false;
            }
        }
        return true;
    }

    bool Olt_Device::activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific) 
    {
        bcmbal_subscriber_terminal_cfg sub_term_obj = {};
        bcmbal_subscriber_terminal_key subs_terminal_key;
        bcmbal_serial_number serial_num = {};
        bcmbal_registration_id registration_id = {};

        printf("ActivateOnu intf_id[%d] onu_id[%d] vendor_id[%s]  vendor_specific[0x%04X] \r\n", intf_id, onu_id, vendor_id,vendor_specific );

        subs_terminal_key.sub_term_id = onu_id;
        subs_terminal_key.intf_id = intf_id;

        BCMBAL_CFG_INIT(&sub_term_obj, subscriber_terminal, subs_terminal_key);

        memcpy(serial_num.vendor_id, vendor_id, 4);
        memcpy(serial_num.vendor_specific, vendor_specific, 4);

        BCMBAL_CFG_PROP_SET(&sub_term_obj, subscriber_terminal, serial_number, serial_num);

        /*
           memset(registration_id.arr, 0, sizeof(registration_id.arr));
           BCMBAL_CFG_PROP_SET(&sub_term_obj, subscriber_terminal, registration_id, registration_id);
           */

        BCMBAL_CFG_PROP_SET(&sub_term_obj, subscriber_terminal, admin_state, BCMBAL_STATE_UP);

        if(d_bcmbal_cfg_set)			
        {       
            if (d_bcmbal_cfg_set(DEFAULT_ATERM_ID, &(sub_term_obj.hdr))) 
            {
                printf("ERROR: Failed to enable ONU: %d", onu_id);
                return false;
            }
            else
            {
                printf("Add ONU: %d on %d sucessfully DEFAULT_ATERM_ID[%d]!!!!\r\n", onu_id, intf_id , DEFAULT_ATERM_ID);            
            }
        }

        sched_add(intf_id, onu_id, (1023+onu_id) );	

        return true;
    }

    bool Olt_Device::sched_add(int intf_id, int onu_id, int agg_port_id) 
    {
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
            //val.u.agg_port.presence_mask = BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_INTF_ID;
            //BCMBAL_CFG_PROP_SET(&cfg, tm_sched, owner, val);

            val.u.agg_port.sub_term_id = (bcmbal_sub_id) onu_id;
            val.u.agg_port.presence_mask = val.u.agg_port.presence_mask | BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_SUB_TERM_ID;
            //val.u.agg_port.presence_mask = BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_SUB_TERM_ID;
            //BCMBAL_CFG_PROP_SET(&cfg, tm_sched, owner, val);

            val.u.agg_port.agg_port_id = (bcmbal_aggregation_port_id) agg_port_id;
            val.u.agg_port.presence_mask = val.u.agg_port.presence_mask | BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_AGG_PORT_ID;
            //val.u.agg_port.presence_mask = BCMBAL_TM_SCHED_OWNER_AGG_PORT_ID_AGG_PORT_ID;            
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

        return true;
    }

#define MAX_CHAR_LEN  20
#define MAX_OMCI_MSG_LEN 44
    bool Olt_Device::omci_msg_out(int intf_id, int onu_id, const std::string pkt) 
    {
        bcmbal_u8_list_u32_max_2048 buf; /* A structure with a msg pointer and length value */
        bcmos_errno err = BCM_ERR_INTERNAL;

        /* The destination of the OMCI packet is a registered ONU on the OLT PON interface */
        bcmbal_dest proxy_pkt_dest;

        proxy_pkt_dest.type = BCMBAL_DEST_TYPE_ITU_OMCI_CHANNEL;
        proxy_pkt_dest.u.itu_omci_channel.sub_term_id = onu_id;
        proxy_pkt_dest.u.itu_omci_channel.intf_id = intf_id;

        if ((pkt.size()/2) > MAX_OMCI_MSG_LEN) 
        {
            buf.len = MAX_OMCI_MSG_LEN;
        } 
        else 
        {
            buf.len = pkt.size()/2;
        }

        /* Send the OMCI packet using the BAL remote proxy API */
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

        buf.val = (uint8_t *)malloc((buf.len)*sizeof(uint8_t));
        memcpy(buf.val, (uint8_t *)arraySend, buf.len);

        if(d_bcmbal_pkt_send)
        {
            err = d_bcmbal_pkt_send(0, proxy_pkt_dest, (const char *)(buf.val), buf.len);

            if(err != BCM_ERR_OK)
                printf("ERROR: Failed to sent omci to ONU [%d] through PON intf [%d]\r\n", onu_id, intf_id);
            else
                printf("OMCI request msg of length [%d] sent to ONU [%d] through PON intf [%d] OK !!\r\n", buf.len, onu_id, intf_id);
        }
        //decode_OMCI_hex_packet(buf.val);

        printf("NICK DEBUG : omci raw data: ");
        int count =0;
        int buf_len = buf.len ;
		
        for(count = 1 ; count <= buf_len ; count++)
        {
            printf("%02X", buf.val[count-1]);
        }
		
        printf("\r\n");

        free(buf.val);

        return true;
    }

    bool Olt_Device::flow_add(int onu_id, int flow_id, const std::string flow_type, const std::string pkt_tag_type, int access_intf_id,
            int network_intf_id, int gemport_id, int classifier, 
            int action ,int action_cmd, struct action_val a_val, struct class_val c_val)
    {
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

        return true;
    }
}
