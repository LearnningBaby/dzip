#pragma once

#include "HashTable.h"
#include "HuffmanTree.hpp"
#include "Common.h"
struct ByteLengthInfo
{
	unshort _elem;// ������ԭ�ַ���Ҳ�����ǳ���
	unchar _codeLength; //����λ��
	unshort _appearCount;//���ִ���
	uint _code; // ����

	ByteLengthInfo(size_t appearCount = 0)
		:_appearCount(appearCount)
	{}

	//����+ ��ΪByteInfo���Ͳ���ֱ����ӣ������Ǵ���Huffman��ʱ��ʹ�õ���ByteInfo���͵�vector�������Ҫ����+��������
	//�ӷ����ܸı�Ԫ�����ͣ���˷���ֵ��Ҫǿ������ת��ΪByteInfo
	ByteLengthInfo operator+(const ByteLengthInfo& other)const
	{
		return ByteLengthInfo(_appearCount + other._appearCount);
	}

	//����> ����Huffman����Ҫ�Ƚ�Ԫ�س��ֵĴ�������ByteInfo����ֱ�ӱȽϣ����ش˷���
	bool operator>(const ByteLengthInfo& other)const
	{
		return _appearCount > other._appearCount;
	}

	//����!= ����Huffman����Ҫ�ж�Ԫ�س��ִ����Ƿ�Ϊ0���Ƿ�ʹ����vaildһ����
	bool operator!=(const ByteLengthInfo& other)const
	{
		return _appearCount != other._appearCount;
	}

	bool operator==(const ByteLengthInfo& other)const
	{
		return _appearCount == other._appearCount;
	}

	bool operator<(const ByteLengthInfo& other)const
	{
		if ((_codeLength < other._codeLength) || (_codeLength == other._codeLength && _elem < other._elem))
		{
			return true;
		}

		return false;
	}
};


//�����
struct DecodeTable
{
	unchar _decodeLen;//����λ��
	uint _code;//���ַ�����
	unshort _lenCount;//��ͬ���볤�ȸ���
	unshort _charIndex;//��������
};

class DZzip
{
public:
	DZzip();
	~DZzip();
	void Deflate(const string& fileName);
	void UnDeflate(const string& fileName);

private:
	void SaveLZ77Result(unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount, unll lookahead);
	unshort LongestMatch(unshort matchHead, unshort& matchDist, unshort start);
	void FillWindow(FILE* fIn, unll& lookahead, unshort& start);


private:
	/// huffman���
	void CompressBlock();
	void StatAppearCount();
	//��ȡ����λ��
	void GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo);
	void GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo, unchar len);
	void GenerateHuffmanCode(vector<ByteLengthInfo>& elemInfo);
	void WriteInfo(FILE* fOut);
	void CompressByte(unshort byte, unchar& compressBiteInfo, unchar& compressBiteCount);
	void CompressLengthDist(unshort length, unshort dist, unchar& compressBiteInfo, unchar& compressBiteCount);
	void WriteCode(uint code, unchar codeLen, unchar& CompressBiteInfo, unchar& compressBiteCount);
	void ClearPrevStatInfo();


	unshort GetIndexLengthCode(unshort matchLenth);
	unshort GetIndexDistCode(unshort dist);

	/// <summary>
	/// ///////////////////////////////////////////////////////////////
	/// 	��ѹ������
	/// </summary>
	void GetCodeBitLenInfo(FILE* fIn);
	void GenerateDecode(vector<DecodeTable>& decodeTable, vector<ByteLengthInfo>& elemInfo);
	unshort UNcompressSymbol(FILE* fIn, vector<ByteLengthInfo>& codeInfo, vector<DecodeTable>& decTable, unchar& ch, unchar& bitCount);
	void GetNextBit(FILE* fIn, unshort& code, unchar& ch, unchar& bitCount);
private:
	unchar* _pWin;
	HashTable _ht;

	//��������ʣ���ַ���С��MIN_LOOKHEADʱ�������¶�ȡ
	const static unshort MIN_LOOKHEAD = MIN_MATCH + MAX_MATCH + 1;
	//���������MAX_DISTʱ��������ǰ��ѯ
	const static unshort MAX_DIST = WIN_SIZE - MIN_LOOKHEAD;

	/// <summary>
	///����lz77�Ľ������������huffman����һ��ѹ�� 
	vector<unchar> _byteLengthLZ77;
	vector<unshort> _distLZ77;
	vector<unchar> _flagLZ77;

	//DZzip�����ǽ��ļ�����ѹ�����֮�󽻸�Huffman
	//���ǰ�����У���LZ77ѹ�����ȴﵽ0X8000ʱ����ʱ����Ҫ���ý��Huffman��ѹ��
	const unshort BUFF_SIZE = 0x8000;

	vector<ByteLengthInfo> _byteLengthInfo;
	vector<ByteLengthInfo> _distInfo;

	//���ѹ�����Ƿ�Ϊ���һ����
	bool _isLast = false;

	FILE* fOut = nullptr;
};