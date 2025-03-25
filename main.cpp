#include "actions.h"
#include "storage.h"
#include <cstdio>
#include <cstdlib>

int main() {
    // 读取全局参数
    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);
    /*
        T:时间片数量 1<=T<=86400
        M:对象标签数 1<=M<=16
        V:每个硬盘的空间有多少 1<=V<=16384
        G:每个磁头每个时间片最多消耗的令牌数 64<=G<=1000
    */

    // 全局预处理阶段：读取三个分块数据（删除、写入、读取的统计数据）
    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%d",&fre_del[i][j]);
        }
    }
    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%d",&fre_write[i][j]);
        }
    }
    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%d",&fre_read[i][j]);
        }
    }
    
    // 输出预处理完成标志
    printf("OK\n");
    fflush(stdout);

    // 初始化硬盘指针（起始位置）
    for (int i = 1; i <= N; i++) {
        disk_point[i] = 1;
    }

    // 按时间片循环处理各类交互事件
    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        timestamp_action();
        delete_action();
        write_action();
        read_action();
    }
    clean();
    return 0;
}
