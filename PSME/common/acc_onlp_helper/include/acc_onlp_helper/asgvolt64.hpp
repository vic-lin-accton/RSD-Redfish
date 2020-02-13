#ifndef ACC_ASGVOLT64_HPP
#define ACC_ASGVOLT64_HPP

#include "acc_onlp_helper/switch_sys_mode.hpp"
namespace acc_onlp_helper
{
using namespace std;
class Asgvolt64 : virtual public Switch_sys_module 
{
public:
    Asgvolt64()
    {
        printf("//////Asgvolt64//////\r\n");
        get_per_port_sys_file();
    };
    ~Asgvolt64() {;};
};
}
#endif
