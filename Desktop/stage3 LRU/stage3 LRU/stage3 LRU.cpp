#include"KVDBHandler.h"
#include"LRUCache.h"
#include"logger.h"
using namespace std;
#define LRU_NUM 100
LRUCache LRU(LRU_NUM);  //建立LRU缓存，最多缓存LRU_NUM个

int main()
{
	string logger_text;
	logger log("D:\\logger.txt");   //设置日志存储路径
	std::string file;
	std::string op;
	std::string key, value;
	char ch;

	std::cout << "请输入要打开的数据库：" << std::endl << "格式如：D:\\database.txt" << std::endl;

	std::cin >> file;
	for (int i = 0; i < file.length(); i++) {
		if (file[i + 1] == '.') {
			f += file[i];
			f += "time";
		}
		else f += file[i];
	}   //对每个数据库设置不同的过期存储文件

	KVDBHandler handler((const std::string)file);
	KVDBHandler ttime(f);
	createIndex(&handler);   //建立index索引
	createTime(&ttime);    //建立储存key过期时间的索引

	if (file_size(file.c_str()) > 10000) {
		//文件大小超过指定值后进行purge操作
		purge(file);
		index.clear();
		KVDBHandler handler((const std::string)file);
		createIndex(&handler);   //purge后重新建立索引
	}

	std::cout << "请输入数字对数据库进行操作" << std::endl;
	std::cout << "[1]写入数据" << std::endl << "[2]读取数据" << std::endl << "[3]删除数据" << std::endl;
	std::cout << "[4]设置key过期时间" << endl << "[5]结束程序" << std::endl;

	while (true)
	{
		KVDBHandler handler((const std::string)file);

		KVDBHandler ttime(f);
		std::cin >> op;
		ch = getchar();
		if (op == "1") {
			std::cout << "请输入需要插入的key值和value值" << std::endl;
			getline(std::cin, key);
			getline(std::cin, value);
			if(set(&handler, key, value)==1)
				expires(&ttime, key, 0);  //如果对key的值重新设置，清除原有key的过期操作
			logger_text = "进行set操作 key为" + key + ",value为" + value;
			if(value!="")
			log.write("%s", logger_text.c_str());
			LRU.del(key);
			std::cout << "插入成功" << std::endl;
		}
		else if (op == "2") {
			time_t timep;
			time(&timep);
			if (!pq.empty()) {
				while (pq.top().second <= timep) {
					del(&handler, pq.top().key);
					expires(&ttime, pq.top().key, 0);    //如果堆顶元素过期则删除过期的key值
					pq.pop();
					if (pq.empty()) break;
				}
			}
			std::cout << "请输入需要获得value的key值 " << std::endl;
			getline(std::cin, key);

			if (LRU.get(key)!="") {          //判断LRU中是否存在key值，并对此进行操作
				std::cout << "value值为" << value << std::endl;
				//cout << "缓存" << endl;
			}
			else {
				get(&handler, key, value);
				if (!value.empty()) {
					std::cout << "value值为" << value << std::endl;
					logger_text = "进行get操作 key为" + key + ",检索到的value为" + value;
					LRU.put(key, value);          //LRU中没有则将其加到LRU中
				}
				else {
					logger_text = "进行get操作 key为" + key + "检索失败";
					std::cout << "查找失败" << std::endl;
				}
			}
			log.write("%s", logger_text.c_str());
		}
		else if (op == "3") {
			
			log.write("%s", logger_text.c_str());
			std::cout << "请输入需要删除的key值 " << std::endl;
			getline(std::cin, key);
			if (del(&handler, key)) {
				std::cout << "删除成功" << std::endl;
				LRU.del(key);             //删除时同时从LRU中删除
				expires(&ttime, key, 0);   //清除key的过期时间
				logger_text = "进行del操作 key为" + key + "删除成功" ;
			}
			else {
				std::cout << "您要删除的key不存在" << std::endl;
				logger_text = "进行del操作 key为" + key + "删除失败";
			}
			log.write("%s", logger_text.c_str());
		}
		else if (op == "4") {
			int t;
			std::cout << "请输入需要设置到期时间的key值 " << std::endl;
			getline(std::cin, key);
			std::cout << "请输入还需要多少秒到期 " << std::endl;
			cin >> t;
			get(&handler, key, value);
			if (value.empty()) cout << "输入的key不存在" << endl;
			else {
				time_t timep;
				time(&timep);
				t += timep;
				expires(&ttime, key, t);
			}
		}
		else if (op == "5") {
			exit(1);
		}
		else
			std::cout << "请输入有效的命令" << std::endl;
	}
}

