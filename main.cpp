#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <bits/stdc++.h>

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)
using namespace std;
typedef struct Request_ {
    int object_id;
    int prev_id;
    bool is_done;
    vector<int>rest;
    int loc_id;
} Request;

typedef struct Object_ {
    int replica[REP_NUM + 1];
    int* unit[REP_NUM + 1];
    
    int size;
    int last_request_point;
    bool is_delete;
    
} Object;

Request request[MAX_REQUEST_NUM];
Object object[MAX_OBJECT_NUM];

int T, M, N, V, G;
int disk[MAX_DISK_NUM][MAX_DISK_SIZE];
int disk_point[MAX_DISK_NUM];

void timestamp_action()
{
    int timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);

    fflush(stdout);
}

void do_object_delete(const int* object_unit, int* disk_unit, int size)
{
    for (int i = 1; i <= size; i++) {
        disk_unit[object_unit[i]] = 0;
    }
}

void delete_action()
{
    int n_delete;
    int abort_num = 0;
    static int _id[MAX_OBJECT_NUM];

    scanf("%d", &n_delete);
    for (int i = 1; i <= n_delete; i++) {
        scanf("%d", &_id[i]);
    }

    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        while (current_id != 0) {
            if (request[current_id].is_done == false) {
                abort_num++;
            }
            current_id = request[current_id].prev_id;
        }
    }

    printf("%d\n", abort_num);
    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        while (current_id != 0) {
            if (request[current_id].is_done == false) {
                printf("%d\n", current_id);
            }
            current_id = request[current_id].prev_id;
        }
        for (int j = 1; j <= REP_NUM; j++) {
            do_object_delete(object[id].unit[j], disk[object[id].replica[j]], object[id].size);
        }
        object[id].is_delete = true;
    }

    fflush(stdout);
}

void do_object_write(int* object_unit, int* disk_unit, int size, int object_id)
{
    int current_write_point = 0;
    for (int i = 1; i <= V; i++) {
        if (disk_unit[i] == 0) {
            disk_unit[i] = object_id;
            object_unit[++current_write_point] = i;
            if (current_write_point == size) {
                break;
            }
        }
    }

    assert(current_write_point == size);
}

void write_action()
{
    int n_write;
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) 
    {
        int id, size;
        scanf("%d%d%*d", &id, &size);
        object[id].last_request_point = 0;
        
        for (int j = 1; j <= REP_NUM; j++) {
            object[id].replica[j] = (id + j) % N + 1;
            object[id].unit[j] = static_cast<int*>(malloc(sizeof(int) * (size + 1)));
            object[id].size = size;
            object[id].is_delete = false;
            do_object_write(object[id].unit[j], disk[object[id].replica[j]], size, id);
        }
        
        printf("%d\n", id);
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
vector<int>vec[20];
int Siz[20];
int dealing[20];
int ptr[20],last_time[20];
array<int,2> cal (int pos,vector<int>rest,int rest_time,int pre_time)
{
    int timestamp=0;
    while (true)
    {
        while (rest_time)
        {
            int to=rest.back();
            int dis=to-pos-1;
            if (dis<0) dis+=V;
            if (!dis)//针就在要读的位置
            {
                int cost_time=min(64,max(16,(int)(ceil(0.8*pre_time)+0.5)));
                if (cost_time>rest_time) rest_time=0;
                else rest_time-=cost_time,pos=(pos+1)%V,pre_time=cost_time,rest.pop_back();
                if (!rest.size()) 
                {
                    return {timestamp,-rest_time};
                }
            }
            else if (dis<=rest_time)//走过去够时间
            {
                rest_time-=dis;
                while (dis--) pos=(pos+1)%V;
                pre_time=100;
            } 
            else if (rest_time==G)//可以跳过去
            {
                rest_time=0;
                pre_time=100;
                pos=to-1;
            }
            else //不能跳，还走不到，只能尽量走
            {   
                while (rest_time--) pos=(pos+1)%V;
                rest_time=0;
                pre_time=100;
            }
            

        }
        timestamp++,rest_time=G;
    }
     //rest_time越多越好，直接给负值，不考虑结果中pre_time对未来的影响了，有点复杂
}
void read_action()
{
/*  int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++) {
        scanf("%d%d", &request_id, &object_id);
        request[request_id].object_id = object_id;
        request[request_id].prev_id = object[object_id].last_request_point;
        object[object_id].last_request_point = request_id;
        request[request_id].is_done = false;
    }

    static int current_request = 0;
    static int current_phase = 0;
    if (!current_request && n_read > 0) {
        current_request = request_id;
    }
    if (!current_request) {
        for (int i = 1; i <= N; i++) {
            printf("#\n");
        }
        printf("0\n");
    } else {
        current_phase++;
        object_id = request[current_request].object_id;
        for (int i = 1; i <= N; i++) {
            if (i == object[object_id].replica[1]) {
                if (current_phase % 2 == 1) {
                    printf("j %d\n", object[object_id].unit[1][current_phase / 2 + 1]);
                } else {
                    printf("r#\n");
                }
            } else {
                printf("#\n");
            }
        }

        if (current_phase == object[object_id].size * 2) {
            if (object[object_id].is_delete) {
                printf("0\n");
            } else {
                printf("1\n%d\n", current_request);
                request[current_request].is_done = true;
            }
            current_request = 0;
            current_phase = 0;
        } else {
            printf("0\n");
        }
    }
*/
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
        int d=1;
        for (int j=1;j<=3;j++)
        {
            int mn=object[object_id].replica[d],now=object[object_id].replica[j];
            if (Siz[mn]>Siz[now]) d=j; 
        }
        int mn=object[object_id].replica[d];
        vec[mn].push_back(request_id);
        Siz[mn]+=object[object_id].size;
        request[request_id].loc_id=d;
    }

    vector<int>finish;
    for (int i = 1; i <= N; i++) 
    {
        int rest_time=G;
        string res="";
        int pre_time=last_time[i];
        if (!pre_time) pre_time=100;
        while (rest_time)
        {
            if (!dealing[i]) 
            {
                if (vec[i].size()) dealing[i]=vec[i].back(),vec[i].pop_back(),Siz[i]-=object[request[dealing[i]].object_id].size;
                else break;
            }
            int x=dealing[i];
            if (request[x].is_done||object[request[x].object_id].is_delete) 
            {
                dealing[i]=0;
                continue;
            }
            
            if (!request[x].rest.size())
            {
                for (int j=1;j<=object[request[x].object_id].size;j++) 
                    request[x].rest.push_back(object[request[x].object_id].unit[request[x].loc_id][j]);
                reverse(request[x].rest.begin(),request[x].rest.end());

                array<int,2> cost_time={10000000,0};
                vector<int>tmp_vec;
                sort(request[x].rest.begin(),request[x].rest.end());
                do {
                    auto tmp_res=cal(ptr[i]+1,request[x].rest,rest_time,pre_time);
                    if (tmp_res<cost_time) cost_time=tmp_res,tmp_vec.assign(request[x].rest.begin(),request[x].rest.end());
                    
                } while (next_permutation(request[x].rest.begin(),request[x].rest.end()));

                request[x].rest.assign(tmp_vec.begin(),tmp_vec.end());
            }
            int to=request[x].rest.back();
            int dis=to-ptr[i]-1;
            if (dis<0) dis+=V;
            if (!dis)//针就在要读的位置
            {
                int cost_time=min(64,max(16,(int)(ceil(0.8*pre_time)+0.5)));
                if (cost_time>rest_time) rest_time=0;
                else rest_time-=cost_time,res+="r",ptr[i]=(ptr[i]+1)%V,pre_time=cost_time,request[x].rest.pop_back();

            }
            else if (dis<=rest_time)//走过去够时间
            {
                rest_time-=dis;
                while (dis--) res+="p",ptr[i]=(ptr[i]+1)%V;
                pre_time=100;
            } 
            else if (rest_time==G)//可以跳过去
            {
                res+="j "+to_string(to);
                rest_time=0;
                pre_time=100;
                ptr[i]=to-1;
            }
            else //不能跳，还走不到，只能尽量走
            {   
                while (rest_time--) res+="p",ptr[i]=(ptr[i]+1)%V;
                rest_time=0;
                pre_time=100;
            }
            if (!request[x].rest.size())//搞完了
            {
                int d=dealing[i];
                dealing[i]=0;
                while (d) 
                {
                    if (request[d].is_done) break;
                    request[d].is_done=true;
                    if (!object[request[d].object_id].is_delete) finish.push_back(d);
                    d=request[d].prev_id;
                }
            }

        }
        last_time[i]=pre_time;
        if (res[0]!='j') res+="#";
        cout<<res<<'\n';
    }    

    cout<<finish.size()<<'\n';
    for (int v:finish) cout<<v<<'\n';

    fflush(stdout);
}

void clean()
{
    for (auto& obj : object) {
        for (int i = 1; i <= REP_NUM; i++) {
            if (obj.unit[i] == nullptr)
                continue;
            free(obj.unit[i]);
            obj.unit[i] = nullptr;
        }
    }
}

int main()
{
    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);

    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%*d");
        }
    }

    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%*d");
        }
    }

    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%*d");
        }
    }

    printf("OK\n");
    fflush(stdout);

    for (int i = 1; i <= N; i++) {
        disk_point[i] = 1; 
    }

    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        timestamp_action();
        delete_action();
        write_action();
        read_action();
    }
    clean();

    return 0;
}