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


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stats.h"

#define _POW 1.08

#ifdef GNUPLOT
#include "gnuplot_i.h"
static int nm=0;
#endif

void stat_inner_add_to_bin(stat_inner_t *s, double v);

void stat_inner_add_to_bin(stat_inner_t *s, double v) {
	double dbin = log(v)/log(_POW);
	size_t bin=(size_t)dbin;

	if (dbin < 0) {
	  bin = 0;
	} else if (bin >= NUM_BINS) {
	  bin = NUM_BINS - 1;
	}

	s->bins[bin]++;
}

void stat_inner_init(stat_inner_t *s) {
	s->min=1e100;
	s->max=-1e100;
	s->sum=0.;
	s->sum_sq=0.;
	s->count=0;
	memset(s->bins,0,sizeof(size_t));
}

void stat_inner_add(stat_inner_t *s, double v) {
	s->sum+=v;
	s->sum_sq+=v*v;
	s->count++;
	if (v>s->max) s->max=v;
	if (v<s->min) s->min=v;
	stat_inner_add_to_bin(s,v);
}

void stat_inner_update(stat_inner_t *s) {
	double count=(double)s->count;
	s->avg=s->sum/count;
	s->stdev=sqrt(( s->sum_sq - ( s->sum * s->sum / count) )/(count-1.0));
}

//Get nth pctile, e.g. get_nth(.99) gets the 99th pctile.
static double get_nth(stat_inner_t *s,double nth) {
    size_t count = s->count;
	size_t *bins=s->bins;
    size_t n = 0;
    double target = count * nth;

    for (size_t i = 0; i < NUM_BINS; i++) {
      n += bins[i];

      if (n > target) { // The nth is inside bins[i].
        double left = target - (n - bins[i]);
        return pow(_POW, (double) i) +
          left / bins[i] * (pow(_POW, (double) (i+1)) - pow(_POW, (double) i));
      }
    }

    return pow(_POW, NUM_BINS);
} 

void stat_inner_print(stat_inner_t *s) {
	printf(" - Count:%d\n - Total:%6.3f\n",s->count,s->sum);
	printf(" - Avg:%6.3f\n - Stdev:%6.3f\n",s->avg,s->stdev);
	printf(" - Max:%6.3f\n - Min:%6.3f\n",s->max,s->min);
	printf(" - 99th:%6.3f\n",get_nth(s,.99));
}

void stat_init(stat_t *s, char *description) {
	stat_inner_init(&s->latency);
	stat_inner_init(&s->bandwidth);
	if (description==NULL)
		s->desc="Statistics";
	else
		s->desc=strdup(description);
}
void stat_add(stat_t *s, double v_latency, double v_bandwidth) {
	stat_inner_add(&s->latency,v_latency);
	stat_inner_add(&s->bandwidth,v_bandwidth);
}
void stat_update(stat_t *s) {
	stat_inner_update(&s->latency);
	stat_inner_update(&s->bandwidth);
}
void stat_print(stat_t *s) {
	printf("%s\n",s->desc);
	printf("- Latency metrics:\n");
	stat_inner_print(&s->latency);
	printf("- Bandwidth metrics:\n");
	stat_inner_print(&s->bandwidth);
	stat_plot(&s->latency, s->desc);
}

#ifdef GNUPLOT
void stat_plot(stat_inner_t *s, const char *tag) {
	gnuplot_ctrl    *   h1;
	char fn[42];
	char plot_name[80];
	int size,i,ifirst,ilast;
	size_t *bins=s->bins;
	
	if (sum<100) return;
	//find start of latency bins
	for (i=0; i<bins.size(); i++) {
		if (bins[i] > 0)
			break;
	}
	ifirst=i;
	for (; i<bins.size(); i++) {
		if ((bins[i] == 0 ) && (bins[i+1] == 0))
			break;
	}
	ilast=i+1;
	V("Plotting bins %d to %d\n",ifirst,ilast);
	//find end of latency bins
	size=ilast-ifirst;

	double *x=(double *)malloc(size * sizeof(double));
	double *y=(double *)malloc(size * sizeof(double));
	//data for the plot
	for (i=ifirst; i<ilast; i++) {
		int id=i-ifirst;
		x[id]=pow(_POW, (double) i) / 1000.0;
		y[id]=bins[i];
	}
	//plot the bins
	h1 = gnuplot_init() ;
	const char *hstyle="impulses";
    	gnuplot_setstyle(h1, (char *)hstyle) ;
	gnuplot_cmd(h1, "set terminal png");
	gnuplot_cmd(h1, "set xtics rotate");
	sprintf(fn,"set output \"histogram_%s_%02d.png\"",tag,nm);
    	gnuplot_cmd(h1, fn);
	sprintf(plot_name,"%s Histogram (Total Samples=%ld)",tag, s->count);
	gnuplot_plot_xy(h1, x, y, size, plot_name) ;
        sprintf(fn,"histogram_%s_%02d.csv",tag,nm);
	gnuplot_write_xy_csv(fn,x,y,size,plot_name);
	nm++;
	gnuplot_close(h1);
	free(x);
	free(y);
}
#else
void stat_plot(stat_inner_t *s, const char *tag) {  }
#endif


