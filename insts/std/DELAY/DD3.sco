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

set_option("full_duplex_on")
rtsetparams(44100, 2, 512)
load("DELAY")
rtinput("AUDIO", "MIC")
makegen(1, 24, 1000, 0,1, 100,1)
DELAY(0, 0, 14, 1.0, .078, 0.8, 3.5, 0, 0.1)

DELAY(7, 0, 10, 1, .415, 0.5, 3, 0, 0.9)
