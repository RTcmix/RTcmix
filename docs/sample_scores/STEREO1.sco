/* Mix any number of inputs to stereo outputs with global amplitude control
   and individual pans.

      p0 = output start time
      p1 = input start time
      p2 = duration (-endtime)
      p3 = amplitude multiplier
      p4-n = channel mix maxtrix (see below)

   p3 (amplitude) can receive dynamic updates from a table or real-time
   control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The mix matrix works like this.  For every input channel, the corresponding
   number in the matrix gives the output stereo pan for that channel, in
   percent-to-left form (0 is right; 1 is left).  p4 corresponds to input
   channel 0, p5 corresponds to input channel 1, etc.  If the value of one
   of these pfields is negative, then the corresponding input channel will 
   not be played.  Note that you cannot send a channel to more than one
   output pan location.

	Each mix matrix pfield (pctleft) can receive dynamic updates.
*/

rtsetparams(44100, 2)
load("STEREO")

rtinput("../../snd/input.wav")

amp = maketable("line", 1000, 0,0, 1, 1, 1.1, 0) * 1.5
STEREO(0, 0, DUR(), amp, 0.8)

amp = maketable("line", 1000, 0,0, 0.1, 1, 1, 0) * 1.5
STEREO(DUR() + .2, 0, DUR(), amp, 0.1)

