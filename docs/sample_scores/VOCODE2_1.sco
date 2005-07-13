load("VOCODE2")
load("WAVETABLE")

writeit = 0  // set this to 1 to write a file, in case CPU maxes out

if (writeit) {
	set_option("play = off", "clobber = on")
	rtoutput("/tmp/VOCODE2_1.wav")
}
else
	rtsetparams(44100, 2)

// modulator
bus_config("MIX", "in 0", "aux 1 out")
rtinput("../../snd/nucular.wav")
inskip = 0
dur = DUR() - inskip
amp = 1
MIX(0, inskip, dur, amp, 0)

// carrier
bus_config("WAVETABLE", "in 0", "aux 0 out")
amp = 5000
wavet = maketable("wave", 10000, "buzz20")
pitchtab = { 8.00, 8.02, 8.05, 8.07, 8.08, 8.10, 9.00 }
numpitches = len(pitchtab)
transp = octpch(0.00)
for (i = 0; i < numpitches; i += 1) {
	freq = cpsoct(octpch(pitchtab[i]) + transp)
	WAVETABLE(0, dur, amp, freq, 0, wavet)
}


// --------------------------------------------------------------------------
bus_config("VOCODE2", "aux 0-1 in", "out 0-1")

dur += 5
maxamp = 35.0
amp = maketable("line", "nonorm", 1000, 0,0, 0.1,maxamp, dur-2,maxamp, dur,0)

numfilt = 22
lowcf = 8.07
interval = 0.025	// oct.pc
spacemult = cpspch(interval) / cpspch(0)

cartransp = 0.00
bw = 0.0002
resp = 0.02
hipass = 0.00
hpcf = 3000
noise = 0.2
noisubsamp = 8

VOCODE2(0, 0, dur, amp, numfilt, lowcf, spacemult, cartransp, bw,
   resp, hipass, hpcf, noise, noisubsamp, pan=1)

spacemult += 0.008	// make right channel sound different

VOCODE2(0, 0, dur, amp, numfilt, lowcf, spacemult, cartransp, bw,
   resp, hipass, hpcf, noise, noisubsamp, pan=0)

