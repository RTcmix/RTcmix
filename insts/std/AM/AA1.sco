/* am -- amplitude modulate an input signal
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude
*  p4 = AM modulator frequency (hz)
*  p5 = input channal [optional]
*  p6 = stereo spread <0-1> [optional]
*  assumes function table 1 is the amplitude envelope
*  function table 2 is the AM modulator waveform
*
*/

rtsetparams(44100, 2)
load("AM")
rtinput("/sndh/bob.dole.mono")
makegen(1, 24, 1000, 0,0, 2,1, 5,1, 7,0)
makegen(2, 10, 1000, 1)
AM(0, 0, 7, 1, 14)
AM(8, 0, 7, 1, 187)
