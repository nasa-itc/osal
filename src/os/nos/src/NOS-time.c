/* Copyright (C) 2022 - 2022 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

   This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
   limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
   for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
   any warranty that the software will be error free.

   In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
   arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
   contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
   documentation or services provided hereunder

   ITC Team
   NASA IV&V
   ivv-itc@lists.nasa.gov
*/

#include <errno.h>
#include <pthread.h>
#include "NOS-time.h"

#include "Client/CInterface.h"

extern NE_Bus          *CFE_PSP_Bus;
extern int64_t          CFE_PSP_ticks_per_second;
extern pthread_mutex_t  CFE_PSP_sim_time_mutex;
extern NE_SimTime       CFE_PSP_sim_time;
#define NOS_NANO 1000000000

int NOS_clock_getres (clockid_t clock_id, struct timespec * res)
{
    res->tv_sec = 0;
    res->tv_nsec = NOS_NANO / CFE_PSP_ticks_per_second;
    return 0;
}

int NOS_clock_gettime (clockid_t clock_id, struct timespec * tp)
{
    pthread_mutex_lock(&CFE_PSP_sim_time_mutex);
    NE_SimTime sim_time = CFE_PSP_sim_time;
    pthread_mutex_unlock(&CFE_PSP_sim_time_mutex);
    tp->tv_sec = sim_time / CFE_PSP_ticks_per_second;
    tp->tv_nsec = (sim_time % CFE_PSP_ticks_per_second) * NOS_NANO / CFE_PSP_ticks_per_second;
    return 0;
}

int NOS_clock_nanosleep (clockid_t clock_id, int flags, const struct timespec * req, struct timespec * rem)
{
    if ((req->tv_sec < 0) || (req->tv_nsec < 0) || (req->tv_nsec >= NOS_NANO)) {
        errno = EINVAL;
        return -1;
    }
    pthread_mutex_lock(&CFE_PSP_sim_time_mutex);
    NE_SimTime sim_time = CFE_PSP_sim_time;
    pthread_mutex_unlock(&CFE_PSP_sim_time_mutex);
    NE_SimTime end_time = req->tv_sec * CFE_PSP_ticks_per_second + req->tv_nsec * CFE_PSP_ticks_per_second / NOS_NANO;
    if (flags == 0) end_time += sim_time; // relative time
    struct timespec delay, real_rem;
    delay.tv_sec = 0;
    delay.tv_nsec = NOS_NANO / CFE_PSP_ticks_per_second;
    int interrupt = 0;
    while ((sim_time < end_time) && !interrupt) {
        interrupt = clock_nanosleep (clock_id, flags, &delay, &real_rem);
        pthread_mutex_lock(&CFE_PSP_sim_time_mutex);
        sim_time = CFE_PSP_sim_time;
        pthread_mutex_unlock(&CFE_PSP_sim_time_mutex);
    }
    if (rem != NULL) {
        NE_SimTime delta = end_time - sim_time;
        if (delta < 0) delta = 0;
        rem->tv_sec = delta / CFE_PSP_ticks_per_second;
        rem->tv_nsec = ((delta % CFE_PSP_ticks_per_second) * NOS_NANO / CFE_PSP_ticks_per_second) + real_rem.tv_nsec;
    }
    return interrupt;
}

int NOS_clock_settime (clockid_t clock_id, const struct timespec * tp)
{
    if ((tp->tv_sec < 0) || (tp->tv_nsec < 0) || (tp->tv_nsec >= NOS_NANO)) {
        errno = EINVAL;
        return -1;
    }
    NE_SimTime sim_time = tp->tv_sec * CFE_PSP_ticks_per_second + tp->tv_nsec * CFE_PSP_ticks_per_second / NOS_NANO;
    NE_bus_set_time(CFE_PSP_Bus, sim_time);
    return 0;
}

int NOS_timer_create (clockid_t clock_id, struct sigevent * evp, timer_t * timerid)
{
    return timer_create(clock_id, evp, timerid);
}

int NOS_timer_delete (timer_t timerid)
{
    return timer_delete(timerid);
}

int NOS_timer_gettime (timer_t timerid, struct itimerspec * value)
{
    return timer_gettime(timerid, value);
}

int NOS_timer_settime (timer_t timerid, int flags, const struct itimerspec * value, struct itimerspec * ovalue)
{
    return timer_settime(timerid, flags, value, ovalue);
}

void NOS_canonicalize_timespec(struct timespec *ts)
{
    while (ts->tv_nsec < 0) {
        ts->tv_nsec += 1000000000L;
        ts->tv_sec--;
    }
    while (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
}

void NOS_minus_real_timeoffset(struct timespec *offset)
{
    struct timespec nos, real;
    clock_gettime(CLOCK_REALTIME, &real);
    NOS_clock_gettime (CLOCK_REALTIME, &nos);
    offset->tv_sec = nos.tv_sec - real.tv_sec;
    offset->tv_nsec = nos.tv_nsec - real.tv_nsec;
    NOS_canonicalize_timespec(offset);
}

void NOS_to_real_timespec(const struct timespec *nos, struct timespec *real)
{
    struct timespec offset;
    NOS_minus_real_timeoffset(&offset);
    real->tv_sec = nos->tv_sec - offset.tv_sec;
    real->tv_nsec = nos->tv_nsec - offset.tv_nsec;
    NOS_canonicalize_timespec(real);
}