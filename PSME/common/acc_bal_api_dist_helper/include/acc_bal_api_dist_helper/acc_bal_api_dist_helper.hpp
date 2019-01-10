#ifndef ACC_BAL_API_DIST_HELPER_HPP
#define ACC_BAL_API_DIST_HELPER_HPP 

#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <ostream>
#include <string>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include "json/json.hpp"

typedef enum acc_bcmbal_state
{
    ACC_BCMBAL_STATE_UP                                                     = 0,   
    ACC_BCMBAL_STATE_DOWN                                                   = 1
} acc_bcmbal_state;

typedef enum acc_bcmbal_status
{
    ACC_BCMBAL_STATUS_UP                                                    = 0, 
    ACC_BCMBAL_STATUS_DOWN                                                  = 1,
    ACC_BCMBAL_STATUS_NOT_PRESENT                                           = 2
} acc_bcmbal_status;


constexpr const size_t MAX_PON_PORT_NUM = 16;
constexpr const size_t MAX_NNI_PORT_NUM = 4;
constexpr const size_t TOTAL_INTF_NUM (MAX_PON_PORT_NUM+MAX_NNI_PORT_NUM);

namespace acc_bal_api_dist_helper 
{
    using namespace std;

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
            bool           m_status         = {1}; // Disable //
            int            m_port_id        = {0};
            port_statistic m_port_statistic = {0};  
            void set_status(bool status){m_status = status;};
    };

    class nni_port : public port
    {
        public:
            std::string m_nni_port_type[MAX_NNI_PORT_NUM] = {""};
    };

    class pon_port : public port
    {
        public:
            std::string m_pon_port_type[MAX_PON_PORT_NUM] = {""};
    };

    class Olt_Device
    {
        public:

            Olt_Device(int argc, char** argv);
            ~Olt_Device();
            bool is_bal_init(){return  m_bcmbal_init;};
            bool is_bal_lib_init(){return  m_bal_lib_init;};
            void enter_cmd_shell(){while(1){usleep(1000);}};
            static Olt_Device& get_instance();
            static void cleanup();
            bool connect_bal(int argc, char *argv[]); 
            int  get_board_basic_info();
            void register_callback();
            void get_pon_port_type();
            void set_olt_state(bool state);
            void set_olt_status(bool status);
            void set_intf_type(int port,int type);
            void set_pon_status(int port,int status);
            void set_nni_status(int port,int status);
            json::Value get_port_statistic(int port);
            void *fHandle;

        private:

            std::string m_VENDOR_ID         = {""};
            std::string m_MODEL_ID          = {""};
            std::string m_hardware_version  = {""};
            std::string m_firmware_version  = {""};
            std::string m_pon_type          = {""};
            std::string m_bal_version       = {""};

            pon_port m_pon_port[MAX_PON_PORT_NUM]  = {};
            nni_port m_nni_port[MAX_NNI_PORT_NUM]  = {};

            acc_bcmbal_state  m_olt_state  {ACC_BCMBAL_STATE_DOWN};
            acc_bcmbal_status m_olt_status {ACC_BCMBAL_STATUS_DOWN};

            bool m_bal_state         = {false};
            bool m_bal_status        = {false};
            bool m_bcmbal_init       = {false};
            bool m_bal_lib_init      = {false};
            bool m_subscribed        = {false};
            int  m_pon_ports_num     = {0};
            int  m_nni_ports_num     = {0};
            int  m_mac_devs_num      = {0};

            json::Value get_pon_statistic(int port);
            json::Value get_nni_statistic(int port);



    };
}



#endif
