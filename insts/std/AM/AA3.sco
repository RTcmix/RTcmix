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

set_option("full_duplex_on")
rtsetparams(44100, 2, 256)
load("AM")
rtinput("AUDIO", "MIC", 2)
/* rtinput("AUDIO", "LINE", 2) */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.2,1, 0.3,0)
makegen(2, 10, 1000, 1)

for(start = 0; start < 15.0; start = start + 0.1) {
        freq = random() * 400.0
        AM(start, 0, 0.3, 1, freq, 0, random())
        }
