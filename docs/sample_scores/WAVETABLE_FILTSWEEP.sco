rtsetparams(44100, 2)
load("WAVETABLE")
load("FILTSWEEP")

bus_config("WAVETABLE", "aux 0 out")
bus_config("FILTSWEEP", "aux 0 in", "out 0-1")

//----------------------------------------------------------------------------
maxamp = 6000
amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 9,maxamp, 10,0)
wavet = maketable("wave", 10000, "buzz21")

start = 0
totdur = 10
dur = 0.06
incr = 0.18

for (st = start; st < totdur; st += incr) {
   WAVETABLE(st, dur, amp, 6.00, pan=0, wavet)
   WAVETABLE(st, dur, amp, 7.00, pan=0, wavet)
   WAVETABLE(st, dur, amp, 7.07, pan=0, wavet)
}

//----------------------------------------------------------------------------
amp = maketable("line", 1000, 0,0, 1,1, 4,1, 5,0)
balance = 0
sharpness = 2.2
ringdur = .2
bypass = false

pan = makerandom("even", rfreq=3.5, min=0, max=1, seed=2)
pan = makefilter(pan, "smooth", lag=70)  // smooth the abrupt random changes

// This adjusts the amplitude to compensate for loss of power when panned
// to the center.
boost = makeconverter(pan, "boost")
amp = amp * boost

lowcf = 300
highcf = 4000
cf = maketable("line", "nonorm", 2000,
			0,lowcf, 1,highcf, 2,lowcf, 2.2,lowcf, 3,highcf, 4,lowcf)
bw = -0.17	// NB: negative means percentage of cf

reset(2000)

FILTSWEEP(start, inskip=0, totdur, amp, ringdur, sharpness, balance,
	inchan=0, pan, bypass, cf, bw)

