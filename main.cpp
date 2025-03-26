#include "actions.h"
#include "storage.h"
#include <cstdio>
#include <cstdlib>

int main() {
    // 读取全局参数
    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);

    // 全局预处理阶段：读取三个分块数据（删除、写入、读取的统计数据，此处用 %*d 跳过）
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
