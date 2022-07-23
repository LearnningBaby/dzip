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
	std::string compressFileName = GetFileInfoHead(fileName);
	compressFileName += ".lzp";
	FILE* fout = fopen(compressFileName.c_str(), "wb");


	// ��������Դ�ֽںͲ����Ⱦ���Եı���λ��Ϣ,�Լ�Դ�ļ���׺��Ϣ
	std::string tmpInfoFileName = GetFileInfoHead(fileName);
	tmpInfoFileName += ".fzp";
	FILE* fFlag = fopen(tmpInfoFileName.c_str(), "wb");

	// ��������ʱ�ļ���Դ�ļ���׺��
	std::string fileSuff = GetFileInfoHead(fileName);
	fileSuff += ".suffix";
	FILE* Suf = fopen(fileSuff.c_str(), "wb");
	std::string suffix = (GetFileSuffix(fileName) + "\n");
	fwrite(suffix.c_str(),suffix.size(),1, Suf);
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
			matchLength = LongestMatch(matchHead, matchDist,start);
		}
		
		if (matchLength < MIN_MATCH) {
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
		if (lookahead <= MIN_LOOKHEAD) {
			 FillWindow(fIn,lookahead, start);
		}

	}

	// ע�� bitInfo ���ܲ���8bit
	if (bitCount > 0 && bitCount < 8) {
		bitInfo <<= (8 - bitCount);
		fputc(bitInfo, fFlag);
	}

	fwrite(&fileSize, 1,sizeof(fileSize), fFlag); // ���ļ���С��Ϣд��fFlag ��ĩβ

	fclose(fIn);
	fclose(fout);
	fclose(fFlag);
}


/*
	��ȡ�ƥ�䳤��
*/
unshort LZ77::LongestMatch(unshort matchHead, unshort& matchDist,unshort start) {
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

	if (curMatchDist > MAX_DIST) {
		bestMatchLength = 0;
	}
	return bestMatchLength;
}


// ��д����λ��Ϣ
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


// ���Ҵ��е��������ݰ��Ƶ���ߴ�����
// ����hash��
// �����в���32k������
void LZ77::FillWindow(FILE* fIn, unll& lookahead, unshort& start) {
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





// ��ѹ�ļ�
void LZ77::UNCompressFile(const string& fileName) {
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	std::string tmpFileName = GetFileInfoHead(fileName);
	tmpFileName += ".fzp";
	FILE* fFlag = fopen(tmpFileName.c_str(), "rb");

	// �ӱ���ļ��л�ȡѹ���ļ��Ĵ�С
	unll fileSize = 0;
	fseek(fFlag, -8, SEEK_END);
	fread(&fileSize, sizeof(fileSize), 1, fFlag);
	fseek(fFlag, 0, SEEK_SET);

	std::cout << fileSize << std::endl;

	std::string suffile = GetFileInfoHead(fileName);
	suffile += ".suffix";

	FILE* suf = fopen(suffile.c_str(), "rb");
	std::string suffix;
	while (true) {
		unchar ch = fgetc(suf);
		if (ch == '\n') break;
		suffix += ch;
	}
	fclose(suf);

	std::string unCompressFileName = GetFileInfoHead(fileName);
	unCompressFileName += "_lz77.";
	unCompressFileName += suffix;
	std::cout << unCompressFileName << std::endl;
	

	FILE* fout = fopen(unCompressFileName.c_str(), "wb"); // ��ѹ�ļ�
	FILE* fR = fopen(unCompressFileName.c_str(), "rb");

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
			fread(&matchDist, sizeof(matchDist), 1, fIn);
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

