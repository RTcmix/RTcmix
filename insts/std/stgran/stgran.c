/* stgran - granular sampling

   pfields:

      0      output start time
      1      input start time
      2      output duration
      3      input duration     [ignored for now]
      4      overall amplitude multiplier

      grain rates (seconds per grain):
      5      input file beginning grain rate
      6      input file ending grain rate
      7      output file beginning grain rate
      8      output file ending grain rate

      Each of the following parameters are controlled by 2 groups of 4 pfields.
      One group represents the beginning state of the parameter; the other,
      its ending state. The progress between these states over time is
      determined by a "shape of change" function table for the parameter.
      (See below, at "functions".)

      Each group of 4 pfields has low, mid, high and tightness values
      for a state. These values define a range (low, high) within which a
      random number is chosen, a preferred value (mid) within the range, and
      the strength of adherence to the preferred value (tight). Values of
      <tight> work like this:

         0   values adhere to the extreme (low or high) furthest from <mid>
         1   even distribution within range
         2+  values adhere more and more closely to <mid>; when <tight> is 200,
             most values are equal to <mid>, with the rest very close to it.

      variation in grain rate (percentage of rate, 0-2 is 0-200%):
      9-12   input file zero:  low, mid, high, tight   [not yet working]
      13-16  input file one:  low, mid, high, tight   [not yet working]
      17-20  output file zero: low, mid, high, tight
      21-24  output file one: low, mid, high, tight

      grain duration:
      25-28  zero: low, mid, high, tight
      29-32  one: low, mid, high, tight

      grain transposition (oct.pc):
      33-36  zero: low, mid, high, tight
      37-40  one: low, mid, high, tight

      grain amp (0-1):
      41-44  zero: low, mid, high, tight
      45-48  one: low, mid, high, tight

      grain stereo location (percent to left; ignored if output is mono):
      49-52  zero: low, mid, high, tight
      53-56  one: low, mid, high, tight

      57     random seed (integer)  [optional]

      58     input channel  [optional - default is 0]

   functions:

      1      overall envelope (or call setline)
             (was grain envelope prior to 12 June, 1999)

      shape of change (usually linear for all shapes):
      2      grain input rate (density)               [not yet working]
      3      grain output rate (density)
      4      grain duration
      5      grain transposition
      6      grain amp
      7      grain stereo location

      8      grain envelope (note: was function 1 in Mara's version)

   The output file must not have more than 2 channels.

   Instrument by Mara Helmuth. (Revised for RTcmix by John Gibson.)
*/

/* Changes:

   22 april 2000 - JGG
      * remove mono input and output file restrictions:
          - add input channel pfield
          - add stereo location control, as in sgran
      * dynamic allocation of input and output buffers
      * convert transposition pfields to linear octaves before using them
      * update grain envelope on every sample
      * consolidate identical calls to tablei
      * change grain env function table assignment so as not to conflict
        with setline -- part of conformance with RTcmix setline scheme.
      * outskip, inskip, outdur, indur fields reordered to conform to RTcmix
      * expanded usage comments
*/
#include <ugens.h>
#include <sfheader.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <math.h>
#include <assert.h>

//#define DEBUG

#define INPUT  0
#define OUTPUT 1

#define MAX(val1, val2)  (((val1) > (val2)) ? (val1) : (val2))

extern SFHEADER sfdesc[NFILES];
extern int resetval;            /* defined in sys/minc_functions.c */
extern float SR();



static double
interp(double y0, double y1, double y2, double t)
{
   register double hy2, hy0, a, b, c;

   a = y0;
   hy0 = y0 / 2.0;
   hy2 = y2 / 2.0;
   b = (-3.0 * hy0) + (2.0 * y1) - hy2;
   c = hy0 - y1 + hy2;

   return (a + b * t + c * t * t);
}


/* Return a value within a range close to a preferred value.
   tightness values:
      0     adheres to the extreme furthest from mid
      1     even distribution
      2+    hugs mid more and more with increasing values of tight
      no negative allowed
*/
static double
prob(double low, double mid, double high, double tight)
{
   int repeat;
   double lowrange, hirange, range, num, tightrand;

   if (tight < 0.0)
      tight = 0.0;

   lowrange = mid - low;
   hirange = high - mid;
   range = MAX(lowrange, hirange);

   repeat = 0;
   do {
      tightrand = pow((rrand() + 1.0) * 0.5, tight);

      if (rrand() > 0.0)
         num = mid + (tightrand * range);
      else
         num = mid - (tightrand * range);

      if (num < low || num > high)
         repeat++;
      else
         repeat = 0;
   } while (repeat > 0);

   return num;
}


double
stgran(float p[], int n_args)
{
   int inchans, outchans, inchan;
   float indur, outdur, insk, outsk;
   long bgrainsamps, bgraindist, bgrainslide, inbgraindist;
   long i, nsamps, gstt_var, in_gstt_var, count, outcount, branch, skip;
   long s;                      /* sample number */
   long egrainsamps, egraindist, egrainslide, inegraindist;
   long grainsamps, grainslide, graindistdiff;
   long ingrainsamps, ingrainslide, ingraindistdiff;
   long  maxinsamps, maxoutsamps;
   float val, amp, aamp;
   float gdist_inc, in_gdist_inc;
   float tab1[2], tab2[2], tab3[2], tab4[2], tab5[2], tab6[2], tab7[2], tab8[2];
   double *in_rate_shape, *rate_shape, *dur_shape, *transp_shape, *amp_shape;
   double *loc_shape, *envel, *slarray;
   float *inarr, *outarr;
   double gstt_per, in_gstt_per, lo, mid, hi, ti, sig, table_val;
   double slodiff, smiddiff, shidiff, stidiff;
   double ilodiff, imiddiff, ihidiff, itidiff;
   double dlodiff, dmiddiff, dhidiff, dtidiff;
   double alodiff, amiddiff, ahidiff, atidiff, grainamp;
   double tlodiff, tmiddiff, thidiff, ttidiff;
   double llodiff, lmiddiff, lhidiff, ltidiff, pctleft = 0.0;
   double tlobeg, tmidbeg, thibeg, tloend, tmidend, thiend;
   double voldsig = 0.0, oldsig, newsig, interval, increment;
   double counter = 0., frac;
   register int incount = 1, getflag = 1;

   outsk = p[0];
   insk = p[1];
   outdur = p[2];
   indur = p[3];
   amp = p[4];

   setnote(insk, indur, INPUT);
   nsamps = setnote(outsk, outdur, OUTPUT);

   tlobeg = (double) octpch(p[33]);
   tmidbeg = (double) octpch(p[34]);
   thibeg = (double) octpch(p[35]);
   tloend = (double) octpch(p[37]);
   tmidend = (double) octpch(p[38]);
   thiend = (double) octpch(p[39]);

   inchan = (int) p[58];
   inchans = sfchans(&sfdesc[INPUT]);
   if (inchan >= inchans)
      die("stgran", "You asked for channel %d of a %d-channel file.",
                                                          inchan, inchans);
   outchans = sfchans(&sfdesc[OUTPUT]);
   if (outchans > 2)
      die("stgran", "Output file must be either mono or stereo.");

   /* allocate input and output buffers */
   interval = MAX(thibeg, thiend);      /* maximum transp. value (lin oct) */
   increment = cpsoct(10.0 + interval) / cpsoct(10.0);
   hi = MAX(p[31], p[27]) * SR();         /* maximum grain duration */
   maxinsamps = (long) (hi * increment + 1.0);
   inarr = malloc(maxinsamps * inchans * sizeof(float));
   if (inarr == NULL)
      die("stgran", "Can't allocate input buffer.");
   maxoutsamps = (long) (hi + 1.0);
   outarr = malloc(maxoutsamps * outchans * sizeof(float));
   if (outarr == NULL)
      die("stgran", "Can't allocate output buffer.");

   bgrainsamps = grainsamps = p[26] * SR();
   bgraindist = p[7] * SR();
   bgrainslide = grainslide = bgraindist - bgrainsamps;
   egrainsamps = p[30] * SR();
   egraindist = p[8] * SR();
   egrainslide = egraindist - egrainsamps;
   graindistdiff = egraindist - bgraindist;

   inbgraindist = p[5] * SR();
   inegraindist = p[6] * SR();
   ingraindistdiff = inegraindist - inbgraindist;

   in_rate_shape = floc(2);
	if (in_rate_shape == NULL)
		die("stgran",
				"You haven't made the grain input rate function (table 2).");
   tableset(SR(), indur - p[6], fsize(2), tab2);

   rate_shape = floc(3);
	if (rate_shape == NULL)
		die("stgran",
				"You haven't made the grain output rate function (table 3).");
   tableset(SR(), outdur - p[8], fsize(3), tab3);

   dur_shape = floc(4);
	if (dur_shape == NULL)
		die("stgran", "You haven't made the grain duration function (table 4).");
   tableset(SR(), outdur - p[8], fsize(4), tab4);

   transp_shape = floc(5);
	if (transp_shape == NULL)
		die("stgran",
				"You haven't made the grain transposition function (table 5).");
   tableset(SR(), outdur - p[8], fsize(5), tab5);

   amp_shape = floc(6);
	if (amp_shape == NULL)
		die("stgran", "You haven't made the grain amplitude function (table 6).");
   tableset(SR(), outdur - p[8], fsize(6), tab6);

   loc_shape = floc(7);
	if (loc_shape == NULL)
		die("stgran",
				"You haven't made the grain stereo location function (table 7).");
   tableset(SR(), outdur - p[8], fsize(7), tab7);

   envel = floc(8);           /* tableset in sample loop */
	if (envel == NULL)
		die("stgran", "You haven't made the grain envelope (table 8).");

   /* get infile stt var zero/one differences */
   ilodiff = (double) (p[13] - p[9]) / nsamps;
   imiddiff = (double) (p[14] - p[10]) / nsamps;
   ihidiff = (double) (p[15] - p[11]) / nsamps;
   itidiff = (double) (p[16] - p[12]) / nsamps;

   /* get outfile stt var zero/one differences */
   slodiff = (double) (p[21] - p[17]) / nsamps;
   smiddiff = (double) (p[22] - p[18]) / nsamps;
   shidiff = (double) (p[23] - p[19]) / nsamps;
   stidiff = (double) (p[24] - p[20]) / nsamps;

   /* get dur zero/one differences */
   dlodiff = (double) (p[29] - p[25]);
   dmiddiff = (double) (p[30] - p[26]);
   dhidiff = (double) (p[31] - p[27]);
   dtidiff = (double) (p[32] - p[28]);

   /* transp zero/one differences */
   tlodiff = tloend - tlobeg;
   tmiddiff = tmidend - tmidbeg;
   thidiff = thiend - thibeg;
   ttidiff = (double) (p[40] - p[36]);

   /* amp zero/one differences */
   alodiff = (double) (p[45] - p[41]);
   amiddiff = (double) (p[46] - p[42]);
   ahidiff = (double) (p[47] - p[43]);
   atidiff = (double) (p[48] - p[44]);

   /* loc zero/one differences */
   llodiff = (double) (p[53] - p[49]);
   lmiddiff = (double) (p[54] - p[50]);
   lhidiff = (double) (p[55] - p[51]);
   ltidiff = (double) (p[56] - p[52]);

   if (p[57] > 0)
      srrand(p[57]);
   else
      srrand(3);

   skip = SR() / (float) resetval;               /* control rate for amp curve */

   gstt_var = in_gstt_var = 0;
   count = 0;

   aamp = amp;                       /* in case there is no setline function */
   slarray = floc(1);
   if (slarray) {
      int len = fsize(1);
      tableset(SR(), outdur, len, tab1);
   }
   else
      rtcmix_advise("stgran", "Setting phrase curve to all 1's.");

   for (i = 0; i < nsamps; i++) {
      count++;
      table_val = (double) tablei(i, transp_shape, tab5);
      lo = tlobeg + (tlodiff * table_val);
      mid = tmidbeg + (tmiddiff * table_val);
      hi = thibeg + (thidiff * table_val);
      ti = p[36] + (ttidiff * table_val);
      lo = (lo > mid) ? mid : lo;
      hi = (hi < mid) ? mid : hi;
      interval = prob(lo, mid, hi, ti);                      /* in lin oct */
      increment = cpsoct(10.0 + interval) / cpsoct(10.0);    /* samp incr. */

      /* calculate next grain duration */
      table_val = (double) tablei(i, dur_shape, tab4);
      lo = p[25] + (dlodiff * table_val);
      mid = p[26] + (dmiddiff * table_val);
      hi = p[27] + (dhidiff * table_val);
      ti = p[28] + (dtidiff * table_val);
      lo = (lo > mid) ? mid : lo;
      hi = (hi < mid) ? mid : hi;
      grainsamps = (long) (prob(lo, mid, hi, ti) * SR());
      tableset(SR(), grainsamps / SR(), fsize(8), tab8);

      /* calculate grain amplitude */
      table_val = (double) tablei(i, amp_shape, tab6);
      lo = p[41] + (alodiff * table_val);
      mid = p[42] + (amiddiff * table_val);
      hi = p[43] + (ahidiff * table_val);
      ti = p[44] + (atidiff * table_val);
      lo = (lo > mid) ? mid : lo;
      hi = (hi < mid) ? mid : hi;
      grainamp = prob(lo, mid, hi, ti);

      /* calculate grain stereo location */
      if (outchans > 1) {
         table_val = (double) tablei(i, amp_shape, tab7);
         lo = p[49] + (llodiff * table_val);
         mid = p[50] + (lmiddiff * table_val);
         hi = p[51] + (lhidiff * table_val);
         ti = p[52] + (ltidiff * table_val);
         lo = (lo > mid) ? mid : lo;
         hi = (hi < mid) ? mid : hi;
         pctleft = prob(lo, mid, hi, ti);
      }

      /* get percentage to vary next stt of grain */
      lo = p[17] + slodiff * i;
      mid = p[18] + smiddiff * i;
      hi = p[19] + shidiff * i;
      ti = p[20] + stidiff * i;
      lo = (lo > mid) ? mid : lo;
      hi = (hi < mid) ? mid : hi;
      gstt_per = prob(lo, mid, hi, ti);
      gstt_var = (long) (gstt_per * (grainsamps + grainslide));

      /* calculate grainslide */
      gdist_inc = tablei(i, rate_shape, tab3);
      grainslide = (float) bgraindist
                     + (float) graindistdiff * gdist_inc - grainsamps;

      ingrainsamps = grainsamps * increment;

#ifdef DEBUG
      printf("ingrainsamps: %ld, maxinsamps: %ld\n", ingrainsamps, maxinsamps);
      assert(ingrainsamps <= maxinsamps);
      assert(grainsamps <= maxoutsamps);
#endif

      bgetin(inarr, INPUT, ingrainsamps * inchans);  /* read in grain */

      ingrainslide = ((float) inbgraindist) * increment
                      + (float) ingraindistdiff * increment - ingrainsamps;

      /* transpose the grain and write it to output file */
      oldsig = inarr[inchan];
      newsig = inarr[inchans + inchan];
      incount = 1;
      outcount = 0;
      counter = 0;
      branch = 0;
      for (s = 0; s < grainsamps; s++) {
         while (getflag) {
            voldsig = oldsig;
            oldsig = newsig;
            newsig = inarr[(incount * inchans) + inchan];
            incount++;
            if ((counter - (float) incount) < 0.5) {
               getflag = 0;
            }
         }
         /* update overall amp envelope at control rate */
         while (!branch--) {
            if (slarray)
               aamp = tablei(i, slarray, tab1) * amp;
            branch = skip;
         }
         /* update grain envelope on every sample */
         val = tablei(s, envel, tab8) * grainamp * aamp;

         frac = counter - incount + 2.0;               /* the interp value */

         sig = interp(voldsig, oldsig, newsig, frac) * val;

         if (outchans == 2) {
            outarr[outcount] = sqrt(pctleft) * sig;
            outarr[outcount + 1] = sqrt(1.0 - pctleft) * sig;
         }
         else
            outarr[outcount] = sig;

         counter += increment;            /* keeps track of interp pointer */
         if ((counter - (float) incount) >= -0.5) {
            getflag = 1;
         }
         outcount += outchans;
      }
      baddout(outarr, OUTPUT, grainsamps * outchans);
      inrepos(ingrainslide, INPUT);

      if ((i + grainslide + gstt_var + grainsamps) < 0) {
         outrepos(grainslide, OUTPUT);
         i += grainsamps;
         i += grainslide;
      }
      else {
         outrepos((grainslide + gstt_var), OUTPUT);
         i += grainsamps;
         i += grainslide;
         i += gstt_var;
      }
   }
   printf("\n%ld grains\n", count);
   endnote(OUTPUT);

   free(inarr);
   free(outarr);

   return 0.0;
}


int
profile()
{
   UG_INTRO("stgran", stgran);
   return 0;
}

