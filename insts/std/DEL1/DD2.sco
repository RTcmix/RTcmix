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

set_option("full_duplex_on")
rtsetparams(44100, 2, 512)
load("DEL1")
rtinput("AUDIO", "MIC")
makegen(1, 24, 1000, 0,0, 1,1, 16,1, 17,0)
DEL1(0, 0, 17, 1, 4.3, 1)
