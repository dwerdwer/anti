#pragma once
#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H
//一些业务函数， 诸如 登录、注册

int login(module_t* p_module);

int center_login(module_t* p_module, module_data_t *p_module_data);

/*
 	n_flag  ： 1； 表示 success  0：表示失败
 */
int on_login(module_t* p_module, char* p_buf,int n_length);

int center_reg(module_t* p_module, module_data_t *p_module_data);

int on_reg(module_t* p_module, char* p_buf, int n_length);


int center_heart_beat(module_t* p_module, module_data_t *p_module_data);
int on_heart_beat(module_t* p_module, char* p_buf, int n_length);

int center_logout(module_t* p_module);

int on_logout(module_t* p_module, char* p_buf, int n_length);

int center_report(module_t* p_module);

int on_report(module_t* p_module, char* p_buf, int n_length);

#endif
