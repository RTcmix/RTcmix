rtsetparams(44100, 2)
load("VOCODE2")

rtinput("/snd/beckett/furniture1.aiff")

/* carrier */
bus_config("MIX", "in 0", "aux 0 out")
inskip = 0
amp = 1
dur = DUR() - inskip
MIX(0, inskip, dur, amp, 0)

/* modulator */
bus_config("MIX", "in 0", "aux 1 out")
inskip = 0
dur = DUR() - inskip
amp = 1
MIX(0, inskip, dur, amp, 0)


/* -------------------------------------------------------------------------- */
bus_config("VOCODE2", "aux 0-1 in", "out 0-1")
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

