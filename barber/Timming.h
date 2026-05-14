// Timming.h
// by juan.daniel.rangel.avila@gmail.com
// GNU/GPL License

#ifndef TIMMING_H
#define TIMMING_H

#include <sys/resource.h>
#include <sys/time.h>

static void uswtime(double *utime, double *stime, double *wtime) {
    struct rusage t;
    struct timeval timev;
    getrusage(RUSAGE_SELF, &t);
    *utime = t.ru_utime.tv_sec + t.ru_utime.tv_usec / 1e6;
    *stime = t.ru_stime.tv_sec + t.ru_stime.tv_usec / 1e6;
    gettimeofday(&timev, NULL);
    *wtime = timev.tv_sec + timev.tv_usec / 1e6;
}

#endif
