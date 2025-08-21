#ifndef ABC_H
#define ABC_H

// ---------- <dyn_array.h> ----------
#ifndef DEFINE_DYN_ARRAY

#include <assert.h>
#include <stdlib.h>

#define DEFINE_DYN_ARRAY(type, name) \
    typedef struct \
    { \
        size_t size, capacity; \
        type* data; \
    } name; \
    static inline void name##_alloc(name* arr, size_t capacity) \
    { \
        assert(arr != NULL); \
        arr->size     = 0; \
        arr->data     = malloc(capacity * sizeof(*arr->data)); \
        assert(arr->data != NULL); \
        arr->capacity = capacity; \
    } \
    static inline void name##_free(name* arr) \
    { \
        assert(arr != NULL); \
        assert(arr->data != NULL); \
        free(arr->data); \
    } \
    static inline void name##_realloc(name* arr, size_t capacity) \
    { \
        assert(arr != NULL); \
        if (arr->capacity == capacity) \
            return; \
        arr->data     = realloc(arr->data, capacity * sizeof(*arr->data)); \
        assert(arr->data != NULL); \
        arr->capacity = capacity; \
    } \
    static inline void name##_append(name* arr, type val) \
    { \
        assert(arr != NULL); \
        while (arr->size >= arr->capacity) \
            name##_realloc(arr, arr->capacity * 2); \
        arr->data[arr->size++] = val; \
    }

#endif // DEFINE_DYN_ARRAY
// ---------- </dyn_array.h> ----------

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#define ABC_MSG_PREFIX        "[abc] "
#define ABC_MSG_ERROR_AT      "ERROR (at %s): "
#define ABC_MSG_CLOCK_FMT     "%.3fs"
#define ABC_MSG_INVALID_CLOCK "Invalid clock!"

#define ABC_LOG(msg)           printf(ABC_MSG_PREFIX msg "\n")
#define ABC_LOGCLK(name, time) printf(ABC_MSG_PREFIX "@%s@: " ABC_MSG_CLOCK_FMT ".\n", name, time)
#define ABC_LOGVAR(fmt, name)  printf(ABC_MSG_PREFIX "%s: " fmt "\n", #name, name)

typedef uint64_t abc_clock_t;
typedef double abc_second_t;

typedef struct
{
    abc_clock_t id;
    const char* name;
    abc_second_t total, start;
} ABClock;

DEFINE_DYN_ARRAY(ABClock, dyn_array_abclock)

/*
#ifdef _WIN32
#include <windows.h>
static inline abc_second_t abc_now()
{
    static int is_frequency_initialized = 0;
    static LARGE_INTEGER frequency;
    if (!is_frequency_initialized)
    {
        QueryPerformanceFrequency(&frequency); 
        is_frequency_initialized = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (abc_second_t)now.QuadPart / (abc_second_t)frequency.QuadPart;
}
#else
*/
#include <time.h>
static inline abc_second_t abc_now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
/*
#endif
*/

extern dyn_array_abclock* abc__clock_central;
extern int abc_is_clock_central_initialized;

#ifdef ABC_IMPLEMENTATION
dyn_array_abclock* abc__clock_central = 0;
int abc_is_clock_central_initialized  = 0;
#endif

static inline dyn_array_abclock* abc_clock_central(void)
{
    if (!abc_is_clock_central_initialized)
    {
        abc__clock_central = malloc(sizeof(dyn_array_abclock));
        dyn_array_abclock_alloc(abc__clock_central, 16);
        abc_is_clock_central_initialized = 1;
    }
    return abc__clock_central;
}

static inline void abc_clock_central_free(void)
{
    if (abc__clock_central)
    {
        dyn_array_abclock_free(abc__clock_central);
        free(abc__clock_central);
    }
}

static inline ABClock* abc_clock_get(abc_clock_t id)
{
    if (id >= abc_clock_central()->size)
    {
        printf(ABC_MSG_PREFIX ABC_MSG_ERROR_AT "No clock with id %llu.\n", __func__, id);
        return NULL;
    }
    return &abc_clock_central()->data[id];
}

static inline abc_clock_t abc_clock_register(const char* name)
{
    
    const abc_clock_t id = abc_clock_central()->size;
    dyn_array_abclock_append(abc_clock_central(), (ABClock)
    {
        .id    = id,
        .name  = name,
        .total = 0,
        .start = 0,
    });
    printf(ABC_MSG_PREFIX "Registered clock '%s' with id %llu.\n", name, id);
    return id;
}

static inline void abc_clock_start(abc_clock_t id)
{
    ABClock* clock = abc_clock_get(id);
    if (clock)
        clock->start = abc_now();
}

static inline abc_second_t abc_clock_stop(abc_clock_t id)
{
    ABClock* clock = abc_clock_get(id);
    if (clock)
    {
        const abc_second_t delta_t = abc_now() - clock->start;
        clock->start = 0;
        clock->total += delta_t;
        return delta_t;
    }
    return 0;
}

static inline void abc_clock_reset(abc_clock_t id)
{
    ABClock* clock = abc_clock_get(id);
    if (clock)
    {
        clock->start = 0;
        clock->total = 0;
    }
}

static inline void abc_clock_log_round(abc_clock_t id)
{
    const ABClock* clock = abc_clock_get(id);
    if (clock)
    {
        const abc_second_t delta_t = abc_now() - clock->start;
        ABC_LOGCLK(clock->name, delta_t);
    }
}

#define abc_pie_chart(...) \
    abc__pie_chart(sizeof((abc_clock_t[]){__VA_ARGS__})/sizeof(abc_clock_t), __VA_ARGS__)

static inline void abc__pie_chart(int count, ...)
{
    va_list args, copy;
    va_start(args, count);
    va_copy(copy, args);
    double sum_totals = 0;
    for (int i=0; i<count; ++i)
    {
        const ABClock* clock = abc_clock_get(va_arg(args,abc_clock_t));
        if (clock)
            sum_totals += clock->total;
    }
    printf(ABC_MSG_PREFIX "Pie chart (sum=" ABC_MSG_CLOCK_FMT "):\n", sum_totals);
    for (int i=0; i<count; ++i)
    {
        const ABClock* clock = abc_clock_get(va_arg(copy,abc_clock_t));
        if (clock)
            printf(ABC_MSG_PREFIX "  %s: %.2f%% (" ABC_MSG_CLOCK_FMT ")\n",
                clock->name,
                100.0f * clock->total / sum_totals,
                clock->total);
    }
}

static inline void abc_pie_chart_all(void)
{
    double sum_totals = 0;
    for (size_t i=0; i<abc_clock_central()->size; ++i)
    {
        const ABClock* clock = abc_clock_get(i);
        if (clock)
            sum_totals += clock->total;
    }
    printf(ABC_MSG_PREFIX "Pie chart (sum=" ABC_MSG_CLOCK_FMT "):\n", sum_totals);
    for (size_t i=0; i<abc_clock_central()->size; ++i)
    {
        const ABClock* clock = abc_clock_get(i);
        if (clock)
            printf(ABC_MSG_PREFIX "  %s: %.2f%% (" ABC_MSG_CLOCK_FMT ")\n",
                clock->name,
                100.0f * clock->total / sum_totals,
                clock->total);
    }
}

#endif // ABC_H

