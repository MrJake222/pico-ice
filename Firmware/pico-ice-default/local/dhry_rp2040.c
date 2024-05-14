#include "dhry_rp2040.h"
#include "dhry.h"

#include <stdio.h>
#include <pico/time.h>
#include <hardware/clocks.h>

static absolute_time_t t_start, t_end;

void run_dhrystone_rp2040() {
    const int runs = 1000000;
    dhrystone(runs);

    const double time_us = (double) absolute_time_diff_us(t_start, t_end);
    const double time_s = time_us / 1e6;

    printf("runs:   %d\n", runs);
    // printf("cycles: %d\n", cycles);
    // printf("instr:  %d\n", instrs);
    printf("time:   %f us (%.3f s)\n", time_us, time_s);
    printf("\n");

    // const uint64_t cpmi = ((uint64_t) cycles) * 1000 / instrs;
    // printf("cpi: %llu.%03llu\n", cpmi/1000, cpmi%1000);

    const double dhps = runs / time_s;
    printf("dhrystones per second: %.3f\n", dhps);

    const double dmips = dhps / 1757;
    printf("dmips: %.3f\n", dmips);

    const double f_clk_mhz = (double)(clock_get_hz(clk_sys)) / 1e6;
    const double dmips_per_mhz = dmips / f_clk_mhz;
    printf("dmips / MHz: %.3f  (%f MHz)\n", dmips_per_mhz, f_clk_mhz);
}

void dhrystone_timer_start() {
    t_start = get_absolute_time();
}

void dhrystone_timer_end() {
    t_end = get_absolute_time();
}