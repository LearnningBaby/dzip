#pragma once

#include <vector>
#include <queue>

template <class W>
struct HuffmanTreeNode {
	HuffmanTreeNode<W>* _left;
	HuffmanTreeNode<W>* _right;
	HuffmanTreeNode<W>* _parent;

	W _weight;
	HuffmanTreeNode(const W& weight = W()):_left(nullptr)
		,_right(nullptr)
		, _parent(nullptr)
		,_weight(weight)
	{}
};

template<class W>
class HuffmanTree {
	typedef HuffmanTreeNode<W> Node;
	class Compare {
	public:
		bool operator()(const Node* x, const Node* y)const {
			return x->_weight > y->_weight;
		}
	};

public:
	HuffmanTree():_root(nullptr){}
	HuffmanTree(const std::vector<W>& vw, const W& invalid) { // invalid ȨֵУ��
		// 1. �����е�Ȩֵ����ֻ�и��ڵ�Ķ�����ɭ��;
		// ɭ���ж�����Ӧ��ʹ�ö�������;
		// lambda ���ʽ,����С��!
		std::priority_queue< Node*, std::vector<Node*>, Compare>  queue;

		for (auto& e : vw) {
			if(invalid != e) queue.push(new Node(e));
		}
		
		while (queue.size() > 1) {
			Node* left = queue.top();
			queue.pop();

			Node* right = queue.top();
			queue.pop();

			// ��left �� right ��Ϊһ���½ڵ����������
			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			left->_parent = parent;

			parent->_right = right;
			right->_parent = parent;

			queue.push(parent);
		}
		_root = queue.top();
	}

	Node* GetRoot() const{
		return _root;
	}

	~HuffmanTree() {
		Destroy(_root);
	}
private:
	void Destroy(Node*& root) {
		if (root) {
			Destroy(root->_left);
			Destroy(root->_right);
			delete root;
			root = nullptr;
		}
	}
	Node* _root;
};


#if 0
void TetsHuffmanTree() {
	std::vector<int> v{ 7,5,3,1 };
	HuffmanTree<int> ht(v);
}
#endif 