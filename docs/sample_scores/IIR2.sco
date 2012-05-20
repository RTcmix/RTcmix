/* IINOISE - process white noise with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

   Each filter has a center frequency (cf), bandwidth (bw) and gain control.
   Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
   is a multiplier of the center frequency.  Gain is the amplitude of this
   filter relative to the other filters in the bank.  There can be as many
   as 64 filters in the bank.

   Then call IINOISE:

      p0 = output start time
      p1 = duration
      p2 = amplitude
      p3 = pan (in percent-to-left form: 0-1) [optional, default is 0]

   p2 (amplitude) and p3 (pan) can receive dynamic updates from a table
   or real-time control source.
*/

rtsetparams(44100, 2)
load("IIR")

env = maketable("line", 1000, 0,0, 1,1, 2,0)
amp = 5000

start = 0
for (pc = 0; pc < 0.25; pc += 0.01) {
	setup(8.00 + pc, 1.0, 1.0)
	IINOISE(start, dur=0.2, amp * env, random())
	start = start + 0.1
}

