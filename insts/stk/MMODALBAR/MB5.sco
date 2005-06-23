rtsetparams(44100, 2)
load("./libMMODALBAR.so")

/* MMODALBAR - the "ModalBar" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = stick hardness (0.0-1.0)
   p5 = stick position (0.0-1.0)
   p6 = modal preset
	- Marimba = 0
	- Vibraphone = 1
	- Agogo = 2
	- Wood1 = 3
	- Reso = 4
	- Wood2 = 5
	- Beats = 6
	- Two Fixed = 7
	- Clump = 8
   p7 = percent of signal to left output channel [optional, default is .5]
*/

MMODALBAR(0, 1.0, 2*30000, 243.0, 0.4, 0.4, 1)

amp = maketable("line", 1000, 0,0, 4,1, 5,0)
MMODALBAR(2, 1.0, amp*9*30000, 243.0, 0.4, 0.4, 1)

reset(44100)
bamp = maketable("line", 10000, 0,1, 0.0003,0, 1,0)
MMODALBAR(4, 1.0, 9*30000, 243.0, 0.4, 0.4, 1, 0.5, bamp)

reset(1000)
freq = maketable("line", "nonorm", 1000, 0,243.0, 1,149.0)
pan = makeLFO("saw", 1.0, 1.0, 0.0)
MMODALBAR(6, 1.0, 30000, freq, 0.4, 0.4, 1, pan)
