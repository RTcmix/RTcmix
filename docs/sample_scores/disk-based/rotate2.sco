/*  rotate -- a pitch-shifting instrument based upon the idea
*	of old rotating tape-head pitch shifters
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude multiplier
*  p4 = pitch shift up or down (oct.pc)
*  p5 = window size  (IN SECONDS!!!!!!!!!!!!!!!)
*  p6 = input channel number
*  p7 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*  assumes function table 2 is the window envelope
*	<usually a hanning window -- use "makegen(2, 25, 1000, 1)">
*
*/

load("rotate")

input("/snd/pablo1.snd")
output("ttt.snd")
makegen(1, 24, 1000, 0,0, 1,1, 8,1, 8.7,0)
makegen(2, 25, 1000, 1) /* hanning window */
rotate(0, 0, 8.7, 0.2, 0, 0.1, 0, 0.5)
rotate(0, 0, 8.7, 0.2,  1.00, 0.11, 0, 0)
rotate(0, 0, 8.7, 0.2, -1.07, 0.14, 1, 0.99)

