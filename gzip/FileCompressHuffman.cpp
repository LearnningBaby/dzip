#pragma warning(disable : 4996)
#include "FileCompressHuffman.h"
#include "HuffmanTree.hpp"
#include <iostream>

FileCompressHuffman::FileCompressHuffman() {
	_fileInfo.resize(256, 0);
	for (int i = 0; i < 256; i++) {
		_fileInfo[i]._ch = i;
	}
}


void FileCompressHuffman::CompressFile(const std::string& filePath) {
	// 1. 统计源文件中每个字符出现的次数
	FILE* fIn = fopen(filePath.c_str(), "r");
	if (nullptr == fIn) {
		std::cerr << "待压缩文件路径出错无法打开" << std::endl;
		return;
	}

	char rdBuff[1024];
	while (true) {
		size_t rdSize = fread(rdBuff, 1, 1024, fIn);
		if (rdSize == 0) break; // read end 
		for (size_t i = 0; i < rdSize; i++) {
			_fileInfo[rdBuff[i]]._appearCount++;
		}
	}

	// 2. 用统计结果创建Huffman树
	ByteInfo invalid;
	HuffmanTree<ByteInfo> ht(_fileInfo,invalid);

	// 3. 获取huffman 编码
	GenerateHuffmanCode(ht.GetRoot());

}

void  FileCompressHuffman::UNCompressFile(const std::string& filePath) {

}



// 为每个字符获取哈夫曼的编码
void FileCompressHuffman::GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root) {
	if (nullptr == root)
		return;
	GenerateHuffmanCode(root->_left);
	GenerateHuffmanCode(root->_right);

	// 到这里root就是叶子!
	if (nullptr == root->_left && nullptr == root->_right) {

		HuffmanTreeNode<ByteInfo>* cur = root;
		HuffmanTreeNode<ByteInfo>* parent = cur->_parent;
		while (parent) {
			if (cur == parent->_left)
				_fileInfo[root->_weight._ch]._chCode += '0';
			else
				_fileInfo[root->_weight._ch]._chCode += '1';
			cur = parent;
			parent = cur->_parent;
		}
		reverse(_fileInfo[root->_weight._ch]._chCode.begin(), _fileInfo[root->_weight._ch]._chCode.end());
	}
}

