#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cachelab.h"

typedef struct CacehLine_
{
    int valid;      // 有效位
    int tag;        // 标记位
    int time_stamp; // 时间戳
} CacheLine;

typedef struct Cache_
{
    int S; // 2**S sets
    int E; // E lines per set
    int B; // 2**B bytes per cache block
    CacheLine **line;
} Cache;

int hit_count = 0;      // 命中，要操作的数据在对应组的其中一行
int miss_count = 0;     // 不命中次数，要操作的数据不在对应组的任何一行
int eviction_count = 0; // 驱赶次数，要操作的数据的对应组已满，进行了替换操作

int print_detail = 0; // 是否打印详细信息

char trace_file[1000]; // 存 getopt 中选项内容，表示的是验证中需使用的trace文件名
Cache *cache = NULL;

void print_help();
void free_cache();
void init_cache(int, int, int);
int get_index(int, int);
int find_lru(int);
int get_emptyline(int);
void update(int, int, int);
void update_info(int, int);
void get_trace(int, int, int);

int main(int argc, char *argv[])
{
    int s, E, b;
    char opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != EOF)
    {
        switch (opt)
        {
        case 'h':
            print_help();
            break;
        case 'v':
            print_detail = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(trace_file, optarg);
            break;
        default:
            print_help();
            break;
        }
    }

    init_cache(s, E, b);
    get_trace(s, E, b);
    free_cache();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

// print help message
void print_help()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");

    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional print_detail flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux> ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_cache(int s, int E, int b)
{
    int S = 1 << s;
    int B = 1 << b;
    cache = (Cache *)malloc(sizeof(Cache));
    cache->S = S;
    cache->E = E;
    cache->B = B;
    cache->line = (CacheLine **)malloc(sizeof(CacheLine *) * S);
    for (int i = 0; i < S; i++)
    {
        cache->line[i] = (CacheLine *)malloc(sizeof(CacheLine) * E);
        for (int j = 0; j < E; j++)
        {
            cache->line[i][j].valid = 0;
            cache->line[i][j].tag = -1;
            cache->line[i][j].time_stamp = 0;
        }
    }
}

void get_trace(int s, int E, int b)
{
    FILE *pf;
    if ((pf = fopen(trace_file, "r")) == NULL)
    {
        exit(-1);
    }
    char identifier;
    unsigned addr;
    int sz;

    // addr:  |----------|---------|----------|
    //           t bit      s bit      b bit
    while (fscanf(pf, " %c %x,%d", &identifier, &addr, &sz) > 0)
    {
        int op_t = addr >> (s + b);
        int op_s = (addr >> b) & (-1U >> (8 * sizeof(unsigned) - s));

        switch (identifier)
        {
        case 'M':
            update_info(op_t, op_s); // store
            update_info(op_t, op_s); // load
            break;
        case 'L':
            update_info(op_t, op_s); // load
            break;
        case 'S':
            update_info(op_t, op_s); // store
            break;
        }
    }
    fclose(pf);
}

int get_index(int op_s, int op_t)
{
    for (int i = 0; i < cache->E; i++)
    {
        if (cache->line[op_s][i].valid && cache->line[op_s][i].tag == op_t)
        {
            return i;
        }
    }
    return -1;
}

int get_emptyline(int op_s)
{
    for (int i = 0; i < cache->E; i++)
    {
        if (cache->line[op_s][i].valid == 0)
            return i;
    }
    return -1; // 組已經滿了
}

int find_lru(int op_s)
{
    int max_index = 0;
    int max_timestamp = 0;
    for (int i = 0; i < cache->E; i++)
    {
        if (cache->line[op_s][i].time_stamp > max_timestamp)
        {
            max_timestamp = cache->line[op_s][i].time_stamp;
            max_index = i;
        }
    }
    return max_index;
}

void update(int index, int op_s, int op_t)
{
    cache->line[op_s][index].valid = 1;
    cache->line[op_s][index].tag = op_t;
    for (int i = 0; i < cache->E; i++)
    {
        // 时间戳越大则表示该行最后访问的时间越久远
        if (cache->line[op_s][i].valid == 1)
            cache->line[op_s][i].time_stamp++;
    }
    cache->line[op_s][index].time_stamp = 0;
}

void update_info(int op_t, int op_s)
{
    int index = get_index(op_s, op_t);
    if (index == -1)
    {
        miss_count++;
        if (print_detail)
            printf("miss\n");
        int i = get_emptyline(op_s);
        if (i == -1)
        {
            eviction_count++;
            if (print_detail)
                printf("eviction\n");
            i = find_lru(op_s);
        }
        update(i, op_s, op_t);
    }
    else
    {
        hit_count++;
        if (print_detail)
            printf("hit\n");
        update(index, op_s, op_t);
    }
}

void free_cache()
{
    for (int i = 0, s = cache->S; i < s; i++)
    {
        free(cache->line[i]);
    }
    free(cache->line);
    free(cache);
}
