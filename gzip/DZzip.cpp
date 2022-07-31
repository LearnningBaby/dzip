#pragma warning(disable : 4996)
#include"DZzip.h"
#include<map>


// 区间码结构体
struct IntervalSolution
{
	unshort code;               // 区间编号
	unchar extraBit;           // 扩展码
	unshort interval[2];        // 该区间中包含多少个数字
};

/*************************************************************/
// 距离区间码
static IntervalSolution distInterval[] = {
	{ 0, 0, { 1, 1 } },
	{ 1, 0, { 2, 2 } },
	{ 2, 0, { 3, 3 } },
	{ 3, 0, { 4, 4 } },
	{ 4, 1, { 5, 6 } },
	{ 5, 1, { 7, 8 } },
	{ 6, 2, { 9, 12 } },
	{ 7, 2, { 13, 16 } },
	{ 8, 3, { 17, 24 } },
	{ 9, 3, { 25, 32 } },
	{ 10, 4, { 33, 48 } },
	{ 11, 4, { 49, 64 } },
	{ 12, 5, { 65, 96 } },
	{ 13, 5, { 97, 128 } },
	{ 14, 6, { 129, 192 } },
	{ 15, 6, { 193, 256 } },
	{ 16, 7, { 257, 384 } },
	{ 17, 7, { 385, 512 } },
	{ 18, 8, { 513, 768 } },
	{ 19, 8, { 769, 1024 } },
	{ 20, 9, { 1025, 1536 } },
	{ 21, 9, { 1537, 2048 } },
	{ 22, 10, { 2049, 3072 } },
	{ 23, 10, { 3073, 4096 } },
	{ 24, 11, { 4097, 6144 } },
	{ 25, 11, { 6145, 8192 } },
	{ 26, 12, { 8193, 12288 } },
	{ 27, 12, { 12289, 16384 } },
	{ 28, 13, { 16385, 24576 } },
	{ 29, 13, { 24577, 32768 } }
};

// 长度区间码
static IntervalSolution lengthInterval[] = {
	{ 257, 0, { 3, 3 } },
	{ 258, 0, { 4, 4 } },
	{ 259, 0, { 5, 5 } },
	{ 260, 0, { 6, 6 } },
	{ 261, 0, { 7, 7 } },
	{ 262, 0, { 8, 8 } },
	{ 263, 0, { 9, 9 } },
	{ 264, 0, { 10, 10 } },
	{ 265, 1, { 11, 12 } },
	{ 266, 1, { 13, 14 } },
	{ 267, 1, { 15, 16 } },
	{ 268, 1, { 17, 18 } },
	{ 269, 2, { 19, 22 } },
	{ 270, 2, { 23, 26 } },
	{ 271, 2, { 27, 30 } },
	{ 272, 2, { 31, 34 } },
	{ 273, 3, { 35, 42 } },
	{ 274, 3, { 43, 50 } },
	{ 275, 3, { 51, 58 } },
	{ 276, 3, { 59, 66 } },
	{ 277, 4, { 67, 82 } },
	{ 278, 4, { 83, 98 } },
	{ 279, 4, { 99, 114 } },
	{ 280, 4, { 115, 130 } },
	{ 281, 5, { 131, 162 } },
	{ 282, 5, { 163, 194 } },
	{ 283, 5, { 195, 226 } },
	{ 284, 5, { 227, 257 } },
	{ 285, 0, { 258, 258 } } };
/******************************************************************/



//构造LZ77,压缩窗口由于距离2字节，所以最大距离为65535，一次读入65535字符
DZzip::DZzip()
	:_pWin(new unchar[2 * WIN_SIZE])
{
	_byteLengthLZ77.reserve(BUFF_SIZE);
	_distLZ77.reserve(BUFF_SIZE);
	_flagLZ77.reserve(BUFF_SIZE / 8);

	_byteLengthInfo.resize(286);
	for (size_t i = 0; i < _byteLengthInfo.size(); i++)
	{
		_byteLengthInfo[i]._appearCount = 0;
		_byteLengthInfo[i]._elem = i;
		_byteLengthInfo[i]._code = 0;
		_byteLengthInfo[i]._codeLength = 0;
	}

	//将距离区间写入
	_distInfo.resize(30);
	for (size_t i = 0; i < _distInfo.size(); i++)
	{
		_distInfo[i]._appearCount = 0;
		_distInfo[i]._elem = i;
		_distInfo[i]._code = 0;
		_distInfo[i]._codeLength = 0;
	}
}

DZzip::~DZzip()
{
	if (_pWin)
	{
		delete[] _pWin;
		_pWin = nullptr;
	}
}

//压缩文件
void DZzip::Deflate(const string& fileName)
{
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "待文件打开失败" << endl;
		return;
	}


	//获取文件大小
	fseek(fIn, 0, SEEK_END);
	unll fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);


	//如果文件大小小于等于3字节，不压缩
	if (fileSize < MIN_MATCH)
	{
		cout << "文件太小,无法压缩" << endl;
		fclose(fIn);
		return;
	}

	//往pwin读取数据
	unll lookahead = fread(_pWin, 1, 2 * WIN_SIZE, fIn);


	unshort matchHead = 0;
	unshort hashAddr = 0;//使用MinMatch - 1计算hashAddr

	//从第三个开始查找，所以先把前两个放入哈希表，在下面的循环中边插入边查找
	for (unchar i = 0; i < MIN_MATCH - 1; i++)
	{
		_ht.InsertString(hashAddr, _pWin[i], i, matchHead);
	}
	unshort start = 0;


	string zFileName = GetFileName(fileName);
	zFileName += ".dzp";
	//压缩
	fOut = fopen(zFileName.c_str(), "wb");

	//先将文件的后缀写入
	string filePostFix = GetFilePostFix(fileName);
	filePostFix += '\n';
	fwrite(filePostFix.c_str(), 1, filePostFix.size(), fOut);

	//比特位信息
	unchar bitInfo = 0;
	//对比特位进行计数，写满8位之后插入文件
	unchar bitCount = 0;
	while (lookahead)
	{
		//长度
		unshort matchLength = 0;
		//距离
		unshort matchDist = 0;

		matchHead = 0;
		//1.先将三个字符一组，在哈希表进行插入,插入第三个字符
		_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);

		//找到匹配(matchHead != 0 ,说明在前文中找到匹配)
		//matchHead就是离当前要匹配的三个字节最近的一次匹配
		if (matchHead != 0)
		{
			//找最长匹配
			//matchLength = 0;
			//matchDist = 0
			matchLength = LongestMatch(matchHead, matchDist, start);
			//如果matchHead = 0,说明前文无匹配
		}

		if (matchLength < MIN_MATCH)
		{
			//没有找到匹配，存入第一个字符
			//fputc(_pWin[start], fOut);
			SaveLZ77Result(_pWin[start], 0, bitInfo, bitCount, lookahead);
			start++;
			lookahead--;
		}
		else
		{
			//找到了最长匹配，因此需要将重复出现的字符串替换为<长度，距离>对
			//fputc(matchLength - 3, fOut);
			//fwrite(&matchDist, 1, sizeof(matchDist), fOut);



			SaveLZ77Result(matchLength, matchDist, bitInfo, bitCount, lookahead);
			lookahead = lookahead - matchLength;
			matchLength -= 1;
			start++;
			//将压缩的字符也全部放入哈希桶中
			while (matchLength--)
			{
				//放入哈希桶的是存入字符的后两个字符
				_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);
				start++;
			}
		}

		//当先行缓冲区中甚于的数据小于MIN_LOOKHEAD时，需要读取
		//如果文件小于64K
		if (lookahead <= MIN_LOOKHEAD)
		{
			FillWindow(fIn, lookahead, start);
		}
	}

	//最后可能不足8位，把最后的写入文件
	if (_byteLengthLZ77.size() < BUFF_SIZE)
	{

		if (bitCount > 0 && bitCount < 8)
		{
			//将信息放在高位上
			bitInfo <<= (8 - bitCount);
			//fputc(bitInfo, fFlag);
			_flagLZ77.push_back(bitInfo);
		}

		_isLast = true;
		CompressBlock();
	}

	//向标记文件写入压缩源文件大小
	//fwrite(&fileSize, 1,sizeof(fileSize), fFlag);
	fclose(fIn);
	fclose(fOut);
}

//顺着matchHead链，往前找最长匹配
//matchHead前面最近匹配   matchDist距离    start字符本次下标
unshort DZzip::LongestMatch(unshort matchHead, unshort& matchDist, unshort start)
{
	unshort curMatchLength = 0;
	unshort maxLength = 0;
	unshort curMatchDist = 0;

	//最大匹配次数
	unchar maxMatchCount = 255;

	//向左侧找的最大距离
	unshort limit = start > MAX_DIST ? start - MAX_DIST : 0;
	do
	{
		curMatchLength = 0;
		curMatchDist = 0;
		unchar* pstart = _pWin + start;//指向本次压缩字符串
		unchar* pend = pstart + MAX_MATCH;//最长匹配
		unchar* pbegin = _pWin + matchHead;//指向获取的匹配字符串下标
		while (pstart < pend && *pstart == *pbegin)
		{
			pstart++;
			pbegin++;
			curMatchLength++;
		}
		curMatchDist = start - matchHead;

		if (maxLength < curMatchLength)
		{
			maxLength = curMatchLength;
			matchDist = curMatchDist;
		}
	} while ((_ht.GetPrevMatch(matchHead) >= limit) && maxMatchCount--);//获取上次matchHead

	if (matchDist > MAX_DIST)
	{
		maxLength = 0;
	}

	return maxLength;
}

void DZzip::SaveLZ77Result(unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount, unll lookahead)
{
	//当matchDist为0的时候，matchLength表示原子符
	//当matchDist不为0的时候，matchLength表示长度

	bitInfo <<= 1;
	//如果长度大于0，说明一定是长度，距离对，将该位改写为1,
	//在读时表示该位与该位后两位为长度距离对
	if (matchDist > 0)
	{
		bitInfo |= 1;
		_byteLengthLZ77.push_back(matchLength - 3);
		_distLZ77.push_back(matchDist);
	}
	else
	{
		_byteLengthLZ77.push_back(matchLength);
	}

	bitCount++;
	//该字节已经填充满，写入文件
	if (bitCount == 8)
	{
		//fputc(bitInfo, fFlag);
		_flagLZ77.push_back(bitInfo);
		bitInfo = 0;
		bitCount = 0;
	}

	if (BUFF_SIZE == _byteLengthLZ77.size())
	{
		if (bitCount > 0 && bitCount < 8)
		{
			bitInfo <<= (8 - bitCount);
			_flagLZ77.push_back(bitInfo);
		}
		//将该块数据交给Huffman树进一步压缩

		if (0 == lookahead)
		{
			_isLast = true;
		}
		CompressBlock();
	}
}

void DZzip::FillWindow(FILE* fIn, unll& lookahead, unshort& start)
{
	//如果start在缓冲区后半部分，说明是个大文件，读取部分快要压缩完毕，需要新读取
	if (start >= WIN_SIZE + MAX_DIST)
	{
		//将右窗中的数据全部放入左窗
		memcpy(_pWin, _pWin + WIN_SIZE, WIN_SIZE);
		start -= WIN_SIZE;

		//更新哈希表
		//搬移过程，下标发生变化
		_ht.UpdateTable();

		//往右窗中补充32k数据
		if (!feof(fIn))
		{
			lookahead += fread(_pWin + WIN_SIZE, 1, WIN_SIZE, fIn);
		}
	}
}



/////////////////////////////////////////////
//huffman相关
void DZzip::CompressBlock()
{
	//1.统计每个字节出现的次数
	StatAppearCount();

	//2.创建Huffman树
	//后一个参数用来判断_byteLengthInfo中的数据是否在压缩文件中被写入，没有写的数据不创建Huffman树
	HuffmanTree<ByteLengthInfo> lengthTree(_byteLengthInfo, ByteLengthInfo());
	HuffmanTree<ByteLengthInfo> distTree(_distInfo, ByteLengthInfo());

	//3.获取编码位长:每个叶子节点的高度
	//4.生成Huffman编码
	//获取原字符和长度对应的huffman中编码位长及具体编码
	GetCodeLen(lengthTree.GetRoot(), _byteLengthInfo);
	GenerateHuffmanCode(_byteLengthInfo);

	//获取距离对应的huffman中编码位长及具体编码
	GetCodeLen(distTree.GetRoot(), _distInfo);
	GenerateHuffmanCode(_distInfo);

	//5.写解压缩需要用到的信息 - 编码位长
	WriteInfo(fOut);

	//6.压缩
	unchar bitInfo = 0;
	unchar bitCount = 0;
	size_t flagIdx = 0;
	size_t distIdx = 0;
	unchar compressBiteInfo = 0, compressBiteCount = 0;
	for (size_t i = 0; i < _byteLengthLZ77.size(); i++)
	{
		if (0 == bitCount)
		{
			bitInfo = _flagLZ77[flagIdx++];
			bitCount = 8;
		}

		if (bitInfo & 0x80)
		{
			//_byteLengthLZ77[i]是长度
			//压缩一个长度距离对,
			CompressLengthDist(_byteLengthLZ77[i], _distLZ77[distIdx++], compressBiteInfo, compressBiteCount);

		}
		else
		{
			//_byteLengthLZ77[i]是原字符
			//压缩原字符
			CompressByte(_byteLengthLZ77[i], compressBiteInfo, compressBiteCount);
		}

		bitInfo <<= 1;
		bitCount--;
	}

	CompressByte(256, compressBiteInfo, compressBiteCount);
	//压缩256，表示块结束了

	//读取的编码不足8个，但块中再没有数据读取
	if (compressBiteCount != 0)
	{
		compressBiteInfo <<= (8 - compressBiteCount);
		fputc(compressBiteInfo, fOut);
	}

	//将前一次LZ77结果清空
	_byteLengthLZ77.clear();
	_distLZ77.clear();
	_flagLZ77.clear();
}


void DZzip::ClearPrevStatInfo()
{
	//清空前一次Huffman压缩完成之后_byteLengthInfo
	for (auto& e : _byteLengthInfo)
	{
		e._appearCount = 0;
		e._codeLength = 0;
		e._code = 0;
	}

	for (auto& e : _distInfo)
	{
		e._appearCount = 0;
		e._codeLength = 0;
		e._code = 0;
	}
}


void DZzip::StatAppearCount()
{
	//清空前一个块的压缩信息
	ClearPrevStatInfo();

	size_t index = 0;
	unchar biteInfo = 0;
	unchar biteCount = 0;
	unshort distIndex = 0;
	for (size_t i = 0; i < _byteLengthLZ77.size(); i++)
	{
		if (0 == biteCount)
		{
			biteInfo = _flagLZ77[index++];
			biteCount = 8;
		}

		if (biteInfo & 0x80)
		{
			//遇到的是长度
			//huffman树并没有对长度距离对进行压缩
			//而是对长度和距离进行区间的划分，最终利用长度和距离对应的区间码来构建Huffman树
			//不需要统计  长度  和  距离  出现的次数，统计它们的区间码出现的次数
			_byteLengthInfo[GetIndexLengthCode(_byteLengthLZ77[i])]._appearCount++;
			_distInfo[GetIndexDistCode(_distLZ77[distIndex++])]._appearCount++;
		}
		else
		{
			//遇到的是原字符
			_byteLengthInfo[_byteLengthLZ77[i]]._appearCount++;
		}

		biteCount--;
		biteInfo <<= 1;
	}

	//让256出现一次,256是块结束标记，当解压缩到256时，块压缩结束了
	_byteLengthInfo[256]._appearCount = 1;
}


void DZzip::GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo)
{
	// 层数    目的：取得编码位长
	unshort len = 0;
	GetCodeLen(root, elemInfo, len);
}

void DZzip::GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo, unchar len)
{
	if (nullptr == root)
	{
		return;
	}

	if (nullptr == root->_left && nullptr == root->_right)
	{
		elemInfo[root->_weight._elem]._codeLength = len;
		return;
	}

	len++;

	GetCodeLen(root->_left, elemInfo, len);
	GetCodeLen(root->_right, elemInfo, len);
}

void DZzip::GenerateHuffmanCode(vector<ByteLengthInfo>& elemInfo)
{
	//1.按编码位长为第一字段，字节大小为第二字段进行排序
	vector<ByteLengthInfo> temp(elemInfo);
	sort(temp.begin(), temp.end());

	//2.找到第一个编码位长不为0的位置
	size_t index = 0;
	for (; index < temp.size(); index++)
	{
		if (temp[index]._codeLength > 0)
		{
			break;
		}
	}
	assert(index < temp.size());

	elemInfo[temp[index]._elem]._code = 0;
	temp[index]._code = 0;
	size_t levelCount = 1;
	for (size_t i = index + 1; i < temp.size(); i++)
	{
		// 同一层编码：前一个元素加1
		if (temp[i]._codeLength == temp[i - 1]._codeLength)
		{
			//temp[i]._code = temp[i - 1]._code + 1;
			elemInfo[temp[i]._elem]._code = elemInfo[temp[i - 1]._elem]._code + 1;
			levelCount++;
		}
		else
		{
			//不在同一层:将上一层的第一个编码+层的个数 << 层数差
			//temp[i]._code = (temp[i - levelCount]._code + levelCount) << (temp[i]._codeLength - temp[i - 1]._codeLength);
			elemInfo[temp[i]._elem]._code = (elemInfo[temp[i - levelCount]._elem]._code + levelCount) << (elemInfo[temp[i]._elem]._codeLength - elemInfo[temp[i - 1]._elem]._codeLength);
			levelCount = 1;
		}
	}
}

void DZzip::WriteInfo(FILE* fOut)
{
	//标记为最后一个块的标记
	if (false == _isLast)
	{
		fputc(0, fOut);
	}
	else
	{
		fputc(1, fOut);
	}

	//写字节长度编码位长信息
	for (size_t i = 0; i < _byteLengthInfo.size(); i++)
	{
		fputc(_byteLengthInfo[i]._codeLength, fOut);
	}

	//写距离编码位长信息
	for (size_t i = 0; i < _distInfo.size(); i++)
	{
		fputc(_distInfo[i]._codeLength, fOut);
	}
}

void DZzip::CompressByte(unshort byte, unchar& compressBiteInfo, unchar& compressBiteCount)
{
	uint code = _byteLengthInfo[byte]._code;
	unchar codeLen = _byteLengthInfo[byte]._codeLength;

	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);
}

void DZzip::CompressLengthDist(unshort length, unshort dist, unchar& compressBiteInfo, unchar& compressBiteCount)
{
	//huffman树中计算length的编码
	unshort index = GetIndexLengthCode(length);
	uint code = _byteLengthInfo[index]._code;
	unchar codeLen = _byteLengthInfo[index]._codeLength;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);

	//写长度对应的额外的比特位
	//index = GetIndexLengthCode(length);
	length += 3;
	index -= 257;
	code = length - lengthInterval[index].interval[0];
	codeLen = lengthInterval[index].extraBit;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);

	//压缩距离
	//huffman树中计算的dist的编码
	index = GetIndexDistCode(dist);
	code = _distInfo[index]._code;
	codeLen = _distInfo[index]._codeLength;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);

	//写距离对应的额外的比特位
	code = dist - distInterval[index].interval[0];
	codeLen = distInterval[index].extraBit;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);


}

void DZzip::WriteCode(uint code, unchar codeLen, unchar& compressBiteInfo, unchar& compressBiteCount)
{
	code <<= (32 - codeLen);
	for (size_t i = 0; i < codeLen; i++)
	{
		compressBiteInfo <<= 1;
		if (code & 0x80000000)
		{
			compressBiteInfo |= 1;
		}
		code <<= 1;
		compressBiteCount++;
		if (8 == compressBiteCount)
		{
			fputc(compressBiteInfo, fOut);
			compressBiteCount = 0;
			compressBiteInfo = 0;
		}
	}
}


unshort DZzip::GetIndexLengthCode(unshort matchLenth)
{
	matchLenth += 3;
	size_t size = sizeof(lengthInterval) / sizeof(lengthInterval[0]);
	for (size_t i = 0; i < size; i++)
	{
		if (matchLenth >= lengthInterval[i].interval[0] && matchLenth <= lengthInterval[i].interval[1])
		{
			return lengthInterval[i].code;
		}
	}
	assert(false);
	return 0;
}

unshort DZzip::GetIndexDistCode(unshort dist)
{
	size_t size = sizeof(distInterval) / sizeof(distInterval[0]);
	for (size_t i = 0; i < size; i++)
	{
		if (dist >= distInterval[i].interval[0] && dist <= distInterval[i].interval[1])
		{
			return distInterval[i].code;
		}
	}
	assert(false);
	return 0;
}



/// /////////////////////////////////////////////////////////////////////
void DZzip::UnDeflate(const string& fileName)
{
	if (GetFilePostFix(fileName) != "dzp")
	{
		cout << "文件格式错误" << endl;
		return;
	}

	FILE* fIn = fopen(fileName.c_str(), "rb");

	if (nullptr == fIn)
	{
		cout << "压缩文件打开失败" << endl;
		return;
	}

	string unCompressFileName(GetFileName(fileName));
	unCompressFileName += "_dzp.";
	string filePostFix;
	GetLine(fIn, filePostFix);
	unCompressFileName += filePostFix;

	FILE* fOut = fopen(unCompressFileName.c_str(), "wb");
	FILE* fR = fopen(unCompressFileName.c_str(), "rb");

	while (true)
	{
		_isLast = fgetc(fIn);

		// 1.获取编码位长信息
		GetCodeBitLenInfo(fIn);

		// 2.生成解码表
		//处理原子符和长度
		vector<DecodeTable> byteLengthDT;
		GenerateDecode(byteLengthDT, _byteLengthInfo);


		//处理距离
		vector<DecodeTable> distDT;
		GenerateDecode(distDT, _distInfo);

		//解压缩
		unchar ch = 0;
		unchar bitCount = 0;
		while (true)
		{
			unshort symbol = UNcompressSymbol(fIn, _byteLengthInfo, byteLengthDT, ch, bitCount);
			if (symbol < 256)
			{
				//解压缩出原子符
				fputc(symbol, fOut);
			}
			else if (256 == symbol)
			{
				//一个块解压缩结束
				break;
			}
			else
			{
				//长度
				size_t index = symbol - 257;
				unshort length = lengthInterval[index].interval[0];
				//length += 3;

				//获取额外对应的编码
				unchar extraBitLen = lengthInterval[index].extraBit;
				unshort value = 0;
				while (extraBitLen)
				{
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}
				length += value;

				//解压缩距离
				index = UNcompressSymbol(fIn, _distInfo, distDT, ch, bitCount);
				unshort dist = distInterval[index].interval[0];

				//获取额外的比特位
				extraBitLen = distInterval[index].extraBit;
				value = 0;
				while (extraBitLen)
				{
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}

				dist += value;

				//之前解压缩的结果可能还在缓冲区，在使用长度距离对还原前，必须将数据放到新压缩的文件中
				fflush(fOut);

				//使fR偏移到解压缩文件末尾(就是我们正在写的这个文件)，向前偏移matchDist个就是对应的相同部分
				fseek(fR, 0 - dist, SEEK_END);

				while (length)
				{
					//从压缩文件中读取一个字节
					char ch = fgetc(fR);
					//将该字节写入解压缩文件
					fputc(ch, fOut);
					//从缓冲区放入内存文件，保证在长度大于距离的情况下最后可以读取到刚放入的字符
					fflush(fOut);
					length--;
				}
			}
		}

		if (_isLast)
		{
			break;
		}
	}
	fclose(fIn);
	fclose(fOut);
	fclose(fR);
}


void DZzip::GetCodeBitLenInfo(FILE* fIn)
{
	//获取原子符和长度的编码位长信息
	_byteLengthInfo.clear();

	for (size_t i = 0; i < 286; i++)
	{
		//只保存编码位长不为0的字节和长度位长信息
		unshort codeLen = fgetc(fIn);
		if (0 != codeLen)
		{
			ByteLengthInfo temp;
			temp._codeLength = codeLen;
			temp._elem = i;
			_byteLengthInfo.push_back(temp);
		}
	}

	//获取距离编码位长信息
	_distInfo.clear();

	for (size_t i = 0; i < 30; i++)
	{
		//只保存编码位长不为0的距离信息
		unshort codeLen = fgetc(fIn);
		if (0 != codeLen)
		{
			ByteLengthInfo temp;
			temp._codeLength = codeLen;
			temp._elem = i;
			_distInfo.push_back(temp);
		}
	}
}

void DZzip::GenerateDecode(vector<DecodeTable>& decodeTable, vector<ByteLengthInfo>& elemInfo)
{
	//1.统计不同编码位长在elemInfo中出现的次数
	//  编码位长必须有序-----map
	//  key: 编码位长   value：出现次数
	std::map<unchar, unshort> m;
	for (auto& e : elemInfo)
	{
		m[e._codeLength]++;
	}

	sort(elemInfo.begin(), elemInfo.end());

	size_t index = 0;

	for (auto& e : m)
	{
		DecodeTable decode;
		decode._decodeLen = e.first;
		decode._lenCount = e.second;
		decode._code = 0;

		if (0 == index)
		{
			decode._code = 0;
			decode._charIndex = 0;

		}
		else
		{
			DecodeTable& prev = decodeTable[index - 1];
			decode._code = (prev._code + prev._lenCount) << (decode._decodeLen - prev._decodeLen);
			decode._charIndex = prev._charIndex + prev._lenCount;
		}
		decodeTable.push_back(decode);

		index++;
	}
}

unshort DZzip::UNcompressSymbol(FILE* fIn, vector<ByteLengthInfo>& codeInfo, vector<DecodeTable>& decTable, unchar& ch, unchar& bitCount)
{
	unshort i = 0;
	unshort codeLen = decTable[0]._decodeLen;
	unshort code = 0;

	//根据解码表中的首元素获取相应的比特位
	while (codeLen--)
	{
		GetNextBit(fIn, code, ch, bitCount);
	}

	unshort num = 0;

	while ((num = code - decTable[i]._code) >= decTable[i]._lenCount)
	{
		i++;
		unshort lenGap = decTable[i]._decodeLen - decTable[i - 1]._decodeLen;
		while (lenGap--)
		{
			GetNextBit(fIn, code, ch, bitCount);
		}

	}

	num += decTable[i]._charIndex;
	return codeInfo[num]._elem;

}

void DZzip::GetNextBit(FILE* fIn, unshort& code, unchar& ch, unchar& bitCount)
{
	if (0 == bitCount)
	{
		ch = fgetc(fIn);
		bitCount = 8;
	}

	code <<= 1;
	if (ch & 0x80)
	{
		code |= 1;
	}

	ch <<= 1;
	bitCount--;
}
