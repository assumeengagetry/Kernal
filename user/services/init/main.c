#include <stddef.h>
#include <stdint.h>
#include "../../../include/types.h"
#include "../../../include/sched.h"

// 初始服务进程，负责启动其他系统服务

void init_service_start() {
    // 启动顺序：
    // 1. devd - 设备管理守护进程
    // 2. vfsd - 虚拟文件系统守护进程
    // 3. netd - 网络守护进程
    
    // TODO: 实现服务启动逻辑
    while(1) {
        // 主循环监控服务状态
    }
}

int main() {
    init_service_start();
    return 0;
}
