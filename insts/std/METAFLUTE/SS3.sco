/* vsflute -- vibrato version of Perry Cook's flute physical model
*
*  p0 = start
*  p1 = dur
*  p2 = noise amp
*  p3 = length1low
*  p4 = length1high
*  p5 = length2low
*  p6 = length2high
*  p7 = amp multiplier
*  p8 = vibrato freq 1 low
*  p9 = vibrato freq 1 high
*  p10 = vibrato freq 2 low
*  p11 = vibrato freq 2 high
*  p12 = stereo spread (0-1) <optional>
*  function slot 1 is the noise amp envelope
*  function slot 2 is the output amp envelope
*  function slot 3 is the vibrato function for length 1
*  function slot 4 is the vibrato function for length 2
*
*/


rtsetparams(44100, 2)
load("METAFLUTE")

makegen(1, 7, 1000, 1, 1000, 1)
makegen(2, 7, 1000, 1, 1000, 1)
makegen(3, 10, 1000, 1)
makegen(4, 10, 1000, 1)
VSFLUTE(0, 3.5, 0.1, 60,62, 30,40, 5000, 1.0,4.0, 1.0,5.0)
VSFLUTE(4, 3.5, 0.1, 48,50, 30,45, 5000, 4.0,7.0, 3.0,5.0)
