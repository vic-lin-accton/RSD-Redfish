#ifndef ACC_ASGVOLT64_HPP
#define ACC_ASGVOLT64_HPP

#include "acc_onlp_helper/acc_onlp_helper.hpp"
namespace acc_onlp_helper
{
using namespace std;
class Asgvolt64 : virtual public Switch
{
public:
    Asgvolt64()
    {
        printf("//////Asgvolt64//////\r\n");
        get_per_port_sys_file();
    };
    int get_psu_info_by_(int psuid, Psu_Content id);
    int get_port_tx_status(int port);
    void set_port_tx_status(int port, bool tx_status);
    void get_per_port_sys_file();
    std::string m_sys_tx_name = {"/module_tx_disable_"};
    ~Asgvolt64() {;};
};
}
#endif
