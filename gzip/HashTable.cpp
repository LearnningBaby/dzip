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
	_prev[pos & HASH_MASK] = _head[hashAddr];  // HASH_MASK 解决越界问题 ,将pos的高位置清0
	macthHead = _head[hashAddr];
	_head[hashAddr] = pos;
}


size_t HashTable::GetPrevMatch(unshort& macthHead) {
	macthHead = _prev[macthHead & HASH_MASK];
	return macthHead;
}



void HashTable::UpdateTable() {
	//更新head
	for (unshort i = 0; i < HASH_SIZE; ++i) {
		if (_head[i] < WIN_SIZE) {
			_head[i] = 0;
		}
		else {
			_head[i] -= WIN_SIZE;
		}
	}
	// 更新prev
	for (unshort i = 0; i < HASH_SIZE; ++i) {
		if (_prev[i] < WIN_SIZE) {
			_prev[i] = 0;
		}
		else {
			_prev[i] -= WIN_SIZE;
		}
	}
}