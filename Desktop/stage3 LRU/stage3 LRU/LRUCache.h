#pragma once
using namespace std;
#include<unordered_map>
struct DLinkedNode {
	string key;
	string value;
	DLinkedNode* prev;
	DLinkedNode* next;
	DLinkedNode() : key(""), value(""), prev(nullptr), next(nullptr) {}
	DLinkedNode(string _key, string _value) : key(_key), value(_value), prev(nullptr), next(nullptr) {}
};

class LRUCache
{
private:
	unordered_map<string, DLinkedNode*> cache;
	DLinkedNode* head; // pseudo head
	DLinkedNode* tail; // pseudo head
	int size;
	int capacity;
public:
	LRUCache(int _capacity);
	string get(string key);
	void put(string key, string value);
	void addToHead(DLinkedNode* node);
	void untieNode(DLinkedNode* node);
	void del(string key);
	void moveToHead(DLinkedNode* node);
	DLinkedNode* removeTail();
};

