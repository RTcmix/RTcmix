rtsetparams(44100, 2)
load("VOCODE2")

rtinput("../../snd/nucular.wav")

// carrier
bus_config("MIX", "in 0", "aux 0 out")
inskip = 0
amp = 1
dur = DUR() - inskip
MIX(0, inskip, dur, amp, 0)

// modulator
bus_config("MIX", "in 0", "aux 1 out")
inskip = 0
dur = DUR() - inskip
amp = 1
MIX(0, inskip, dur, amp, 0)


// --------------------------------------------------------------------------
bus_config("VOCODE2", "aux 0-1 in", "out 0-1")

maxamp = 4.0
amp = maketable("line", "nonorm", 1000, 0,maxamp, dur-1,maxamp, dur,0)

numfilt = 0 // flag indicating that we're using cftab instead of interval stack

cftab = maketable("literal", "nonorm", 0,
   7.00,
   7.07,
   8.02,
   8.09,
   9.04,
   9.11,
  10.06,
  11.01
)
numpitches = tablelen(cftab)

transp = 0.07
freqmult = 2.02
cartransp = -0.02
bw = 0.008
resp = 0.0001
hipass = 0.1
hpcf = 5000
noise = 0.01
noisubsamp = 4

dur += 2
VOCODE2(0, 0, dur, amp, numfilt, transp, freqmult, cartransp, bw,
   resp, hipass, hpcf, noise, noisubsamp, pan=1, cftab)
transp = transp + 0.002
VOCODE2(0, 0, dur, amp, numfilt, transp, freqmult, cartransp, bw,
   resp, hipass, hpcf, noise, noisubsamp, pan=0, cftab)

