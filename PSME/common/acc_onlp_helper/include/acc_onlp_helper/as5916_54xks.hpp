#ifndef ACC_AS5916_54XKS_HPP
#define ACC_AS5916_54XKS_HPP

#include "acc_onlp_helper/acc_onlp_helper.hpp"
namespace acc_onlp_helper
{
using namespace std;
class As5916_54xks : virtual public Switch
{
public:
    As5916_54xks()
    {
        printf("//////As5916_54xks//////\r\n");
    };
    ~As5916_54xks() { ; };
    int get_psu_info_by_(int psuid, Psu_Content id);
    int get_port_tx_status(int port);
    void set_port_tx_status(int port, bool tx_status);
    std::string m_sys_tx_name = {"/module_tx_disable_"};
};
}
#endif