load("sgran")

system("rm -f sgran3.wav")
system("sfcreate -t wav -f sgran3.wav")
output("sgran3.wav")

setline(0,0, 1,1, 2,1, 4,0)
makegen(2,7,1000, 0, 1000, 1)          /* density */
makegen(3,7,1000, 0, 1000, 1)          /* duration */
makegen(4,7,1000, 0, 1000, 1)          /* location */
makegen(5,7,1000, 0, 1000, 1)          /* frequency */
makegen(6,10,8000, 1, .2, .05)         /* grain waveform */
makegen(8,25,4000, 1)                  /* grain envelope */

sgran(start=0, dur=20, amp=1,
/* grain rate: */
.001, .001,                 /* start, end rates */
0, .1, .2, .5,              /* start variation in rate */
0, .1, .2, .5,              /* end variation in rate */
/* grain duration: */
.01, .015, .02, .5,         /* start dur */
.02, .02, .03, .5,          /* end dur */
/* grain channel location (pan): */
0, .5, 1, .9,               /* start location */
0, .5, 1, 0,                /* end location */
/* grain frequency: */
900, 950, 1000, .1,         /* start freq */
700, 950, 1200, .2)         /* end freq */


system("rescale -r -P 20000 sgran3.wav")


/*
   sgran:
      0              start time of group
      1              duration of group
      2              amplitude
      3              beginning grain rate (time in seconds btw. grains)
      4              ending grain rate

      amount of variation in rate: (percentage of grain rate)
      5-8            zero: lo, average, hi, tightness (0-1, is 0-100%)
      9-12           one: lo, average, hi, tightness (0-1, is 0-100%)

      average duration:
      13-16          zero: lo, average, hi, tightness
      17-20          one: lo, average, hi, tightness

      location:
      21-24          zero: lo, average, hi, tightness
      25-28          one: lo, average, hi, tightness

      frequency band:
      29-32          zero: lo, average, hi, tightness
                     (if p29 < 0, noise is the input)
      33-36          one: lo, average, hi, tightness

      37             random seed (integer) [optional]

                 *       *       *

   functions: (stt variation changes are linear)

      1              overall envelope (or setline)
                     (caution: was grain envelope prior to 12 June, 1999)

      shape of change (usually linear for all shapes):
      2              grain density
      3              grain duration
      4              grain location
      5              grain frequency

      6              oscillator waveform

      8              grain envelope
*/
