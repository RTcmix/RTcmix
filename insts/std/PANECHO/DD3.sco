/* panecho: stereo panning delay instrument
*
*  p0 = output start time
*  p1 = input start time
*  p2 = duration
*  p3 = amplitude multiplier
*  p4 = channel 0 delay time
*  p5 = channel 1 delay time
*  p6 = regeneration multiplier (< 1!)
*  p7 = ring-down duration
*  p8 = input channel number <optional>
*  assumes function slot 1 is the amplitude envelope
*/

set_option("full_duplex_on")
rtsetparams(44100, 2)
load("PANECHO")
rtinput("AUDIO", "MIC")
makegen(1, 24, 1000, 0,1, 100, 1)
PANECHO(0, 0, 14, 1.0, 5.14, 1.14, .7, 9.5)
PANECHO(10, 0, 7, 1.0, 1.14, 0.14, .7, 3.5)
