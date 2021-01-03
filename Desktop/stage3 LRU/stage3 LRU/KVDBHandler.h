#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<fstream>
#include<string>
#include<stack>
#include<cmath>
#include<map>
#include<ctime>
#include<queue>
#include<unordered_map>
using namespace std;
extern bool flag;
extern string f;
extern std::unordered_map<std::string, int>index;  //index索引
extern std::unordered_map<std::string, int>te;     //临时储存Node
struct Node {
	string key;
	int second;
	Node(string s, int d) :key(s), second(d) {}
};
struct cmp {
	bool operator () (const struct Node& n1, const struct Node& n2) {
		return n1.second > n2.second;
	}
};
extern priority_queue<Node, vector<Node>, cmp>pq;  //过期时间索引
int getSize(std::string s);
int file_size(const char* file);
int purge(std::string file);
class KVDBHandler {
public:
	std::ifstream in;
	std::ofstream out;
	std::string database;
	KVDBHandler(const std::string& db_file);
	~KVDBHandler();
	friend void createIndex(KVDBHandler* handler);
	friend void createTime(KVDBHandler* handler);
	friend int set(KVDBHandler* handler, const std::string& key, const std::string& value);
	friend int get(KVDBHandler* handler, const std::string& key, std::string& value);
	friend int del(KVDBHandler* handler, const std::string& key);
	friend int expires(KVDBHandler* handler, const std::string& key, int n);
};



