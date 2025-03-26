#ifndef STORAGE_H
#define STORAGE_H

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <set>

// 常量定义
// #define MAX_DISK_NUM (10 + 1)
// #define MAX_DISK_SIZE (16384 + 1)
// #define MAX_REQUEST_NUM (30000000 + 1)
// #define MAX_OBJECT_NUM (100000 + 1)
// #define REP_NUM (3)
// #define FRE_PER_SLICING (1800)
// #define EXTRA_TIME (105)

constexpr int MAX_DISK_NUM=11;
constexpr int MAX_DISK_SIZE=16385;
constexpr int MAX_REQUEST_NUM=30000001;
constexpr int MAX_OBJECT_NUM=100001;
constexpr int REP_NUM=3;
constexpr int FRE_PER_SLICING=1800;
constexpr int EXTRA_TIME=105;
constexpr int LABEL_NUM=17;
constexpr int LABEL_GROUP_NUM=53; //ceil(86400/1800)+5


using namespace std;

// 数据结构定义
typedef struct Request_ {
    int object_id;
    int prev_id;//链式绑定相同对象的请求
    bool is_done;
    set<int>rest;//**该请求对应对象尚未被读取的块，（值为1，2，...，object.size）

} Request;

typedef struct Object_ {
    int replica[REP_NUM + 1];//第i个副本的磁盘编号
    int* unit[REP_NUM + 1];//第i个副本的第j块存在哪个单元
    vector<array<int,2> >request[5+1];//**存储该对象的第i个块与哪些请求相关，存的值是request_id
    int size;
    int last_request_point;//链式查询关于该对象的所有请求
    bool is_delete;
    int label_id;
    
} Object;

// 全局变量声明
extern Request request[MAX_REQUEST_NUM];
extern Object object[MAX_OBJECT_NUM];

extern int T, M, N, V, G;
extern int disk[MAX_DISK_NUM][MAX_DISK_SIZE];
//**下标意义与disk[][]完全一样，完全可以与disk合并为array<int,2>数组，
//**其中存的值是第i块硬盘的第j个单元所存的块是这个块所属对象的第几个块
extern int disk_uid[MAX_DISK_NUM][MAX_DISK_SIZE];

#endif // STORAGE_H
