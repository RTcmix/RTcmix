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

load("panecho");

input("sinetone.wav")
system("f2 output.snd");
output("output.snd")
makegen(1, 24, 1000, 0,0, 0.5,1, 3.5,1, 7,0)
panecho(0, 0, 7, 1, .14, 0.069, .7, 3.5)
system("rm -f output.snd");
