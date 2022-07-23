#pragma once
#include "common.h"

class HashTable {
public:
	HashTable();
	~HashTable();
	void InsertString(unshort& hashAddr, unchar ch, unshort pos, unshort& macthHead);
	size_t GetPrevMatch(unshort& macthHead);
	void UpdateTable();
private:
	unshort *_prev;
	unshort *_head;
private:
	void HashFunc(unshort & hashdAddr, unchar ch);
	unshort H_SHIFT();

	// hash 桶的个数为 2^15 
	const unshort HASH_BITS = 15;

	// 哈希表大小
	const unshort HASH_SIZE = (1 << HASH_BITS);
	
	// 哈希掩码: 主要作用是将右窗口数据向左窗口搬移时,用来更新哈希表中的数据
	const unshort HASH_MASK = HASH_SIZE - 1;

};