# **File Based K-V Database**

## 需求

K-V数据库是一种以键值对存储数据的一种数据库，每个键都会对应唯一的一个值。本项目将对K-V数据库进行简单的实现

### API

下面将对项目中实现的API接口进行介绍

#### 1.KVDBHandler类

能够通过KVDBHandler类对数据库进行操作，具有ofstream类型的out数据成员，能够对文件写入，具有ifstream类型的in数据成员，能够对文件进行读取，以及string类型存储数据库名称。并内置了set，get，delete等接口

```c++
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
```

#### 2.set操作

KVDBHandler类具有ofstream类型的out数据成员，通过out.write()将key和value向文件中写入，首先写入两个int类型的key和value的长度便于读取，其次写入两个string类型即为key和value的值，时间复杂度为O(1)。能够实现key，value的写入操作。

```c++
int set(KVDBHandler* handler, const std::string& key, const std::string& value) {
	int judge=0;   //判断原有的key是否存在
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
```

#### 3.get操作

##### (1)使用index操作之前

KVDBHandler类具有ifstream类型的in数据成员，通过in.read()读取数据，从头开始对文件进行遍历，首先读取长度其次读取key，若key匹配则存储key对应的value，否则跳过value的读取，以最后读取到的value的值为准，没有读取到则查找失败。需要对整个文件进行遍历。时间复杂度为O(n)。

##### (2)使用index操作之后

index操作通过哈希表实现，存储唯一有效的key以及key在文件中的offest，如果从index中匹配到对应的key则直接跳到对应的offest处进行读取，若没有匹配到则读取失败。在不考虑哈希冲突的情况时间复杂度为O(1)，即使存在哈希冲突查找效率仍大幅度增加。能够实现获取对应key的value值的操作。

#### 4.delete操作

本数据库设定不存在值为空的key和value，故del操作即向文件尾部写入一值为空的字符串，建立索引时读取到该字符串时会将原本的key和int键值对进行移除，从而查找不到该值，实现删除操作。时间复杂度为O(1)。能够实现删除数据库中的value值操作。

```c++
int del(KVDBHandler* handler, const std::string& key) {
	std::string temp = "";
	if (get(handler, key, temp)) { //key值存在
		set(handler, key, "");
		index.erase(key);    //删除时同时从索引中移除
	}
	else return 0; //不存在key的值，失败
	return 1;
}
```



#### 5.purge操作(文件大小超过指定值自动运行)

##### (1)使用index操作之前

对原文件进行遍历，每读取到一个key值对该值进行get操作，如果成功后再在备用文件中进行一次get操作，若get失败则将该键值对通过set操作写入备用文件中。最后清空原文件内容，将备用文件内容移入原文件中，在清除备用文件。时间复杂度为O(n3)

##### (2)使用index操作之后

因为index存储唯一有效的key值，对index进行遍历，将index的每个key值进行读取并写入备用文件即可，此时的读取操作依然通过index进行读取，最后在将备用文件的内容移入原文件中。时间复杂度为O(n)，能够实现合并多次写⼊的 key，只保留最终有效的⼀项 ，合并过程中，移除被删除的 key以处理文件膨胀的操作。

## 设计

### File Base

内存空间有限，不能大量存储数据，而且如果发生断电等操作内存中存储的数据将会丢失，因此考虑将数据备份到磁盘中，传统磁盘的顺序读、写性能远超过随机读、写，不需要管理⽂件“空洞”。

### index索引

读（GET）操作需要扫描整个 Append-Only File，效率较低，时间复杂度为O(n)，因此考虑增加⼀个索引（Index），保存当前数据库中每⼀个 key在 Append-Only File 中的位置（Offset），索引保存在内存中

#### 建立索引

index索引通过哈希表实现，在打开数据库时自动建立索引，遍历 Append-Only File 中的 K-V Record，并在索引中保存读取的 Key 及 Record 的位置（Offset）

```c++
void createIndex(KVDBHandler* handler) {     //建立哈希表类型索引，存储key和key所在的位置
	std::string value = "";
	handler->in.seekg(0, std::ios::beg);
	int  _key_length, _value_length, pos = 0;
	char temp;
	std::string _key, _value;
	while (handler->in.read((char*)&_key_length, sizeof(int))) {  //对文件进行遍历
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
```

#### 使用索引

##### get操作

使用索引之后，读（GET）操作只需要访问索引（Index）， 若 Key 在索引中，则从索引指向的 Append-Only File 中对应的 K-V Record 中读取数据； 若索引中 Key 不存在，则直接返回结果 ，读操作的时间复杂度能够从 O(n) 降低到 O(1)

```c++
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
	else 
		return 0;
}
```

##### purge操作

使用索引之后，purge操作只需要遍历索引，将索引中的键值对依次set到备用文件中，在将备用文件内容移入原文件。purge操作的时间复杂度能够从O(n)降低到O(1)

```c++
int purge(std::string file) {
	std::string str = "D:\\tool.txt"; //备用文件的存放路径，可自定义修改
	std::ofstream ofs(str.c_str());
	if (ofs.good()) {          //清空备用文件内容
		remove(str.c_str());
	}
	std::ifstream _in;
	unordered_map<string, int>::iterator iter;
	KVDBHandler handler((const std::string)file);
	for (iter = index.begin(); iter != index.end(); iter++) {  //索引存放有效的键值对，根据索引重新建立文件
		flag = true;
		KVDBHandler _handler("D:\\tool.txt");
		string value;
		get(&handler, iter->first, value);
		set(&_handler, iter->first, value);
	}
	flag = false;
	_in.open("D:\\tool.txt");   //将备用文件的内容重新转移回源文件内
	std::ofstream out;
	out.open(file);
	out << _in.rdbuf();
	_in.close();
	out.close();
	return 1;
}
```

#### 维护索引

##### set操作

set操作中，如果set的key已存在，则修改索引中对应key的offest值，如果set的key不存在，则自动将key与key所在的offest加到索引中

##### del操作

del操作中，先按原⽅案将表示删除操作的特殊的 K-V Record 写⼊ Append-Only File 中，再将索引中的 key 删除

##### purge操作

执行purge操作之后，会对purge后的文件重新建立一次索引



### expire操作

本操作可以设定需要过期的key值，以及key的生存周期(s)，key将会在生存周期结束时过期(del)

每对一个key设定过期时间时将过期时间自动转化为一个int值，存放在一个值为<string,int>的最小堆中，每次使用get操作读取前通过while操作判断最小堆堆顶的值是否过期，若过期则将int对应的key删除。

因为此时最小堆存放在内存中，如果退出程序最小堆将会清空，需要将时间写入一个文件中，我的想法为新建一个专门存储过期时间的文件，每次使用过期操作时将key和int写入文件中，过期后在写入一个key和0到文件中。打开程序时扫描一遍时间文件，将最新的key和int写入map里，如果int为0则删除原本的key，否则更新，最后遍历map重建最小堆实现过期操作。坏处是一直处于打开程序时不能新建索引从而导致在程序打开期间设定过期并确实过了过期时间仍能够读取，因此规定过期操作只能重新打开数据库才能生效。解决这个问题可以在每写入一个过期时间时重新扫描一遍文件重新建立最小堆，不过会耗费大量的时间，对我个人而言写入如此短的过期时间是没有必要的，不如直接del该key，因此不支持短时间的过期操作，同样不支持一直打开该数据库进行记录。

建立最小堆时间复杂度为O(n)，出堆删除时间复杂度为O(log(n))

```c++
int expires(KVDBHandler* handler, const std::string& key, int n) {
	int key_length = key.length();
	handler->out.write((char*)&key_length, sizeof(int));    //向过期文件内写入需要过期的key和过期时间
	handler->out.write(key.c_str(), key.length());
	handler->out.write((char*)&n, sizeof(int));
	return 1;
}
```



### LRU Cache

磁盘操作⽐内存要慢，在内存中短暂存储短期内操作的数据可以写入内存中，重复读取时可以通过内存进行读取操作，可以通过哈希链表使用LRU Cache进行实现，LRU Cache分为put，get，delete三个操作

```c++
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

```

#### (1)put

首先判断put的key值在LRU中是否存在，若存在则把该节点移到表头。若不存在新建值为key和value的节点。在哈希表中建立key和节点的对应关系，并将该节点移到表头，然后判断此时的size是否比设定值大，如果大则将最后一个节点删除，并在哈希表中删除该节点对应的key，时间复杂度为O(1)

```c++
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
```

#### (2)get

直接在哈希表查找对应的key值，查找不到则查找失败，查找成功则将查找到的节点移到链表表头，然后返回查找到的key值，时间复杂度为O(1)

```c++
string LRUCache::get(string key) {
	if (!cache.count(key)) {
		return "";
	}
	DLinkedNode* node = cache[key];
	moveToHead(node); 
	return node->value;
}
```

#### (3)del

对哈希链表进行遍历，若查找到对应的key值则首先在链表中移除次节点，然后在哈希表中erase该节点的key值，时间复杂度为O(n)

```c++
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
```



使用get操作时首先在LRU中使用LRU的get操作，如果查找失败进行index的get，如果成功则对查找到的键值对使用LRU的put操作。如果对key进行删除时也会同时使用LRU的del操作。LRU设定不会保存，每次关闭程序自动释放，进行查找时自动建立，因一直打开程序通过过期操作已删除的key仍可通过LRU查找



## 调试

### 分支

该项目的流程图如下

![流程图](https://github.com/w-scyber/K-V-database/blob/master/images/%E6%97%A5%E5%BF%97.png)

### 日志

本项目对日志类进行简单实现，通过对应的string文件名打开对应文件进行日志记录，包含一个成员函数，能够将传入的string写入文件中，并在前面加入实现该操作的时间。每次进行set，get，del等操作时会向日志中记录对应情况，便于调试查找错误。

```c++
class logger
{
public:
	logger(const char* filepath);
	~logger(void);
	bool write(const char* format, ...);
private:
	static int preMark(char* buffer);
private:
	FILE* fp;
	char   m_buffer[LOG_BUFFER_SIZE];
};
```

部分日志使用记录如下

![日志](https://github.com/w-scyber/K-V-database/blob/master/images/%E6%B5%81%E7%A8%8B%E5%9B%BE.png)

## 代码设计

### (1)get和purge的优化

采用哈希表类型的index索引，能够提高get和purge的时间效率，不需要在对文件进行整遍遍历，可以直接根据index索引进行get和purge操作

### (2)expire的优化

个人设计不支持短时间过期操作（确实需要需重新打开数据库重建过期最小堆），只在打开数据库时扫描过期文件建立最小堆，能够较为简单的解决重新set相同的key，重新设置key的过期时间等问题，不需要额外花费时间。如果使用平衡二叉树存储过期时间，重新设置key的过期时间会删除并新增节点，会花费更多时间。



## 问题及解决

最开始不清楚KVDBHandler这个类应该如何实现，成员对象是什么，以及如何向文件内写入内容，从网上查阅了大量资料才逐一解决，而且最开始比如写入32时以为是将其转化为二进制，然后将每个0和1通过ofstream>>直接进行写入，后来通过老师引导才发现是直接通过write和read写入和读取文件。

写purge操作之前以为是直接对原文件进行修改，不清楚应该怎么实现，后面得到启发使用一个备用文件临时存放内容，最开始的purge操作时间复杂度很高，不过在实现index索引后时间复杂度也大大降低了。

写日志类时最开始查阅网上的内容，发现基本看不懂如何实现，也不清楚该如何使用。于是按照自己的想法对日志类进行实现，每进行操作时向文件中写入相应内容自己查阅方便即可。工程化的日志类还需要设置不同的等级，不过目前实现的数据库功能较少，应该用不到不同的调试层级，于是没有进行设置。在日志类的实现过程中因为在实现不同操作时经常会套用其他操作，比如使用expire操作时会使用get操作判断要过期的key值是否存在，最开始思考了很久如果不将expire操作中套用的get操作写入日志中，后面发现写入才是正常的，不写入的话反而会丢失部分内容。

