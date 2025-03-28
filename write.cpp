#include "write.h"
#include "storage.h"
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <array>
#include <algorithm>
using namespace std;
// 磁盘剩余空间大的优先
array<int, 3> select_disk1(int id)
{
    // 为每个副本处理写入
    vector<array<int, 2>> vec_disk_size; //**暂时的，用于找到占用单元最少的磁盘
    for (int j = 1; j <= N; j++)
    {
        vec_disk_size.push_back({disk_size[j][0], j});
    }
    sort(vec_disk_size.begin(), vec_disk_size.end());
    return {vec_disk_size[0][1], vec_disk_size[1][1], vec_disk_size[2][1]};
}

array<int, 3> select_disk2(int id)
{
    // 为每个副本处理写入
    vector<array<int, 2>> vec_disk_size; //**暂时的，用于找到占用单元最少的磁盘
    for (int j = 1; j <= N; j++)
    {
        vec_disk_size.push_back({disk_size[j][object[id].tag], j});
    }

    sort(vec_disk_size.begin(), vec_disk_size.end(), [&](const array<int, 2> &x, const array<int, 2> &y)
         {
        if (x[0] == y[0]) return disk_size[x[1]][0] < disk_size[y[1]][0];
        return x[0] > y[0]; }); // 当前tag在磁盘中数量多的优先

    int now = 0, cnt = 0;
    array<int, 3> res;
    while (cnt < 3)
    {
        assert(now < N);
        int t = vec_disk_size[now][1];
        if (V - disk_size[t][0] >= object[id].size)
            res[cnt] = t, cnt++;
        now++;
    }
    return res;
}

inline void write_single_rep2(int disk_id, int id, int rep_id)
{
    int siz = object[id].size;
    int start = ceil((long double)tag_weights[object[id].tag - 1] * V / total_tag_weights);
    int current_write_point = 0;
    if (object[id].tag & 1)
    {
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
    else
    {
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
}

inline void write_single_rep4(int disk_id, int id, int rep_id)
{
    int siz = object[id].size;
    total_object_num += siz;
    // 维护前缀和数组
    for (int ltn = object[id].tag; ltn <= M; ltn++) // ltn -> larger than and equal to now object.id
        tag_num[ltn] += siz;

    static int history_tag_num[20], history_total_object_num;
    if (timestamp % FRE_PER_SLICING == 1)
    {
        memcpy(history_tag_num, tag_num, sizeof history_tag_num);
        history_total_object_num = total_object_num;
    }

    int start = ceil((long double)history_tag_num[object[id].tag - 1] * V / history_total_object_num);
    int current_write_point = 0;
    if (object[id].tag & 1)
    {
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
    else
    {
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
}

inline void write_single_rep5(int disk_id, int id, int rep_id)
{
    int siz = object[id].size;
    int state = ceil((long double)timestamp / FRE_PER_SLICING);
    int start = ceil((long double)fre_tag_num[object[id].tag - 1][state] * V / fre_total_num[state]);
    int current_write_point = 0;
    if (object[id].tag & 1)
    {
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
    else
    {
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
}

inline void write_single_rep6(int disk_id, int id, int rep_id)
{
    int siz = object[id].size;
    int start = ceil((long double)tag_weights[object[id].tag - 1] * V / total_tag_weights);
    int current_write_point = 0;
    if (disk_id & 1)
    {
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            if (current_write_point == siz / 2 * 2)
                break;
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
            }
        }
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            if (current_write_point == siz)
                break;
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
            }
        }
    }
    else
    {
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            if (current_write_point == siz / 2 * 2)
                break;
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
            }
        }
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            if (current_write_point == siz)
                break;
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
            }
        }
    }
}

inline void write_single_rep7(int disk_id, int id, int rep_id)
{
    int siz = object[id].size;
    int start = ceil((long double)tag_weights[object[id].tag - 1] * V / total_tag_weights);
    int current_write_point = 0;
    if (object[id].tag & 1)
    {
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            int free_units = 0;
            for (; free_units < siz; free_units++)
            {
                if (disk[disk_id][(i1 + free_units) % V + 1])
                    break;
            }
            if (free_units == siz)
            {
                for (;; i1++)
                {
                    disk[disk_id][i1 % V + 1] = id;
                    object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                    disk_uid[disk_id][i1 % V + 1] = current_write_point;
                    if (current_write_point == siz)
                        break;
                }
       
                return;
            }
            
        }
        for (int i1 = start; i1 <= V + start - 1; i1++)
        {
            int free_units = 0;
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
    else
    {
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            int free_units = 0;
            for (; free_units < siz; free_units++)
            {
                if (disk[disk_id][(i1 - free_units + V) % V + 1]) break;
            }
            if (free_units == siz)
            {
                i1 += V;
                for (;; --i1)
                {
                    disk[disk_id][i1 % V + 1] = id;
                    object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                    disk_uid[disk_id][i1 % V + 1] = current_write_point;
                    if (current_write_point == siz) break;
                }
                return;
            }
        }
        for (int i1 = V + start - 1; i1 >= start; --i1)
        {
            if (disk[disk_id][i1 % V + 1] == 0)
            {
                disk[disk_id][i1 % V + 1] = id;
                object[id].unit[rep_id][++current_write_point] = i1 % V + 1;
                disk_uid[disk_id][i1 % V + 1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
}

void write_action()
{
    int n_write; // 当前时间片写入请求数量
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++)
    {
        int id, size, tag;
        // 读取对象编号和对象大小，%*d 表示忽略对象标签（或其他无用信息）
        scanf("%d%d%d", &id, &size, &tag);
        // 初始化该对象的请求链为空
        object[id].last_request_point = 0;
        object[id].tag = tag;
        object[id].size = size;
        auto dist_list = select_disk2(id);
        for (int j = 0; j < 3; j++)
        {
            int disk_id = dist_list[j];
            disk_size[disk_id][0] += size; // 更新占用单元数
            disk_size[disk_id][tag] += size;
            object[id].replica[j + 1] = disk_id;
            object[id].unit[j + 1] = static_cast<int *>(malloc(sizeof(int) * (size + 1)));
            object[id].is_delete = false;
            write_single_rep7(disk_id, id, j + 1);
        }

        // 输出写入结果：先输出对象编号
        printf("%d\n", id);
        // 对于每个副本，输出硬盘编号和写入的存储单元位置
        for (int j = 1; j <= REP_NUM; j++)
        {
            printf("%d ", object[id].replica[j]);
            for (int k = 1; k <= size; k++)
            {
                printf(" %d", object[id].unit[j][k]);
            }
            printf("\n");
        }
    }
    fflush(stdout);
}