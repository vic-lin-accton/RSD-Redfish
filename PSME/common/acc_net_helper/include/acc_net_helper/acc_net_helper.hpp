#ifndef ACC_NET_HELPER_HPP
#define ACC_NET_HELPER_HPP

#include <syslog.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <ostream>
#include <string>
#include <limits.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <algorithm>
#include <regex>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <json/json.h>
extern char *__progname;
namespace acc_net_helper
{

using namespace std;

constexpr const size_t MAC_ADDRESS_LENGTH = 18;
constexpr const size_t BUFFER_SIZE = 512;

class ADbg
{
public:
    std::string m_tmp_file_dbg_file = "/tmp/ADbg";
    ifstream m_source_files = {};
    int m_active_log = 0;

    explicit ADbg()
    {
        m_source_files.open(m_tmp_file_dbg_file);
        Json::Value content;

        if (m_source_files.good())
        {
            Json::Reader reader;
            bool isJsonOK = (m_source_files != NULL && reader.parse(m_source_files, content));

            if (isJsonOK)
            {
                m_active_log = content["ADbg"].asInt();
                printf("Debug level [%d].....\r\n", m_active_log);
            }
            else
                printf("Disable debug mode .....\r\n");
        }
        else
        {
            printf("Disable debug mode .....\r\n");
            m_active_log = 0;
        }
        m_source_files.close();
    }
    void acc_printf(const char *format, ...) __attribute__((format(printf, 2, 3)))
    {
        if (m_active_log & 1)
        {
            va_list arglist;
            va_start(arglist, format);
            vprintf(format, arglist);
            va_end(arglist);
        }
        return;
    };

    static ADbg &get_instance();
    void release();

    ~ADbg();
};

class HelperTools
{
public:
    void exec_shell(const char *cmd, char *result_a);
};

////////////////////////////////////////////////////////////////////
// To replace /etc/hosts /etc/hostname old hostname to new hostname
////////////////////////////////////////////////////////////////////
class ConfigMody
{
public:
    void virtual Restart(){};
    std::string m_tmp_file_name_1 = {};
    std::string m_tmp_file_name_2 = {};
    HelperTools m_help_tools = {};
};

class HostnameConfig : public ConfigMody
{
public:
    std::string m_file_hosts = {};
    std::string m_file_hostname = {};
    ifstream m_source_hosts = {};
    ifstream m_source_hostname = {};
    ofstream m_dest_hosts = {};
    ofstream m_dest_hostname = {};
    explicit HostnameConfig()
    {
        m_file_hosts = "/etc/hosts";
        m_file_hostname = "/etc/hostname";
        m_tmp_file_name_1 = "/tmp/tmp_hostname";
        m_tmp_file_name_2 = "/tmp/tmp_hostname_";
    }
    int Get_Hostname(std::string &hostname);
    int Set_Hostname(std::string &hostname);
    void Restart();

private:
    void Open();
    void Close();
};

////////////////////////////////////////////////////////////////////
// To modify /etc/network/interfaces dhcp/static content
////////////////////////////////////////////////////////////////////
class InterfaceConfig : public ConfigMody
{
public:
    std::string m_ifname = {};
    std::string m_file_network_interfaces = {};
    std::string m_file_network_dhcpv6 = {};
    std::string m_ipaddrmask = {};
    unsigned char m_mac[6];
    ifstream m_source_v4_interfaces = {};
    ofstream m_dest_v4_interfaces = {};

    ifstream m_source_v6_interfaces = {};
    ofstream m_dest_v6_interfaces = {};

    explicit InterfaceConfig() : m_ifname("ma1")
    {
        m_file_network_interfaces = "/etc/network/interfaces.d/" + m_ifname;
        m_tmp_file_name_1 = "/tmp/tmp_interfaces";
        m_dhcpv4_enable = false;

        m_file_network_dhcpv6 = "/etc/network/dhcpv6" + m_ifname; //Can't put it in interfaces.d or STATIC IPv4/v6 will have problem.
        m_tmp_file_name_2 = "/tmp/tmp_dhcpv6";
        m_dhcpv6_enable = false;
        m_Ipv6auto_enable = false;
        Get_Interface_DHCP_Enable(m_ifname);
    }

    int Get_Interface_Ip_Info(std::string &inf_name);
    int Set_Interface_Ipv4_Info(std::string &inf_name, std::string &ifr_ip, std::string &ifr_ipmask, std::string &ifr_gateway, std::string &ifr_nameserver);
    int Set_Interface_Ipv6_Info(std::string &inf_name, std::string &ifr_ip, std::string &ifr_ipmask, std::string &ifr_gateway, std::string &ifr_nameserver);
    int Get_Interface_DHCP_Enable(std::string &inf_name);
    int Set_Interface_DHCPv4_Enable(std::string &inf_name);
    int Set_Interface_IPv6_SLAAC_Enable(std::string &inf_name);
    int Set_Interface_IPv6_DHCP_Enable(std::string &inf_name);
    int Set_Interface_IPv6_Disable(std::string &inf_name);

    void Set_Interface_name(std::string &name)
    {
        m_ifname = name;
    }

    const std::string Get_Static_DNS_Name_Server(std::string &inf_name);

    const std::string &get_ip_address() const
    {
        return m_ipaddr;
    }

    const std::string &get_full_hal_status() const
    {
        return m_full;
    }

    bool get_auto_nego_status() const
    {
        return m_auto;
    }

    bool get_dhcpv4_enable() const
    {
        return m_dhcpv4_enable;
    }

    bool get_ipv6_auto_enable() const
    {
        return m_Ipv6auto_enable;
    }

    bool get_dhcpv6_enable() const
    {
        return m_dhcpv6_enable;
    }

    int get_link_speed() const
    {
        return m_speed;
    }

    int get_mtu() const
    {
        return m_mtu;
    }

    int get_link_up_down() const
    {
        return m_linkup;
    }

    void Restart();

private:
    std::string m_ipaddr = {};
    std::string m_full = {};
    bool m_auto = {};
    int m_speed = {};
    int m_mtu = {};
    int m_linkup = {};
    bool m_dhcpv4_enable = {};
    bool m_dhcpv6_enable = {};
    bool m_Ipv6auto_enable = {};

    void Open(std::string &type);
    void Close(std::string &type);
};

class RFLogEntry
{
public:
    void set_log_entry(std::string &event_type, std::string &sensor_type, std::string &Serverity, std::string &message_content, int sensor_id);
    void clean_log();
    void set_log_status(bool enalbe);

    int get_log_status();
    int get_max_entries_num() { return m_max_entry_num; };
    Json::Value get_log_entry_content();
    std::string get_current_time();
    std::string get_zone_offset();

    std::string m_rf_log_enabled_path = {"/etc/psme/nosrvlog"};
    std::string m_rf_log_path = {"/var/log/rf_server.log"};

    HelperTools m_help_tools = {};
    int m_max_entry_num = {1000};

private:
    std::string m_timeStr = {};
};

} // namespace acc_net_helper

#endif
