#pragma once

#include <string>

/*
#======================================================================
# ���任һ������
#
# ���룺
#   src         Դ����
#   idxBegin    Դ������ʼƫ��
#   idxEnd      Դ���ݽ�����������
#   dest        Ŀ������
#   destBegin   Ŀ��������ʼƫ��
#
# ���أ� Ŀ������
#
*/

std::string XorData(const char* src, int idxBegin, int idxEnd, std::string& dest, int destBegin);

/*
#======================================================================
# CRC16 ����
#  ����: crc16
#*/
unsigned short crc_16(const unsigned char *ptr, unsigned int len);

