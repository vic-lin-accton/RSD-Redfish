#ifndef ASGVOLT64_HPP 
#define ASGVOLT64_HPP

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
#include <mutex>
#include <string.h>
#include "acc_bal_api_dist_helper.hpp"

namespace acc_bal_api_dist_helper
{
using namespace std;

class G_PON_Olt_Device : public Olt_Device
{
public:
    G_PON_Olt_Device() : Olt_Device()
    {
        m_sla_guaranteed_bw = 5120000;
        m_sla_maximum_bw = 5120000;
        m_onu_id_start = 1;
        m_onu_id_end = 127;
        m_alloc_id_start = 256;
        m_alloc_id_end = 767;
        m_gemport_id_start = 256;
        m_gemport_id_end = 4095;
        m_flow_id_start = 1;
        m_flow_id_end = 16383;
    };
    ~G_PON_Olt_Device(){};
    int maple_num = 4;
    int get_max_pon_num() { return G_PON_MAX_PON_PORT_NUM; };
    int get_max_nni_num() { return G_PON_MAX_NNI_PORT_NUM; };
    int get_total_port_num() { return get_max_pon_num() + get_max_nni_num(); };
    int get_maple_num() { return maple_num; };
    bool activate_onu(int intf_id, int onu_id, const char *vendor_id, const char *vendor_specific);
    bool alloc_id_add(int intf_id, int onu_id, int alloc_id);
    std::string platform_type = "asgvolt64";
    std::string get_platform() { return platform_type; };

private:
};
} // namespace acc_bal_api_dist_helper

#endif
