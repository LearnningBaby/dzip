#include "common.h"

// 获取文件后缀
std::string GetFileSuffix(const std::string& filePath) {
	return filePath.substr(filePath.find_last_of('.') + 1);
}

// 获取文件名字,不包含后缀
std::string GetFileInfoHead(const std::string& filePath) {
	return filePath.substr(0, filePath.find_last_of('.'));
}