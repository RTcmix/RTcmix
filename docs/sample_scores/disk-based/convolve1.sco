input("/snd/Sounds3/Line1.snd")
output("ttt.snd")
input("/snd/Sounds3/Cchord1rvb.snd", 2)

/* source window (2nd arg is duration) */
setwindow(0, 0.2, 0,0, 20,1, 80,1, 100,0)
/* impulse window (2nd arg is duration) */
setwindow(1, 0.2, 0,0, 20,1, 80,1, 100,0)

/* convolve
*
*  p0 = output skip
*  p1 = source input skip
*  p2 = output duration
*  p3 = impulse file number (p1 on the input() command)
*  p4 = duration to read impulse response file
*  p5 = impulse input skip
*  p6 = amplitude multiplier
*  p7 = invert flag (0 or 1, swaps the imp + src, I think)
*	-- usually "0"
*  p8 = "dry" (usually 0)
*  p9 = outpan (0-1)
*
*/

outsk = 0.0
srcinsk = 0.0
impinsk = 0.0
impdur = 0.1

for(i = 0; i < 30; i = i + 1) { /*  0.1 skips means 3 seconds done */
	convolve(outsk, srcinsk, 0.1, 2, impdur, impinsk, 1, 0, 0, 0.5)
	outsk = outsk + 0.1
	srcinsk = srcinsk + 0.1
	impinsk = impinsk + 0.1
	}
