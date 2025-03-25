#include "storage.h"

// 全局变量定义
Request request[MAX_REQUEST_NUM];
Object object[MAX_OBJECT_NUM];

int T, M, N, V, G;
int disk[MAX_DISK_NUM][MAX_DISK_SIZE];
int disk_point[MAX_DISK_NUM];
int fre_del[MAX_LABLE][DIV_1800];
int fre_write[MAX_LABLE][DIV_1800];
int fre_read[MAX_LABLE][DIV_1800];