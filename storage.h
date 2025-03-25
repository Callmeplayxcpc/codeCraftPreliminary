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


using namespace std;

// 数据结构定义
struct Request {
    int object_id;
    int prev_id;
    bool is_done;
    vector<int> rest;
    int loc_id;
};

struct Object {
    int replica[REP_NUM + 1];
    int* unit[REP_NUM + 1];
    int size;
    int last_request_point;
    bool is_delete;
};

// 全局变量声明
extern Request request[MAX_REQUEST_NUM];
extern Object object[MAX_OBJECT_NUM];

extern int T, M, N, V, G;
extern int disk[MAX_DISK_NUM][MAX_DISK_SIZE];
extern int disk_point[MAX_DISK_NUM];

#endif // STORAGE_H
