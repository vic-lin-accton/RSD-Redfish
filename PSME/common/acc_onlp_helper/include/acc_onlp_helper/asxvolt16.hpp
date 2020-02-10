#ifndef ACC_ASXVOLT16_HPP
#define ACC_ASXVOLT16_HPP

#include "acc_onlp_helper/acc_onlp_helper.hpp"
namespace acc_onlp_helper
{
using namespace std;
class Asxvolt16 : virtual public Switch
{
public:
    Asxvolt16()
    {
        printf("//////Asxvolt16//////\r\n");
    };
    ~Asxvolt16() { ; };
};
}
#endif