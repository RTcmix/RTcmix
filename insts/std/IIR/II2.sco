/* iir -- creates an iir filter with up to 64 resonance peaks, specifiable
*  center frequency and bandwidth for each peak
*
*  subcommands:
*
*  setup()
*  p0 = center frequency 1 (hertz or oct.pc)
*  p1 = bandwidth 1 (or multiplier of cf if negative)
*  p2 = relative amplitude 1
*  <p3, p4, p5 ... up to 64 triples>
*
*  inputsig()
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude multiplier
*  p4 = input channel (0 or 1)
*  p5 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*
*  noise()
*  p0 = start
*  p1 = duration
*  p2 = amplitude
*  p3 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*
*  buzzit()
*  p0 = start
*  p1 = duration
*  p2 = amplitude
*  p3 = pitch (hz or oct.pc)
*  p4 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*  assumes function table 2 is a sine wave
*
*  pulseit()
*  p0 = start
*  p1 = duration
*  p2 = amplitude
*  p3 = pitch (hz or oct.pc)
*  p4 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*
*/

rtsetparams(44100, 2)
load("IIR")
makegen(1, 24, 1000, 0,0, 0.1,1, 0.2,0)

start = 0
for(pc = 0; pc < 0.25; pc = pc + 0.01) {
	setup(8.00 + pc, 1.0, 1.0)
	IINOISE(start, 0.2, 5000, random())
	start = start + 0.1
	}
