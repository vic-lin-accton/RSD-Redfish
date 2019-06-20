#include "../include/acc_bal30_api_dist_helper/acc_bal30_api_dist_helper.hpp"

using namespace acc_bal30_api_dist_helper ;

int main(int argc, char** argv)
{
    auto& OLT = Olt_Device::Olt_Device::get_instance();

    int pon_if_max = OLT.get_max_pon_num();
    int i = 0,count=0;

    printf("//////////// PON interface num[%d]  !!////////////\r\n", pon_if_max);

    while(!OLT.get_olt_status())
    {
        sleep(1);
        printf("[%d]\r\n",count);
        count++;
    }
    //Wait 16 port into ready states//
    sleep(5);

    for(i = 0; i < pon_if_max ; i++)
    {
        printf("////////////Enable PON interface [%d] to UP !!////////////\r\n", i);
        OLT.enable_pon_if(i);
    }

    char cs_vendor_id[4] = {0x49,0x53,0x4B,0x54};//"ISKT"
    char cs_vendor_spec[4] = {0x71,0xE8,0x01,0x10};

    sleep(5);

    OLT.activate_onu(0 , 1, cs_vendor_id, cs_vendor_spec);
    OLT.enter_cmd_shell();
    return 1;
}

