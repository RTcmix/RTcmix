/* gravy -- a time and pitch-shifting instrument
*  	(modified 11/89 by D.S. to use parabolic interpolation)
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration (on input)
*  p3 = window size (in seconds)
*  p4 = amplitude multiplier
*  p5 = length multiplier (of original)
*  p6 = transposition interval (oct.pc)
*  p7 = input channel [optional]
*  p8 = stereo spread (0-1) [optional]
*  assumes function table 1 contains the window envelope
*
*/

load("gravy")
input("/snd/toys/woodysrc.snd")
output("ttt.snd")
makegen(1, 25, 1000, 1)
gravy(0, 0, 2.5, 0.1, 0.2, 3.0, 0.00, 0, 0.5)
gravy(0, 0, 2.5, 0.1, 0.2, 3.0, 0.02, 0, .9)
gravy(0, 0, 2.5, 0.1, 0.2, 3.0, -0.03, 0, 0.01)
