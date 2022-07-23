#pragma warning(disable : 4996)
#include "FileCompressHuffman.h"
#include "HuffmanTree.hpp"
#include "common.h"

// constructor 
FileCompressHuffman::FileCompressHuffman() {
	_fileInfo.resize(256, 0);
	for (int i = 0; i < 256; i++) {
		_fileInfo[i]._ch = i;
	}
}

// public:

// 压缩文件格式,要保存解压缩需要的内容
// 源文件后缀 
// 字符频次信息总行数
// 字符:频次 \n 字符:频次
// + 编码文件!
void FileCompressHuffman::CompressFile(const std::string& filePath) {
	// 0.打开指定文件
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "待压缩文件路径出错无法打开" << std::endl;
		return;
	}

	// 1. 统计源文件中每个字符出现的次数
	unchar rdBuff[1024];
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

	// 4. 写压缩数据信息
	std::string  z_filePath;
	z_filePath = GetFileInfoHead(filePath);
	z_filePath += ".hz";
	FILE* fout = fopen(z_filePath.c_str(), "wb");

	WriteHeadInfo(filePath, fout);

	// 5. 用获取到的编码堆源文件进行改写
	// 先改变文件指针的位置
	fseek(fIn, 0, SEEK_SET);

	char bits = 0;
	int bitCount = 0;
	while (true) {
		size_t rdSize = fread(rdBuff, 1, 1024, fIn);
		if (0 == rdSize) break;
		for (size_t i = 0; i < rdSize; i++) {
			std::string& strCode = _fileInfo[rdBuff[i]]._chCode;

			for (size_t j = 0; j < strCode.size(); j++) {
				bits <<= 1;
				if ('1' == strCode[j]) {
					bits |= 1;
				}
				bitCount++;
				if (bitCount == 8) {
					fputc(bits, fout);
					bits = 0;
					bitCount = 0;
				}
			}
		}
	}
	// 最后一次bits中的8个bit可能没有填充满
	if (bitCount > 0 && bitCount < 8) {
		bits <<= (8 - bitCount);
		fputc(bits, fout);
	}
	fclose(fIn);
	fclose(fout);
}





// 解压缩
void  FileCompressHuffman::UNCompressFile(const std::string& filePath) {
	if ("hz" != GetFileSuffix(filePath)) {
		std::cerr << "文件类型不对应该为 filePath.hz" << std::endl;
		return;
	}
	//0. 读取解压缩需要的信息
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "待压缩文件路径出错无法打开" << std::endl;
		return;
	}

	// 解压后文件的名字
	std::string unCompressFile = GetFileInfoHead(filePath);
	unCompressFile += "_huff.";

	// a. 读取源文件后缀
	std::string suffix;
	GetLine(fIn, suffix);
	unCompressFile += suffix;

	// b. 读取频次信息总行数
	std::string strInfo;
	GetLine(fIn, strInfo);
	size_t lineCount = atoi(strInfo.c_str());

	// c. 重构频次信息
	for (size_t i = 0; i < lineCount; i++) {
		
		strInfo = "";
		GetLine(fIn, strInfo); // A:1
		if ("" == strInfo) {// 换行需要特殊处理
			strInfo += '\n';
			GetLine(fIn, strInfo); 
		}
		unchar idx = strInfo[0];
		_fileInfo[idx]._appearCount = atoi(strInfo.c_str() + 2);
	}
	
	// 2. 还原huffman 树
	HuffmanTree<ByteInfo> ht(_fileInfo, ByteInfo());
	
	// 3. 解压缩
	FILE* fout = fopen(unCompressFile.c_str(), "wb");
	size_t total = 0;

	unchar rdBuff[1024];
	HuffmanTreeNode<ByteInfo>* cur = ht.GetRoot();
	while (true) {
		size_t rdSize = fread(rdBuff, 1024, 1, fIn);
		if (0 == rdSize) break;
		for (size_t i = 0; i < rdSize; i++) {
			unchar ch = rdBuff[i];
			for (int j = 0; j < 8; ++j) {
				if (ch & 0x80)
					cur = cur->_right;
				else
					cur = cur->_left;
				ch <<= 1;

				if (nullptr == cur->_left && nullptr == cur->_right) {
					fputc(cur->_weight._ch, fout);
					cur = ht.GetRoot();
					total += 1;
					if (cur->_weight._appearCount == total) {
						break; // 解压结束
					}
				}
			}
		}
	}
	fclose(fIn);
	fclose(fout);
}










// private :
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

// 写入头部信息(解压缩文件用)
void FileCompressHuffman::WriteHeadInfo(const std::string& filePath, FILE* fout) {
	// 1. 获取源文件后缀
	std::string headInfo;
	headInfo += GetFileSuffix(filePath);
	headInfo += '\n';

	// 2. 构造频次信息
	size_t appearLineCount = 0;
	std::string chInfo;
	for (auto& e : _fileInfo) {
		if (0 == e._appearCount) {
			continue;
		}
		chInfo += e._ch;
		chInfo += ':';
		chInfo += std::to_string(e._appearCount);
		chInfo += '\n';
		appearLineCount++; // 字符:频次所占行数
	}
	headInfo += std::to_string(appearLineCount);
	headInfo += '\n';
	fwrite(headInfo.c_str(), headInfo.size(), 1, fout);
	fwrite(chInfo.c_str(), chInfo.size(), 1, fout);
}


/*

// 获取文件后缀
std::string FileCompressHuffman::GetFileSuffix(const std::string& filePath) {
	return filePath.substr(filePath.find_last_of('.') + 1);
}

// 获取文件名字,不包含后缀
std::string FileCompressHuffman::GetFileInfoHead(const std::string& filePath) {
	return filePath.substr(0,filePath.find_last_of('.'));
}
*/


//按行读取文件,将信息保存到字符串strInfo中
void FileCompressHuffman::GetLine(FILE* fIn, std::string& strInfo) {
	while (true) {
		char ch = fgetc(fIn);
		if (ch == '\n') break;
		strInfo += ch;
	}
}
