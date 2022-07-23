#pragma once
#include "common.h"
#include "HashTable.h"


class LZ77 {
public:
	LZ77();
	~LZ77();

	void CompressFile(const string& fileName);
	void UNCompressFile(const string& fileName);

private:
	unshort LongestMatch(unshort matchHead, unshort& matchDist, unshort start);
	void WriteFlagInfo(FILE* fFlag, unshort matchLength, unshort matchDist, unchar& bitInfo, unchar& bitCount);
	void FillWindow(FILE* fIn, unll& lookahead, unshort& start);
private:
	unchar* _pWin;
	HashTable _ht;

	const static unshort MIN_LOOKHEAD = MIN_MATCH + MAX_MATCH + 1;
	const static unshort MAX_DIST = WIN_SIZE - MIN_LOOKHEAD; // 每个窗口找的最远距离(除了最后一个窗口)
};