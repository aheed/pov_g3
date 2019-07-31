// Nanosecond clock

#ifndef NS_CLOCK_H
#define NS_CLOCK_H


#define NANOS_PER_SEC 1000000000
#define NANOSEC_TYPE long long

typedef NANOSEC_TYPE nsc_time_t;
typedef NANOSEC_TYPE nsc_timeperiod_t;

nsc_time_t nsc_get_current_time();
nsc_time_t nsc_add_time_period(nsc_time_t basetime, nsc_timeperiod_t addedperiod);
nsc_timeperiod_t nsc_diff_time_time(nsc_time_t first, nsc_time_t later);
nsc_timeperiod_t nsc_wrap_period(nsc_timeperiod_t period, nsc_timeperiod_t maxperiod);
//nsc_timeperiod_t nsc_diff_period_period(nsc_timeperiod_t p1, nsc_timeperiod_t p2, nsc_timeperiod_t maxperiod);


#endif // NS_CLOCK_H
