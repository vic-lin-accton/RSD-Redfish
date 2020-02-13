#ifndef ACC_SWITCH_SYS_MODE_HPP
#define ACC_SWITCH_SYS_MODE_HPP

#include "acc_onlp_helper/acc_onlp_helper.hpp"
namespace acc_onlp_helper
{
using namespace std;
// Take ASGVOLT64 as parent class
// XSFP light on/off by using module_tx_disable_X  sys node
class Switch_sys_module : virtual public Switch
{
public:
    Switch_sys_module()
    {
        printf("//////Switch_sys_module//////\r\n");
    };
    int get_psu_info_by_(int psuid, Psu_Content id);
    int get_port_tx_status(int port);
    void set_port_tx_status(int port, bool tx_status);
    std::string m_sys_tx_name = {"/module_tx_disable_"};
    ~Switch_sys_module() {;};
};
}
#endif
