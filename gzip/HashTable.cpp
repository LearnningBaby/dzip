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


void HashTable::InsertString(unshort& hashAddr /*��һ��ǰ�������ֽ��������hashAddr*/, unchar ch /*��ǰ�������ֽ�*/ , unshort pos/*��ǰɨ���ַ���ͷ��λ��*/, unshort& macthHead/*ƥ����*/) {
	HashFunc(hashAddr,ch); // �����ϣ��ַ
	// ����ͻֵ�����_prev ���±�����
	_prev[pos] = _head[hashAddr]; 
	macthHead = _head[hashAddr];
	_head[hashAddr] = pos;
}


size_t HashTable::GetPrevMatch(unshort& macthHead) {
	macthHead = _prev[macthHead];
	return macthHead;
}
