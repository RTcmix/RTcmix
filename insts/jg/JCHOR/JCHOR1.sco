/* JCHOR - random-wait chorus instrument based on Paul Lansky's chor
           and Doug Scott's trans code

   p0  = output start time
   p1  = input start time
   p2  = output duration
   p3  = input duration (not input end time)
   p4  = maintain input duration, regardless of transposition (1: yes, 0: no)
   p5  = transposition (8ve.pc)
   p6  = number of voices (minimum of 1)
   p7  = minimum grain amplitude
   p8  = maximum grain amplitude
   p9  = minimum grain wait (seconds)
   p10 = maximum grain wait (seconds)
   p11 = seed (0 - 1)
   p12 = input channel [optional]
         (If p12 is missing and input has > 1 chan, input channels averaged.)

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Assumes function table 2 is grain window function. (Try gen 25.)

   By default, the amplitude envelope is updated 1000 times per second,
   but this can be changed by calling reset() with a different value.

   Output can be either mono or stereo. If it's stereo, the program randomly
   distributes the voices across the stereo field.

   Differences between JCHOR and chor (besides RT ability):
      - No limit on input duration or number of voices
      - Transpose the input signal
      - Specify the input channel to use (or an average of them)
      - Specify overall amplitude curve and grain window function via makegens

   John Gibson (jgg9c@virginia.edu), 9/20/98, RT'd 6/24/99.
*/
rtsetparams(44100, 2)
load("JCHOR")

rtinput("your_input.snd")

outskip = 0
outdur = 10
inskip = 0.20
indur = 0.20
maintain_dur = 1
transposition = 0.07
nvoices = 60
minamp = 0.01
maxamp = 1.0
minwait = 0.00
maxwait = 0.60
seed = 0.9371

makegen(2, 25, 1000, 1)                /* last arg: 1=hanning, 2=hamming */

JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

