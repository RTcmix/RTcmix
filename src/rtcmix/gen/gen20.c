#include "../H/ugens.h"
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

gen20(gen)

     /* fills a gen with random numbers b/t 0 and 1 --
      * the argument is the distribution type:
      * 0 = even distribution
      * 1 = low weighted linear distribution
      * 2 = high weighted linear distribution
      * 3 = triangle linear distribution
      * 4 = gaussian distribution
      * 5 = cauchy distribution
      *
      * by r. luke dubois - CMC/CU, 1998.
      *
      * distribution equations adapted from dodge and jerse.
      *
      */

register struct gen *gen;

{
int i, j, k;
int N=12;
float halfN = 6;
float scale = 1;
float mu = 0.5;
float sigma = .166666;
float randnum = 0.0;
float randnum2 = 0.0;
struct timeval tv;
float output;
float alpha = .00628338;
static long randx=1;


 switch((int)(gen->pvals[1])) {    /* added optional seed  -JG */
 case 0:
   gettimeofday(&tv, NULL);
   randx = tv.tv_usec;
   break;
 default:
   randx = (int)(gen->pvals[1]);
 }

 switch((int)(gen->pvals[0])) {
 case 0: /* even distribution */
   for(i=0;i<gen->size;i++){
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     gen->array[i] = (float)k/32768.0;
   }
   break;
 case 1: /* low weighted */
   for(i=0;i<gen->size;i++) {
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     randnum = (float)k/32768.0;
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     randnum2 = (float)k/32768.0;
     if(randnum2 < randnum) {
       randnum = randnum2;
     }
     gen->array[i] = randnum;
   }
   break;
 case 2: /* high weighted */
   for(i=0;i<gen->size;i++) {
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     randnum = (float)k/32768.0;
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     randnum2 = (float)k/32768.0;
     if(randnum2 > randnum) {
       randnum = randnum2;
     }
     gen->array[i] = randnum;
   }
   break;
 case 3: /* triangle */
   for(i=0;i<gen->size;i++) {
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     randnum = (float)k/32768.0;
     k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
     randnum2 = (float)k/32768.0;
     gen->array[i] = 0.5*(randnum+randnum2);
   }
   break;
 case 4: /* gaussian */
   i=0;
   while(i<gen->size) {
     randnum = 0.0;
     for(j=0;j<N;j++)
       {
	 k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
	 randnum += (float)k/32768.0;
       }
     output = sigma*scale*(randnum-halfN)+mu;
     if((output<=1.0) && (output>=0.0)) {
       gen->array[i] = output;
       i++;
     }
   }
   break;
 case 5: /* cauchy */
   i=0;
   while(i<gen->size)
     {
       do {
	 k = ((randx = randx*1103515245 + 12345)>>16) & 077777;
       randnum = (float)k/32768.0;
       }
       while(randnum==0.5);
       randnum = randnum*PI;
       output=(alpha*tan(randnum))+0.5;
       if((output<=1.0) && (output>=0.0)) {
	 gen->array[i] = output;
	 i++;
       }
     }
   break;
 default:
   fprintf(stderr, "gen20:  don't know about distribution %d\n",(int)(gen->pvals[0]));
                        exit(-1);
 }
}




