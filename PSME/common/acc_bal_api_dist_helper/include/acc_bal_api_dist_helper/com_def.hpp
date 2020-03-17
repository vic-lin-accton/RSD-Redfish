#ifndef COM_DEF_HPP
#define COM_DEF_HP

extern void (*d_bcmbal_api_set_prop_present)(bcmolt_msg *msg, const void *prop_ptr);
extern bcmos_errno (*d_bcmbal_cfg_set)(bcmolt_oltid olt, bcmolt_cfg *objinfo);
extern bcmos_errno (*d_bcmbal_cfg_get)(bcmolt_oltid olt, bcmolt_cfg *objinfo);
extern std::string vendor_specific_to_str(char const *const vendor_specific);
extern bcmolt_oltid dev_id;

#define UNUSED(x) (void)(x)

#undef BCMOLT_MSG_FIELD_SET
#define BCMOLT_MSG_FIELD_SET(_msg_ptr, _fully_qualified_field_name, _field_value)                                   \
    do                                                                                                              \
    {                                                                                                               \
        (_msg_ptr)->data._fully_qualified_field_name = (_field_value);                                              \
        if (d_bcmbal_api_set_prop_present)                                                                          \
            d_bcmbal_api_set_prop_present(&((_msg_ptr)->hdr.hdr), &((_msg_ptr)->data._fully_qualified_field_name)); \
        else                                                                                                        \
            printf("bcmbal_api_set_prop_present error\r\n");                                                        \
    } while (0)

#undef BCMOLT_MSG_FIELD_GET
#define BCMOLT_MSG_FIELD_GET(_msg_ptr, _fully_qualified_field_name) \
    d_bcmbal_api_set_prop_present(&((_msg_ptr)->hdr.hdr), &((_msg_ptr)->data._fully_qualified_field_name))

#define INTERFACE_STATE_IF_DOWN(state)                \
    ((state == BCMOLT_INTERFACE_STATE_INACTIVE ||     \
      state == BCMOLT_INTERFACE_STATE_PROCESSING ||   \
      state == BCMOLT_INTERFACE_STATE_ACTIVE_STANDBY) \
         ? BCMOS_TRUE                                 \
         : BCMOS_FALSE)
#define INTERFACE_STATE_IF_UP(state) \
    ((state == BCMOLT_INTERFACE_STATE_ACTIVE_WORKING) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_STATE_IF_DOWN(state)                    \
    ((state == BCMOLT_ONU_OPERATION_INACTIVE ||     \
      state == BCMOLT_ONU_OPERATION_DISABLE ||      \
      state == BCMOLT_ONU_OPERATION_ACTIVE_STANDBY) \
         ? BCMOS_TRUE                               \
         : BCMOS_FALSE)
#define ONU_STATE_IF_UP(state) \
    ((state == BCMOLT_ONU_OPERATION_ACTIVE) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_RANGING_STATE_IF_UP(state) \
    ((state == BCMOLT_RESULT_SUCCESS) ? BCMOS_TRUE : BCMOS_FALSE)
#define ONU_RANGING_STATE_IF_DOWN(state) \
    ((state != BCMOLT_RESULT_SUCCESS) ? BCMOS_TRUE : BCMOS_FALSE)

#endif