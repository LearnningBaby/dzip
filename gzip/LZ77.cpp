#pragma warning(disable : 4996)
#include "LZ77.h"
 
LZ77::LZ77(): _pWin(new unchar[2 * WIN_SIZE])
{}

LZ77::~LZ77() {
	if (_pWin) {
		delete[] _pWin;
		_pWin = nullptr;
	}
}


// ���ļ�������δ���
void LZ77::compressFile(const string& fileName) {
	// LZ77 ��ͨ�����͵�ѹ���㷨,�ı��ļ��Լ������Ƹ�ʽ���ļ������Գ�����
	// ֱ���Զ����Ƶķ�ʽ��
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "��ѹ���ļ���ʧ��!" << std::endl;
		return;
	}

	// �ļ���С <= MIN_MATCH ʱ������ѹ��
	fseek(fIn,0, SEEK_END);
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
	FILE* fout = fopen("zzz.lzp", "wb");

	// ��������Դ�ֽںͲ����Ⱦ���Եı���λ��Ϣ
	FILE* fFlag = fopen("temp.fzp", "wb");
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
			matchLength = LongestMatch(matchHead, matchDist,start);
		}
		
		if (matchLength < 3) {
			// û���ҵ�ƥ��
			fputc(_pWin[start], fout);
			WriteFlagInfo(fFlag, _pWin[start], 0, bitInfo, bitCount);
			start++;
			lookahead--;
		}
		else {
			// ��ǰ�����ҵ����ƥ��,�����Ҫ���ظ����ֵ��ַ��滻Ϊ<����,����>��
			fputc(matchLength-3, fout);
			fwrite(&matchDist, 1, sizeof(matchDist), fout);
			WriteFlagInfo(fFlag, _pWin[start], matchDist, bitInfo, bitCount);
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

		if (lookahead < MIN_LOOKHEAD) {
			// TODO: FillWindow(fIn, start);
		}


	}

	// ע�� bitInfo ���ܲ���8bit
	if (bitCount > 0 && bitCount < 8) {
		bitInfo <<= (8 - bitCount);
		fputc(bitInfo, fFlag);
	}

	fwrite(&fileSize, 1, sizeof(fileSize), fFlag);

	fclose(fIn);
	fclose(fout);
	fclose(fFlag);
}





unshort LZ77::LongestMatch(unshort matchHead, unshort& matchDist,unshort start) {
	unshort curMatchDist = 0;
	unshort curMatchLength = 0;
	unshort bestMatchLength = 0;
	do {
		unchar* pstart = _pWin + start;
		unchar* pend = pstart + MAX_MATCH;

		curMatchDist = 0;
		curMatchLength = 0;

		unchar* pbegin = _pWin + matchHead;
		while (pstart <= pend && *pstart == *pbegin) {
			pstart++;
			pbegin++;
			curMatchLength++;
		}
		curMatchDist = start - matchHead;
		
		if (curMatchLength > bestMatchLength) {
			bestMatchLength = curMatchLength;
			matchDist = curMatchDist;
		}

	} while (_ht.GetPrevMatch(matchHead));

	return bestMatchLength;
}


void LZ77::WriteFlagInfo(FILE* fFlag, unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount) {
	bitInfo <<= 1;
	if (matchDist > 0) {
		bitInfo |= 1;
	}
	bitCount++;
	if (bitCount == 8) {
		fputc(bitInfo, fFlag);
		bitInfo = 0;
		bitCount = 0;
	}

}



void LZ77::UNCompressFile(const string& fileName) {
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
		return;
	}
	FILE* fFlag = fopen("temp.fzp", "rb");

	// �ӱ���ļ��л�ȡ�ļ��Ĵ�С
	unll fileSize = 0;
	fseek(fFlag, -8, SEEK_END);
	fread(&fileSize, 1, sizeof(fileSize), fFlag);
	fseek(fFlag, 0, SEEK_SET);

	FILE* fout = fopen("unCompress.txt", "wb"); // ��ѹ�ļ�
	FILE* fR = fopen("unCompress.txt", "rb");

	unchar bitInfo = 0;
	unchar bitCount = 0;
	unll uncCompressSize = 0;

	while (uncCompressSize < fileSize) {
		if (0 == bitCount) {
			bitInfo = fgetc(fFlag);
			bitCount = 8;
		}

		if (bitInfo & 0x80) {
			// �������ǳ��Ⱦ����!
			unshort matchLength = fgetc(fIn) + 3;
			unshort matchDist = 0;
			fread(&matchDist, 1, sizeof(matchDist), fIn);
			// ֮ǰд�Ŀ����ڻ�������,��ԭ֮ǰҪˢ�µ��ļ���
			fflush(fout);

			fseek(fR, 0 - matchDist, SEEK_END);
			uncCompressSize += matchLength;
		
			while (matchLength) {
				// ��ѹ���ļ��ж�ȡ
				unchar ch = fgetc(fR);
				fputc(ch, fout);
				fflush(fout); // ��ֹ�ص�����
				matchLength--;
			}

		}
		else {
			// ��������ԭ�ַ�
			unchar ch = fgetc(fIn);
			fputc(ch,fout);
			uncCompressSize++;
		}
		bitInfo <<= 1;
		bitCount--;
	}

	fclose(fIn);
	fclose(fout);
	fclose(fR);
}

