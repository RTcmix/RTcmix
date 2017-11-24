/* PHASER

   Runs samples through a user-specified number of allpass filters.  It
   processes only one input channel for a given note.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = stages (should be even number, recommended 2 to 16 or 24)
   p5 = LFO frequency (in cps)
   p6 = reverb time (seconds)
   p7 = wet/dry mix [optional (0.0 - 1.0), default is .5 (50% wet/50% dry)]
   p8 = input channel [optional, default is 0]
   p9 = pan (in percent-to-left format) [optional, default is .5]

   p3 (amp) and p9 (pan) can receive updates from a table or real-time
   control source.

   Jenny Bernard <bernarjf at email dot uc dot edu>, 12/7/05
*/
rtsetparams(44100, 2)
load("PHASER")

rtinput("../../../snd/loocher.aiff")

makegen(2, 10, 1000, 1)
ampenv = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)
PHASER(0, 0, DUR(), 1*ampenv, 4, 0.3, 7.1, 1.0, 0, 0.5)
