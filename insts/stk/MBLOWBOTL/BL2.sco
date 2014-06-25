rtsetparams(44100, 2)
load("./libMBLOWBOTL.so")

/* MBLOWBOTL - the "BlowBotl" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0 - 1.0, I think...)
   p6 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is breathPressure (amplitude) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

makegen(1, 24, 1000, 0,0, 1,1, 2,0)
st = 0
noiseamp = 0.1
amp = 20000.0 * 5
MBLOWBOTL(st, 2.0, amp, cpspch(7.03), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(7.05), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(7.07), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(7.08), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(7.10), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(8.00), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(8.02), noiseamp, 0.9)
st = st + 2.0
MBLOWBOTL(st, 2.0, amp, cpspch(8.03), noiseamp, 0.9)
