/* EQ - equalizer instrument (peak/notch, shelving and high/low pass types)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = EQ type (0: low pass, 1: high pass, 2: low shelf, 3: high shelf,
        4: peak/notch)
   p5 = input channel [optional, default is 0]
   p6 = percent of signal to left output channel [optional, default is .5]
   p7 = bypass filter (0: no, 1: yes) [optional, default is 0]

   Function tables:
      1  amplitude curve (or use setline)
         If no setline or function table 1, uses flat amplitude curve.
      2  frequency (Hz)
      3  Q (with values from 0.5 to 10.0, roughly)
      4  gain (dB) [shelf and peak/notch only]

   John Gibson <johgibso at indiana dot edu>, 7 Dec 2003

   Based on formulas by Robert Bristow-Johnson ("Audio-EQ-Cookbook") and code
   by Tom St Denis <tomstdenis.home.dhs.org> (see musicdsp.org)
*/

rtsetparams(44100, 2)
load("EQ")

rtinput("mystereofile.wav")
inskip = 0
dur = DUR()
amp = 1
bypass = 0

//type = 0    // low pass
//type = 1    // high pass
type = 2    // low shelf
//type = 3    // high shelf
//type = 4    // peak/notch

freq = 100
Q = 3.0
gain = 6.0

makegen(2, 18, 1000, 0, freq, 1, freq)
makegen(3, 18, 1000, 0, Q, 1, Q)
makegen(4, 18, 1000, 0, gain, 1, gain)

EQ(0, inskip, dur, amp, type, 0, 1)
EQ(0, inskip, dur, amp, type, 1, 0)

