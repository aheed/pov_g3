#include <time.h>
#include "ns_clock.h"

#define NANOS_PER_SEC 1000000000

#define HANDLE_WRAP_SIGNED(n, maxn) (n > (maxn / 2) ? n - maxn : (n < (-maxn / 2) ? n + maxn : n))
#define HANDLE_WRAP_UNSIGNED(n, maxn) (n > maxn ? n - maxn : (n < 0 ? n + maxn : n))
#define HANDLE_CLOCK_WRAP_SIGNED(n)  HANDLE_WRAP_SIGNED(n, NANOS_PER_SEC)
#define HANDLE_CLOCK_WRAP_UNSIGNED(n)  HANDLE_WRAP_UNSIGNED(n, NANOS_PER_SEC)

nsc_time_t nsc_get_current_time()
{
    struct timespec resolution;
    clock_gettime(CLOCK_MONOTONIC, &resolution);
    return resolution.tv_nsec;
}

nsc_timeperiod_t nsc_diff_time_time(nsc_time_t first, nsc_time_t later)
{
    nsc_timeperiod_t diff = first - later;
    return HANDLE_CLOCK_WRAP_SIGNED(diff);
}

nsc_time_t nsc_add_time_period(nsc_time_t basetime, nsc_timeperiod_t addedperiod)
{
    nsc_time_t newtime = basetime + addedperiod;
    return HANDLE_CLOCK_WRAP_UNSIGNED(newtime);
}

nsc_timeperiod_t nsc_wrap_period(nsc_timeperiod_t period, nsc_timeperiod_t maxperiod)
{
    return HANDLE_WRAP_SIGNED(period, maxperiod);
}

/*nsc_timeperiod_t nsc_diff_period_period(nsc_timeperiod_t p1, nsc_timeperiod_t p2, nsc_timeperiod_t maxperiod)
{
    nsc_timeperiod_t diff =  p1 - p2;
    return HANDLE_WRAP(diff, maxperiod);
}*/
