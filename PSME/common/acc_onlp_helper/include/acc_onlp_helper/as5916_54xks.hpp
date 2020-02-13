#ifndef ACC_AS5916_54XKS_HPP
#define ACC_AS5916_54XKS_HPP

#include "acc_onlp_helper/switch_sys_mode.hpp"
namespace acc_onlp_helper
{
using namespace std;
class As5916_54xks : virtual public Switch_sys_module 
{
public:
    As5916_54xks()
    {
        printf("//////As5916_54xks//////\r\n");
    };
    ~As5916_54xks() {;};
};
}
#endif