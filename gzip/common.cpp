#include "common.h"

// ��ȡ�ļ���׺
std::string GetFileSuffix(const std::string& filePath) {
	return filePath.substr(filePath.find_last_of('.') + 1);
}

// ��ȡ�ļ�����,��������׺
std::string GetFileInfoHead(const std::string& filePath) {
	return filePath.substr(0, filePath.find_last_of('.'));
}