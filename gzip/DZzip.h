#pragma once

#include "HashTable.h"
#include "HuffmanTree.hpp"
#include "Common.h"
struct ByteLengthInfo
{
	unshort _elem;// 可能是原字符，也可能是长度
	unchar _codeLength; //编码位长
	unshort _appearCount;//出现次数
	uint _code; // 编码

	ByteLengthInfo(size_t appearCount = 0)
		:_appearCount(appearCount)
	{}

	//重载+ 因为ByteInfo类型不能直接相加，但我们创建Huffman树时，使用的是ByteInfo类型的vector，因此需要重载+来创建树
	//加法不能改变元素类型，因此返回值需要强制类型转换为ByteInfo
	ByteLengthInfo operator+(const ByteLengthInfo& other)const
	{
		return ByteLengthInfo(_appearCount + other._appearCount);
	}

	//重载> 创建Huffman树需要比较元素出现的次数，但ByteInfo不能直接比较，重载此方法
	bool operator>(const ByteLengthInfo& other)const
	{
		return _appearCount > other._appearCount;
	}

	//重载!= 创建Huffman树需要判断元素出现次数是否为0（是否和传入的vaild一样）
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


//解码表
struct DecodeTable
{
	unchar _decodeLen;//编码位长
	uint _code;//首字符编码
	unshort _lenCount;//相同编码长度个数
	unshort _charIndex;//符号索引
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
	/// huffman相关
	void CompressBlock();
	void StatAppearCount();
	//获取编码位长
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
	/// 	解压缩函数
	/// </summary>
	void GetCodeBitLenInfo(FILE* fIn);
	void GenerateDecode(vector<DecodeTable>& decodeTable, vector<ByteLengthInfo>& elemInfo);
	unshort UNcompressSymbol(FILE* fIn, vector<ByteLengthInfo>& codeInfo, vector<DecodeTable>& decTable, unchar& ch, unchar& bitCount);
	void GetNextBit(FILE* fIn, unshort& code, unchar& ch, unchar& bitCount);
private:
	unchar* _pWin;
	HashTable _ht;

	//当窗口内剩余字符数小于MIN_LOOKHEAD时，需重新读取
	const static unshort MIN_LOOKHEAD = MIN_MATCH + MAX_MATCH + 1;
	//当距离大于MAX_DIST时，不再向前查询
	const static unshort MAX_DIST = WIN_SIZE - MIN_LOOKHEAD;

	/// <summary>
	///保存lz77的结果，将来交给huffman来进一步压缩 
	vector<unchar> _byteLengthLZ77;
	vector<unshort> _distLZ77;
	vector<unchar> _flagLZ77;

	//DZzip并不是将文件整体压缩完成之后交给Huffman
	//而是按块进行，当LZ77压缩长度达到0X8000时，此时就需要将该结果Huffman树压缩
	const unshort BUFF_SIZE = 0x8000;

	vector<ByteLengthInfo> _byteLengthInfo;
	vector<ByteLengthInfo> _distInfo;

	//标记压缩的是否为最后一个块
	bool _isLast = false;

	FILE* fOut = nullptr;
};