#pragma once
#include "common.h"
#include "HashTable.h"
#include "HuffmanTree.hpp"
#include <map>






struct DecodeTable {
	unchar _decodeLen;//����λ��
	uint _code; // ���ַ�����
	unshort _lenCount; // ��ͬ���볤�ȸ���
	unshort _charIndex; // ��������
};

struct ByteLengthInfo {
	unshort _elem; // ������ԭ�ַ�,Ҳ�����ǳ��� 
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
	// huffman ѹ��,���
	void CompressBlock();
	void StatAppearCount();
	void GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo );// ��ȡ����λ��
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
	// huffman ��ѹ�����
	void GetCodeBitLenInfo(FILE* fIn);
	void GenerateDecode(vector<DecodeTable>& decodeTab, vector<ByteLengthInfo>& elemInfo);
	unshort UNCompressSymbol(FILE* fIn, vector<ByteLengthInfo>& codeInfo,vector<DecodeTable>& decTable, unchar& ch, unchar& bitCount);
	void GetNextBit(FILE* fIn, unshort& code,unchar& ch, unchar& bitCount);
private:
	unchar* _pWin;
	HashTable _ht;
	// ��������LZ77 �Ľ��,����Ҫ���佻��huffman�����н�һ��ѹ�� 
	vector<unchar> _byteLengthLz77; // �ֽڳ�����Ϣ
	vector<unshort> _distLz77; // ������Ϣ
	vector<unchar> _flagLz77; // ���λ��Ϣ
	// zip�����ǽ��ļ�����ѹ�����֮�󽻸�huffmanѹ����
	// ���ǰ�����е�,����LZ77ѹ���ĳ��ȵ���0x8000 ʱ,��ʱ����Ҫ���ý������huffman
	// ѹ����
	const unshort BUFF_SIZE = 0x8000;



	const static unshort MIN_LOOKHEAD = MIN_MATCH + MAX_MATCH + 1;
	const static unshort MAX_DIST = WIN_SIZE - MIN_LOOKHEAD; // ÿ�������ҵ���Զ����(�������һ������)

	//////////////////////////////////////////////////////////////////
	// ����Lz77 
	vector<ByteLengthInfo> _byteLengthInfo;
	vector<ByteLengthInfo> _distInfo;

	bool isLast = false;

	FILE* fout;
};