/* waveshaping instrument
*  p0 = start
*  p1 = duration
*  p2 = pitch (hz or oct.pc)
*  p3 = index low point
*  p4 = index high point
*  p5 = amp
*  p6 = stereo spread (0-1) <optional>
*  function slot 1 is waveform to be shaped (generally sine)
*     slot 2 is amp envelope
*     slot 3 is the transfer function
*     slot 4 is the index envelope
*/

rtsetparams(44100, 2)
load("WAVESHAPE")
makegen(1, 10, 1000, 1)
makegen(2, 24, 1000, 0,0, 3.5,1, 7,0)
makegen(3, 17, 1000, 0.9, 0.3, -0.2, 0.6, -0.7)
makegen(4, 24, 1000, 0,0, 3.5,1, 7,0)
WAVESHAPE(0, 7, 7.02, 0, 1, 9000, 0.99)
makegen(2, 24, 1000, 0,0, 1.5,1, 7,0)
makegen(4, 24, 1000, 0,1, 7,0)
WAVESHAPE(4, 7, 6.091, 0, 1, 10000, 0.01)
