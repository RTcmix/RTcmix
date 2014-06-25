# include <math.h>

/*---------------------------
	cfr4tr.c

Radix 4 iteration subroutine.
---------------------------*/

void
cfr4tr(long off, long nn, float *b0, float *b1, float *b2, float *b3,
                          float *b4, float *b5, float *b6, float *b7)
{
	extern	double	pii, p7;

	long	i, off4, jj0, jlast, k0, kl, ji, jl, jr, j, k,
		jj1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12,
		j13, j14, j15, j16, j17, j18, jthet, th2,
		l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12,
		l13, l14, l15, l16, l17, l18, l19,  l[19];

	float	piovn, arg, c1, c2, c3, s1, s2, s3, r1, r5, pr, pi,
		t0, t1, t2, t3, t4, t5, t6, t7;

/* jthet is a reversed binary counter; jr steps two at a time to
	locate real parts of intermediate results, and ji locates
	the imaginary part corresponding to jr.
*/

	l[0] = nn / 4;
	for (i=1; i<19; i++){
		if (l[i-1] < 2) l[i-1] = 2;
		else if (l[i-1] != 2){
			l[i] = l[i-1] / 2;
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

	/* for (j1=2; jj1<=l1; jj1+=2){ */
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
			   if (th2 <= 0){
			    for (i=0; i<off; i++){
			     t0 = b0[i] + b2[i];
			     t1 = b1[i] + b3[i];
			     b2[i] = b0[i] - b2[i];
			     b3[i] = b1[i] - b3[i];
			     b0[i] = t0 + t1;
			     b1[i] = t0 - t1;
			    }
			    if (nn > 4){
			     k0 = 4 * off;
			     kl = k0 + off;
			     for (i=k0; i<kl; i++){
			      pr = p7 * (b1[i] - b3[i]);
			      pi = p7 * (b1[i] + b3[i]);
			      b3[i] = b2[i] + pi;
			      b1[i] = pi - b2[i];
			      b2[i] = b0[i] - pr;
			      b0[i] = b0[i] + pr;
			     }
			    }
			   }
			   else{
			    arg = th2 * piovn;
			    c1 = cos(arg);
			    s1 = sin(arg);
			    c2 = c1*c1 - s1*s1;
			    s2 = c1*s1 + c1*s1;
			    c3 = c1*c2 - s1*s2;
			    s3 = c2*s1 + s2*c1;
			    off4 = 4 * off;
			    jj0 = jr * off4;
			    k0 = ji * off4;
			    jlast = jj0 + off;
			    for (j=jj0; j<jlast; j++){
			     k = k0 + j - jj0;
			     r1 = b1[j]*c1 - b5[k]*s1;
			     r5 = b1[j]*s1 + b5[k]*c1;
			     t2 = b2[j]*c2 - b6[k]*s2;
			     t6 = b2[j]*s2 + b6[k]*c2;
			     t3 = b3[j]*c3 - b7[k]*s3;
			     t7 = b3[j]*s3 + b7[k]*c3;
			     t0 = b0[j]+t2;
			     t4 = b4[k]+t6;
			     t2 = b0[j]-t2;
			     t6 = b4[k]-t6;
			     t1 = r1+t3;
			     t5 = r5+t7;
			     t3 = r1-t3;
			     t7 = r5-t7;
			     b0[j] = t0+t1;
			     b7[k] = t4+t5;
			     b6[k] = t0-t1;
			     b1[j] = t5-t4;
			     b2[j] = t2-t7;
			     b5[k] = t6+t3;
			     b4[k] = t2+t7;
			     b3[j] = t3-t6;
			    }
			    jr += 2;
			    ji -= 2;
			    if (ji <= jl){
			     ji = 2*jr - 1;
			     jl = jr;
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
}

