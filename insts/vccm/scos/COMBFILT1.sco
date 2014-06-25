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
rtinput("/snd/Public_Sounds/jimi3dSound.aiff")

pgen("line", 24, 1000, 0,0,1,1)


outsk=0
insk=0
dur=DUR()
amp=1.1
pitch=6.09
a=0.1
b=0.9
ftype=1
wetdry=0
inchan=0
spread=0.5

inst_pfield_path("line","COMBFILT",4, 0, cpspch(pitch), dur/2, cpspch(8.00))

COMBFILT(outsk, insk, dur, amp, cpspch(pitch), a, b, ftype, wetdry, inchan, spread)

pgen("exp", 5, 1000, 0.0001, 1000, 1)
inst_pfield_path("exp","COMBFILT",4, dur, cpspch(pitch), dur+(dur/2), cpspch(8.00))
outsk=outsk+dur

COMBFILT(outsk, insk, dur, amp, cpspch(pitch), a, b, ftype, wetdry, inchan, spread)
