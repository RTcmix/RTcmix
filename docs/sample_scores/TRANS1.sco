/* TRANS - transpose a mono input signal using cubic spline interpolation

   p0 = output start time
   p1 = input start time
   p2 = output duration (time to end if negative)
   p3 = amplitude multiplier
   p4 = interval of transposition, in octave.pc
   p5 = input channel [optional, default is 0]
   p6 = percent to left [optional, default is .5]

   Processes only one channel at a time.

   Assumes function table 1 is amplitude curve for the note.
   You can call setline for this.

   TRANS was written by Doug Scott.
   Revised by John Gibson <johngibson@virginia.edu>, 2/29/00.
*/

rtsetparams(44100, 2)
load("TRANS")

rtinput("../../snd/input.wav");

setline(0,0, 1,1, 9,1, 10,0)
makegen(2, 24, 1000, 0, 0, 1, 1)
makegen(3, 24, 1000, 0, 1, 1, 1)
makegen(4, 24, 1000, 0, 1, 1, 0)
dur = DUR()
transp = -2.0;

TRANS(start=0, inskip=0, translen(dur, transp), amp=2, transp, 0, 0.5);

