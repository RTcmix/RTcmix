/* combitfilt --  new comb filter instrument
*
* p0 = output skip
* p1 = input skip
* p2 = input duration
* p3 = amplitude multiplier
* p4 = pitch (cps)
* p5 = a
* p6 = b
* p7 = filter type (0=FIR,1=IIR)
* p8 = wetdry (not used)
* p9 = input channel [optional]
* p10 = stereo spread [optional]
*
*/

load("iCOMBFILT")
rtsetparams(44100, 2)
rtinput("/snd/jmr6u/eyes")

outsk=0
insk=0
dur=10
amp=1.1
pitch=6.09
a=0.1
b=0.9
ftype=1
wetdry=0
inchan=0
spread=0.5
dur = 3
COMBFILT(outsk, insk, dur, amp, pitch, a, b, ftype, wetdry, inchan, spread)
outsk = outsk+dur
insk = insk+dur
pitch = 8.00
COMBFILT(outsk, insk, dur, amp, pitch, a, b, ftype, wetdry, inchan, spread)
outsk = outsk+dur
insk = insk+dur
pitch = 8.06
COMBFILT(outsk, insk, dur, amp, pitch, a, b, ftype, wetdry, inchan, spread)


