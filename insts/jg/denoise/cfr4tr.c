#include <math.h>

/*-------------------------------
	cfr4syn.c

radix 4 synthesis
-------------------------------*/

void
cfr4syn(long off, long nn, float *b0, float *b1, float *b2, float *b3,
                           float *b4, float *b5, float *b6, float *b7)
{
	extern	double	pii, p7two;

	float	piovn, arg, c1, c2, c3, s1, s2, s3, 
		t0, t1, t2, t3, t4, t5, t6, t7;

	long	i, off4, jj0, jlast, k0, kl, ji, jl, jr, j, k,
		jj1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12,
		j13, j14, j15, j16, j17, j18, jthet, th2,
		l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12,
		l13, l14, l15, l16, l17, l18, l19,
		l[19];

/* jthet is a reversed binary counter; jr steps two at a time to
	locate real parts of intermediate results, and ji locates
	the imaginary part corresponding to jr.
*/

	l[0] = nn/4;
	for (i=1; i<19; i++){
		if (l[i-1]<2) l[i-1] = 2;
		else if (l[i-1]!=2) {
			l[i] = l[i-1]/2;
			continue;
		}
		l[i] = 2;
	}

	l1 = l[18];
	l2 = l[17];
	l3 = l[16];
	l4 = l[15];
	l5 = l[14];
	l6 = l[13];
	l7 = l[12];
	l8 = l[11];
	l9 = l[10];
	l10 = l[9];
	l11 = l[8];
	l12 = l[7];
	l13 = l[6];
	l14 = l[5];
	l15 = l[4];
	l16 = l[3];
	l17 = l[2];
	l18 = l[1];
	l19 = l[0];

	piovn = pii / nn;
	ji = 3;
	jl = 2;
	jr = 2;

	/* for (jj1=2; jj1<=l1; jj1+=2){ */
	jj1 = 2;
loop1:	 /* for (j2=jj1; j2<=l2; j2+=l1){ */
	 j2 = jj1;
loop2:	  /* for (j3=j2; j3<=l3; j3+=l2){ */
	  j3 = j2;
loop3:	   /* for (j4=j3; j4<=l4; j4+=l3){ */
	   j4 = j3;
loop4:	    /* for (j5=j4; j5<=l5; j5+=l4){ */
	    j5 = j4;
loop5:	     for (j6=j5; j6<=l6; j6+=l5){
	      for (j7=j6; j7<=l7; j7+=l6){
	       for (j8=j7; j8<=l8; j8+=l7){
		for (j9=j8; j9<=l9; j9+=l8){
		 for (j10=j9; j10<=l10; j10+=l9){
		  for (j11=j10; j11<=l11; j11+=l10){
		   for (j12=j11; j12<=l12; j12+=l11){
		    for (j13=j12; j13<=l13; j13+=l12){
		     for (j14=j13; j14<=l14; j14+=l13){
		      for (j15=j14; j15<=l15; j15+=l14){
		       for (j16=j15; j16<=l16; j16+=l15){
			for (j17=j16; j17<=l17; j17+=l16){
			 for (j18=j17; j18<=l18; j18+=l17){
			  for (jthet=j18; jthet<=l19; jthet+=l18){
			   th2 = jthet - 2;
			   if (th2 > 0) {
			    arg = th2 * piovn;
			    c1 = cos(arg);
			    s1 = -sin(arg);
			    c2 = c1*c1 - s1*s1;
			    s2 = c1*s1 + c1*s1;
			    c3 = c1*c2 - s1*s2;
			    s3 = c2*s1 + s2*c1;
			    off4 = 4 * off;
			    jj0 = jr * off4;
			    k0 = ji * off4;
			    jlast = jj0 + off;
			    for (j=jj0; j<jlast; j++){
			     k = k0+j-jj0;
			     t0 = b0[j] + b6[k];
			     t1 = b7[k] - b1[j];
			     t2 = b0[j] - b6[k];
			     t3 = b7[k] + b1[j];
			     t4 = b2[j] + b4[k];
			     t5 = b5[k] - b3[j];
			     t6 = b5[k] + b3[j];
			     t7 = b4[k] - b2[j];
			     b0[j] = t0 + t4;
			     b4[k] = t1 + t5;
			     b1[j] = (t2+t6)*c1 - (t3+t7)*s1;
			     b5[k] = (t2+t6)*s1 + (t3+t7)*c1;
			     b2[j] = (t0-t4)*c2 - (t1-t5)*s2;
			     b6[k] = (t0-t4)*s2 + (t1-t5)*c2;
			     b3[j] = (t2-t6)*c3 - (t3-t7)*s3;
			     b7[k] = (t2-t6)*s3 + (t3-t7)*c3;
			    }
			    jr = jr + 2;
			    ji = ji - 2;
			    if (ji <= jl){
			     ji = 2*jr - 1;
			     jl = jr;
			    }
			   }
			   else {
			    for (i=0; i<off; i++){
			     t0 = b0[i] + b1[i];
			     t1 = b0[i] - b1[i];
			     t2 = b2[i] * 2.0;
			     t3 = b3[i] * 2.0;
			     b0[i] = t0 + t2;
			     b2[i] = t0 - t2;
			     b1[i] = t1 + t3;
			     b3[i] = t1 - t3;
			    }
			    if (nn>4) {
			     k0 = 4*off;
			     kl = k0 + off;
			     for (i=k0; i<kl; i++) {
			      t2 = b0[i] - b2[i];
			      t3 = b1[i] + b3[i];
			      b0[i] = (b0[i] + b2[i]) * 2.0;
			      b2[i] = (b3[i] - b1[i]) * 2.0;
			      b1[i] = (t2 + t3) * p7two;
			      b3[i] = (t3 - t2) * p7two;
			     }
			    }
			   }
			  }
			 }
			}
		       }
		      }
		     }
		    }
		   }
		  }
		 }
		}
	       }
	      }
	     }
	    j5 += l4;
	    if (j5 <= l5) goto loop5;
	    /* } */
	   j4 += l3;
	   if (j4 <= l4) goto loop4;
	   /* } */
	  j3 += l2;
	  if (j3 <= l3) goto loop3;
	  /* } */
	 j2 += l1;
	 if (j2 <= l2) goto loop2;
	 /* } */
	jj1 += 2;
	if (jj1 <= l1) goto loop1;
	/* } */
}

