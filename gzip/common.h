
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cassert>

using std::vector;
using std::string;
typedef unsigned char unchar; // ��������
using std::cout;
using std::endl;
using std::cin;

typedef unsigned long long unll;
typedef unsigned int uint;
typedef unsigned short unshort; // ��ϣ��ͻ��
static const int MAX_MATCH = 258; // ���ƥ�䳤��
static const int MIN_MATCH = 3; // ���ƥ�䳤��

static const unshort WIN_SIZE = 32 * 1024;// ���ڳ���

// ��ȡ�ļ���׺
std::string GetFileSuffix(const std::string& filePath);

// ��ȡ�ļ�����,��������׺
std::string GetFileInfoHead(const std::string& filePath);

string GetFileName(const string& filePath);

string GetFilePostFix(const string& filePath);

void GetLine(FILE* fIn, string& strInfo);