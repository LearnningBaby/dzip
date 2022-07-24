#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cassert>

using std::vector;
using std::string;
typedef unsigned char unchar; // 中文问题


typedef unsigned long long unll;
typedef unsigned int uint;
typedef unsigned short unshort; // 哈希冲突表
static const int MAX_MATCH = 258; // 最大匹配长度
static const int MIN_MATCH = 3; // 最短匹配长度

static const unshort WIN_SIZE = 32 * 1024;// 窗口长度

// 获取文件后缀
std::string GetFileSuffix(const std::string& filePath);

// 获取文件名字,不包含后缀
std::string GetFileInfoHead(const std::string& filePath);