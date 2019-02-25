#pragma once

#include <string>

/*
#======================================================================
# 异或变换一段数据
#
# 输入：
#   src         源数据
#   idxBegin    源数据起始偏移
#   idxEnd      源数据结束（不含）
#   dest        目的数据
#   destBegin   目的数据起始偏移
#
# 返回： 目的数据
#
*/

std::string XorData(const char* src, int idxBegin, int idxEnd, std::string& dest, int destBegin);

/*
#======================================================================
# CRC16 计算
#  返回: crc16
#*/
unsigned short crc_16(const unsigned char *ptr, unsigned int len);

