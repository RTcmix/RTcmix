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
CLAR(0, 1, 0.02, 78, 31, 7000, 0)
CLAR(1, 1, 0.02, 35, 4, 7000, 0)
CLAR(2, 1, 0.02, 35, 9, 7000, 0)
CLAR(3, 1, 0.02, 51, 20, 7000, 0.5)
