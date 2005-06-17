/* DELAY - delay instrument with feedback

      p0 = output start time
      p1 = input start time
      p2 = input duration
      p3 = amplitude multiplier
      p4 = delay time
      p5 = delay feedback (i.e., regeneration multiplier) [0-1]
      p6 = ring-down duration
      p7 = input channel [optional, default is 0]
      p8 = pan (in percent-to-left form: 0-1) [optional, default is 0]

   p3 (amplitude), p4 (delay time), p5 (feedback) and p8 (pan) can receive
   dynamic updates from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The point of the ring-down duration parameter is to let you control
   how long the delay will sound after the input has stopped.  Too short
   a time, and the sound may be cut off prematurely.
*/

rtsetparams(44100, 2)
load("DELAY")
rtinput("../../../snd/input.wav")
dur = DUR()
inchan = 0

makegen(1, 24, 1000, 0,0, 0.5,1, 3.5,1, 7,0)
DELAY(0, 0, dur, amp=.5, deltime=.14, feedback=.7, ringdur=3.5, inchan, pan=.1)

makegen(1, 24, 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
DELAY(3.5, 0, dur, amp=1, deltime=1.4, feedback=.3, ringdur=5, inchan, pan=.9)

