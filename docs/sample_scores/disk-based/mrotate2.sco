/*  mrotate -- a pitch-shifting instrument based upon the idea
*	of old rotating tape-head pitch shifters
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude multiplier
*  p4 = pitch shift up or down (oct.pc) when function table 3 is 0
*  p5 = pitch shift up or down (oct.pc) when function table 3 is 1
*  p6 = window size
*  p7 = input channel number
*  p8 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*  assumes function table 2 is the window envelope
*	<usually a hanning window -- use "makegen(2, 25, 1000, 1)">
*  assumes function table 3 is the pitch shift envelope
*
*/

load("mrotate")
input("/snd/pablo1.snd")
output("ttt.snd")
makegen(1, 24, 1000, 0,0, 1,1, 7.0,1, 7.8,0)
makegen(2, 25, 1000, 1) /* hanning window */
makegen(3, 7, 1000, 0, 500, 1, 500, 0)
mrotate(0, 0, 7.8, 1, -0.07, 0.09, 0.14, 0, 0)
mrotate(0, 0, 7.8, 1, 0.07, -0.09, 0.15, 0, 1)
