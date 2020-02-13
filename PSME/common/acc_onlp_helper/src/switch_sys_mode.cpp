#include "acc_onlp_helper/switch_sys_mode.hpp"
using namespace acc_onlp_helper;
namespace acc_onlp_helper
{
int Switch_sys_module::get_port_tx_status(int port)
{
    try
    {
        std::lock_guard<std::mutex> lock{m_data_mutex};
        int ii = 0;
        for (ii = 1; ii <= m_port_max_num; ii++)
        {
            for (vector<Port_Info *>::iterator pObj = m_vec_Port_Info.begin(); pObj != m_vec_Port_Info.end(); ++pObj)
            {
                if ((*pObj)->m_ID == port)
                {
                    std::string sysfs_path = (*pObj)->get_sysfile_path();
                    return (*pObj)->get_tx_status(sysfs_path + m_sys_tx_name + std::to_string(port));
                }
            }
        }
        return -1;
    }
    catch (const std::exception &e)
    {
        std::cout << "get_port_tx_status() - exception : " << e.what() << std::endl;
        return -1;
    }
}

void Switch_sys_module::set_port_tx_status(int port, bool tx_status)
{
    try
    {
        std::lock_guard<std::mutex> lock{m_data_mutex};
        int ii = 0;
        for (ii = 1; ii <= m_port_max_num; ii++)
        {
            for (vector<Port_Info *>::iterator pObj = m_vec_Port_Info.begin(); pObj != m_vec_Port_Info.end(); ++pObj)
            {
                if ((*pObj)->m_ID == port)
                {
                    std::string sysfs_path = (*pObj)->get_sysfile_path();
                    (*pObj)->set_tx(tx_status, sysfs_path + m_sys_tx_name + std::to_string(port));
                    return;
                }
            }
        }
        return;
    }
    catch (const std::exception &e)
    {
        std::cout << "set_port_tx_status() - exception : " << e.what() << std::endl;
        return;
    }
}

int Switch_sys_module::get_psu_info_by_(int psuid, Psu_Content id)
{
    try
    {
        int ii = 0;

        for (ii = 1; ii <= get_psu_num(); ii++)
        {
            for (vector<Psu_Info *>::iterator pObj = m_vec_Psu_Info.begin(); pObj != m_vec_Psu_Info.end(); ++pObj)
            {
                if ((*pObj)->m_ID == psuid)
                {
                    switch (id)
                    {
                    case Vin:
                        return (*pObj)->m_Vin;
                        break;
                    case Vout:
                        return (*pObj)->m_Vout;
                        break;
                    case Iin:
                        return (*pObj)->m_Iin;
                        break;
                    case Iout:
                        return (*pObj)->m_Iout;
                        break;
                    case Pin:
                        return (*pObj)->m_Pin;
                        break;
                    case Pout:
                        return (*pObj)->m_Pout;
                        break;
                    case Psu_type:
                        if ((*pObj)->m_Present)
                            return 0; // ONLP cannot provide info. Hardcode to AC type.
                        else
                            return -1; // Unknown type //
                        break;
                    case Psu_Present:
                        return (*pObj)->m_Present;
                        break;
                    default:
                        return 0;
                        break;
                    }
                }
            }
        }
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cout << "Switch get_psu_info_by_() - exception : " << e.what() << std::endl;
        return 0;
    }
}
} // namespace acc_onlp_helper
