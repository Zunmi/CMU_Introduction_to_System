#include "cachelab.h"
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>

typedef struct {
    int valid;
    unsigned long long tag;
    unsigned long long last_used;
} Line;

typedef struct {
    int s, E, b;
    Line *lines;
    unsigned long long time;
    int hits, misses, evictions;
} Cache;

static const char *access_cache(Cache *cache, unsigned long long address){
    size_t set_index = (size_t)((address >> cache->b) & ((1ULL << cache->s) - 1));
    unsigned long long tag = address >> (cache->s + cache->b);
    Line *set = cache->lines + set_index * (size_t)cache->E;
    Line *empty = NULL;
    Line *lru = NULL;
    cache->time++;
    for (int i = 0; i < cache->E; i++){
        if (set[i].valid && set[i].tag == tag){
            cache->hits++;
            set[i].last_used = cache->time;
            return "hit";
        }
        if (!set[i].valid && empty == NULL)
            empty = &set[i];
        if (set[i].valid && (!lru || set[i].last_used < lru->last_used))
            lru = &set[i];
    }
    cache->misses++;
    Line *target = empty;
    const char *result = "miss";
    if (!target){
        target = lru;
        cache->evictions++;
        result = "miss eviction";
    }
    target->valid = 1;
    target->tag = tag;
    target->last_used = cache->time;

    return result;
}
int main(int argc, char **argv)
{
    Cache cache = {0};
    const char *trace_path = NULL;
    int verbose = 0;
    int have_s = 0, have_E = 0, have_b = 0;
    int option;
    while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(option){
        case 'h':
            printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n",
                   argv[0]);
            return 0;
        case 'v':
            verbose = 1;
            break;
        case 's':
            cache.s = atoi(optarg);
            have_s = 1;
            break;
        case 'E':
            cache.E = atoi(optarg);
            have_E = 1;
            break;
        case 'b':
            cache.b = atoi(optarg);
            have_b = 1;
            break;
        case 't':
            trace_path = optarg;
            break;
        default:
            return 1;
        }
    }

    if (!have_s || !have_E || !have_b || trace_path == NULL ||
    cache.s < 0 || cache.E <= 0 || cache.b < 0 ||
    cache.s + cache.b >= 64) {
    fprintf(stderr,
            "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n",
            argv[0]);
    return 1;
    }

    FILE *trace = fopen(trace_path, "r");
    if (trace == NULL) {
        perror(trace_path);
        return 1;
    }

    size_t set_count = (size_t)1 << cache.s;
    if (set_count > (size_t)-1 / (size_t)cache.E) {
        fclose(trace);
        return 1;
    }

    cache.lines = calloc(set_count * (size_t)cache.E,
                        sizeof(*cache.lines));
    if (cache.lines == NULL) {
        fclose(trace);
        return 1;
    }

    char operation;
    unsigned long long address;
    int size;

    while (fscanf(trace, " %c %llx,%d",
                &operation, &address, &size) == 3) {
        if (operation == 'I')
            continue;

        if (operation == 'L' || operation == 'S' || operation == 'M') {
        if (verbose)
            printf("%c %llx,%d ", operation, address, size);

        const char *result = access_cache(&cache, address);
        if (verbose)
            printf("%s ", result);

        if (operation == 'M') {
            result = access_cache(&cache, address);
            if (verbose)
                printf("%s ", result);
        }

        if (verbose)
            putchar('\n');
        }
    }
    fclose(trace);
    printSummary(cache.hits, cache.misses, cache.evictions);
    free(cache.lines);
    return 0;
}
