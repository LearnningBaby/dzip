#pragma once

#include "HuffmanTree.hpp"
#include "common.h"

struct ByteInfo {
	unchar _ch;
	size_t _appearCount;
	std::string _chCode;

	ByteInfo(size_t _appearCount = 0):_appearCount(_appearCount){}

	ByteInfo operator+(const ByteInfo& other) const {
		return ByteInfo(this->_appearCount + other._appearCount);
	}

	bool operator>(const ByteInfo& other) const{
		return _appearCount > other._appearCount;
	}

	bool operator!=(const ByteInfo& other) const{
		return _appearCount != other._appearCount;
	}

	bool operator==(const ByteInfo& other) const{
		return _appearCount == other._appearCount;
	}
};


class FileCompressHuffman {
public:
	FileCompressHuffman();
	void CompressFile(const std::string& filePath); // 压缩源文件
	void UNCompressFile(const std::string& filePath); // 解压缩源文件
private:
	void GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root); // 获取哈夫曼编码
	void WriteHeadInfo(const std::string& filePath, FILE* fout);
	std::string GetFileSuffix(const std::string& filePath);
	void GetLine(FILE* fIn, std::string& strInfo);
	std::string GetFileInfoHead(const std::string& filePath);
private:
	std::vector<ByteInfo> _fileInfo;
};


