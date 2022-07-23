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
	_prev[pos & HASH_MASK] = _head[hashAddr];  // HASH_MASK ���Խ������ ,��pos�ĸ�λ����0
	macthHead = _head[hashAddr];
	_head[hashAddr] = pos;
}


size_t HashTable::GetPrevMatch(unshort& macthHead) {
	macthHead = _prev[macthHead & HASH_MASK];
	return macthHead;
}



void HashTable::UpdateTable() {
	//����head
	for (unshort i = 0; i < HASH_SIZE; ++i) {
		if (_head[i] < WIN_SIZE) {
			_head[i] = 0;
		}
		else {
			_head[i] -= WIN_SIZE;
		}
	}
	// ����prev
	for (unshort i = 0; i < HASH_SIZE; ++i) {
		if (_prev[i] < WIN_SIZE) {
			_prev[i] = 0;
		}
		else {
			_prev[i] -= WIN_SIZE;
		}
	}
}