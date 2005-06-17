/* BUZZ - process a buzz wave signal with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

   Each filter has a center frequency (cf), bandwidth (bw) and gain control.
   Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
   is a multiplier of the center frequency.  Gain is the amplitude of this
   filter relative to the other filters in the bank.  There can be as many
   as 64 filters in the bank.

   Then call BUZZ:

      p0 = output start time
      p1 = duration
      p2 = amplitude
      p3 = pitch (Hz or oct.pc)
      p4 = pan (in percent-to-left form: 0-1) [optional, default is 0]

   p2 (amplitude), p3 (pitch) and p4 (pan) can receive dynamic updates
   from a table or real-time control source.

   PULSE has the same syntax, but uses a pulse, instead of a buzz, waveform.
*/

rtsetparams(44100, 2)
load("IIR")

env = maketable("line", 1000, 0,1, 0.1,0)

amp = 22000

pitch = 134.0
for (start = 0; start < 7.8; start = start + 0.1) {
	setup((random() * 2000.0) + 300.0, -0.5, 1)
	BUZZ(start, 0.1, amp * env, pitch, random())
	BUZZ(start, 0.1, amp * env, pitch + 2.5, random())
//	pitch = pitch + 0.5	// try uncommenting this
}

amp *= 2

for (start = 7.8; start < 15; start = start + 0.1) {
	setup((random() * 2000.0) + 200.0, -0.5, 1)
	PULSE(start, 0.1, amp * env, pitch, random())
	PULSE(start, 0.1, amp * env, pitch + 2.5, random())
	pitch = pitch - 0.5
}

