/*  wow -- apply fm to an input soundfile
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude
*  p4 = modulator depth (index)
*  p5 = modulator frequency (hz)
*  p6 = input channel [optional]
*  p7 = stereo spread <0-1> [optional]
*  assumes function table 1 is the amplitude envelope
*  function table 2 is modulator waveform
*
*/

load("wow")

input("/sndgr/bob.dole.mono")
output("/sndgr/tryit.snd")
makegen(1, 24, 1000, 0,0, 1,1, 7,1, 8,0)
makegen(2, 10, 1000, 1)
wow(0, 0, 8, 1, 1, 4, 0, 0.5)
