rtsetparams(44100, 1)
load("FMINST")
makegen(1, 24, 1000, 0, 0, 0.1,1, 4,1, 7,0)
makegen(2, 10, 1000, 1)
makegen(3, 24, 1000, 0,1, 7,0)
/*
   FMINST -- simple FM instrument
 
   p0 = start time
   p1 = duration
   p2 = amp *
   p3 = frequency of carrier (Hz or oct.pc **)
   p4 = frequency of modulator (Hz or oct.pct)
   p5 = FM index low point
   p6 = FM index high point
   p7 = pan (in percent-to-left form: 0-1) [optional; default is 0]
   p8 = reference to wavetable [optional; if missing, must use gen 2 ***]
   p9 = index guide [optional; if missing, must use gen 3 ****]

   p2 (amplitude), p3 (carrier freq), p4 (modulator freq), p5 (index low),
   p6 (index high), p7 (pan) and p9 (index guide) can receive dynamic updates
   from a table or real-time control source.

   * If an old-style gen table 1 is present, its values will be multiplied
   by p2 (amp), even if the latter is dynamic.

   ** oct.pc format generally will not work as you expect for p3 and p4
   (osc freq) if the pfield changes dynamically.  Use Hz instead in that case.

   *** If p8 is missing, you must use an old-style gen table 2 for the
   oscillator waveform.

   **** If p9 is missing, you must use an old-style gen table 3 for the
   index guide function.

                                                rev for v4, JGG, 7/12/04
*/


pitch = 100;
count = 0;
start = 0;
num = 9
denom = 8
subcount = 0;
incr = 1;

while (count < 52)
{
    FMINST(start, 3, 1000, pitch, pitch, 0, 0, 0.0)
    pitch = pitch * (num / denom);
    start = start + 0.2;
    count = count + 1;
    subcount = subcount + 1;
    if (subcount >= 5) {
	incr = incr * -1;
	subcount = 0;
    }
    num = num + incr;
    denom = denom + incr;
}
