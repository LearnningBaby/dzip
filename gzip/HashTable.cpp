#include "HashTable.h"

HashTable::HashTable() :_prev(new unshort[2 * WIN_SIZE])
, _head(_prev + WIN_SIZE) {
	memset(_prev, 0, sizeof(unshort) * 2 * WIN_SIZE);
}

HashTable::~HashTable() {
	delete[] _prev;
	_prev = _head = nullptr;
}

void HashTable::HashFunc(unshort& hashAddr, unchar ch) {
	hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
}

unshort HashTable::H_SHIFT() {
	return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
}


void HashTable::InsertString(unshort& hashAddr /*上一次前面两个字节算出来的hashAddr*/, unchar ch /*当前第三个字节*/ , unshort pos/*当前扫描字符串头部位置*/, unshort& macthHead/*匹配结果*/) {
	HashFunc(hashAddr,ch); // 计算哈希地址
	// 将冲突值构造成_prev 的下标链表
	_prev[pos] = _head[hashAddr]; 
	macthHead = _head[hashAddr];
	_head[hashAddr] = pos;
}


size_t HashTable::GetPrevMatch(unshort& macthHead) {
	macthHead = _prev[macthHead];
	return macthHead;
}
