/* del1: single delay instrument
*       into stereo output
*
*  p0 = output start time
*  p1 = input start time
*  p2 = duration
*  p3 = amplitude multiplier
*  p4 = delay time
*  p5 = delay amplitude multiplier
*  p6 = input channel number <optional>
*  assumes function slot 1 is the amplitude envelope
*/

rtsetparams(44100, 2)
load("DEL1")
rtinput("../../../snd/input.wav");
makegen(1, 24, 1000, 0,0, 0.5,1, 3.5,1, 7,0)
DEL1(0, 0, 7, 1, .14)

makegen(1, 24, 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
DEL1(6.9, 0, 7, 1, 3.14)
