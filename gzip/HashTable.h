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

	// hash Ͱ�ĸ���Ϊ 2^15 
	const unshort HASH_BITS = 15;

	// ��ϣ���С
	const unshort HASH_SIZE = (1 << HASH_BITS);
	
	// ��ϣ����: ��Ҫ�����ǽ��Ҵ����������󴰿ڰ���ʱ,�������¹�ϣ���е�����
	const unshort HASH_MASK = HASH_SIZE - 1;

};