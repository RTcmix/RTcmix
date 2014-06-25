rtinput("AUDIO")

bus_config("WAVETABLE", "aux 0 out")
/* carrier */
bus_config("MIX", "in 0", "aux 0 out")
/* modulator */
bus_config("MIX", "in 1", "aux 1 out")
bus_config("VOCODE2", "aux 0-1 in", "out 0-1")


dur = 10.0
amp = 10000
pitch = 7.00
makegen(2, 10, 15000,
   1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9, 1/10, 1/11, 1/12,
   1/13, 1/14, 1/15, 1/16, 1/18, 1/19, 1/20, 1/21, 1/22, 1/23, 1/24)  // saw
reset(10000)
WAVETABLE(0, dur, amp, pitch)
WAVETABLE(0, dur, amp, pitch+.0005)

inskip = 0
amp = 1
dur = 10
MIX(0, inskip, dur, amp, 0)


inskip = 0
dur = 10
amp = 1
MIX(0, inskip, dur, amp, 0)

amp = 1.0
numfilt = 0
makegen(2, 2, numpitches = 8,
   7.00,
   7.07,
   8.02,
   8.09,
   9.04,
   9.11,
  10.06,
  11.01
)
transp = 0.07
freqmult = 2.02
cartransp = -0.02
bw = 0.008
resp = 0.00
hipass = 0.2
hpcf = 5000
noise = 0.01
noisubsamp = 5

dur = dur + 2
setline(0,1, dur-1,1, dur,0)
VOCODE2(0, 0, dur, amp, numfilt, transp, freqmult, cartransp, bw,
   resp, hipass, hpcf, noise, noisubsamp, 1)
transp = transp + 0.002
VOCODE2(0, 0, dur, amp, numfilt, transp, freqmult, cartransp, bw,
   resp, hipass, hpcf, noise, noisubsamp, 0)


