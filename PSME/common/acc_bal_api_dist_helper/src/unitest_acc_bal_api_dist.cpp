#include "../include/acc_bal_api_dist_helper/acc_bal_api_dist_helper.hpp"

using namespace acc_bal_api_dist_helper ;

int main(int argc, char** argv)
{
    auto& OLT = Olt_Device::Olt_Device::get_instance();

    int pon_if_max = OLT.get_max_pon_num();
    int i = 0,count=0;

    printf("//////////// PON interface num[%d]  !!////////////\r\n", pon_if_max);

    //Wait BAL Ready !! 
    while(!OLT.get_bal_status())
    {
        sleep(1);
        printf("////////////Wait BAL Ready[%d] seconds\r\n",count);
        count++;
    }

    while(!OLT.get_olt_status())
    {
        sleep(1);
        printf("////////////Wait OLT Ready[%d] seconds\r\n",count);
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
    //char cs_vendor_spec[4] = {0x71,0xE8,0x01,0x10};
    char cs_vendor_spec[4] = {0x42,0x8D,0xA0,0x6E};

    sleep(10);

    OLT.activate_onu(0 , 1, cs_vendor_id, cs_vendor_spec);
    OLT.enter_cmd_shell();
    return 1;
}

