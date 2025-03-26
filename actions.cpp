#include "actions.h"
#include "storage.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>

using namespace std;
void timestamp_action()
{ // 仅输入输出，不用优化
    int timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);
    fflush(stdout);
}

void do_object_delete(const int *object_unit, int *disk_unit, int size)
{ //
    for (int i = 1; i <= size; i++)
    {
        disk_unit[object_unit[i]] = 0;
    }
}

set<int> disk_vector[20]; //**容器，存储每个硬盘的所有待读取单元
int disk_size[20];        //**存储磁盘的被占用单元数，因为写入策略是优先挑空闲空间大的磁盘
void delete_action()
{
    int n_delete;                   // 当前时间片需要删除的对象数量
    int abort_num = 0;              // 累计被取消的读请求数量
    static int _id[MAX_OBJECT_NUM]; // 存储删除对象的编号

    // 读取删除对象的数量
    scanf("%d", &n_delete);
    for (int i = 1; i <= n_delete; i++)
    {
        scanf("%d", &_id[i]);
    }

    // 对于每个要删除的对象，遍历其相关的读请求链（通过 last_request_point 链表维护）
    for (int i = 1; i <= n_delete; i++)
    {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        // 遍历该对象对应的所有请求，统计未完成的请求
        while (current_id != 0)
        {
            if (!request[current_id].is_done)
            {
                abort_num++;
            }
            current_id = request[current_id].prev_id;
        }
    }

    // 输出被取消的请求数量
    printf("%d\n", abort_num);
    // 再次遍历，输出每个被取消的读请求编号，并清理对象对应硬盘中的数据
    for (int i = 1; i <= n_delete; i++)
    {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        while (current_id != 0)
        {
            if (!request[current_id].is_done)
            {
                printf("%d\n", current_id);
            }
            current_id = request[current_id].prev_id;
        }
        // 对于该对象的每个副本，清除对应硬盘中的数据块
        for (int j = 1; j <= REP_NUM; j++)
        {
            do_object_delete(object[id].unit[j], disk[object[id].replica[j]], object[id].size);
            for (int k = 1; k <= object[id].size; k++)
            { //**删除对象时候顺便把磁盘中相关的待读取单元都删了
                if (disk_vector[object[id].replica[j]].count(disk[object[id].replica[j]][k]))
                    disk_vector[object[id].replica[j]].erase(disk[object[id].replica[j]][k]);
            }
            disk_size[object[id].replica[j]] -= object[id].size; //**更新占用单元数
        }

        // 标记该对象已被删除
        object[id].is_delete = true;
    }
    fflush(stdout);
}

//单个副本的写入策略
inline void write_single_rep1(int disk_id,int id,int rep_id){
    int siz=object[id].size;
    int current_write_point = 0;
    //**存放策略为把原块拆成size/2个大小为2的块和size%2个大小为1的块
    //**把大小为2的块从前往后放，大小为1的块从后往前放
    //**优点是减少碎片化，缺点是在同一个磁盘中，同一对象的不同块可能隔得很远
    for (int i1 = 1; i1 <= V; i1++)
    { // 放大小为2的块
        if (current_write_point == siz / 2 * 2)
            break;
        if (disk[disk_id][i1] == 0)
        {
            disk[disk_id][i1] = id;
            object[id].unit[rep_id][++current_write_point] = i1;
            disk_uid[disk_id][i1] = current_write_point;
            if (current_write_point == siz / 2 * 2)
                break;
        }
    }
    if (current_write_point < siz)
    {
        for (int i1 = V; i1 >= 1; i1--)
        { // 放大小为1的块
            if (disk[disk_id][i1] == 0)
            {
                disk[disk_id][i1] = id;
                ;
                object[id].unit[rep_id][++current_write_point] = i1;
                disk_uid[disk_id][i1] = current_write_point;
                if (current_write_point == siz)
                    break;
            }
        }
    }
}

//利用tag设置起点
inline void write_single_rep2(int disk_id,int id,int rep_id){
    int siz=object[id].size;
    int start=(object[id].tag-1)*(V/M);
    int current_write_point = 0;
    for (int i1 = start; i1 <= V+start-1; i1++)
    { 
        if (disk[disk_id][i1%V+1] == 0)
        {
            disk[disk_id][i1%V+1] = id;
            object[id].unit[rep_id][++current_write_point] = i1%V+1;
            disk_uid[disk_id][i1%V+1] = current_write_point;
            if(current_write_point==siz) break;
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
        scanf("%d%d%*d", &id, &size, &tag);
        object[id].tag = tag;
        // 初始化该对象的请求链为空
        object[id].last_request_point = 0;
        // 为每个副本处理写入
        vector<array<int, 2>> vec_disk_size; //**暂时的，用于找到占用单元最少的磁盘

        for (int j = 1; j <= N; j++)
        {
            vec_disk_size.push_back({disk_size[j], j});
        }
        sort(vec_disk_size.begin(), vec_disk_size.end());
        for (int j = 0; j < 3; j++)
        {
            int disk_id=vec_disk_size[j][1];
            disk_size[disk_id] += size; // 更新占用单元数
            object[id].replica[j+1] = disk_id;
            object[id].unit[j+1] = static_cast<int *>(malloc(sizeof(int) * (size + 1)));
            object[id].size = size;
            object[id].is_delete = false;
            write_single_rep2(disk_id,id,j+1);
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

int ptr[20], last_time[20];
//**ptr代表第i个磁盘的指针在哪个单元，为了方便实现，它的值是0-V-1，实际位置是ptr[i]+1
//**last_time表示第i个磁盘上个时间片最后一次操作的读取时间是多少，是为了跨时间片维护，如果该操作是移动，那就置为大值
array<int, 2> cal(int pos, vector<int> rest, int rest_time, int pre_time)
//**计算当前顺序下读取rest中所有单元所需时间是多少，这个函数在该版本代码中没有被使用到
{ // 返回值第一位是花费时间片数，第二位是 -当前时间片剩余时间，即返回值越小越好
    int timestamp = 0;
    while (true)
    {
        while (rest_time)
        {
            int to = rest.back();
            int dis = to - pos - 1;
            if (dis < 0)
                dis += V;
            if (!dis) // 针就在要读的位置
            {
                int cost_time = min(64, max(16, (int)(ceil(0.8 * pre_time) + 0.5)));
                if (cost_time > rest_time)
                    rest_time = 0;
                else
                    rest_time -= cost_time, pos = (pos + 1) % V, pre_time = cost_time, rest.pop_back();
                if (!rest.size())
                {
                    return {timestamp, -rest_time};
                }
            }
            else if (dis <= rest_time) // 走过去够时间
            {
                rest_time -= dis;
                while (dis--)
                    pos = (pos + 1) % V;
                pre_time = 100;
            }
            else if (rest_time == G) // 可以跳过去
            {
                rest_time = 0;
                pre_time = 100;
                pos = to - 1;
            }
            else // 不能跳，还走不到，只能尽量走
            {
                while (rest_time--)
                    pos = (pos + 1) % V;
                rest_time = 0;
                pre_time = 100;
            }
        }
        timestamp++, rest_time = G;
    }
    // rest_time越多越好，直接给负值，不考虑结果中pre_time对未来的影响了，有点复杂
}
int cal_min_dist(int disk_id, int to)
//**计算 指针以及第disk_id个磁盘中所有待读单元 到to单元的最短距离
//**神奇的贪心策略，但是很奇怪
//**这个贪心没有干过 按磁盘待读取单元数多少去排序 的策略
//**而且注释掉 if (disk_vector[disk_id].size())整个block后，本地测试结果完全没有变化（未交）
{
    int res = to - ptr[disk_id] - 1;
    if (res < 0)
        res += V;
    if (disk_vector[disk_id].size())
    {
        int tmp;
        auto it = disk_vector[disk_id].upper_bound(to);
        if (it == disk_vector[disk_id].begin())
            tmp = to - *prev(disk_vector[disk_id].end());
        else
            tmp = to - *prev(it);
        if (tmp < 0)
            tmp += V;
        to = min(res, tmp);
    }
    return res;
}
int cal_to_pos(int disk_id, int pos) //**计算第disk_id个磁盘从第pos个单元出发下一个待读取单元在哪
{
    auto it = disk_vector[disk_id].lower_bound(pos);
    if (it == disk_vector[disk_id].end())
        return *disk_vector[disk_id].begin();
    else
        return *it;
}

void read_action()
{
    int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++)
    {
        scanf("%d%d", &request_id, &object_id);
        request[request_id].object_id = object_id;
        request[request_id].prev_id = object[object_id].last_request_point;
        object[object_id].last_request_point = request_id;
        request[request_id].is_done = false;

        for (int k = 1; k <= object[object_id].size; k++)
        {
            request[request_id].rest.insert(k);
            int d = 1;
            for (int j = 1; j <= 3; j++)
            {
                int mn = object[object_id].replica[d], now = object[object_id].replica[j];
                // 第d个副本是当前最优的副本，第j个副本是现在的副本
                // mn代表第d个副本对应的磁盘编号，now代表第j个
                if (disk_vector[mn].size() > disk_vector[now].size())
                    d = j; //**按待处理单元数判断磁盘优劣
            }
            int mn = object[object_id].replica[d];
            disk_vector[mn].insert(object[object_id].unit[d][k]);    // 待处理单元放入磁盘容器
            object[object_id].request[k].push_back({request_id, k}); // 这个vec存储该对象的第i个块与哪些请求相关，存的值是request_id
        }
    }

    vector<int> finish; // 此次完成的请求
    for (int i = 1; i <= N; i++)
    {
        int rest_time = G;           // 当前时间片剩余时间
        string res = "";             //**该磁盘在该时间片内的操作
        int pre_time = last_time[i]; // 上次读取时间
        if (!pre_time)
            pre_time = 100; // 没有给last_time赋初始值，当它是0时说明之前没有用过

        while (rest_time)
        {
            if (!disk_vector[i].size())
                break;                          // 这个磁盘没有待读的单元了就退出
            int to = cal_to_pos(i, ptr[i] + 1); //**读取顺序策略是不管进入容器顺序，优先读取距离最近的
            int dis = to - ptr[i] - 1;          // 距离目标单元的距离
            if (dis < 0)
                dis += V;
            if (!dis) // 针就在要读的位置
            {
                int cost_time = min(64, max(16, (int)(ceil(0.8 * pre_time) + 0.5))); // 维护读取时间，+0.5纯多余
                if (cost_time > rest_time)
                    rest_time = 0; // 不够时间读，又不能走，直接结束时间片
                else
                {
                    rest_time -= cost_time;
                    res += "r";
                    ptr[i] = (ptr[i] + 1) % V;
                    pre_time = cost_time;
                    for (auto [request_id, uid] : object[disk[i][to]].request[disk_uid[i][to]]) // 更新相关请求
                    {

                        if (request[request_id].is_done)
                            continue;
                        if (!request[request_id].rest.count(uid))
                            continue;
                        request[request_id].rest.erase(uid);
                        if (!request[request_id].rest.size()) //**该请求被完成
                        {
                            finish.push_back(request_id);
                            request[request_id].is_done = true;
                        }
                    }
                    vector<array<int, 2>>().swap(object[disk[i][to]].request[disk_uid[i][to]]); // 清空并释放空间
                    disk_vector[i].erase(to);
                }
            }
            else if (dis <= rest_time) // 走过去够时间
            {
                rest_time -= dis;
                while (dis--)
                    res += "p", ptr[i] = (ptr[i] + 1) % V;
                pre_time = 100; // 上一次不是读取，直接赋值为大值
            }
            else if (rest_time == G) // 可以跳过去
            {
                res += "j " + to_string(to);
                rest_time = 0;
                pre_time = 100;
                ptr[i] = to - 1;
            }
            else // 不能跳，还走不到，只能尽量走
            {
                while (rest_time--)
                    res += "p", ptr[i] = (ptr[i] + 1) % V;
                rest_time = 0;
                pre_time = 100;
            }
        }
        last_time[i] = pre_time; // 更新
        if (res[0] != 'j')
            res += "#";
        cout << res << '\n';
    }

    cout << finish.size() << '\n';
    for (int v : finish)
        cout << v << '\n';

    fflush(stdout);
}

void clean()
{
    for (auto &obj : object)
    {
        for (int i = 1; i <= REP_NUM; i++)
        {
            if (obj.unit[i] == nullptr)
                continue;
            free(obj.unit[i]);
            obj.unit[i] = nullptr;
        }
    }
}