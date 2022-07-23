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

// ѹ���ļ���ʽ,Ҫ�����ѹ����Ҫ������
// Դ�ļ���׺ 
// �ַ�Ƶ����Ϣ������
// �ַ�:Ƶ�� \n �ַ�:Ƶ��
// + �����ļ�!
void FileCompressHuffman::CompressFile(const std::string& filePath) {
	// 0.��ָ���ļ�
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "��ѹ���ļ�·�������޷���" << std::endl;
		return;
	}

	// 1. ͳ��Դ�ļ���ÿ���ַ����ֵĴ���
	unchar rdBuff[1024];
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

	// 4. дѹ��������Ϣ
	std::string  z_filePath;
	z_filePath = GetFileInfoHead(filePath);
	z_filePath += ".hz";
	FILE* fout = fopen(z_filePath.c_str(), "wb");

	WriteHeadInfo(filePath, fout);

	// 5. �û�ȡ���ı����Դ�ļ����и�д
	// �ȸı��ļ�ָ���λ��
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
	// ���һ��bits�е�8��bit����û�������
	if (bitCount > 0 && bitCount < 8) {
		bits <<= (8 - bitCount);
		fputc(bits, fout);
	}
	fclose(fIn);
	fclose(fout);
}





// ��ѹ��
void  FileCompressHuffman::UNCompressFile(const std::string& filePath) {
	if ("hz" != GetFileSuffix(filePath)) {
		std::cerr << "�ļ����Ͳ���Ӧ��Ϊ filePath.hz" << std::endl;
		return;
	}
	//0. ��ȡ��ѹ����Ҫ����Ϣ
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "��ѹ���ļ�·�������޷���" << std::endl;
		return;
	}

	// ��ѹ���ļ�������
	std::string unCompressFile = GetFileInfoHead(filePath);
	unCompressFile += "_huff.";

	// a. ��ȡԴ�ļ���׺
	std::string suffix;
	GetLine(fIn, suffix);
	unCompressFile += suffix;

	// b. ��ȡƵ����Ϣ������
	std::string strInfo;
	GetLine(fIn, strInfo);
	size_t lineCount = atoi(strInfo.c_str());

	// c. �ع�Ƶ����Ϣ
	for (size_t i = 0; i < lineCount; i++) {
		
		strInfo = "";
		GetLine(fIn, strInfo); // A:1
		if ("" == strInfo) {// ������Ҫ���⴦��
			strInfo += '\n';
			GetLine(fIn, strInfo); 
		}
		unchar idx = strInfo[0];
		_fileInfo[idx]._appearCount = atoi(strInfo.c_str() + 2);
	}
	
	// 2. ��ԭhuffman ��
	HuffmanTree<ByteInfo> ht(_fileInfo, ByteInfo());
	
	// 3. ��ѹ��
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
						break; // ��ѹ����
					}
				}
			}
		}
	}
	fclose(fIn);
	fclose(fout);
}










// private :
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

// д��ͷ����Ϣ(��ѹ���ļ���)
void FileCompressHuffman::WriteHeadInfo(const std::string& filePath, FILE* fout) {
	// 1. ��ȡԴ�ļ���׺
	std::string headInfo;
	headInfo += GetFileSuffix(filePath);
	headInfo += '\n';

	// 2. ����Ƶ����Ϣ
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
		appearLineCount++; // �ַ�:Ƶ����ռ����
	}
	headInfo += std::to_string(appearLineCount);
	headInfo += '\n';
	fwrite(headInfo.c_str(), headInfo.size(), 1, fout);
	fwrite(chInfo.c_str(), chInfo.size(), 1, fout);
}


/*

// ��ȡ�ļ���׺
std::string FileCompressHuffman::GetFileSuffix(const std::string& filePath) {
	return filePath.substr(filePath.find_last_of('.') + 1);
}

// ��ȡ�ļ�����,��������׺
std::string FileCompressHuffman::GetFileInfoHead(const std::string& filePath) {
	return filePath.substr(0,filePath.find_last_of('.'));
}
*/


//���ж�ȡ�ļ�,����Ϣ���浽�ַ���strInfo��
void FileCompressHuffman::GetLine(FILE* fIn, std::string& strInfo) {
	while (true) {
		char ch = fgetc(fIn);
		if (ch == '\n') break;
		strInfo += ch;
	}
}
