/* CLAR -- Perry Cook's clarinet physical model
*
*  p0 = start
*  p1 = dur
*  p2 = noise amp
*  p3 = length1
*  p4 = length2
*  p5 = output amp
*  p6 = d2 gain
*  p7 = stereo spread (0-1) <optional>
*  function slot 1 is the noise amp envelope
*  function slot 2 is the output amp envelope
*
*/

rtsetparams(44100, 1)
load("CLAR")
makegen(1, 24, 1000, 0, 1, 1, 1)
makegen(2, 24, 1000, 0, 1, 1, 1)

d2 = 0
for (start = 0; start < 10; start = start + 0.5) {
	CLAR(start, 0.5, 0.02, 69, 34, 7000, d2)
	d2 = d2 + 0.05
	}
