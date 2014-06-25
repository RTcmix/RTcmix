/* This script implements synchronous granular synthesis, producing
   a steady pitch. One grain follows another at the rate determined
   by <freq>, which gives the fundamental frequency of the sound.
   The frequency of the oscillator producing the grain is <gfreq>,
   which yields a strong formant peak at that frequency. The grain
   envelope shape affects the timbre: the steeper the envelope, the
   wider the spectral envelope of the grain. See Dodge for more on
   all this.     -JGG
*/
load("sgran")

system("rm -f sgran4.wav")
system("sfcreate -c 1 -t wav sgran4.wav")
output("sgran4.wav")

setline(0,0, 1,1, 4,1, 5,0)
makegen(2,7,1000, 0, 1000, 1)          /* density */
makegen(3,7,1000, 0, 1000, 1)          /* duration */
makegen(4,7,1000, 0, 1000, 1)          /* location */
makegen(5,7,1000, 0, 1000, 1)          /* frequency */
makegen(6,10,8000, 1, .2, .05)         /* grain waveform */

/* grain envelope (the second one will be in effect) */
makegen(8,18,4000, 0,0, 1,1, 2,1, 3,0) /* ramp */
makegen(8,25,4000, 1)                  /* hanning window */

freq = 220
rate = 1 / freq
gfreq = 440

sgran(start=0, dur=5, amp=20000,
/* grain rate: */
rate, rate,                 /* start, end rates */
0, 0, 0, 1,                 /* start variation in rate */
0, 0, 0, 1,                 /* end variation in rate */
/* grain duration: */
rate, rate, rate, 1,        /* start dur */
rate, rate, rate, 1,        /* end dur */
/* grain channel location (pan):  NOTE: we're using mono output anyway */
0, .5, 1, 1,                /* start location */
0, .5, 1, 1,                /* end location */
/* grain frequency: */
gfreq, gfreq, gfreq, 1,     /* start freq */
gfreq, gfreq, gfreq, 1)     /* end freq */

