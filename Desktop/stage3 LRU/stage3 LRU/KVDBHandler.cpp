#include "KVDBHandler.h"
#include"logger.h"
using namespace std;

string f = "";
bool flag = false;  //�ж��Ƿ��ڽ���purge����
std::unordered_map<std::string, int>index;  //����
std::unordered_map<std::string, int>te;   //�洢����ʱ��
priority_queue<Node, vector<Node>, cmp>pq;

KVDBHandler::KVDBHandler(const std::string& db_file) {
	database = db_file;
	out.open(db_file, std::ios::app);
	in.open(db_file);
	if (!out) {   //���������ݿ����½�
		std::ofstream fout(db_file);
		fout.close();
	}
}

KVDBHandler::~KVDBHandler() {
	out.close();
	in.close();
}

void createIndex(KVDBHandler* handler) {     //������ϣ�������������洢key��key���ڵ�λ��
	std::string value = "";
	handler->in.seekg(0, std::ios::beg);
	int  _key_length, _value_length, pos = 0;
	char temp;
	std::string _key, _value;
	while (handler->in.read((char*)&_key_length, sizeof(int))) {  //���ļ����б���

		handler->in.read((char*)&_value_length, sizeof(int));
		_key.clear(), _value.clear();
		for (int i = 0; i < _key_length; i++) {
			handler->in.read((char*)&temp, sizeof(temp));
			_key += temp;
		}
		for (int i = 0; i < _value_length; i++) {
			handler->in.read((char*)&temp, sizeof(temp));
			_value += temp;
		}
		if (index.count(_key)) {
			if (_value == "") {
				index.erase(_key);
			}
			else {
				index.erase(_key);
				index.insert(std::pair<std::string, int>(_key, pos));
			}
		}
		else
			index.insert(std::pair<std::string, int>(_key, pos));
		pos += (8 + _key_length + _value_length);
	}
}

void createTime(KVDBHandler* handler) {     //��ȡ�����ļ�������Ҫ���ڵ�key�͹���ʱ��������С����
	int num = 0;
	handler->in.seekg(0, std::ios::beg);
	int  _key_length, _time;
	char temp;
	std::string _key, _value;
	while (handler->in.read((char*)&_key_length, sizeof(int))) {
		_key.clear();
		for (int i = 0; i < _key_length; i++) {
			handler->in.read((char*)&temp, sizeof(temp));
			_key += temp;
		}
		handler->in.read((char*)&_time, sizeof(int));
		if (te.count(_key)) {
			if (_time == 0) {
				te.erase(_key);
			}
			else {
				te.erase(_key);
				te.insert(std::pair<std::string, int>(_key, _time));    
			}
		}
		else
			te.insert(std::pair<std::string, int>(_key, _time));
	}
	for (auto it = te.begin(); it != te.end(); it++) {
		Node t(it->first, it->second);
		pq.push(t);
	}
}

int set(KVDBHandler* handler, const std::string& key, const std::string& value) {
	int judge=0;   //�ж�ԭ�е�key�Ƿ����
	int pos = getSize(handler->database);
	if (index.count(key) && flag == false) {   
		index.erase(key);
		index.insert(std::pair<std::string, int>(key, pos));
		judge = 1;
	}
	else
		index.insert(std::pair<std::string, int>(key, pos));

	int key_length = key.length();
	int value_length = value.length();
	handler->out.write((char*)&key_length, sizeof(int));
	handler->out.write((char*)&value_length, sizeof(int));
	handler->out.write(key.c_str(), key.length());
	handler->out.write(value.c_str(), value.length());
	return judge;
}

int get(KVDBHandler* handler, const std::string& key, std::string& value) {
	value = "";
	int  _key_length, _value_length;
	char temp;
	std::string _key, _value;
	unordered_map<string, int>::iterator iter;
	if ((iter = index.find(key)) != index.end()) {
		int pos = iter->second;
		handler->in.seekg(pos, std::ios::beg);
		handler->in.read((char*)&_key_length, sizeof(int));
		handler->in.read((char*)&_value_length, sizeof(int));
		handler->in.seekg(_key_length, std::ios::cur);
		for (int i = 0; i < _value_length; i++) {
			handler->in.read((char*)&temp, sizeof(temp));
			_value += temp;
		}
		value = _value;
		return 1;
	}
	else {
		return 0;
	}

}

int del(KVDBHandler* handler, const std::string& key) {
	std::string temp = "";
	if (get(handler, key, temp)) { //keyֵ����
		set(handler, key, "");
		index.erase(key);    //ɾ��ʱͬʱ���������Ƴ�
	}
	else return 0; //������key��ֵ��ʧ��
	return 1;
}

int expires(KVDBHandler* handler, const std::string& key, int n) {
	int key_length = key.length();
	handler->out.write((char*)&key_length, sizeof(int));    //������ļ���д����Ҫ���ڵ�key�͹���ʱ��
	handler->out.write(key.c_str(), key.length());
	handler->out.write((char*)&n, sizeof(int));
	return 1;
}

int getSize(std::string s) {
	FILE* fp = fopen(s.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}

int file_size(const char* file)  //��ȡ�ļ���С
{
	struct stat statbuf;
	stat(file, &statbuf);
	int size = statbuf.st_size;
	return size;
}

int purge(std::string file) {
	std::string str = "D:\\tool.txt"; //�����ļ��Ĵ��·��
	std::ofstream ofs(str.c_str());
	if (ofs.good()) {          //��ձ����ļ�����
		remove(str.c_str());
	}
	std::ifstream _in;
	unordered_map<string, int>::iterator iter;
	KVDBHandler handler((const std::string)file);
	for (iter = index.begin(); iter != index.end(); iter++) {  //���������Ч�ļ�ֵ�ԣ������������½����ļ�
		flag = true;
		KVDBHandler _handler("D:\\tool.txt");
		string value;
		get(&handler, iter->first, value);
		set(&_handler, iter->first, value);
	}
	flag = false;
	_in.open("D:\\tool.txt");   //�������ļ�����������ת�ƻ�Դ�ļ���
	std::ofstream out;
	out.open(file);
	out << _in.rdbuf();
	_in.close();
	out.close();
	return 1;
}

