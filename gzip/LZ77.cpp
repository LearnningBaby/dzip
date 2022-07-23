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
	// LZ77 是通用类型的压缩算法,文本文件以及二进制格式的文件都可以出处理
	// 直接以二进制的方式打开
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cerr << "待压缩文件打开失败!" << std::endl;
		return;
	}

	// 文件大小 <= MIN_MATCH 时不进行压缩
	fseek(fIn,0, SEEK_END);
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
	compressFileName += ".lzp";
	FILE* fout = fopen(compressFileName.c_str(), "wb");


	// 保存区分源字节和产长度距离对的比特位信息,以及源文件后缀信息
	std::string tmpInfoFileName = GetFileInfoHead(fileName);
	tmpInfoFileName += ".fzp";
	FILE* fFlag = fopen(tmpInfoFileName.c_str(), "wb");

	// 另外搞个零时文件存源文件后缀名
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
		// 1. 先将三个字节一组,插入hashtable
		_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);

		if (matchHead) { // 找到匹配了,需要去找最长匹配
			// 找最长匹配
			matchLength = LongestMatch(matchHead, matchDist,start);
		}
		
		if (matchLength < MIN_MATCH) {
			// 没有找到匹配
			fputc(_pWin[start], fout);
			WriteFlagInfo(fFlag, _pWin[start], 0, bitInfo, bitCount);
			start++;
			lookahead--;
		}
		else {
			// 在前文中找到最长的匹配,因此需要将重复出现的字符替换为<长度,距离>对
			fputc(matchLength-3, fout);
			fwrite(&matchDist, 1, sizeof(matchDist), fout);
			WriteFlagInfo(fFlag, _pWin[start], matchDist, bitInfo, bitCount);
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
			 FillWindow(fIn,lookahead, start);
		}

	}

	// 注意 bitInfo 可能不够8bit
	if (bitCount > 0 && bitCount < 8) {
		bitInfo <<= (8 - bitCount);
		fputc(bitInfo, fFlag);
	}

	fwrite(&fileSize, 1,sizeof(fileSize), fFlag); // 将文件大小信息写入fFlag 的末尾

	fclose(fIn);
	fclose(fout);
	fclose(fFlag);
}


/*
	获取最长匹配长度
*/
unshort LZ77::LongestMatch(unshort matchHead, unshort& matchDist,unshort start) {
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

	if (curMatchDist > MAX_DIST) {
		bestMatchLength = 0;
	}
	return bestMatchLength;
}


// 填写比特位信息
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


// 将右窗中的所有数据搬移到左边窗口中
// 更新hash表
// 往左窗中补充32k的数据
void LZ77::FillWindow(FILE* fIn, unll& lookahead, unshort& start) {
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





// 解压文件
void LZ77::UNCompressFile(const string& fileName) {
	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn) {
		std::cout << "压缩文件打开失败" << std::endl;
		return;
	}

	std::string tmpFileName = GetFileInfoHead(fileName);
	tmpFileName += ".fzp";
	FILE* fFlag = fopen(tmpFileName.c_str(), "rb");

	// 从标记文件中获取压缩文件的大小
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
	

	FILE* fout = fopen(unCompressFileName.c_str(), "wb"); // 解压文件
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
			// 遇到的是长度距离对!
			unshort matchLength = fgetc(fIn) + 3;
			unshort matchDist = 0;
			fread(&matchDist, sizeof(matchDist), 1, fIn);
			// 之前写的可能在缓冲区中,还原之前要刷新到文件中
			fflush(fout);

			fseek(fR, 0 - matchDist, SEEK_END);
			uncCompressSize += matchLength;
		
			while (matchLength) {
				// 从压缩文件中读取
				unchar ch = fgetc(fR);
				fputc(ch, fout);
				fflush(fout); // 防止重叠问题
				matchLength--;
			}

		}
		else {
			// 遇到的是原字符
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

