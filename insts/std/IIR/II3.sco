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
makegen(1, 24, 1000, 0,1, 0.1,0)
makegen(2, 10, 1024, 1)

pitch = 134.0
for(start = 0; start < 7.8; start = start + 0.1) {
	setup((random()*2000.0) + 300.0, -0.5, 1)
	BUZZ(start, 0.1, 40000, pitch, random())
	BUZZ(start, 0.1, 40000, pitch + 2.5, random())
/*	pitch = pitch + 0.5 */
	}
for(start = 7.8; start < 15; start = start + 0.1) {
	setup((random()*2000.0) + 200.0, -0.5, 1)
	PULSE(start, 0.1, 30000, pitch, random())
	PULSE(start, 0.1, 30000, pitch + 2.5, random())
	pitch = pitch - 0.5
	}
