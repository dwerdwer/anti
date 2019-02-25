#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <map>
#include <json/json.h>
#include <iostream>
#include <regex>
#include <time.h>
#include "Mutex.h"
#include "module_def.h"
#include "center_log.h"
#include "if_info.h"
using namespace std;

typedef struct Eth {
	std::vector<unsigned long>	ipv4;
	std::vector<std::string>		ipv6;
	unsigned long	gateway_ipv4;
	std::string		gateway_ipv6;
	std::vector<uint32_t>		maskv4;
	std::vector<uint32_t>		maskv6;
	std::string		eth_name;
	std::string		mac;

	Eth() { gateway_ipv6 = ""; eth_name = ""; mac = ""; gateway_ipv4 = 0; gateway_ipv6 = ""; }

}eth_t;

int url2domain(const std::string &url, std::string &domain, unsigned &port)
{

	int ret = -1;
/*
	std::regex reg_domain_port("/");  //按/符拆分字符串
	std::cregex_token_iterator itrBegin(url.c_str(), url.c_str() + url.size(), reg_domain_port, -1);
	std::cregex_token_iterator itrEnd;
	int i = 0;

	std::string domain_port;
	for (std::cregex_token_iterator itr = itrBegin; itr != itrEnd; ++itr)
	{
		i++;
		if (i == 3)
			domain_port = *itr;
	}

	if (domain_port.size() == 0)
		domain_port = url;

	//考虑带端口的情况
	std::regex reg_port(":");
	std::cregex_token_iterator itrBegin2(domain_port.c_str(), domain_port.c_str() + domain_port.size(), reg_port, -1);
	std::cregex_token_iterator itrEnd2;
	int j = 0;
	for (std::cregex_token_iterator itr = itrBegin2; itr != itrEnd2; ++itr) {
		j++;
		if (j == 1) {
			domain = *itr;
		}
		if (j == 2)
		{
			port = std::stold(*itr);
			//itoa(port,*itr,5);;
		}
	}
*/
    char tmp_url[1024] = { 0 };

    strncpy(tmp_url, url.c_str(), sizeof(tmp_url));

    // 拆分 url 例: http://www.cppprog.com/2009/0112/48.html
    char *tmp_str = NULL;

    tmp_str = strtok(tmp_url, "/");

    tmp_str=strtok(NULL, "/");

    std::string domain_port;
    domain_port = tmp_str;

    if (domain_port.size() == 0)
        domain_port = url;

    //考虑带端口的情况
    char *tmp_port = strtok(tmp_str, ":");

    if(NULL != tmp_port)
        domain = tmp_port;

    tmp_port = strtok(NULL, ":");

    if(NULL != tmp_port)
        port = atoi(tmp_port);

    if (domain.size() == 0)
        domain = domain_port;

    return ret;
}



static char* getMac(char* mac,const char* dv){
    struct   ifreq   ifreq;
    int   sock;
    if(!mac || !dv)
        return mac;
    if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
    {
        perror( "socket ");
        return mac;
    }
    strcpy(ifreq.ifr_name,dv);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
    {
        perror( "ioctl ");
        close(sock);
        return mac;
    }
    //pHx((unsigned char*)ifreq.ifr_hwaddr.sa_data,sizeof(ifreq.ifr_hwaddr.sa_data));
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", //以太网MAC地址的长度是48位
           (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
           (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
           (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
           (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
           (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
           (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
    close(sock);
    return mac;
}

bool get_local_ip(module_t *p_module, const char *host, int port,  unsigned long* ip)
{
	int socketfd;
	struct sockaddr_in sockaddr;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0)
	{
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		char szbuf[200] = {0};
		sprintf(szbuf, "create socket error: %s (errno: %d)\n", strerror(errno), errno);
		center_log(p_module, szbuf);
		return false;
	}
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, host, &sockaddr.sin_addr);
	if ((connect(socketfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr))) < 0)
	{
		printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
		char szbuf[200] = {0};
		sprintf(szbuf, "create socket error: %s (errno: %d)\n", strerror(errno), errno);
		center_log(p_module, szbuf);
		close(socketfd);
		return false;
	}

	sockaddr_in my_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	getsockname(socketfd, (struct sockaddr*)&my_addr,&addr_len);

	std::string str = inet_ntoa(my_addr.sin_addr);
//	std::cout << "local ip[" << str.c_str() << "]" << endl;
    *ip = my_addr.sin_addr.s_addr;

    close(socketfd);

    return true;

}

static std::vector<eth_t> get_all_eth()
{
	struct ifaddrs *ifap0 = NULL, *ifap = NULL;
	void *tmpAddrPtr = NULL;

	std::map<std::string, eth_t> eth_map;

	getifaddrs(&ifap0);
	ifap = ifap0;
	while(ifap != NULL) {
		if (ifap->ifa_addr->sa_family == AF_INET) {  // check it is IP4
			tmpAddrPtr = &((struct sockaddr_in *)ifap->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
            memset(addressBuffer, 0,INET_ADDRSTRLEN);
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if (strcmp(addressBuffer, "127.0.0.1") != 0) {
				auto it = eth_map.find(ifap->ifa_name);
				if (it != eth_map.end()) {
					it->second.ipv4.push_back(((struct in_addr*)tmpAddrPtr)->s_addr);
					it->second.eth_name = ifap->ifa_name;
					//子网掩码、网关先不管
				}
				else {
					std::pair<std::string, eth_t> pair;
					eth_t eth;
					eth.ipv4.push_back( ((struct in_addr*)tmpAddrPtr)->s_addr);
					eth.eth_name = ifap->ifa_name;
					pair = std::make_pair(ifap->ifa_name, eth);
					eth_map.insert(pair);
				}
			}

		}
		else if (ifap->ifa_addr->sa_family == AF_INET6) {  // check it is IP6
			tmpAddrPtr = &((struct sockaddr_in*)ifap->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
            memset(addressBuffer, 0,INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			if (strcmp(addressBuffer, "::") != 0) {

				auto it = eth_map.find(ifap->ifa_name);
				if (it != eth_map.end()) {
					std::string str_ipv6 = addressBuffer;
					it->second.ipv6.push_back(str_ipv6);



				} else {
					std::pair<std::string, eth_t> pair;
					eth_t eth;
					std::string str_ipv6 = addressBuffer;
					eth.ipv6.push_back(str_ipv6);
					eth.eth_name = ifap->ifa_name;
					pair = std::make_pair(ifap->ifa_name,eth);
					eth_map.insert(pair);
				}
			}

		}
		ifap=ifap->ifa_next;
	}

	//获取mac 地址
	for (auto it = eth_map.begin(); it != eth_map.end(); it++) {
		char mac[30];
		it->second.mac = string(getMac(mac,it->second.eth_name.c_str()));
	}

	std::vector<eth_t> eth_vct;
	for (auto it = eth_map.begin(); it != eth_map.end(); it++) {

		eth_vct.push_back(it->second);
	}
    freeifaddrs(ifap0);
	return eth_vct;
}

std::string generate_report_json()
{
    Json::Value encode;

    encode["virus_name"] = "virus_name";
    encode["vlib_version"] = "vlib_version";
    encode["virus_findby"] = 1;
    encode["virus_op"] = 1;
    encode["date"] = time((time_t*)NULL);
    encode["find_time"] = time((time_t*)NULL);
    encode["filepath"] = "ceshipath";
    encode["virus_type"] = 2;


    Json::Value root;
    root["node_file_virus"] = encode;
    root["samps"] = "";
    root["virus_type"] = "";
    root["virus_features"] = "";

	Json::FastWriter writer;
	std::string out2 = writer.write(root);
    return out2;
}

std::string generate_json(unsigned long local_ip)
{
	std::vector<eth_t>  vct_eth = get_all_eth();
//	cout << "vector eth length is" << vct_eth.size() << endl;

	//	auto it = std::find_if(vct_eth.begin(), vct_eth.end(), isEqual);

	bool find = false;
	auto it = vct_eth.begin();
	for (it; it != vct_eth.end(); it++) {
		auto j = it->ipv4.begin();
		int index_mask = 0;
		for (; j != it->ipv4.end(); j++,index_mask++) {
			if (*j == local_ip) {
				find = true;
				it->ipv4.erase(j);
				it->ipv4.insert(it->ipv4.begin(), local_ip);

				if ( index_mask < it->maskv4.size())
				{
					uint32_t maskv4 = it->maskv4[index_mask];
					it->maskv4.erase(it->maskv4.begin() + index_mask);
					it->maskv4.insert(it->maskv4.begin(), maskv4);
				}
				break;  // it 为连控制中心的网卡
			}
		}
		if (find)
		{
			eth_t eth0 = *it;
			vct_eth.erase(it);
			vct_eth.insert(vct_eth.begin(), eth0);
			break;
		}
	}

	Json::Value arrayObj, json_obj;

	for (auto k = vct_eth.begin(); k != vct_eth.end(); k++) {
		Json::Value obj;
		obj["name"] = k->eth_name.c_str();
		obj["mac"] = k->mac.c_str();

		std::string ipv4 = "";
		char buf[200]; memset(buf, 0, 200);
		for (auto l = k->ipv4.begin(); l != k->ipv4.end(); l++) {
            uint64_t iEndian = ntohl(*l);
        	sprintf(buf, "%x;", iEndian);
			ipv4.append(buf);
		}
//		std::cout << ipv4.c_str() << ipv4.length() << endl;

		string tmp = ipv4.c_str();

		obj["ipv4"] = ipv4.c_str();

		string ipv6 = "";
		for (auto m = k->ipv6.begin(); m != k->ipv6.end(); m++) {
			ipv6.append(*m);
			ipv6.append(";");
		}
		obj["ipv6"] = ipv6.c_str();

		memset(buf, 0, 200);
        uint64_t lEndian = ntohl(k->gateway_ipv4);
		sprintf(buf, "%x", k->gateway_ipv4);
		string gwv4 = ""; gwv4.append(buf);
		obj["gwv4"] = gwv4;

		obj["gwv6"] = k->gateway_ipv6.c_str();

		string netmaskv4 = "";
		for (auto j = k->maskv4.begin(); j != k->maskv4.end(); j++) {
			memset(buf, 0, 200);
			sprintf(buf, "%d;", *j);
			netmaskv4.append(buf);
		}
		obj["maskv4"] = netmaskv4.c_str();


		string netmaskv6 = "";
		for (auto n = k->maskv6.begin(); n != k->maskv6.end(); n++) {
			sprintf(buf, "%d;", *n);
			netmaskv6.append(buf);
		}
		obj["maskv6"] = netmaskv6.c_str();

		arrayObj.append(obj);

	}
	json_obj["eth"] = arrayObj;


	//有格式输出
//	std::string out = json_obj.toStyledString();
	//std::cout << out.c_str() << endl;

	//无格式
	Json::FastWriter writer;
	std::string out2 = writer.write(json_obj);
//	std::cout << out2.c_str() << endl;
	return out2;
}
std::string generate_rule_json()
{
	Json::Value ruleArray, json_obj;
	Json::Value ruleObj, objarray;
	// TODO: add sth to objarray
	Json::Value obj;

	objarray.append(obj);

	ruleObj["rule_id"] = "12345";
	ruleObj["object_type"] = "4";
	ruleObj["objects"] = "acf8da07c687dbfb0579af4a6dd31871";
	ruleObj["action"] = "1";
	ruleObj["finish_time"] = 0;

	ruleArray.append(ruleObj);

	json_obj["interval_time"] = 10;
	json_obj["rules"] = ruleArray;
	json_obj["report_host"] = 5;

	Json::FastWriter writer;
	std::string out = writer.write(json_obj);
//	std::cout << out.c_str() << endl;
	return out;
}

std::string generate_conn_json()
{
	Json::Value json_obj;
	json_obj["pid"] = "1878";
	json_obj["destip"] = "";
	json_obj["port"] = 0;
	json_obj["domain"] = "www.baidu.com";
	json_obj["time"] = 88;

	Json::FastWriter writer;
	std::string out = writer.write(json_obj);
	return out;
}

/*
int main (int argc, const char * argv[])
{
	std::string host;
	unsigned int port = 0;
	url2domain("http://192.168.10.246:8000/dfk",host, port);
	cout << "host is [" << host.c_str() << "]  port is [" << port << "]" << endl;
	return 1;
	unsigned long local_ip = 0;
	if (get_local_ip("192.168.10.246", 8000, &local_ip)) {
		std::string interfaces = generate_json(local_ip);
	}
    return 0;
}

*/
