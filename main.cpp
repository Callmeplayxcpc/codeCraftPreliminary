#include "actions.h"
#include "storage.h"
#include <cstdio>
#include <cstdlib>

int read_cnt[20];

int main()
{
    // 读取全局参数
    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);

    // 全局预处理阶段：读取三个分块数据（删除、写入、读取的统计数据，此处用 %*d 跳过）
    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++)
        {
            scanf("%*d");
        }
    }
    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++)
        {
            scanf("%*d");
        }
    }
    int read_tot = 0;
    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int t;
            scanf("%d", &t);
            read_cnt[i] += t;
            read_tot += t;
        }
    }
    for (int i = 1; i <= M; ++i)
    {
        tag_weights[i] = ceil((long double)read_cnt[i] / read_tot * 1e6);
        total_tag_weights += tag_weights[i];
        tag_weights[i] += tag_weights[i - 1];
    }
    // 输出预处理完成标志
    printf("OK\n");
    fflush(stdout);

    // 按时间片循环处理各类交互事件
    for (int t = 1; t <= T + EXTRA_TIME; t++)
    {
        timestamp_action();
        delete_action();
        write_action();
        read_action();
    }
    clean();
    return 0;
}
