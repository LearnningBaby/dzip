#pragma warning(disable : 4996)
#include "common.h"

// ��ȡ�ļ���׺
std::string GetFileSuffix(const std::string& filePath) {
	return filePath.substr(filePath.find_last_of('.') + 1);
}

// ��ȡ�ļ�����,��������׺
std::string GetFileInfoHead(const std::string& filePath) {
	return filePath.substr(0, filePath.find_last_of('.'));
}

string GetFileName(const string& filePath){
	size_t start = filePath.rfind('\\') + 1;
	return filePath.substr(start, filePath.rfind('.') - start);
}

string GetFilePostFix(const string& filePath){
	return filePath.substr(filePath.rfind('.') + 1);
}

void GetLine(FILE* fIn, string& strInfo){
	while (!feof(fIn)){
		char ch = fgetc(fIn);
		if (ch == '\n'){
			break;
		}
		strInfo += ch;
	}
}