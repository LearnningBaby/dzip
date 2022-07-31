#pragma warning(disable : 4996)
#include"DZzip.h"
#include<map>


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



//����LZ77,ѹ���������ھ���2�ֽڣ�����������Ϊ65535��һ�ζ���65535�ַ�
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

	//����������д��
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

//ѹ���ļ�
void DZzip::Deflate(const string& fileName)
{
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "���ļ���ʧ��" << endl;
		return;
	}


	//��ȡ�ļ���С
	fseek(fIn, 0, SEEK_END);
	unll fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);


	//����ļ���СС�ڵ���3�ֽڣ���ѹ��
	if (fileSize < MIN_MATCH)
	{
		cout << "�ļ�̫С,�޷�ѹ��" << endl;
		fclose(fIn);
		return;
	}

	//��pwin��ȡ����
	unll lookahead = fread(_pWin, 1, 2 * WIN_SIZE, fIn);


	unshort matchHead = 0;
	unshort hashAddr = 0;//ʹ��MinMatch - 1����hashAddr

	//�ӵ�������ʼ���ң������Ȱ�ǰ���������ϣ���������ѭ���б߲���߲���
	for (unchar i = 0; i < MIN_MATCH - 1; i++)
	{
		_ht.InsertString(hashAddr, _pWin[i], i, matchHead);
	}
	unshort start = 0;


	string zFileName = GetFileName(fileName);
	zFileName += ".dzp";
	//ѹ��
	fOut = fopen(zFileName.c_str(), "wb");

	//�Ƚ��ļ��ĺ�׺д��
	string filePostFix = GetFilePostFix(fileName);
	filePostFix += '\n';
	fwrite(filePostFix.c_str(), 1, filePostFix.size(), fOut);

	//����λ��Ϣ
	unchar bitInfo = 0;
	//�Ա���λ���м�����д��8λ֮������ļ�
	unchar bitCount = 0;
	while (lookahead)
	{
		//����
		unshort matchLength = 0;
		//����
		unshort matchDist = 0;

		matchHead = 0;
		//1.�Ƚ������ַ�һ�飬�ڹ�ϣ����в���,����������ַ�
		_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);

		//�ҵ�ƥ��(matchHead != 0 ,˵����ǰ�����ҵ�ƥ��)
		//matchHead�����뵱ǰҪƥ��������ֽ������һ��ƥ��
		if (matchHead != 0)
		{
			//���ƥ��
			//matchLength = 0;
			//matchDist = 0
			matchLength = LongestMatch(matchHead, matchDist, start);
			//���matchHead = 0,˵��ǰ����ƥ��
		}

		if (matchLength < MIN_MATCH)
		{
			//û���ҵ�ƥ�䣬�����һ���ַ�
			//fputc(_pWin[start], fOut);
			SaveLZ77Result(_pWin[start], 0, bitInfo, bitCount, lookahead);
			start++;
			lookahead--;
		}
		else
		{
			//�ҵ����ƥ�䣬�����Ҫ���ظ����ֵ��ַ����滻Ϊ<���ȣ�����>��
			//fputc(matchLength - 3, fOut);
			//fwrite(&matchDist, 1, sizeof(matchDist), fOut);



			SaveLZ77Result(matchLength, matchDist, bitInfo, bitCount, lookahead);
			lookahead = lookahead - matchLength;
			matchLength -= 1;
			start++;
			//��ѹ�����ַ�Ҳȫ�������ϣͰ��
			while (matchLength--)
			{
				//�����ϣͰ���Ǵ����ַ��ĺ������ַ�
				_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);
				start++;
			}
		}

		//�����л����������ڵ�����С��MIN_LOOKHEADʱ����Ҫ��ȡ
		//����ļ�С��64K
		if (lookahead <= MIN_LOOKHEAD)
		{
			FillWindow(fIn, lookahead, start);
		}
	}

	//�����ܲ���8λ��������д���ļ�
	if (_byteLengthLZ77.size() < BUFF_SIZE)
	{

		if (bitCount > 0 && bitCount < 8)
		{
			//����Ϣ���ڸ�λ��
			bitInfo <<= (8 - bitCount);
			//fputc(bitInfo, fFlag);
			_flagLZ77.push_back(bitInfo);
		}

		_isLast = true;
		CompressBlock();
	}

	//�����ļ�д��ѹ��Դ�ļ���С
	//fwrite(&fileSize, 1,sizeof(fileSize), fFlag);
	fclose(fIn);
	fclose(fOut);
}

//˳��matchHead������ǰ���ƥ��
//matchHeadǰ�����ƥ��   matchDist����    start�ַ������±�
unshort DZzip::LongestMatch(unshort matchHead, unshort& matchDist, unshort start)
{
	unshort curMatchLength = 0;
	unshort maxLength = 0;
	unshort curMatchDist = 0;

	//���ƥ�����
	unchar maxMatchCount = 255;

	//������ҵ�������
	unshort limit = start > MAX_DIST ? start - MAX_DIST : 0;
	do
	{
		curMatchLength = 0;
		curMatchDist = 0;
		unchar* pstart = _pWin + start;//ָ�򱾴�ѹ���ַ���
		unchar* pend = pstart + MAX_MATCH;//�ƥ��
		unchar* pbegin = _pWin + matchHead;//ָ���ȡ��ƥ���ַ����±�
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
	} while ((_ht.GetPrevMatch(matchHead) >= limit) && maxMatchCount--);//��ȡ�ϴ�matchHead

	if (matchDist > MAX_DIST)
	{
		maxLength = 0;
	}

	return maxLength;
}

void DZzip::SaveLZ77Result(unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount, unll lookahead)
{
	//��matchDistΪ0��ʱ��matchLength��ʾԭ�ӷ�
	//��matchDist��Ϊ0��ʱ��matchLength��ʾ����

	bitInfo <<= 1;
	//������ȴ���0��˵��һ���ǳ��ȣ�����ԣ�����λ��дΪ1,
	//�ڶ�ʱ��ʾ��λ���λ����λΪ���Ⱦ����
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
	//���ֽ��Ѿ��������д���ļ�
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
		//���ÿ����ݽ���Huffman����һ��ѹ��

		if (0 == lookahead)
		{
			_isLast = true;
		}
		CompressBlock();
	}
}

void DZzip::FillWindow(FILE* fIn, unll& lookahead, unshort& start)
{
	//���start�ڻ�������벿�֣�˵���Ǹ����ļ�����ȡ���ֿ�Ҫѹ����ϣ���Ҫ�¶�ȡ
	if (start >= WIN_SIZE + MAX_DIST)
	{
		//���Ҵ��е�����ȫ��������
		memcpy(_pWin, _pWin + WIN_SIZE, WIN_SIZE);
		start -= WIN_SIZE;

		//���¹�ϣ��
		//���ƹ��̣��±귢���仯
		_ht.UpdateTable();

		//���Ҵ��в���32k����
		if (!feof(fIn))
		{
			lookahead += fread(_pWin + WIN_SIZE, 1, WIN_SIZE, fIn);
		}
	}
}



/////////////////////////////////////////////
//huffman���
void DZzip::CompressBlock()
{
	//1.ͳ��ÿ���ֽڳ��ֵĴ���
	StatAppearCount();

	//2.����Huffman��
	//��һ�����������ж�_byteLengthInfo�е������Ƿ���ѹ���ļ��б�д�룬û��д�����ݲ�����Huffman��
	HuffmanTree<ByteLengthInfo> lengthTree(_byteLengthInfo, ByteLengthInfo());
	HuffmanTree<ByteLengthInfo> distTree(_distInfo, ByteLengthInfo());

	//3.��ȡ����λ��:ÿ��Ҷ�ӽڵ�ĸ߶�
	//4.����Huffman����
	//��ȡԭ�ַ��ͳ��ȶ�Ӧ��huffman�б���λ�����������
	GetCodeLen(lengthTree.GetRoot(), _byteLengthInfo);
	GenerateHuffmanCode(_byteLengthInfo);

	//��ȡ�����Ӧ��huffman�б���λ�����������
	GetCodeLen(distTree.GetRoot(), _distInfo);
	GenerateHuffmanCode(_distInfo);

	//5.д��ѹ����Ҫ�õ�����Ϣ - ����λ��
	WriteInfo(fOut);

	//6.ѹ��
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
			//_byteLengthLZ77[i]�ǳ���
			//ѹ��һ�����Ⱦ����,
			CompressLengthDist(_byteLengthLZ77[i], _distLZ77[distIdx++], compressBiteInfo, compressBiteCount);

		}
		else
		{
			//_byteLengthLZ77[i]��ԭ�ַ�
			//ѹ��ԭ�ַ�
			CompressByte(_byteLengthLZ77[i], compressBiteInfo, compressBiteCount);
		}

		bitInfo <<= 1;
		bitCount--;
	}

	CompressByte(256, compressBiteInfo, compressBiteCount);
	//ѹ��256����ʾ�������

	//��ȡ�ı��벻��8������������û�����ݶ�ȡ
	if (compressBiteCount != 0)
	{
		compressBiteInfo <<= (8 - compressBiteCount);
		fputc(compressBiteInfo, fOut);
	}

	//��ǰһ��LZ77������
	_byteLengthLZ77.clear();
	_distLZ77.clear();
	_flagLZ77.clear();
}


void DZzip::ClearPrevStatInfo()
{
	//���ǰһ��Huffmanѹ�����֮��_byteLengthInfo
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
	//���ǰһ�����ѹ����Ϣ
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
			//�������ǳ���
			//huffman����û�жԳ��Ⱦ���Խ���ѹ��
			//���ǶԳ��Ⱥ;����������Ļ��֣��������ó��Ⱥ;����Ӧ��������������Huffman��
			//����Ҫͳ��  ����  ��  ����  ���ֵĴ�����ͳ�����ǵ���������ֵĴ���
			_byteLengthInfo[GetIndexLengthCode(_byteLengthLZ77[i])]._appearCount++;
			_distInfo[GetIndexDistCode(_distLZ77[distIndex++])]._appearCount++;
		}
		else
		{
			//��������ԭ�ַ�
			_byteLengthInfo[_byteLengthLZ77[i]]._appearCount++;
		}

		biteCount--;
		biteInfo <<= 1;
	}

	//��256����һ��,256�ǿ������ǣ�����ѹ����256ʱ����ѹ��������
	_byteLengthInfo[256]._appearCount = 1;
}


void DZzip::GetCodeLen(HuffmanTreeNode<ByteLengthInfo>* root, vector<ByteLengthInfo>& elemInfo)
{
	// ����    Ŀ�ģ�ȡ�ñ���λ��
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
	//1.������λ��Ϊ��һ�ֶΣ��ֽڴ�СΪ�ڶ��ֶν�������
	vector<ByteLengthInfo> temp(elemInfo);
	sort(temp.begin(), temp.end());

	//2.�ҵ���һ������λ����Ϊ0��λ��
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
		// ͬһ����룺ǰһ��Ԫ�ؼ�1
		if (temp[i]._codeLength == temp[i - 1]._codeLength)
		{
			//temp[i]._code = temp[i - 1]._code + 1;
			elemInfo[temp[i]._elem]._code = elemInfo[temp[i - 1]._elem]._code + 1;
			levelCount++;
		}
		else
		{
			//����ͬһ��:����һ��ĵ�һ������+��ĸ��� << ������
			//temp[i]._code = (temp[i - levelCount]._code + levelCount) << (temp[i]._codeLength - temp[i - 1]._codeLength);
			elemInfo[temp[i]._elem]._code = (elemInfo[temp[i - levelCount]._elem]._code + levelCount) << (elemInfo[temp[i]._elem]._codeLength - elemInfo[temp[i - 1]._elem]._codeLength);
			levelCount = 1;
		}
	}
}

void DZzip::WriteInfo(FILE* fOut)
{
	//���Ϊ���һ����ı��
	if (false == _isLast)
	{
		fputc(0, fOut);
	}
	else
	{
		fputc(1, fOut);
	}

	//д�ֽڳ��ȱ���λ����Ϣ
	for (size_t i = 0; i < _byteLengthInfo.size(); i++)
	{
		fputc(_byteLengthInfo[i]._codeLength, fOut);
	}

	//д�������λ����Ϣ
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
	//huffman���м���length�ı���
	unshort index = GetIndexLengthCode(length);
	uint code = _byteLengthInfo[index]._code;
	unchar codeLen = _byteLengthInfo[index]._codeLength;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);

	//д���ȶ�Ӧ�Ķ���ı���λ
	//index = GetIndexLengthCode(length);
	length += 3;
	index -= 257;
	code = length - lengthInterval[index].interval[0];
	codeLen = lengthInterval[index].extraBit;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);

	//ѹ������
	//huffman���м����dist�ı���
	index = GetIndexDistCode(dist);
	code = _distInfo[index]._code;
	codeLen = _distInfo[index]._codeLength;
	WriteCode(code, codeLen, compressBiteInfo, compressBiteCount);

	//д�����Ӧ�Ķ���ı���λ
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
		cout << "�ļ���ʽ����" << endl;
		return;
	}

	FILE* fIn = fopen(fileName.c_str(), "rb");

	if (nullptr == fIn)
	{
		cout << "ѹ���ļ���ʧ��" << endl;
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

		// 1.��ȡ����λ����Ϣ
		GetCodeBitLenInfo(fIn);

		// 2.���ɽ����
		//����ԭ�ӷ��ͳ���
		vector<DecodeTable> byteLengthDT;
		GenerateDecode(byteLengthDT, _byteLengthInfo);


		//�������
		vector<DecodeTable> distDT;
		GenerateDecode(distDT, _distInfo);

		//��ѹ��
		unchar ch = 0;
		unchar bitCount = 0;
		while (true)
		{
			unshort symbol = UNcompressSymbol(fIn, _byteLengthInfo, byteLengthDT, ch, bitCount);
			if (symbol < 256)
			{
				//��ѹ����ԭ�ӷ�
				fputc(symbol, fOut);
			}
			else if (256 == symbol)
			{
				//һ�����ѹ������
				break;
			}
			else
			{
				//����
				size_t index = symbol - 257;
				unshort length = lengthInterval[index].interval[0];
				//length += 3;

				//��ȡ�����Ӧ�ı���
				unchar extraBitLen = lengthInterval[index].extraBit;
				unshort value = 0;
				while (extraBitLen)
				{
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}
				length += value;

				//��ѹ������
				index = UNcompressSymbol(fIn, _distInfo, distDT, ch, bitCount);
				unshort dist = distInterval[index].interval[0];

				//��ȡ����ı���λ
				extraBitLen = distInterval[index].extraBit;
				value = 0;
				while (extraBitLen)
				{
					GetNextBit(fIn, value, ch, bitCount);
					extraBitLen--;
				}

				dist += value;

				//֮ǰ��ѹ���Ľ�����ܻ��ڻ���������ʹ�ó��Ⱦ���Ի�ԭǰ�����뽫���ݷŵ���ѹ�����ļ���
				fflush(fOut);

				//ʹfRƫ�Ƶ���ѹ���ļ�ĩβ(������������д������ļ�)����ǰƫ��matchDist�����Ƕ�Ӧ����ͬ����
				fseek(fR, 0 - dist, SEEK_END);

				while (length)
				{
					//��ѹ���ļ��ж�ȡһ���ֽ�
					char ch = fgetc(fR);
					//�����ֽ�д���ѹ���ļ�
					fputc(ch, fOut);
					//�ӻ����������ڴ��ļ�����֤�ڳ��ȴ��ھ��������������Զ�ȡ���շ�����ַ�
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
	//��ȡԭ�ӷ��ͳ��ȵı���λ����Ϣ
	_byteLengthInfo.clear();

	for (size_t i = 0; i < 286; i++)
	{
		//ֻ�������λ����Ϊ0���ֽںͳ���λ����Ϣ
		unshort codeLen = fgetc(fIn);
		if (0 != codeLen)
		{
			ByteLengthInfo temp;
			temp._codeLength = codeLen;
			temp._elem = i;
			_byteLengthInfo.push_back(temp);
		}
	}

	//��ȡ�������λ����Ϣ
	_distInfo.clear();

	for (size_t i = 0; i < 30; i++)
	{
		//ֻ�������λ����Ϊ0�ľ�����Ϣ
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
	//1.ͳ�Ʋ�ͬ����λ����elemInfo�г��ֵĴ���
	//  ����λ����������-----map
	//  key: ����λ��   value�����ִ���
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

	//���ݽ�����е���Ԫ�ػ�ȡ��Ӧ�ı���λ
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
