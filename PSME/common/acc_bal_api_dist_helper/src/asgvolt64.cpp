#include "../include/acc_bal_api_dist_helper/asgvolt64.hpp"
#include <dlfcn.h>
#include <stdlib.h>
using namespace acc_bal_api_dist_helper;

#ifdef __cplusplus
extern "C"
{
#include <bcmolt_api.h>
#include <bcmolt_host_api.h>
#include <bcmolt_api_model_supporting_enums.h>
#include <bcmolt_api_conn_mgr.h>
#include <bcmos_common.h>
#include <bcm_config.h>
}
#endif
#include "../include/acc_bal_api_dist_helper/com_def.hpp"
namespace acc_bal_api_dist_helper
{
bool G_PON_Olt_Device::activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific)
{
    std::lock_guard<std::mutex> lock{m_data_mutex};
    try
    {
        bcmos_errno err = BCM_ERR_INTERNAL;
        bcmolt_serial_number serial_number;
        bcmolt_onu_cfg onu_cfg;
        bcmolt_onu_key onu_key;
        onu_key.onu_id = onu_id;
        onu_key.pon_ni = intf_id;
        BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
        BCMOLT_FIELD_SET_PRESENT(&onu_cfg.data, onu_cfg_data, onu_state);

        if (is_onu_active(intf_id, onu_id))
            return true;

        printf("GPON Enabling ONU %d on PON %d : vendor id %s,  vendor specific %s", onu_id, intf_id, vendor_id, vendor_specific_to_str(vendor_specific).c_str());
        memcpy(serial_number.vendor_id.arr, vendor_id, 4);
        memcpy(serial_number.vendor_specific.arr, vendor_specific, 4);
        BCMOLT_CFG_INIT(&onu_cfg, onu, onu_key);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.serial_number, serial_number);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.auto_learning, BCMOS_TRUE);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.gpon.ds_ber_reporting_interval, 1000000);
        BCMOLT_MSG_FIELD_SET(&onu_cfg, itu.gpon.omci_port_id, onu_id);

        if (d_bcmbal_cfg_set)
            err = d_bcmbal_cfg_set(dev_id, &onu_cfg.hdr);

        if (err != BCM_ERR_OK)
        {
            printf(" ERROR [%d]\n", err);
            return false;
        }
        alloc_id_add(intf_id, onu_id, (MAG_BASE_VAL + onu_id));
        return true;
    }
    catch (const exception &e)
    {
        printf("GPON activate_onu error\r\n");
        return false;
    }
}

bool G_PON_Olt_Device::alloc_id_add(int intf_id, int in_onu_id, int alloc_id)
{
    try
    {
        bcmos_errno err = BCM_ERR_OK;
        bcmolt_itupon_alloc_cfg cfg;      /* declare main API struct */
        bcmolt_itupon_alloc_key key = {}; /* declare key */

        key.pon_ni = intf_id;
        key.alloc_id = alloc_id;

        printf("GPON alloc_id_add pon_id[%d] onu_id[%d] alloc_id[%d]", intf_id, in_onu_id, alloc_id);

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

        BCMOLT_FIELD_SET(&sla, pon_alloc_sla, guaranteed_bw, m_sla_guaranteed_bw);
        BCMOLT_FIELD_SET(&sla, pon_alloc_sla, maximum_bw, m_sla_maximum_bw);

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

        if (d_bcmbal_cfg_set)
        {
            err = d_bcmbal_cfg_set(dev_id, &cfg.hdr);

            if (err != BCM_ERR_OK)
            {
                printf(" ERROR!! \n");
                return false;
            }
            return true;
        }
        else
            return false;
    }
    catch (const exception &e)
    {
        printf("GPON alloc_id_add error\r\n");
        return false;
    }
}

} // namespace acc_bal_api_dist_helper
