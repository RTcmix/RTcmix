/* BSFLUTE -- pitch-bending version of Perry Cook's flute physical model
*
*  p0 = start
*  p1 = dur
*  p2 = noise amp
*  p3 = length1low
*  p4 = length1high
*  p5 = length2low
*  p6 = length2high
*  p7 = amp multiplier
*  p8 = stereo spread (0-1) <optional>
*  p9 = delete flag, for deleting the delay-line arrays <optional>
*  function slot 1 is the noise amp envelope
*  function slot 2 is the output amp envelope
*  function slot 3 is the pitch-tracking curve for length 1
*  function slot 4 is the pitch-tracking curve for length 2
*
*/


rtsetparams(44100, 1)
load("METAFLUTE")
makegen(1, 7, 1000, 1, 1000, 1)
makegen(2, 7, 1000, 1, 1000, 1)
makegen(3, 7, 1000, 0, 500, 1, 500, 0)
makegen(4, 7, 1000, 0, 1000, 1)
BSFLUTE(0, 1.5, 0.1, 20,25, 14,19, 10000)
