#include<iostream>
#include<mutex>
using namespace std;

mutex m;//互斥量

//使用的是懒汉模式，使用的时候才初始化，不浪费内存

class Singleton
{
public:
	static Singleton* getInstance();//获取对象函数

private:
	Singleton();//private的构造函数
	static Singleton* instance;//静态必须在外部定义

};

Singleton::Singleton()
{
}

Singleton* Singleton::instance = nullptr;

//单线程安全，不是多线程安全
Singleton* Singleton::getInstance(){
	if (!instance) {
		instance = new Singleton();
	}
	return instance;
}

//多线程安全，双重判断
Singleton * Singleton::getInstance()
{
	if (!instance) {
		lock_guard<mutex> guard(mutex);//RAII
		if (!instance) {
			instance = new Singleton();
		}
	}
	return instance;
}

//多线程安全，更优雅的写法
//利用c++11的call_once
once_flag f;
Singleton * Singleton::getInstance()
{
	call_once(f, []() {instance = new Singleton(); });//匿名函数
	return instance;
}


void print() {
	Singleton* ptr = Singleton::getInstance();
	cout << ptr << endl;
}

//test
int main() {
	for (int i = 0; i < 100; i++) {
		thread t(print);
		t.join();
	}
	return 0;
}