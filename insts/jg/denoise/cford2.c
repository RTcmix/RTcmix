#include <math.h>

/*-------------------------------
	cford2.c

in-place reordering subroutine
-------------------------------*/

void
cford2(int m, float *b)
{
	float	t;
	long	n, ij, ji, k,
		jj1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12,
		j13, j14, j15, j16, j17, j18, 
		l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12,
		l13, l14, l15, l16, l17, l18, l19,  l[19];

	n = pow(2.,(float)m);
	l[0] = n;
	for (k = 2; k <= m; k++) l[k-1] = l[k-2]/2;
	for (k = m; k <= 18; k++) l[k] = 2;
	ij = 2;

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
			  for (ji=j18; ji<=l19; ji+=l18){
		 	   if (ij<ji) {
			    t = b[ij-2];
			    b[ij-2] = b[ji-2];
			    b[ji-2] = t;
			    t = b[ij-1];
			    b[ij-1] = b[ji-1];
			    b[ji-1] = t;
			   }
			   ij = ij+2;
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

