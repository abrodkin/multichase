/*
Copyright (c) 2014-2017, Shay Gal-On
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Shay Gal-On nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Shay Gal-On BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED

#define NUM_BINS 200

typedef struct {
	double min,max,avg,sum,sum_sq,stdev;
	int count, min_idx, max_idx;
	size_t bins[NUM_BINS];
} stat_inner_t;

typedef struct {
	stat_inner_t latency;
	stat_inner_t bandwidth;
	char *desc; // Description of what this tracks for printout.
} stat_t;

/* Single stat functions */
void stat_inner_init(stat_inner_t *s);
void stat_inner_add(stat_inner_t *s, double v); // add a value to stats
void stat_inner_update(stat_inner_t *s); // update before printout
void stat_inner_print(stat_inner_t *s); // print stats to stdout
void stat_plot(stat_inner_t *s, const char *tag); // if built with gnuplot, plot histogram of values

/* Helpers to track both latency and bandwidth metrics */
void stat_init(stat_t *s, char *description); 
void stat_add(stat_t *s, double latency, double bandwidth); // Add both latency and bandwidth stats
void stat_update(stat_t *s); // Update both latency and bandwidth stats
void stat_print(stat_t *s); // Print stats to stdout

#endif
