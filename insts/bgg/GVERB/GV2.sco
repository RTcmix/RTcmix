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

roomsize = maketable("line", "nonorm", 1000, 0, 78.0, 3,200.0, 7,10.0, 10,25.0)
revtime = maketable("line", "nonorm", 1000, 0,1, 10,50)
damping = makeLFO("tri", 0.4, 0.1, 0.9)
amp = 0.7

GVERB(0, 0, 9.8, amp, roomsize, revtime, damping, 0.34, -10.0, -11.0, -9.0, 7.0)


inputbandwidth = makeLFO("tri", 0.5, 0.1, 0.9)
drylevel = maketable("line", "nonorm", 1000, 0,-1.0,  5,-50.0,  7,-1.0, 15,-1.0)
amp = 0.3

GVERB(14, 0, 9.8, amp, 35.0, 15.0, 0.5, inputbandwidth, drylevel, -11.0, -9.0, 5.0)


earlylevel = maketable("line", "nonorm", 1000, 0,-1.0, 5,-68, 9,-10.0, 15,-10.0)
taillevel = maketable("line", "nonorm", 1000, 0,-70, 5,-3.5, 10,-50, 15,-50)
amp = 0.9
GVERB(25, 0, 9.8, amp, 143.0, 9.0, 0.7, 0.7, -27.0, earlylevel, taillevel, 3.0)

