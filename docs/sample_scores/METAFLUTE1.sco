/* SFLUTE -- Perry Cook's flute physical model
*
*  p0 = start
*  p1 = dur
*  p2 = noise amp
*  p3 = length1
*  p4 = length2
*  p5 = amp multiplier
*  p6 = stereo spread (0-1) <optional>
*  p7 = delete flag, for deleting the delay-line arrays <optional>
*  function slot 1 is the noise amp envelope
*  function slot 2 is the output amp envelope
*
*/


rtsetparams(44100, 1)
load("METAFLUTE")
reset(2000)
makegen(1, 24, 1000, 0,1, 1.5,1)
makegen(2, 24, 1000, 0,0, 0.05,1, 1.49,1, 1.5,0)
SFLUTE(0, 1.5, 0.1, 20, 14, 10000)
SFLUTE(1.5, 1.5, 0.1, 20, 11, 10000)
