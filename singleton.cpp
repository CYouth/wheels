#include<iostream>
#include<mutex>
using namespace std;

mutex m;//������

//ʹ�õ�������ģʽ��ʹ�õ�ʱ��ų�ʼ�������˷��ڴ�

class Singleton
{
public:
	static Singleton* getInstance();//��ȡ������

private:
	Singleton();//private�Ĺ��캯��
	static Singleton* instance;//��̬�������ⲿ����

};

Singleton::Singleton()
{
}

Singleton* Singleton::instance = nullptr;

//���̰߳�ȫ�����Ƕ��̰߳�ȫ
Singleton* Singleton::getInstance(){
	if (!instance) {
		instance = new Singleton();
	}
	return instance;
}

//���̰߳�ȫ��˫���ж�
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

//���̰߳�ȫ�������ŵ�д��
//����c++11��call_once
once_flag f;
Singleton * Singleton::getInstance()
{
	call_once(f, []() {instance = new Singleton(); });//��������
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