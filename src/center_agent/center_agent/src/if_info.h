#pragma once
#ifndef _IF_INFO_H_
#define _IF_INFO_H_

int url2domain(const std::string &url, std::string &domain, unsigned &port);
bool get_local_ip(module_t* p_module, const char *host, int port,  unsigned long* ip);
std::string generate_report_json();
std::string generate_json(unsigned long local_ip);
std::string generate_rule_json();
std::string generate_conn_json();
#endif
