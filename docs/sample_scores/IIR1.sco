/* INPUTSIG - process a mono input with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

   Each filter has a center frequency (cf), bandwidth (bw) and gain control.
   Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
   is a multiplier of the center frequency.  Gain is the amplitude of this
   filter relative to the other filters in the bank.  There can be as many
   as 64 filters in the bank.

   Then call INPUTSIG:

      p0 = output start time
      p1 = input start time
      p2 = duration
      p3 = amplitude multiplier
      p4 = input channel [optional, default is 0]
      p5 = pan (in percent-to-left form: 0-1) [optional, default is 0]

   p3 (amplitude) and p5 (pan) can receive dynamic updates from a table
   or real-time control source.
*/

rtsetparams(44100, 2)
load("IIR")

rtinput("../../snd/nucular.wav")
inchan = 0
inskip = 0
dur = DUR()

env = maketable("line", 1000, 0,0, 1,1, 5,1, 7,0)

setup(cf=149.0, bw=25.0, gain=1.0, cf=1415.0, bw=100.0, gain=0.8)
INPUTSIG(0, inskip, dur, env * amp=1.0, inchan, pan=.5)

start = dur + 1
setup(cf=9.0, bw=25.0, gain=1.0, cf=10.0, bw=100.0, gain=0.8)
INPUTSIG(start, inskip, dur, amp=0.45, inchan, pan=.5)

