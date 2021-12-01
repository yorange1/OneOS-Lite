
#include <os_task.h>
#include <iostream>     //包含头文件iostream
using namespace std;    //使用命名空间std

extern "C" {
int test(void);
void test_task(void *parameter);
}
int test()
{
    os_task_t *task;
    cout << "This is a C++ program!\r\n" << endl;

    task = os_task_create("test", test_task, NULL, 512, 5);
    os_task_startup(task);
    return 0;
}
