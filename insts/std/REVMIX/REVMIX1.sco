/* REVMIX - plays the input file in reverse

   Plays the input file backward for the specified duration, starting
   at the input start time. If you specify a duration that would result
   in an attempt to read before the start of the input file, it will
   shorten the note to prevent this.

   Note that you can't use this instrument with a real-time input
   (microphone or aux bus), only with input from a sound file. (That's
   because the input start time of a an inst taking real-time input must
   be zero, but this instrument reads backward from its inskip.)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]
   p5 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Ivica "Ico" Bukvic <ico@fuse.net>, 27 May 2000
   (with John Gibson <johngibson@virginia.edu>)
*/
rtsetparams(44100, 1)
load("REVMIX")
bus_config("REVMIX", "in 0", "out 0")

rtinput("/snd/Public_Sounds/steeldrums.aiff")

setline(0,0, 1,1, 9,1, 10,0)

dur = DUR()

REVMIX(start=0, inskip=dur, dur, amp=1)

