/* GVERB -- long reverberator

	based on the original "gverb/gigaverb" by Juhana Sadeharju
		kouhia at nic.funet.fi
	code taken from the Max/MSP port by Olaf Matthes, <olaf.matthes@gmx.de>
	(see "origcopyright.txt" accompanying this instrument)

   p0 = output start time
   p1 = input start time
   p2 = duration
   *p3 = output amp multiplier
   *p4 = roomsize (1.0 -- 300.0)
   *p5 = reverb time (0.1 -- 360.0)
   *p6 = damping (0.0 -- 1.0)
   *p7 = input bandwidth (0.0 -- 1.0)
   *p8 = dry level (inverse dB, -90.0 -- 0.0)
   *p9 = early reflection level (inverse dB, -90.0 -- 0.0)
   *p10 = tail level (inverse dB, -90.0 -- 0.0)
	p11 = ring-down time (added to duration)
	p12 = input channel [optional, default = 0]

   * p-fields marked with an asterisk can receive dynamic updates
   from a table or real-time control source

	pfield tables will extend over the duration + ring-down time

   BGG, 5/2010
*/

rtsetparams(44100, 2)
load("./libGVERB.so")

rtinput("../../../snd/loocher.aiff")

GVERB(0, 0, 9.8, 0.9, 78.0, 7.0, 0.71, 0.34, -10.0, -11.0, -9.0, 7.0)
GVERB(10, 0.0, 0.15, 1.0, 178.0, 21.0, 0.91, 0.14, -30.0, -7.0, -1.0, 5.0)
GVERB(11, 0.0, 0.15, 1.0, 178.0, 21.0, 0.91, 0.54, -30.0, -7.0, -1.0, 5.0)
GVERB(12, 0.0, 0.15, 1.0, 178.0, 21.0, 0.91, 0.95, -30.0, -7.0, -1.0, 5.0)
GVERB(14, 4.5, 3.5, 0.45, 18.0, 1.0, 0.91, 0.54, -30.0, -1.0, -9.0, 1.0)


