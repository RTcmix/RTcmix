/* DEL1 - split mono source to two channels, delay right channel

      p0 = output start time
      p1 = input start time
      p2 = output duration
      p3 = amplitude multiplier
      p4 = right channel delay time
      p5 = right channel amplitude multiplier (relative to left channel)
      p6 = input channel [optional, default is 0]
      p7 = ring-down duration [optional, default is first delay time value]

   p3 (amplitude), p4 (delay time) and p5 (delay amplitude) can receive
   dynamic updates from a table or real-time control source.

   The point of the ring-down duration parameter is to let you control
   how long the delay will sound after the input has stopped.  If the
   delay time is constant, DEL1 will figure out the correct ring-down
   duration for you.  If the delay time is dynamic, you must specify a
   ring-down duration if you want to ensure that your sound will not be
   cut off prematurely.
*/

rtsetparams(44100, 2)
load("DEL1")

rtinput("../../snd/huhh.wav")
dur = DUR()

env = maketable("line", 1000, 0,0, 0.5,1, 3.5,1, 7,0)
rtchandel = 0.001
rtchanamp = 1
DEL1(0, 0, dur, env, rtchandel, rtchanamp)

env = maketable("line", 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
rtchandel = 0.14
rtchanamp = 1
DEL1(dur + .5, 0, dur, env, rtchandel, rtchanamp)

