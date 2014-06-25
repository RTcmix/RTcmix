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
load("METAFLUTE")

reset(2000)
makegen(1, 24, 1000, 0,1, 1.5,1)
makegen(2, 24, 1000, 0,0, 0.05,1, 1.49,1, 1.5,0)
SFLUTE(0, 1.5, 0.1, 40, 28, 10000, 0.5)
SFLUTE(1.5, 1.5, 0.1, 40, 22, 10000, 0.5)

/* bsflute -- pitch-bending version of Perry Cook's flute physical model
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
*  function slot 1 is the noise amp envelope
*  function slot 2 is the output amp envelope
*  function slot 3 is the pitch-tracking curve for length 1
*  function slot 4 is the pitch-tracking curve for length 2
*
*/
makegen(1, 7, 1000, 1, 1000, 1)
makegen(2, 7, 1000, 1, 1000, 1)
makegen(3, 7, 1000, 0, 500, 1, 500, 0)
makegen(4, 7, 1000, 0, 1000, 1)
BSFLUTE(4, 1.5, 0.1, 40,50, 28,40, 5000, 0.5)

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
makegen(1, 7, 1000, 1, 1000, 1)
makegen(2, 7, 1000, 1, 1000, 1)
makegen(3, 10, 1000, 1)
makegen(4, 10, 1000, 1)
VSFLUTE(6, 3.5, 0.1, 60,62, 30,40, 5000, 1.0,4.0, 1.0,5.0, 0.5)
VSFLUTE(9.5, 3.5, 0.1, 48,50, 30,45, 5000, 4.0,7.0, 3.0,5.0, 0.5)
