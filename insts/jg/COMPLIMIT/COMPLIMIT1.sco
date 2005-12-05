/* COMPLIMIT: a compressor-limiter

   p0  = output start time
   p1  = input start time
   p2  = duration
   p3  = gain applied to input before compression (in dB)
   p4  = gain applied to output after compression - "makeup gain" (in dB)
   p5  = attack time (seconds)
   p6  = release time (seconds)
   p7  = threshold (in dBFS)
   p8  = compression ratio - e.g. 20 means 20:1 (100 is infinity)
   p9  = look-ahead time (seconds)
   p10 = peak detection window size (power of 2 <= RTCmix output buffer size)
   p11 = detection type (0: peak, 1: average peak, 2: rms)
   p12 = bypass (1: bypass on, 0: bypass off)
   p13 = input channel  [optional; default is 0]
   p14 = percent output to left channel (0 - 1)  [optional; default is 0.5]

   BUGS:
      - Sometimes the compressor will stay in sustain for too long.
        This happens when the source sound has a long decaying envelope
        where some of the decay is above the threshold.

   John Gibson <johngibson@virginia.edu>,  21 April, 2000
*/
rtsetparams(44100, 2)
load("COMPLIMIT")

bus_config("COMPLIMIT", "in0-1", "out0-1")

rtinput("../../../snd/input.wav");
inskip = 0;
dur = DUR(0);

ingain = 0
outgain = 0
attack = 0.01
release = 0.06
threshold = -10
ratio = 100
lookahead = attack
windowlen = 128
detect_type = 0
bypass = 0

outskip = 0
COMPLIMIT(outskip, inskip, dur, ingain, outgain, attack, release, threshold,
          ratio, lookahead, windowlen, detect_type, bypass, inchan=0, pctleft=.5)

