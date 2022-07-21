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
	// 1. ͳ��Դ�ļ���ÿ���ַ����ֵĴ���
	FILE* fIn = fopen(filePath.c_str(), "r");
	if (nullptr == fIn) {
		std::cerr << "��ѹ���ļ�·�������޷���" << std::endl;
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

	// 2. ��ͳ�ƽ������Huffman��
	ByteInfo invalid;
	HuffmanTree<ByteInfo> ht(_fileInfo,invalid);

	// 3. ��ȡhuffman ����
	GenerateHuffmanCode(ht.GetRoot());

}

void  FileCompressHuffman::UNCompressFile(const std::string& filePath) {

}



// Ϊÿ���ַ���ȡ�������ı���
void FileCompressHuffman::GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root) {
	if (nullptr == root)
		return;
	GenerateHuffmanCode(root->_left);
	GenerateHuffmanCode(root->_right);

	// ������root����Ҷ��!
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

