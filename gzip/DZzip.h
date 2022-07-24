#pragma once
#include "common.h"
#include "HashTable.h"
#include "HuffmanTree.hpp"
#include <map>






struct DecodeTable {
	unchar _decodeLen;//编码位长
	uint _code; // 首字符编码
	unshort _lenCount; // 相同编码长度个数
	unshort _charIndex; // 符号索引
};

struct ByteLengthInfo {
	unshort _elem; // 可能是原字符,也可能是长度 
	unchar _codeLength;
	unshort _appareCount;
	uint _code;

	ByteLengthInfo(unshort appareCount = 0) :_appareCount(appareCount) {}

	ByteLengthInfo operator+(const ByteLengthInfo& other) const {
		return ByteLengthInfo(this->_appareCount + other._appareCount);
	}

	bool operator>(const ByteLengthInfo& other) const {
		return _appareCount > other._appareCount;
	}

	bool operator!=(const ByteLengthInfo& other) const {
		return _appareCount != other._appareCount;
	}

	bool operator==(const ByteLengthInfo& other) const {
		return _appareCount == other._appareCount;
	}
	bool operator<(const ByteLengthInfo& other) const {
		if ((_codeLength < other._codeLength) || (_codeLength == other._codeLength && _elem < other._elem)) {
			return true;
		}
		return false;
	}
};





class DZzip {
public:
	DZzip();
	~DZzip();

	void Deflated(const string& fileName);
	void UNDeflated(const string& fileName);

private:
	unshort LongestMatch(unshort matchHead, unshort& matchDist, unshort start);
	void FillWindow(FILE* fIn, unll& lookahead, unshort& start);
	void SaveLZ77Result(unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount,unll lookahead);

private:
	//// /////////////////////////////////////
	// huffman 压缩,相关
	void CompressBlock();
	void StatAppearCount();
	void GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo );// 获取编码位长
	void GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo, unchar len);
	void GenerateHuffmanCode(vector<ByteLengthInfo>& elemInfo);
	unshort GetIndexLengthCode(unshort matchLength);
	unshort GetIndexDistCode(unshort matchLength);
	void WriteInfo(FILE* fout);
	void CompressByte(unshort byte, unchar compressbiteInfo, unchar compressbiteCount);
	void CompressLengthDist(unshort length,unshort dist, unchar compressbiteInfo, unchar compressbiteCount);
	void WriteCode(uint code, unchar codeLen, unchar compressbiteInfo, unchar compressbiteCount);
	void ClearPrevStatInfo();

	///////////////////////////////////////////////
	// huffman 解压缩相关
	void GetCodeBitLenInfo(FILE* fIn);
	void GenerateDecode(vector<DecodeTable>& decodeTab, vector<ByteLengthInfo>& elemInfo);
	unshort UNCompressSymbol(FILE* fIn, vector<ByteLengthInfo>& codeInfo,vector<DecodeTable>& decTable, unchar& ch, unchar& bitCount);
	void GetNextBit(FILE* fIn, unshort& code,unchar& ch, unchar& bitCount);
private:
	unchar* _pWin;
	HashTable _ht;
	// 用来保存LZ77 的结果,将来要将其交给huffman来进行进一步压缩 
	vector<unchar> _byteLengthLz77; // 字节长度信息
	vector<unshort> _distLz77; // 距离信息
	vector<unchar> _flagLz77; // 标记位信息
	// zip并不是将文件整体压缩完成之后交给huffman压缩的
	// 而是按块进行的,即当LZ77压缩的长度到达0x8000 时,此时就需要将该结果交给huffman
	// 压缩了
	const unshort BUFF_SIZE = 0x8000;



	const static unshort MIN_LOOKHEAD = MIN_MATCH + MAX_MATCH + 1;
	const static unshort MAX_DIST = WIN_SIZE - MIN_LOOKHEAD; // 每个窗口找的最远距离(除了最后一个窗口)

	//////////////////////////////////////////////////////////////////
	// 保存Lz77 
	vector<ByteLengthInfo> _byteLengthInfo;
	vector<ByteLengthInfo> _distInfo;

	bool isLast = false;

	FILE* fout;
};