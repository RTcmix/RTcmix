load("sgran")

system("rm -f sgran2.wav")
system("sfcreate -t wav -f sgran2.wav")
output("sgran2.wav")

setline(0,0, 1,1, 9,1, 10,0)
makegen(2,7,1000, 0, 1000, 1)
makegen(3,7,1000, 0, 1000, 1)
makegen(4,7,1000, 0, 1000, 1)
makegen(5,7,1000, 0, 500, .9, 500, 1)
makegen(6,10,8000, 1, .5, .2, .1)     /* grain waveform */
makegen(8,25,4000, 1)                 /* grain envelope */

sgran(start=0, dur=30, amp=1,
/* grain rate: */
.02, .01,                   /* start, end rates */
0, 0, 0, 1,                 /* start variation in rate */
0, 0, 0, 1,                 /* end variation in rate */
/* grain duration: */
.01, .01, .01, 1,           /* start dur */
.01, .1, .2, .5,            /* end dur */
/* grain channel location (pan): */
0, .5, 1, 1,                /* start location */
0, .5, 1, 0,                /* end location */
/* grain frequency: */
200, 260, 300, .2,          /* start freq */
200, 200, 200, 1)           /* end freq */

system("rescale -r sgran2.wav")


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
