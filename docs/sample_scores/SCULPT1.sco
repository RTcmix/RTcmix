/* SCULPT: instrument for tracking audiosculpt data placed in makegens
*
*  p0 = start time
*  p1 = duration of each point
*  p2 = overall amplitude
*  p3 = number of points
*  p4 = stereo spread (0-1) <optional>
*  function slot 1 is waveform, slot 2 is overall amp envelope
*  function slot 3 is frequency points, slot 4 is amplitude points
*/
rtsetparams(44100, 1)
load("SCULPT")
makegen(1, 10, 1000, 1)
makegen(2, 24, 1000, 0, 1, 1, 1)
makegen(3, 2, 10, 0)
149.0 159.0 169.0 179.0 189.0 199.0 214.0 215.0 234.0 314.0
makegen(4, 2, 10, 0)
0.0 -7.0 -10.0 -3.0 0.0 -10.0 -20.0 -15.0 -2.1 -1.1

SCULPT(0, 0.5, 10, 10)
