#ifndef ACC_AS5916_54XK_HPP
#define ACC_AS5916_54XK_HPP

#include "acc_onlp_helper/switch_sys_mode.hpp"
namespace acc_onlp_helper
{
using namespace std;
class As5916_54xk : virtual public Switch_sys_module 
{
public:
    As5916_54xk()
    {
        printf("//////As5916_54xk//////\r\n");
        get_per_port_sys_file();
    };
    ~As5916_54xk() {;};
};
}
#endif
