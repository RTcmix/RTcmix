rtsetparams(44100, 2)
load("WAVETABLE")
load("BUTTER")

// feed wavetable into filter
bus_config("WAVETABLE", "aux 0 out")
bus_config("BUTTER", "aux 0 in", "out 0-1")

dur = 10.0
amp = 10000
pitch = 7.00
wavet = maketable("wave", 15000, "saw24")
WAVETABLE(0, dur, amp, pitch, 0, wavet)
WAVETABLE(0, dur, amp, pitch+.0005, 0, wavet)

type = "highpass"
type = "bandpass"
type = "bandreject"
type = "lowpass"

amp = 1.5
steepness = 5
bypass = false

if (type == "lowpass") {
   balance = 0
   lowcf = 500
   highcf = 5000
   bw = 0
}
else if (type == "highpass") {
   balance = 1
   amp *= 0.4
   lowcf = 1
   highcf = 1400
   bw = 0
}
else if (type == "bandpass") {
   balance = 0
   amp *= 2
   lowcf = 1
   highcf = 2400
   bw = 200
}
else if (type == "bandreject") {
   balance = 0
   amp *= 0.5
   lowcf = 500
   highcf = 4000
   bw = 300
}

env = maketable("line", 1000, 0,0, 1,1, 7,1, 10,0)
cf = maketable("line", "nonorm", 2000,
   0,lowcf, dur*.2,lowcf, dur*.5,highcf, dur*.9,lowcf, dur,lowcf)

BUTTER(0, 0, dur, amp * env, type, steepness, balance, inchan=0, pan=0.5,
	bypass, cf, bw)

