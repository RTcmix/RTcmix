/* LSFLUTE -- Perry Cook's flute physical model
*	(changes params without reinitializing the delay arrays
*	 for legato effects)
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

rtsetparams(44100, 1)
reset(10000) 
load("METAFLUTE")
makegen(1, 24, 1000, 0, 1, 1.5, 1)
makegen(2, 24, 1000, 0, 1, 1.5, 1)
SFLUTE(0.000000, 0.100000, 0.050000, 100.000000, 100.000000, 5000.000000)
LSFLUTE(0.000000, 0.280000, 0.040000, 50.000000, 16.000000, 5000.000000)
LSFLUTE(0.280000, 0.280000, 0.040000, 60.000000, 16.000000, 5000.000000)
LSFLUTE(0.560000, 0.280000, 0.040000, 70.000000, 16.000000, 5000.000000)
LSFLUTE(0.840000, 0.280000, 0.040000, 90.000000, 16.000000, 5000.000000)

