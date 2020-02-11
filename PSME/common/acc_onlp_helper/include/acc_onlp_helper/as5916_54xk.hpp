#ifndef ACC_AS5916_54XK_HPP
#define ACC_AS5916_54XK_HPP

#include "acc_onlp_helper/acc_onlp_helper.hpp"
namespace acc_onlp_helper
{
using namespace std;
class As5916_54xk : virtual public Switch
{
public:
    As5916_54xk()
    {
        printf("//////As5916_54xk//////\r\n");
        get_per_port_sys_file();
    };
    int get_port_tx_status(int port);
    void set_port_tx_status(int port, bool tx_status);
    void get_per_port_sys_file();
    std::string m_sys_tx_name = {"/module_tx_disable_"};
    ~As5916_54xk() {;};
};
}
#endif
