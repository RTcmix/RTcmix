/* delay: regenerating delay instrument
*
*  p0 = output start time
*  p1 = input start time
*  p2 = input duration
*  p3 = amplitude multiplier
*  p4 = delay time
*  p5 = regeneration multiplier (< 1!)
*  p6 = ring-down duration
*  p7 = input channel number <optional>
*  p8 = stereo spread (0-1) <optional>
*  assumes function slot 1 is the amplitude envelope
*/

rtsetparams(44100, 2)
load("DELAY")
rtinput("../../../snd/input.wav");
makegen(1, 24, 1000, 0,0, 0.5,1, 3.5,1, 7,0)
DELAY(0, 0, 7, 0.5, .14, 0.7, 3.5, 0, 0.1)

makegen(1, 24, 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
DELAY(3.5, 0, 7, 1, 1.4, 0.3, 5, 1, 0.9)
