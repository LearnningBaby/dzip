#pragma once
#include <string>
#include <vector>
#include <iostream>

using std::string;
typedef unsigned char unchar; // 中文问题


typedef unsigned long long unll;
typedef unsigned short unshort; // 哈希冲突表
static const int MAX_MATCH = 258; // 最大匹配长度
static const int MIN_MATCH = 3; // 最短匹配长度

static const unshort WIN_SIZE = 32 * 1024;// 窗口长度
