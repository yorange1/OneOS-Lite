#include <stdio.h>
#include <os_task.h>

#ifdef __cplusplus
extern "C" {
#endif

void test_task(void *parameter)
{
    printf("this is test task!\r\n");
}

#ifdef __cplusplus
}
#endif

