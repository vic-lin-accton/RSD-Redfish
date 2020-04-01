#ifndef ACC_BAL_API_DIST_HELPER_HPP
#define ACC_BAL_API_DIST_HELPER_HPP

#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <ostream>
#include <string>
#include <map>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include "json/json.hpp"
#include <mutex>
#include <string.h>

enum FLOW_CFG {
    ONU_ID = 0,
    FLOW_TYPE = 1,
    SVC_PORT_ID = 2,
    PRIORITY = 3,
    COOKIE = 4,
    INGRESS_INTF_TYPE= 5,
    EGRESS_INTF_TYPE= 6,
    INGRESS_INTF_ID = 7,
    EGRESS_INTF_ID = 8,
    CLASSIFIER_O_VID = 9,
    CLASSIFIER_O_PBITS = 10,
    CLASSIFIER_I_VID = 11,
    CLASSIFIER_I_PBITS = 12,
    CLASSIFIER_ETHER_TYPE = 13,
    CLASSIFIER_IP_PROTO =14,
    CLASSIFIER_SRC_PORT = 15,
    CLASSIFIER_DST_PORT = 16,
    CLASSIFIER_PKT_TAG_TYPE = 17,
    EGRESS_QOS_TYPE = 18,
    EGRESS_QOS_QUEUE_ID = 19,
    EGRESS_QOS_TM_SCHED_ID = 20,
    ACTION_CMDS_BITMASK = 21,
    ACTION_O_VID = 22,
    ACTION_O_PBITS = 23,
    ACTION_I_VID = 24,
    ACTION_I_PBITS = 25,
    STATE = 26,
    GROUP_ID = 27
};


typedef enum acc_bcmbal_state
{
    ACC_BCMBAL_STATE_UP = 0,
    ACC_BCMBAL_STATE_DOWN = 1
} acc_bcmbal_state;

typedef enum acc_bcmbal_status
{
    ACC_BCMBAL_STATUS_UP = 0,
    ACC_BCMBAL_STATUS_DOWN = 1,
    ACC_BCMBAL_STATUS_NOT_PRESENT = 2
} acc_bcmbal_status;

struct action_val
{
    uint16_t o_vid;  /**< Outer vid. */
    uint8_t o_pbits; /**< Outer pbits. */
    uint16_t o_tpid; /**< Outer tpid. */
    uint16_t i_vid;  /**< Inner vid. */
    uint8_t i_pbits; /**< Inner pbits. */
    uint16_t i_tpid; /**< Inner tpid. */
    uint16_t ether_type;
    uint8_t ip_proto;
    uint16_t src_port; /**< Source port of the packet to be classified */
    uint16_t dst_port; /**< Destination port of the packet to be classified */
};

struct class_val
{
    uint16_t o_vid;  /**< Outer vid. */
    uint8_t o_pbits; /**< Outer pbits. */
    uint16_t o_tpid; /**< Outer tpid. */
    uint16_t i_vid;  /**< Inner vid. */
    uint8_t i_pbits; /**< Inner pbits. */
    uint16_t i_tpid; /**< Inner tpid. */
    uint16_t ether_type;
    uint8_t ip_proto;
    uint16_t src_port; /**< Source port of the packet to be classified */
    uint16_t dst_port; /**< Destination port of the packet to be classified */
};

constexpr const size_t XGS_PON_MAX_PON_PORT_NUM = 16;
constexpr const size_t XGS_PON_MAX_NNI_PORT_NUM = 4;
constexpr const size_t XGS_PON_TOTAL_INTF_NUM(XGS_PON_MAX_PON_PORT_NUM + XGS_PON_MAX_NNI_PORT_NUM);

constexpr const size_t G_PON_MAX_PON_PORT_NUM = 64;
constexpr const size_t G_PON_MAX_NNI_PORT_NUM = 10;
constexpr const size_t G_PON_TOTAL_INTF_NUM(G_PON_MAX_PON_PORT_NUM + G_PON_MAX_NNI_PORT_NUM);

#define MAX(X, Y) ((X > Y) ? (X) : (Y))
#define MAX_PON_PORT_VALUES MAX(XGS_PON_MAX_PON_PORT_NUM, G_PON_MAX_PON_PORT_NUM)
#define MAX_NNI_PORT_VALUES MAX(XGS_PON_MAX_NNI_PORT_NUM, G_PON_MAX_NNI_PORT_NUM)
typedef std::pair<uint16_t, uint16_t> flow_pair;

namespace acc_bal_api_dist_helper
{
using namespace std;
#define MAG_BASE_VAL 1023

struct port_statistic
{
    std::uint64_t rx_bytes;
    std::uint64_t rx_packets;
    std::uint64_t rx_ucast_packets;
    std::uint64_t rx_mcast_packets;
    std::uint64_t rx_bcast_packets;
    std::uint64_t rx_error_packets;
    std::uint64_t tx_bytes;
    std::uint64_t tx_packets;
    std::uint64_t tx_ucast_packets;
    std::uint64_t tx_mcast_packets;
    std::uint64_t tx_bcast_packets;
    std::uint64_t tx_error_packets;
    std::uint64_t rx_crc_errors;
    std::uint64_t bip_errors;
};

class port
{
public:
    bool m_status = {1}; // Disable //
    int m_port_id = {0};
    port_statistic m_port_statistic = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    void set_status(bool status) { m_status = status; };
    //G_PON_MAX_NNI_PORT_NUM should be max value of GPON and XGS-PON or else.
    std::string m_nni_port_type[MAX_NNI_PORT_VALUES] = {""};
    //G_PON_MAX_PON_PORT_NUM should be max value of GPON and XGS-PON or else.
    std::string m_pon_port_type[MAX_PON_PORT_VALUES] = {""};
};

class Olt_Device
{
public:
    Olt_Device();
    ~Olt_Device();

    static Olt_Device &get_instance();
    static void cleanup();
    int get_board_basic_info();
    bool is_bal_init() { return m_bcmbal_init; };
    bool is_bal_lib_init() { return m_bal_lib_init; };
    void enter_cmd_shell()
    {
        while (1)
        {
            usleep(1000);
        }
    };
    void register_callback();
    void set_olt_state(bool state);
    void set_olt_status(bool status);
    void set_bal_status(bool status);
    void set_intf_type(int port, int type);

    bool connect_host();
    bool connect_bal(bool wait_bal_ready);
    bool get_olt_status();
    bool check_bal_ready();
    bool get_bal_status();
    bool get_connection_status();
    bool enable_cli();
    bool enable_pon_if(int intf_id);
    bool enable_nni_if(int intf_id);
    bool disable_pon_if(int intf_id);
    bool disable_nni_if(int intf_id);
    bool deactivate_onu(int intf_id, int onu_id);
    bool delete_onu(int intf_id, int onu_id);
    bool omci_msg_out(int intf_id, int onu_id, const std::string pkt);
    bool flow_add(int onu_id, int flow_id, const std::string flow_type, const std::string pkt_tag_type, int access_intf_id, int network_intf_id, int gemport_id, int classifier, int action, int action_cmd, struct action_val a_val, struct class_val c_val);
    bool flow_remove(uint32_t flow_id, const std::string flow_type);
    int get_flow_size();
    std::map<flow_pair, int32_t> get_flow_map();
    long get_flow_status(uint16_t flow_id, uint16_t flow_type, uint16_t data_id);
    void set_pon_status(int port, int status);
    bool get_pon_status(int port);
    void set_nni_status(int port, int status);
    bool get_nni_status(int port);
    bool get_inf_active_state(int port);
    bool set_inf_active_state(int port, bool status);
    bool is_onu_active(int intf_id, int onu_id);
    bool virtual activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific) = 0;
    bool virtual alloc_id_add(int intf_id, int onu_id, int alloc_id) = 0;
    int virtual get_max_pon_num() = 0;
    int virtual get_max_nni_num() = 0;
    int virtual get_total_port_num() = 0;
    int virtual get_maple_num() = 0;
    std::string get_bal_oper_state() { return m_oper_state; };
    std::string virtual get_platform() = 0;
    mutable std::mutex m_data_mutex{};
    json::Value get_port_statistic(int port);
    json::Value get_flow_info(int flow_id, std::string flowtype);
    port m_pon_port[MAX_PON_PORT_VALUES] = {};
    port m_nni_port[MAX_NNI_PORT_VALUES] = {};
    bool rssi_measurement(int onu_id, int pon_id);
    void *fHandle = 0;
    Olt_Device(const Olt_Device &a) { this->fHandle = a.fHandle; };
    Olt_Device &operator=(const acc_bal_api_dist_helper::Olt_Device &a)
    {
        this->fHandle = a.fHandle;
        return *this;
    };

protected:
    int m_pon_ports_num = {0};
    int m_nni_ports_num = {0};
    uint32_t m_sla_guaranteed_bw = {0};
    uint32_t m_sla_maximum_bw = {0};

    int m_onu_id_start = {0};
    int m_onu_id_end = {0};
    int m_alloc_id_start = {0};
    int m_alloc_id_end = {0};
    int m_gemport_id_start = {0};
    int m_gemport_id_end = {0};
    int m_flow_id_start = {0};
    int m_flow_id_end = {0};
    std::map<flow_pair, int32_t> flow_map ={};

private:
    std::string m_VENDOR_ID = {""};
    std::string m_MODEL_ID = {""};
    std::string m_hardware_version = {""};
    std::string m_pon_type = {""};
    std::string m_bal_version = {""};
    std::string m_firmware_version = {""};
    std::string m_chip_family = {""};
    std::string m_oper_state = {"down"};

    bool m_bal_status = {false};
    bool m_olt_status = {false};
    bool m_bcmbal_init = {false};
    bool m_bcm_host_init = {false};
    bool m_bal_lib_init = {false};
    bool m_subscribed = {false};

    json::Value get_pon_statistic(int port);
    json::Value get_nni_statistic(int port);
};
} // namespace acc_bal_api_dist_helper

#endif
