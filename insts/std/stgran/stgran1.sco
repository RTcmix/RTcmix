load("stgran")

system("rm -f stgran1.wav")
system("sfcreate -c 2 -t wav -f stgran1.wav")
output("stgran1.wav")

setline(0,0, 1,1, 3,1, 4,0)
makegen(2,18,1000, 0,0, 1,1)
makegen(3,18,1000, 0,0, 1,1)
makegen(4,18,1000, 0,0, 1,1)
makegen(5,18,1000, 0,0, 1,1)
makegen(6,18,1000, 0,0, 1,1)
makegen(7,18,1000, 0,0, 1,1)
makegen(8,25,4000, 1)                 /* grain envelope */

input("/snd/Public_Sounds/JF4Scat.aiff")
inchan = 0
inskip = .1

outdur = 10
seed = 3

stgran(start=0, inskip, outdur, indur=0, amp=1,
/* grain rates (seconds per grain): */
.0000, .0001,               /* input file rate: start, end */
.02, .02,                   /* output file rate: start, end */

0, 0, 0, 1,                 /* variation in start input rate [ignored now] */
0, 0, 0, 1,                 /* variation in end input rate [ignored now] */
0, 0, 0, 1,                 /* variation in start output rate */
0, 0, 0, 1,                 /* variation in end output rate */

/* grain duration: */
.3, .4, .5, 1,              /* start grain dur */
.3, .4, .5, 1,              /* end grain dur */

/* grain transposition (oct.pc): */
-.01, 0, 0, 1,              /* start transposition */
-.01, 0, 0, 1,              /* end transposition */

/* grain amp: */
1, 1, 1, 1,                 /* start amp */
1, 1, 1, 1,                 /* end amp */

/* grain stereo location: */
.2, .5, .8, 1,              /* start stereo location */
.2, .5, .8, 1,              /* end stereo location */

seed, inchan)               /* random seed */

system("rescale -r -P 20000 stgran1.wav")


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
