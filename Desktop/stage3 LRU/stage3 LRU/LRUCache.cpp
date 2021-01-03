#include "LRUCache.h"
using namespace std;

LRUCache::LRUCache(int _capacity) : capacity(_capacity), size(0) {
	head = new DLinkedNode(); 
	tail = new DLinkedNode(); 
	head->next = tail;
	tail->prev = head;
}
string LRUCache::get(string key) {
	if (!cache.count(key)) {
		return "";
	}
	DLinkedNode* node = cache[key];
	moveToHead(node); 
	return node->value;
}

void LRUCache::put(string key, string value) {
	if (!cache.count(key)) {
		DLinkedNode* node = new DLinkedNode(key, value);
		cache[key] = node;
		addToHead(node);
		++size;
		if (size > capacity) {
			// 删除双向列表的尾部元素
			DLinkedNode* removed = removeTail();
			// 删除哈希表中对应的元素
			cache.erase(removed->key);
			delete removed;
			--size;
		}
	}
	else {
		// already exist
		DLinkedNode* node = cache[key];
		node->value = value;
		moveToHead(node);
	}
}

void LRUCache::addToHead(DLinkedNode* node) {
	node->next = head->next;
	node->prev = head;
	head->next->prev = node;
	head->next = node;
}

void LRUCache::untieNode(DLinkedNode* node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

void LRUCache::del(string key) {
	auto node = head;
	while (node != NULL) {
		if (node->key == key) {
			untieNode(node);
			cache.erase(node->key);
			delete node;
			break;
		}
		node = node->next;
	}
}

void LRUCache::moveToHead(DLinkedNode* node) {
	untieNode(node);
	addToHead(node);
}

DLinkedNode* LRUCache::removeTail() {
	auto node = tail->prev;
	untieNode(node);
	return node;
}


