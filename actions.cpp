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

/*
 * 函数：timestamp_action
 * 说明：读取输入中的时间片编号并回显。交互时，每个时间片开始时会先进行时间片同步。
 */
void timestamp_action() {
    int timestamp;
    // 读取输入，忽略第一个字符串（例如 "TIMESTAMP"），直接读取后面的时间片编号
    scanf("%*s%d", &timestamp);
    // 输出同步后的时间片，格式必须与要求一致
    printf("TIMESTAMP %d\n", timestamp);
    fflush(stdout);
}

/*
 * 函数：do_object_delete
 * 说明：辅助函数，在删除操作时将某个对象的各个块在指定硬盘中清空（标记为 0）。
 * 参数：
 *   object_unit - 数组，存储该对象各块所在的存储单元编号
 *   disk_unit   - 对应硬盘的存储单元数组
 *   size        - 对象大小（即对象块数量）
 */
void do_object_delete(const int* object_unit, int* disk_unit, int size) {
    for (int i = 1; i <= size; i++) {
        // 将存储单元中存放该对象块的标记清空（0 表示空闲）
        disk_unit[object_unit[i]] = 0;
    }
}

/*
 * 函数：delete_action
 * 说明：处理删除事件。读取删除的对象编号，取消该对象所有未完成的读请求，
 *       并将对象数据从各个硬盘上清除，同时标记对象已被删除。
 */
void delete_action() {
    int n_delete;      // 当前时间片需要删除的对象数量
    int abort_num = 0; // 累计被取消的读请求数量
    static int _id[MAX_OBJECT_NUM]; // 存储删除对象的编号

    // 读取删除对象的数量
    scanf("%d", &n_delete);
    for (int i = 1; i <= n_delete; i++) {
        scanf("%d", &_id[i]);
    }

    // 对于每个要删除的对象，遍历其相关的读请求链（通过 last_request_point 链表维护）
    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        // 遍历该对象对应的所有请求，统计未完成的请求
        while (current_id != 0) {
            if (!request[current_id].is_done) {
                abort_num++;
            }
            current_id = request[current_id].prev_id;
        }
    }

    // 输出被取消的请求数量
    printf("%d\n", abort_num);
    // 再次遍历，输出每个被取消的读请求编号，并清理对象对应硬盘中的数据
    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        while (current_id != 0) {
            if (!request[current_id].is_done) {
                printf("%d\n", current_id);
            }
            current_id = request[current_id].prev_id;
        }
        // 对于该对象的每个副本，清除对应硬盘中的数据块
        for (int j = 1; j <= REP_NUM; j++) {
            do_object_delete(object[id].unit[j], disk[object[id].replica[j]], object[id].size);
        }
        // 标记该对象已被删除
        object[id].is_delete = true;
    }
    fflush(stdout);
}

/*
 * 函数：do_object_write
 * 说明：辅助函数，将一个对象的所有对象块写入到硬盘中空闲的存储单元里。
 * 参数：
 *   object_unit - 数组，用于记录对象块写入的存储单元编号
 *   disk_unit   - 目标硬盘的存储单元数组
 *   size        - 对象块数量（即对象大小）
 *   object_id   - 当前写入的对象编号（写入标记）
 */
void do_object_write(int* object_unit, int* disk_unit, int size, int object_id) {
    int current_write_point = 0;  // 记录已写入的块数量
    // 遍历硬盘所有存储单元（1 ~ V）
    for (int i = 1; i <= V; i++) {
        if (disk_unit[i] == 0) { // 如果该存储单元为空
            disk_unit[i] = object_id;           // 写入对象ID作为标记
            object_unit[++current_write_point] = i; // 记录写入的位置
            if (current_write_point == size) {   // 如果已写满需要的块数，则结束
                break;
            }
        }
    }
    // 如果未能写满 size 个块，则程序中止（确保数据完整写入）
    assert(current_write_point == size);
}

/*
 * 函数：write_action
 * 说明：处理写入事件。为每个写入的对象生成三个副本，分别写入到不同的硬盘上。
 *       写入过程包括：分配内存、选取写入硬盘、调用 do_object_write 写入空闲存储单元，
 *       并输出写入结果（对象编号、各副本存储硬盘和具体写入的存储单元编号）。
 */
void write_action() {
    int n_write; // 当前时间片写入请求数量
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) {
        int id, size, tag;
        // 读取对象编号和对象大小和对象标签
        scanf("%d%d%d", &id, &size,&tag);
        // 初始化该对象的请求链为空
        object[id].last_request_point = 0;
        // 为每个副本处理写入
        for (int j = 1; j <= REP_NUM; j++) {
            // 计算写入的硬盘编号，保证三个副本写入不同的硬盘
            object[id].replica[j] = (id + j) % N + 1;
            // 为该副本动态分配存储单元数组（大小为 size+1，索引从 1 开始）
            object[id].unit[j] = static_cast<int*>(malloc(sizeof(int) * (size + 1)));
            object[id].size = size;
            object[id].is_delete = false;
            // 将对象块写入对应硬盘
            do_object_write(object[id].unit[j], disk[object[id].replica[j]], size, id);
        }
        
        // 输出写入结果：先输出对象编号
        printf("%d\n", id);
        // 对于每个副本，输出硬盘编号和写入的存储单元位置
        for (int j = 1; j <= REP_NUM; j++) {
            printf("%d ", object[id].replica[j]);
            for (int k = 1; k <= size; k++) {
                printf(" %d", object[id].unit[j][k]);
            }
            printf("\n");
        }
    }
    fflush(stdout);
}

/*
 * 以下全局变量用于读操作中的磁头调度和请求管理：
 *   vec       - 每个硬盘（下标 1~N）对应的请求队列
 *   Siz       - 每个硬盘累积的待处理对象大小，用于负载均衡
 *   dealing   - 当前正在处理的请求编号，按硬盘分开
 *   ptr       - 每个硬盘当前磁头所在的存储单元位置（指针）
 *   last_time - 每个硬盘上一次操作耗时，用于计算后续操作耗时
 */
vector<int> vec[20];
int Siz[20];
int dealing[20];
int ptr[20], last_time[20];

/*
 * 函数：cal
 * 说明：辅助函数，计算磁头从当前位置到达请求中指定位置的时间花费
 *       并模拟磁头读取操作的过程。
 * 参数：
 *   pos       - 当前磁头位置（存储单元编号）
 *   rest      - 待读取的对象块位置列表（需要顺序读取的块）
 *   rest_time - 当前时间片中磁头剩余的令牌数
 *   pre_time  - 上一次操作耗时，用于计算当前读取操作的耗时
 * 返回值：一个数组 {timestamp, -rest_time}，
 *         timestamp 表示经过的时间片数，-rest_time 用于表示剩余令牌（负值表示多余的令牌）
 *
 * 注意：该函数使用暴力模拟，可能存在性能瓶颈，可以考虑优化调度策略。
 */
array<int, 2> cal(int pos, vector<int> rest, int rest_time, int pre_time) {
    int timestamp = 0;
    while (true) {
        while (rest_time) {
            int to = rest.back();  // 待读取的目标存储单元（取最后一个元素）
            int dis = to - pos - 1;
            if (dis < 0) dis += V; // 盘为环形排列，若差值为负则加上硬盘总容量
            if (!dis) { // 磁头已到达目标位置
                // 计算读取操作所消耗的令牌数：
                // 第一次读取消耗 64，之后每次消耗为 max(16, ceil(0.8 * pre_time))
                int cost_time = min(64, max(16, (int)(ceil(0.8 * pre_time) + 0.5)));
                if (cost_time > rest_time)
                    rest_time = 0; // 不足以完成读取，则剩余令牌置为0
                else {
                    rest_time -= cost_time;
                    pos = (pos + 1) % V; // 读取后磁头移动到下一个存储单元
                    pre_time = cost_time; // 更新本次读取耗时
                    rest.pop_back();      // 移除已读取的块
                }
                // 如果所有块都读取完毕，返回模拟结果
                if (rest.empty()) {
                    return {timestamp, -rest_time};
                }
            }
            else if (dis <= rest_time) { // 剩余令牌足够走到目标位置
                rest_time -= dis;
                while (dis--) {
                    pos = (pos + 1) % V;
                }
                pre_time = 100; // 移动过程中采用默认耗时
            }
            else if (rest_time == G) { // 若剩余令牌正好等于初始最大令牌数，允许使用跳跃操作
                rest_time = 0;
                pre_time = 100;
                pos = to - 1; // 跳跃至目标位置前一个位置
            }
            else { // 令牌不足，走尽可能多的步数
                while (rest_time--) {
                    pos = (pos + 1) % V;
                }
                rest_time = 0;
                pre_time = 100;
            }
        }
        // 增加一个时间片，并重置剩余令牌为初始值 G
        timestamp++;
        rest_time = G;
    }
}

/*
 * 函数：read_action
 * 说明：处理读取事件。读取每个请求的对象编号，将请求加入对应硬盘的调度队列，
 *       并根据当前各硬盘的状态规划磁头移动路径，最终输出各盘的动作序列和完成的读请求编号。
 */
void read_action() {
    int n_read;
    int request_id, object_id;
    // 读取当前时间片的读取请求数量
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++) {
        // 读取每个请求的编号和对象编号
        scanf("%d%d", &request_id, &object_id);
        // 将当前请求关联到该对象，并通过链表结构链接多个请求
        request[request_id].object_id = object_id;
        request[request_id].prev_id = object[object_id].last_request_point;
        object[object_id].last_request_point = request_id;
        request[request_id].is_done = false;
        
        // 在三个副本中选择一个负载较轻的硬盘进行读取
        int d = 1;
        for (int j = 1; j <= 3; j++) {
            int mn = object[object_id].replica[d], now = object[object_id].replica[j];
            if (Siz[mn] > Siz[now]) d = j;
        }
        int mn = object[object_id].replica[d];
        // 将该请求加入选定硬盘的请求队列中
        vec[mn].push_back(request_id);
        Siz[mn] += object[object_id].size;
        request[request_id].loc_id = d; // 记录该请求选择的副本索引
    }

    // 存储本时间片完成读取的请求编号
    vector<int> finish;
    // 对每个硬盘依次处理其请求队列
    for (int i = 1; i <= N; i++) {
        int rest_time = G;      // 当前硬盘剩余令牌数，初始为 G
        string res = "";        // 存储本时间片内磁头的动作序列（例如 "r", "p", "j ..."）
        int pre_time = last_time[i]; // 上一次操作的耗时
        if (!pre_time) pre_time = 100; // 如果没有上一次耗时，则默认值设为 100

        // 在本时间片内，尽可能利用剩余令牌处理请求
        while (rest_time) {
            // 如果当前没有正在处理的请求，则从队列中取出一个请求
            if (!dealing[i]) {
                if (!vec[i].empty()) {
                    dealing[i] = vec[i].back();
                    vec[i].pop_back();
                    Siz[i] -= object[request[dealing[i]].object_id].size;
                }
                else break;
            }
            int x = dealing[i];
            // 如果请求已完成或对应对象已被删除，则跳过该请求
            if (request[x].is_done || object[request[x].object_id].is_delete) {
                dealing[i] = 0;
                continue;
            }
            // 如果当前请求还没有规划读取路径，则初始化路径
            if (request[x].rest.empty()) {
                // 将对象对应副本中的所有存储单元位置作为待读取位置
                for (int j = 1; j <= object[request[x].object_id].size; j++)
                    request[x].rest.push_back(object[request[x].object_id].unit[request[x].loc_id][j]);
                // 反转路径，方便后续以栈的方式读取
                reverse(request[x].rest.begin(), request[x].rest.end());
                // 尝试优化读取顺序，枚举所有排列，选出花费最少的顺序
                array<int, 2> cost_time = {10000000, 0};
                vector<int> tmp_vec;
                sort(request[x].rest.begin(), request[x].rest.end());
                do {
                    auto tmp_res = cal(ptr[i] + 1, request[x].rest, rest_time, pre_time);
                    if (tmp_res < cost_time) {
                        cost_time = tmp_res;
                        tmp_vec.assign(request[x].rest.begin(), request[x].rest.end());
                    }
                } while (next_permutation(request[x].rest.begin(), request[x].rest.end()));
                // 固定最佳读取顺序
                request[x].rest.assign(tmp_vec.begin(), tmp_vec.end());
            }
            // 取出当前请求的下一个目标块
            int to = request[x].rest.back();
            int dis = to - ptr[i] - 1;
            if (dis < 0) dis += V; // 考虑环形结构，调整距离
            // 如果距离为0，表示磁头已在目标位置
            if (!dis) {
                int cost_time = min(64, max(16, (int)(ceil(0.8 * pre_time) + 0.5)));
                if (cost_time > rest_time)
                    rest_time = 0;
                else {
                    rest_time -= cost_time;
                    res += "r";         // 执行读取动作，记为 'r'
                    ptr[i] = (ptr[i] + 1) % V; // 读取后磁头移动
                    pre_time = cost_time;
                    request[x].rest.pop_back(); // 移除已读取的块
                }
            }
            // 如果磁头到目标的距离小于或等于剩余令牌，则通过 "pass" 走到目标位置
            else if (dis <= rest_time) {
                rest_time -= dis;
                for (int k = 0; k < dis; k++) {
                    res += "p";       // 每一步记为 'p'
                    ptr[i] = (ptr[i] + 1) % V;
                }
                pre_time = 100;
            }
            // 如果剩余令牌正好为初始值 G，可以直接跳跃
            else if (rest_time == G) {
                res += "j " + to_string(to); // 'j' 指定跳跃到目标位置
                rest_time = 0;
                pre_time = 100;
                ptr[i] = to - 1;
            }
            // 其他情况下，令牌不足，只能尽可能走动
            else {
                while (rest_time--) {
                    res += "p";
                    ptr[i] = (ptr[i] + 1) % V;
                }
                rest_time = 0;
                pre_time = 100;
            }
            // 当该请求所有目标块都读取完毕后，标记整个请求链为完成，并记录已完成的请求编号
            if (request[x].rest.empty()) {
                int d = dealing[i];
                dealing[i] = 0;
                while (d) {
                    if (request[d].is_done) break;
                    request[d].is_done = true;
                    if (!object[request[d].object_id].is_delete)
                        finish.push_back(d);
                    d = request[d].prev_id;
                }
            }
        }
        // 记录当前硬盘上一次操作的耗时，便于后续计算
        last_time[i] = pre_time;
        // 如果动作序列为空或者没有跳跃动作，则在末尾添加结束标识符 '#'
        if (res.empty() || res[0] != 'j') res += "#";
        // 输出当前硬盘的磁头动作序列
        cout << res << "\n";
    }
    // 输出本时间片内所有完成读取的请求数及其请求编号
    cout << finish.size() << "\n";
    for (int v : finish)
        cout << v << "\n";
    fflush(stdout);
}

/*
 * 函数：clean
 * 说明：释放所有对象写入过程中动态分配的内存，防止内存泄露。
 */
void clean() {
    // 遍历所有对象
    for (auto &obj : object) {
        // 对于每个副本，如果内存已经分配，则释放
        for (int i = 1; i <= REP_NUM; i++) {
            if (obj.unit[i] == nullptr)
                continue;
            free(obj.unit[i]);
            obj.unit[i] = nullptr;
        }
    }
}
