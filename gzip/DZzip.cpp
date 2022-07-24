#pragma warning(disable : 4996)
#include "DZzip.h"




// 区间码结构体
struct IntervalSolution
{
	unshort code;               // 区间编号
	unchar extraBit;           // 扩展码
	unshort interval[2];        // 改区间中包含多少个数字
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



DZzip::DZzip() : _pWin(new unchar[2 * WIN_SIZE])
{
	_byteLengthLz77.reserve(BUFF_SIZE);
	_distLz77.reserve(BUFF_SIZE);
	_flagLz77.reserve(BUFF_SIZE / 8);

	_byteLengthInfo.resize(286);
	for (size_t i = 0; i < _byteLengthInfo.size(); ++i) {
		_byteLengthInfo[i]._appareCount = 0;
		_byteLengthInfo[i]._elem = i;
		_byteLengthInfo[i]._code = 0;
		_byteLengthInfo[i]._codeLength = 0;
	}

	// 距离0 ~29 之间
	_distInfo.resize(30);
	for (size_t i = 0; i < _distInfo.size(); i++) {
		_distInfo[i]._appareCount = 0;
		_distInfo[i]._elem = i;
		_distInfo[i]._code = 0;
		_distInfo[i]._codeLength = 0;
	}
}



DZzip::~DZzip() {
	if (_pWin) {
		delete[] _pWin;
		_pWin = nullptr;
	}
}

void DZzip::Deflated(const string& fileName) {
	// LZ77 是通用类型的压缩算法,文本文件以及二进制格式的文件都可以出处理
	// 直接以二进制的方式打开
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "待压缩文件打开失败!" << std::endl;
		return;
	}

	// 文件大小 <= MIN_MATCH 时不进行压缩
	fseek(fIn, 0, SEEK_END);
	unll fileSize = ftell(fIn);
	std::cout << fileSize << std::endl;
	fseek(fIn, 0, SEEK_SET);

	if (fileSize <= MIN_MATCH) {
		std::cout << "文件大小小于等于三个字节不压缩!" << std::endl;
		fclose(fIn);
		return;
	}

	// 进行压缩
	// 读取一个窗口的数据
	unll lookahead = fread(_pWin, 1, 2 * WIN_SIZE, fIn);

	unshort hashAddr = 0;
	unshort matchHead = 0;
	// 先使用 MIN_MATCH -1 个字节计算 hashAddr ;
	for (unchar i = 0; i < MIN_MATCH - 1; i++) {
		_ht.InsertString(hashAddr, _pWin[i], i, matchHead);
	}

	unshort start = 0;
	unshort matchLength = 0; // 串匹配长度
	unshort matchDist = 0; // 匹配串距离


	// 压缩后文件
	std::string compressFileName = GetFileInfoHead(fileName);
	compressFileName += ".dzp";
	fout = fopen(compressFileName.c_str(), "wb");


	/*
		// 保存区分源字节和产长度距离对的比特位信息,以及源文件后缀信息
		std::string tmpInfoFileName = GetFileInfoHead(fileName);
		tmpInfoFileName += ".fzp";
		FILE* fFlag = fopen(tmpInfoFileName.c_str(), "wb");
	*/

	// 另外搞个零时文件存源文件后缀名
	std::string fileSuff = GetFileInfoHead(fileName);
	fileSuff += ".dzsuf";
	FILE* Suf = fopen(fileSuff.c_str(), "wb");
	std::string suffix = (GetFileSuffix(fileName) + "\n");
	fwrite(suffix.c_str(), suffix.size(), 1, Suf);
	fclose(Suf);


	unchar bitInfo = 0;
	unchar bitCount = 0;

	while (lookahead) {
		matchHead = 0;
		matchLength = 0;
		matchDist = 0;
		// 1. 先将三个字节一组,插入hashtable
		_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);

		if (matchHead) { // 找到匹配了,需要去找最长匹配
			// 找最长匹配
			matchLength = LongestMatch(matchHead, matchDist, start);
		}

		if (matchLength < MIN_MATCH) {
			// 没有找到匹配
			//fputc(_pWin[start], fout);
			SaveLZ77Result(_pWin[start], 0, bitInfo, bitCount,lookahead);
			start++;
			lookahead--;
		}
		else {
			// 在前文中找到最长的匹配,因此需要将重复出现的字符替换为<长度,距离>对
			// fputc(matchLength - 3, fout);
			// fwrite(&matchDist, 1, sizeof(matchDist), fout);
			SaveLZ77Result(matchLength, matchDist, bitInfo, bitCount,lookahead);
			lookahead -= matchLength;
			matchLength -= 1;

			// 注意: 匹配的字符串要向hashtable中插入
			start++;
			while (matchLength--) {
				_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);
				start++;
			}
		}

		// 注意 大文件时问题,当先行缓冲区中的数据小于MIN_LOOKHEAD时,
		// 就需要往窗口中补充数据
		// 目的 : 让匹配达到最优!
		if (lookahead <= MIN_LOOKHEAD) {
			FillWindow(fIn, lookahead, start);
		}

	}
	if (_byteLengthLz77.size() < BUFF_SIZE) {
		// 注意 bitInfo 可能不够8bit
		if (bitCount > 0 && bitCount < 8) {
			bitInfo <<= (8 - bitCount);
			_flagLz77.push_back(bitInfo);
			//fputc(bitInfo, fFlag);
		}
		isLast = true;
		CompressBlock();
	}
	

	//fwrite(&fileSize, 1, sizeof(fileSize), fFlag); // 将文件大小信息写入fFlag 的末尾

	fclose(fIn);
	fclose(fout);
	//fclose(fFlag);
}


/*
	获取最长匹配长度
*/
unshort DZzip::LongestMatch(unshort matchHead, unshort& matchDist, unshort start) {
	unshort curMatchDist = 0;
	unshort curMatchLength = 0;
	unshort bestMatchLength = 0;
	unchar maxMatchCount = 255; // 防止插入时越界解决后值覆盖的问题从而导致环的形成 

	// 向左侧找的时候不能找的太远,最远找start 找 MAX_DICT 的距离
	unshort limit = start > MAX_DIST ? start - MAX_DIST : 0;

	do {
		unchar* pstart = _pWin + start;
		unchar* pend = pstart + MAX_MATCH;

		curMatchDist = 0;
		curMatchLength = 0;

		unchar* pbegin = _pWin + matchHead;
		while (pstart < pend && *pstart == *pbegin) {
			pstart++;
			pbegin++;
			curMatchLength++;
		}
		curMatchDist = start - matchHead;

		if (curMatchLength > bestMatchLength) {
			bestMatchLength = curMatchLength;
			matchDist = curMatchDist;
		}

	} while ((_ht.GetPrevMatch(matchHead) < limit) && maxMatchCount--);

	if (matchDist > MAX_DIST) {
		bestMatchLength = 0;
	}
	return bestMatchLength;
}


// 填写比特位信息
void DZzip::SaveLZ77Result(unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount,unll lookahead) {
	// 当matchDist == 0 时,matchLength 表示原字符
	// 当matchDist > 0 时,macthLength 表示长度
	
	bitInfo <<= 1;
	if (matchDist == 0) {
		_byteLengthLz77.push_back(matchLength);
	}
	else {
		bitInfo |= 1;
		_distLz77.push_back(matchDist);
		_byteLengthLz77.push_back(matchLength - 3);
	}
	bitCount++;
	if (bitCount == 8) {
		//fputc(bitInfo, fFlag);
		_flagLz77.push_back(bitInfo);
		bitInfo = 0;
		bitCount = 0;
	}
	if (BUFF_SIZE == _byteLengthLz77.size()) {
		// 将该块的数据交给huffman树进行进一步压缩
		if (bitCount > 0 && bitCount < 8) {
			bitInfo <<= (8 - bitCount);
			_flagLz77.push_back(bitInfo);
		}
		if (0 == lookahead) {
			isLast = true;
		}
		CompressBlock();
	}

}


// 将右窗中的所有数据搬移到左边窗口中
// 更新hash表
// 往左窗中补充32k的数据
void DZzip::FillWindow(FILE* fIn, unll& lookahead, unshort& start) {
	if (start >= (WIN_SIZE + MAX_DIST)) {
		// 1. 需要将 右窗口中数据搬移到左边窗口
		// 因为从start位置往前找的时候,最远只能找MAX_DIST的距离
		// 即刚好找到右窗口的左边界,此时左窗口的数据就没有用了
		memcpy(_pWin, _pWin + WIN_SIZE, WIN_SIZE);
		start -= WIN_SIZE;
		// 2. 刚才在搬移的过程中,将右窗中查找缓冲区的数据也搬移到
		// 了左窗查找缓冲区中的的字节的下标发生了变化,而hash表中
		// 存储的就是查找缓冲区的中的三个字节一组首字节在窗口中的下标
		// 即该下标发生了变化,必须更新hash表
		_ht.UpdateTable();

		// 3. 往右窗口中补充WIN_SIZE 的数据
		if (!feof(fIn)) {
			lookahead += fread(_pWin + WIN_SIZE, 1, WIN_SIZE, fIn);
		}
	}
}




//////////////////////////////////////////////////////////////////////////////////////////// 
// huffman 
// todo 
void DZzip::CompressBlock() {

	// 0. 清空前一块的压缩信息
	ClearPrevStatInfo();
	// 1. 统计每个字节出现的次数
	StatAppearCount();
	// 2. 创建huffman树
	HuffmanTree<ByteLengthInfo> byteLengthTree(_byteLengthInfo, ByteLengthInfo());
	
	HuffmanTree<ByteLengthInfo> distTree(_distInfo, ByteLengthInfo());
	// 3. 获取编码位长
	GetCodeLen(byteLengthTree.GetRoot(), _byteLengthInfo);
	GetCodeLen(distTree.GetRoot(), _distInfo);
	// 4. 生成huffman 编码: 按照编码步长为第一字段,字节大小为第二字段排序
	GenerateHuffmanCode(_byteLengthInfo);
	GenerateHuffmanCode(_distInfo);

	// 5. 写压缩时需要用到的位的信息 --- 写编码位长
	WriteInfo(fout);
	
	// 6. 压缩
	unchar bitInfo = 0;
	unchar bitCount = 0;
	size_t flagIdx = 0;
	size_t distIdx = 0;
	unchar compressBiteInfo = 0, compressBiteCount = 0;
	for (size_t i = 0; i < _byteLengthLz77.size(); i++) {
		if (0 == bitCount) {
			bitInfo = _flagLz77[flagIdx++];
			bitCount = 8;
		}
		if (bitInfo & 0x80) {
			// byteLengthLz77[i] 是长度距离对
			CompressLengthDist(_byteLengthLz77[i], _distLz77[distIdx++], compressBiteInfo, compressBiteCount);
		}
		else {
			CompressByte(_byteLengthLz77[i], compressBiteInfo, compressBiteCount);
		}
		bitInfo <<= 1;
		bitCount -= 1;
	}
	// 压缩一个256表示这个块已经结束了
	CompressByte(256, compressBiteInfo, compressBiteCount);
	if (compressBiteCount > 0 && compressBiteCount < 8) {
		compressBiteInfo <<= (8 - compressBiteCount);
		fputc(compressBiteInfo, fout);
	}
	// 将前一次Lz77的结果清空
	_byteLengthLz77.clear();
	_distLz77.clear();
	_flagLz77.clear();
}



void DZzip::ClearPrevStatInfo() {
	// 清空前一次huffman压缩完成之后_byteLengthInfo
	for (auto& e : _byteLengthInfo) {
		e._appareCount = 0;
		e._codeLength = 0;	
		e._code = 0;
	}

	// 清空前一次huffman压缩完成之后_distInfo
	for (auto& e : _distInfo) {
		e._appareCount = 0;
		e._codeLength = 0;
		e._code = 0;
	}
}

void DZzip::StatAppearCount() {

	size_t index = 0;
	unchar biteInfo = 0;
	unchar biteCount = 0;
	unshort distIndex = 0;
	for (size_t i = 0; i < _byteLengthLz77.size(); ++i) {
		if (0 == biteCount) {
			biteInfo = _flagLz77[index++];
			biteCount = 8;
		}

		if (biteInfo & 0x80) {
			// 遇到的是长度距离对
			// huffman 树并没有对长度距离对进行压缩
			//而是对长度和距离进行了区分,最终雷勇长度对应的区间码来构建huffman树的
			// 此处: 并不需要统计长度和距离出现的次数,需要统计长度和距离对应取件码出现的次数
			_byteLengthInfo[GetIndexLengthCode(_byteLengthLz77[i])]._appareCount++;
			_distInfo[GetIndexDistCode(_distLz77[distIndex++])]._appareCount++;
		}
		else {
			// 遇到的是源字符
			_byteLengthInfo[_byteLengthLz77[i]]._appareCount++;
		}
		biteInfo <<= 1;
		biteCount -= 1;
	}
	// 让256 出现一次
	_byteLengthInfo[256]._appareCount = 1;
}









unshort DZzip::GetIndexLengthCode(unshort matchLength) {
	matchLength += 3;
	size_t size = sizeof(lengthInterval) / sizeof(lengthInterval[0]);
	for (size_t i = 0; i < size; i++) {
		if (matchLength >= lengthInterval[i].interval[0] && matchLength <= lengthInterval[i].interval[1]) {
			return lengthInterval[i].code;
		}
	}
	assert(false);
	return 0;
}


unshort DZzip::GetIndexDistCode(unshort matchLength) {
	size_t size = sizeof(distInterval) / sizeof(distInterval[0]);
	for (size_t i = 0; i < size; i++) {
		if (matchLength >= distInterval[i].interval[0] && matchLength <= distInterval[i].interval[1]) {
			return distInterval[i].code;
		}
	}
	assert(false);
	return 0;
}



void DZzip::GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo) {
	unchar len = 0;
	GetCodeLen(root, elemInfo, len);
}
void DZzip::GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo,unchar len) {
	if (nullptr == root) {
		return;
	}
	if (nullptr == root->_left && nullptr == root->_right) {
		elemInfo[root->_weight._elem]._codeLength = len;
		return;
	}
	++len;
	GetCodeLen(root->_left, elemInfo, len);
	GetCodeLen(root->_right, elemInfo, len);

}


void DZzip::GenerateHuffmanCode(vector<ByteLengthInfo>& elemInfo) {
	// 1. 按照编码位长为第一字段,字节大小为第二字段进行排序
	vector<ByteLengthInfo> temp(elemInfo);
	sort(temp.begin(), temp.end());

	// 2. 找到第一个编码位长不为0的位置
	size_t index = 0;
	for (; index < temp.size(); index++) {
		if (temp[index]._codeLength > 0) {
			break;
		}
	}
	assert(index < temp.size());

	elemInfo[temp[index]._elem]._code = 0;
	temp[index]._code = 0;

	size_t levelCount = 1;
	for (size_t i = index + 1; i < temp.size(); ++i) {
		// 同义层 前一个+1
		if (temp[i]._codeLength == temp[i - 1]._codeLength) {
			temp[i]._code = temp[i - 1]._code + 1;
			elemInfo[temp[i]._elem]._code = elemInfo[temp[i-1]._elem]._code + 1;
			levelCount++;
		}
		else {
			// 不再同一层, 将上一层的编码 + 层数个数<< 层数差
			temp[i]._code = (temp[i - levelCount]._code + levelCount) << (temp[i]._codeLength - temp[i - 1]._codeLength);
			elemInfo[temp[i]._elem]._code = (elemInfo[temp[i - levelCount]._elem]._code + levelCount) << (elemInfo[temp[i]._elem]._codeLength - elemInfo[temp[i - 1]._elem]._codeLength);
			levelCount = 1;
		}
	}
}



void DZzip::WriteInfo(FILE* fout) {
	// 是否为最后一个块
	if (isLast)
		fputc(0, fout);
	else
		fputc(1, fout);

	// 写字节长度编码位长信息
	for (size_t i = 0; i < _byteLengthInfo.size(); i++) {
		fputc(_byteLengthInfo[i]._codeLength, fout);
	}

	// 写距离的编码位信息
	for (size_t i = 0; i < _distInfo.size(); i++) {
		fputc(_distInfo[i]._codeLength, fout);
	}

}



void DZzip::CompressByte(unshort byte, unchar compressbiteInfo, unchar compressbiteCount) {
	uint code = _byteLengthInfo[byte]._code;
	unchar codeLen =  _byteLengthInfo[byte]._codeLength;
	
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);

}

void DZzip::CompressLengthDist(unshort length, unshort dist, unchar compressbiteInfo, unchar compressbiteCount) {
	// 压缩长度
	unshort index = GetIndexLengthCode(length);
	uint code = _byteLengthInfo[index]._code;
	unchar codeLen = _byteLengthInfo[index]._codeLength;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);

	// 写长度对应额外的比特位
	index = GetIndexLengthCode(length) - 257;
	length += 3;
	code = length - lengthInterval[index].interval[0];
	codeLen = lengthInterval[index].extraBit;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);

	// 压缩距离
	index = GetIndexDistCode(dist);
	code = _distInfo[index]._code;
	codeLen = _distInfo[index]._codeLength;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);
	// 写距离对应的额外比特位
	index = GetIndexDistCode(dist);
	code = dist - distInterval[index].interval[0];
	codeLen = distInterval[index].extraBit;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);
}

void DZzip::WriteCode(uint code, unchar codeLen, unchar compressbiteInfo, unchar compressbiteCount) {
	code <<= (32 - codeLen);
	for (unchar i = 0; i < codeLen; i++) {
		compressbiteInfo <<= 1;
		if (code & 0x80000000) {
			compressbiteInfo |= 1;
		}
		compressbiteCount++;
		if (8 == compressbiteCount) {
			fputc(compressbiteInfo, fout);
			compressbiteInfo = 0;
			compressbiteCount = 0;
		}
		code <<= 1;
	}

}


// 解压文件
void DZzip::UNDeflated(const string& fileName) {

	std::string filesuf = GetFileSuffix(fileName);
	if ("dzp" != filesuf) {
		std::cout << "压缩文件类型应该为filePath.dzp" << std::endl;
		return;
	}

	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cout << "压缩文件打开失败" << std::endl;
		return;
	}


	std::string suffile = GetFileInfoHead(fileName);
	suffile += ".dzsuf";
	FILE* suf = fopen(suffile.c_str(), "rb");
	std::string suffix;
	while (true) {
		unchar ch = fgetc(suf);
		if (ch == '\n') break;
		suffix += ch;
	}
	fclose(suf);


	std::string unCompressFileName = GetFileInfoHead(fileName);
	unCompressFileName += "_zip.";
	unCompressFileName += suffix;
	std::cout << unCompressFileName << std::endl;
	FILE* fout = fopen(unCompressFileName.c_str(), "wb");
	FILE* fR = fopen(unCompressFileName.c_str(), "rb");

	while (true) {
		isLast = fgetc(fIn);

		//1. 获取编码位长信息
		//2. 生成解码表 
		//   处理原字符和长度
		GetCodeBitLenInfo(fIn);
		vector<DecodeTable> byteLengthDT;
		GenerateDecode(byteLengthDT,_byteLengthInfo);

		// 处理距离
		vector<DecodeTable> distDT;
		GenerateDecode(distDT, _distInfo);

		// 解压缩一个块
		unchar ch = 0;
		unchar bitCount = 0;
		while (true) {
			unshort symbol = UNCompressSymbol(fIn, _byteLengthInfo, byteLengthDT, ch, bitCount);
			if (symbol < 256) {
				// 解压缩出来了一个元字符
				fputc(symbol, fout);
			}
			else if (256 == symbol) {
				// 一个块解压缩结束
				break;
			}
			else {
				// 解压缩出来的是长度
				// symobl: 257 ->length: 3
				size_t index = symbol - 257;
				unshort length = lengthInterval[index].interval[0];

				// 获取额外对应的编码
				unchar extraBitLen = lengthInterval[index].extraBit;
				unshort value = 0;
				while (extraBitLen) {
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}
				length += value;
				length += 3;
				// 解压缩距离
				index = UNCompressSymbol(fIn, _distInfo, distDT, ch, bitCount);
				unshort dist = distInterval[index].interval[0];

				// 获取距离额外比特位
				extraBitLen = distInterval[index].extraBit;
				value = 0;
				while (extraBitLen) {
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}
				dist += value;
				fseek(fR, 0 - dist, SEEK_END);

				while (length) {
					unchar ch = fgetc(fR);
					fputc(ch, fout);
					fflush(fout);
					length--;
				}
			}
		}
		// 是最后一个块
		if (isLast) break;
	}


	fclose(fIn);
	fclose(fout);
	fclose(fR);
}

void DZzip::GetCodeBitLenInfo(FILE* fIn) {
	// 获取元字符和产犊的编码位长信息
	_byteLengthInfo.clear();
	for (size_t i = 0; i < 286; ++i) {
		// 只保存编码长度位长不为0的字节和长度位信息
		unchar codeLen = fgetc(fIn);
		if (0 != codeLen) {
			ByteLengthInfo elem;
			elem._codeLength = codeLen;
			elem._elem = i;
			_byteLengthInfo.push_back(elem);
		}
	}
	_distInfo.clear();
	for (size_t i = 0; i < sizeof(distInterval) / sizeof(distInterval[0]); i++) {
		unchar codeLen = fgetc(fIn);
		if (0 != codeLen) {
			ByteLengthInfo elem;
			elem._codeLength = codeLen;
			elem._elem = i;
			_distInfo.push_back(elem);
		}
	}
}

void DZzip::GenerateDecode(vector<DecodeTable>& decodeTab, vector<ByteLengthInfo>& elemInfo) {
	//1. 统计不同编码位长再elemInfo 中出现的次数
	std::map<unchar, unshort> m;
	for (auto& e : elemInfo) {
		m[e._codeLength]++;
	}

	std::sort(elemInfo.begin(), elemInfo.end());

	size_t index = 0;
	for (auto& e : m) {
		DecodeTable decode;
		decode._decodeLen = e.first;
		decode._lenCount = e.second;
		decode._code = 0;
		if (index == 0) {
			decode._code = 0;
			decode._charIndex = 0;
		}
		else {
			DecodeTable& prev = decodeTab[index - 1];
			decode._code = (prev._code + prev._lenCount) << (decode._decodeLen - prev._decodeLen);
			decode._charIndex = prev._charIndex + prev._lenCount;
		}
		decodeTab.push_back(decode);
		index++;
	}
}



unshort DZzip::UNCompressSymbol(FILE* fIn, vector<ByteLengthInfo>& codeInfo, vector<DecodeTable>& decTable, unchar& ch, unchar& bitCount) {
	unshort i = 0;
	unshort codeLen = decTable[0]._decodeLen;
	unshort code = 0;
	while (codeLen--) {
		GetNextBit(fIn, code, ch, bitCount);
	}

	unshort num = 0;
	while ((num = code - decTable[i]._code) >= decTable[i]._lenCount) {
		i++;
		unshort lenGap = decTable[i]._decodeLen - decTable[i - 1]._decodeLen;
		while (lenGap--) {
			GetNextBit(fIn, code, ch, bitCount);
		}
	}
	num += decTable[i]._charIndex;
	return codeInfo[num]._elem;
}




void DZzip::GetNextBit(FILE* fIn, unshort& code, unchar& ch, unchar& bitCount) {
	if (0 == bitCount) {
		ch = fgetc(fIn);
		bitCount = 8;
	}

	code <<= 1;

	if (ch & 0x80) {
		code |= 1;
	}

	ch <<= 1;
	bitCount--;
}