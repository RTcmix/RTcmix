/* sflute -- Perry Cook's flute physical model
*
*  p0 = start
*  p1 = dur
*  p2 = noise amp
*  p3 = length1
*  p4 = length2
*  p5 = amp multiplier
*  p6 = stereo spread (0-1) <optional>
*  function slot 1 is the noise amp envelope
*  function slot 2 is the output amp envelope
*
*/


rtsetparams(44100, 2)
reset(2000)
load("METAFLUTE")

makegen(1, 24, 1000, 0,1, 1.5,1)
makegen(2, 24, 1000, 0,0, 0.05,1, 1.49,1, 1.5,0)
SFLUTE(0, 1.5, 0.1, 40, 28, 7000)
SFLUTE(1.5, 1.5, 0.1, 40, 22, 7000)
