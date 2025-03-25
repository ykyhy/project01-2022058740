#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

int main() {
    int pid = getpid();     // 현재 프로세스의 PID
    int ppid = getppid();   // 부모 프로세스의 PID

    printf("My student ID is 2025123456\n");
    printf("My pid is %d\n", pid);
    printf("My ppid is %d\n", ppid);

    exit(0);
}
