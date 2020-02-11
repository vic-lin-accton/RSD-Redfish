#include "acc_onlp_helper/as5916_54xk.hpp"
using namespace acc_onlp_helper;
namespace acc_onlp_helper
{
int As5916_54xk::get_port_tx_status(int port)
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

void As5916_54xk::set_port_tx_status(int port, bool tx_status)
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

void As5916_54xk::get_per_port_sys_file()
{
    try
    {
        Json::Value mapping_s;
        Json::Reader onulist_j_reader = {};
        std::string mapping_file_path = PORT_MAP_PATH + m_onl_platinfo_name + "-sysfs";
        printf("get_per_port_sys_file() mapping_file_path[%s]\r\n", mapping_file_path.c_str());

        std::ifstream map_files(mapping_file_path);

        bool isJson = (onulist_j_reader.parse(map_files, mapping_s));

        if (isJson)
        {
            printf("Get sys_fs mapping file ok!!\r\n");
            int ii = 0;
            for (ii = 1; ii <= m_port_max_num; ii++)
            {
                for (vector<Port_Info *>::iterator pObj = m_vec_Port_Info.begin(); pObj != m_vec_Port_Info.end(); ++pObj)
                {
                    if ((*pObj)->m_ID == ii)
                    {
                        (*pObj)->set_sysfile_path(mapping_s[std::to_string(ii)].asString());
                        break;
                    }
                }
            }
        }
        else
            printf("Get sys fs mapping file error!!\r\n");
    }
    catch (const std::exception &e)
    {
        std::cout << "As5916_54xk get_per_port_sys_file() - exception : " << e.what() << std::endl;
    }
}
} // namespace acc_onlp_helper
