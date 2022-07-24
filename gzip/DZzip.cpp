#pragma warning(disable : 4996)
#include "DZzip.h"




// ������ṹ��
struct IntervalSolution
{
	unshort code;               // ������
	unchar extraBit;           // ��չ��
	unshort interval[2];        // �������а������ٸ�����
};

/*************************************************************/
// ����������
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

// ����������
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

	// ����0 ~29 ֮��
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
	// LZ77 ��ͨ�����͵�ѹ���㷨,�ı��ļ��Լ������Ƹ�ʽ���ļ������Գ�����
	// ֱ���Զ����Ƶķ�ʽ��
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "��ѹ���ļ���ʧ��!" << std::endl;
		return;
	}

	// �ļ���С <= MIN_MATCH ʱ������ѹ��
	fseek(fIn, 0, SEEK_END);
	unll fileSize = ftell(fIn);
	std::cout << fileSize << std::endl;
	fseek(fIn, 0, SEEK_SET);

	if (fileSize <= MIN_MATCH) {
		std::cout << "�ļ���СС�ڵ��������ֽڲ�ѹ��!" << std::endl;
		fclose(fIn);
		return;
	}

	// ����ѹ��
	// ��ȡһ�����ڵ�����
	unll lookahead = fread(_pWin, 1, 2 * WIN_SIZE, fIn);

	unshort hashAddr = 0;
	unshort matchHead = 0;
	// ��ʹ�� MIN_MATCH -1 ���ֽڼ��� hashAddr ;
	for (unchar i = 0; i < MIN_MATCH - 1; i++) {
		_ht.InsertString(hashAddr, _pWin[i], i, matchHead);
	}

	unshort start = 0;
	unshort matchLength = 0; // ��ƥ�䳤��
	unshort matchDist = 0; // ƥ�䴮����


	// ѹ�����ļ�
	std::string compressFileName = GetFileInfoHead(fileName);
	compressFileName += ".dzp";
	fout = fopen(compressFileName.c_str(), "wb");


	/*
		// ��������Դ�ֽںͲ����Ⱦ���Եı���λ��Ϣ,�Լ�Դ�ļ���׺��Ϣ
		std::string tmpInfoFileName = GetFileInfoHead(fileName);
		tmpInfoFileName += ".fzp";
		FILE* fFlag = fopen(tmpInfoFileName.c_str(), "wb");
	*/

	// ��������ʱ�ļ���Դ�ļ���׺��
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
		// 1. �Ƚ������ֽ�һ��,����hashtable
		_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);

		if (matchHead) { // �ҵ�ƥ����,��Ҫȥ���ƥ��
			// ���ƥ��
			matchLength = LongestMatch(matchHead, matchDist, start);
		}

		if (matchLength < MIN_MATCH) {
			// û���ҵ�ƥ��
			//fputc(_pWin[start], fout);
			SaveLZ77Result(_pWin[start], 0, bitInfo, bitCount,lookahead);
			start++;
			lookahead--;
		}
		else {
			// ��ǰ�����ҵ����ƥ��,�����Ҫ���ظ����ֵ��ַ��滻Ϊ<����,����>��
			// fputc(matchLength - 3, fout);
			// fwrite(&matchDist, 1, sizeof(matchDist), fout);
			SaveLZ77Result(matchLength, matchDist, bitInfo, bitCount,lookahead);
			lookahead -= matchLength;
			matchLength -= 1;

			// ע��: ƥ����ַ���Ҫ��hashtable�в���
			start++;
			while (matchLength--) {
				_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);
				start++;
			}
		}

		// ע�� ���ļ�ʱ����,�����л������е�����С��MIN_LOOKHEADʱ,
		// ����Ҫ�������в�������
		// Ŀ�� : ��ƥ��ﵽ����!
		if (lookahead <= MIN_LOOKHEAD) {
			FillWindow(fIn, lookahead, start);
		}

	}
	if (_byteLengthLz77.size() < BUFF_SIZE) {
		// ע�� bitInfo ���ܲ���8bit
		if (bitCount > 0 && bitCount < 8) {
			bitInfo <<= (8 - bitCount);
			_flagLz77.push_back(bitInfo);
			//fputc(bitInfo, fFlag);
		}
		isLast = true;
		CompressBlock();
	}
	

	//fwrite(&fileSize, 1, sizeof(fileSize), fFlag); // ���ļ���С��Ϣд��fFlag ��ĩβ

	fclose(fIn);
	fclose(fout);
	//fclose(fFlag);
}


/*
	��ȡ�ƥ�䳤��
*/
unshort DZzip::LongestMatch(unshort matchHead, unshort& matchDist, unshort start) {
	unshort curMatchDist = 0;
	unshort curMatchLength = 0;
	unshort bestMatchLength = 0;
	unchar maxMatchCount = 255; // ��ֹ����ʱԽ������ֵ���ǵ�����Ӷ����»����γ� 

	// ������ҵ�ʱ�����ҵ�̫Զ,��Զ��start �� MAX_DICT �ľ���
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


// ��д����λ��Ϣ
void DZzip::SaveLZ77Result(unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount,unll lookahead) {
	// ��matchDist == 0 ʱ,matchLength ��ʾԭ�ַ�
	// ��matchDist > 0 ʱ,macthLength ��ʾ����
	
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
		// ���ÿ�����ݽ���huffman�����н�һ��ѹ��
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


// ���Ҵ��е��������ݰ��Ƶ���ߴ�����
// ����hash��
// �����в���32k������
void DZzip::FillWindow(FILE* fIn, unll& lookahead, unshort& start) {
	if (start >= (WIN_SIZE + MAX_DIST)) {
		// 1. ��Ҫ�� �Ҵ��������ݰ��Ƶ���ߴ���
		// ��Ϊ��startλ����ǰ�ҵ�ʱ��,��Զֻ����MAX_DIST�ľ���
		// ���պ��ҵ��Ҵ��ڵ���߽�,��ʱ�󴰿ڵ����ݾ�û������
		memcpy(_pWin, _pWin + WIN_SIZE, WIN_SIZE);
		start -= WIN_SIZE;
		// 2. �ղ��ڰ��ƵĹ�����,���Ҵ��в��һ�����������Ҳ���Ƶ�
		// ���󴰲��һ������еĵ��ֽڵ��±귢���˱仯,��hash����
		// �洢�ľ��ǲ��һ��������е������ֽ�һ�����ֽ��ڴ����е��±�
		// �����±귢���˱仯,�������hash��
		_ht.UpdateTable();

		// 3. ���Ҵ����в���WIN_SIZE ������
		if (!feof(fIn)) {
			lookahead += fread(_pWin + WIN_SIZE, 1, WIN_SIZE, fIn);
		}
	}
}




//////////////////////////////////////////////////////////////////////////////////////////// 
// huffman 
// todo 
void DZzip::CompressBlock() {

	// 0. ���ǰһ���ѹ����Ϣ
	ClearPrevStatInfo();
	// 1. ͳ��ÿ���ֽڳ��ֵĴ���
	StatAppearCount();
	// 2. ����huffman��
	HuffmanTree<ByteLengthInfo> byteLengthTree(_byteLengthInfo, ByteLengthInfo());
	
	HuffmanTree<ByteLengthInfo> distTree(_distInfo, ByteLengthInfo());
	// 3. ��ȡ����λ��
	GetCodeLen(byteLengthTree.GetRoot(), _byteLengthInfo);
	GetCodeLen(distTree.GetRoot(), _distInfo);
	// 4. ����huffman ����: ���ձ��벽��Ϊ��һ�ֶ�,�ֽڴ�СΪ�ڶ��ֶ�����
	GenerateHuffmanCode(_byteLengthInfo);
	GenerateHuffmanCode(_distInfo);

	// 5. дѹ��ʱ��Ҫ�õ���λ����Ϣ --- д����λ��
	WriteInfo(fout);
	
	// 6. ѹ��
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
			// byteLengthLz77[i] �ǳ��Ⱦ����
			CompressLengthDist(_byteLengthLz77[i], _distLz77[distIdx++], compressBiteInfo, compressBiteCount);
		}
		else {
			CompressByte(_byteLengthLz77[i], compressBiteInfo, compressBiteCount);
		}
		bitInfo <<= 1;
		bitCount -= 1;
	}
	// ѹ��һ��256��ʾ������Ѿ�������
	CompressByte(256, compressBiteInfo, compressBiteCount);
	if (compressBiteCount > 0 && compressBiteCount < 8) {
		compressBiteInfo <<= (8 - compressBiteCount);
		fputc(compressBiteInfo, fout);
	}
	// ��ǰһ��Lz77�Ľ�����
	_byteLengthLz77.clear();
	_distLz77.clear();
	_flagLz77.clear();
}



void DZzip::ClearPrevStatInfo() {
	// ���ǰһ��huffmanѹ�����֮��_byteLengthInfo
	for (auto& e : _byteLengthInfo) {
		e._appareCount = 0;
		e._codeLength = 0;	
		e._code = 0;
	}

	// ���ǰһ��huffmanѹ�����֮��_distInfo
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
			// �������ǳ��Ⱦ����
			// huffman ����û�жԳ��Ⱦ���Խ���ѹ��
			//���ǶԳ��Ⱥ;������������,�������³��ȶ�Ӧ��������������huffman����
			// �˴�: ������Ҫͳ�Ƴ��Ⱥ;�����ֵĴ���,��Ҫͳ�Ƴ��Ⱥ;����Ӧȡ������ֵĴ���
			_byteLengthInfo[GetIndexLengthCode(_byteLengthLz77[i])]._appareCount++;
			_distInfo[GetIndexDistCode(_distLz77[distIndex++])]._appareCount++;
		}
		else {
			// ��������Դ�ַ�
			_byteLengthInfo[_byteLengthLz77[i]]._appareCount++;
		}
		biteInfo <<= 1;
		biteCount -= 1;
	}
	// ��256 ����һ��
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
	// 1. ���ձ���λ��Ϊ��һ�ֶ�,�ֽڴ�СΪ�ڶ��ֶν�������
	vector<ByteLengthInfo> temp(elemInfo);
	sort(temp.begin(), temp.end());

	// 2. �ҵ���һ������λ����Ϊ0��λ��
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
		// ͬ��� ǰһ��+1
		if (temp[i]._codeLength == temp[i - 1]._codeLength) {
			temp[i]._code = temp[i - 1]._code + 1;
			elemInfo[temp[i]._elem]._code = elemInfo[temp[i-1]._elem]._code + 1;
			levelCount++;
		}
		else {
			// ����ͬһ��, ����һ��ı��� + ��������<< ������
			temp[i]._code = (temp[i - levelCount]._code + levelCount) << (temp[i]._codeLength - temp[i - 1]._codeLength);
			elemInfo[temp[i]._elem]._code = (elemInfo[temp[i - levelCount]._elem]._code + levelCount) << (elemInfo[temp[i]._elem]._codeLength - elemInfo[temp[i - 1]._elem]._codeLength);
			levelCount = 1;
		}
	}
}



void DZzip::WriteInfo(FILE* fout) {
	// �Ƿ�Ϊ���һ����
	if (isLast)
		fputc(0, fout);
	else
		fputc(1, fout);

	// д�ֽڳ��ȱ���λ����Ϣ
	for (size_t i = 0; i < _byteLengthInfo.size(); i++) {
		fputc(_byteLengthInfo[i]._codeLength, fout);
	}

	// д����ı���λ��Ϣ
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
	// ѹ������
	unshort index = GetIndexLengthCode(length);
	uint code = _byteLengthInfo[index]._code;
	unchar codeLen = _byteLengthInfo[index]._codeLength;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);

	// д���ȶ�Ӧ����ı���λ
	index = GetIndexLengthCode(length) - 257;
	length += 3;
	code = length - lengthInterval[index].interval[0];
	codeLen = lengthInterval[index].extraBit;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);

	// ѹ������
	index = GetIndexDistCode(dist);
	code = _distInfo[index]._code;
	codeLen = _distInfo[index]._codeLength;
	WriteCode(code, codeLen, compressbiteInfo, compressbiteCount);
	// д�����Ӧ�Ķ������λ
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


// ��ѹ�ļ�
void DZzip::UNDeflated(const string& fileName) {

	std::string filesuf = GetFileSuffix(fileName);
	if ("dzp" != filesuf) {
		std::cout << "ѹ���ļ�����Ӧ��ΪfilePath.dzp" << std::endl;
		return;
	}

	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
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

		//1. ��ȡ����λ����Ϣ
		//2. ���ɽ���� 
		//   ����ԭ�ַ��ͳ���
		GetCodeBitLenInfo(fIn);
		vector<DecodeTable> byteLengthDT;
		GenerateDecode(byteLengthDT,_byteLengthInfo);

		// �������
		vector<DecodeTable> distDT;
		GenerateDecode(distDT, _distInfo);

		// ��ѹ��һ����
		unchar ch = 0;
		unchar bitCount = 0;
		while (true) {
			unshort symbol = UNCompressSymbol(fIn, _byteLengthInfo, byteLengthDT, ch, bitCount);
			if (symbol < 256) {
				// ��ѹ��������һ��Ԫ�ַ�
				fputc(symbol, fout);
			}
			else if (256 == symbol) {
				// һ�����ѹ������
				break;
			}
			else {
				// ��ѹ���������ǳ���
				// symobl: 257 ->length: 3
				size_t index = symbol - 257;
				unshort length = lengthInterval[index].interval[0];

				// ��ȡ�����Ӧ�ı���
				unchar extraBitLen = lengthInterval[index].extraBit;
				unshort value = 0;
				while (extraBitLen) {
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}
				length += value;
				length += 3;
				// ��ѹ������
				index = UNCompressSymbol(fIn, _distInfo, distDT, ch, bitCount);
				unshort dist = distInterval[index].interval[0];

				// ��ȡ����������λ
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
		// �����һ����
		if (isLast) break;
	}


	fclose(fIn);
	fclose(fout);
	fclose(fR);
}

void DZzip::GetCodeBitLenInfo(FILE* fIn) {
	// ��ȡԪ�ַ��Ͳ����ı���λ����Ϣ
	_byteLengthInfo.clear();
	for (size_t i = 0; i < 286; ++i) {
		// ֻ������볤��λ����Ϊ0���ֽںͳ���λ��Ϣ
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
	//1. ͳ�Ʋ�ͬ����λ����elemInfo �г��ֵĴ���
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