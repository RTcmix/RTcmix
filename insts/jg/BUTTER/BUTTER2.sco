// This is merely to test the new optional pfields 12 (ringdur) & 13 (outamp)

rtsetparams(44100, 2)
load("NOISE")
load("BUTTER")
conrol_rate(44100)

bus_config("NOISE", "aux 0 out")
bus_config("BUTTER", "aux 0 in", "out 0-1")

cf = 440
bw = -0.001

indur = 0.5
ringdur = 8.0
outdur = indur + ringdur

inenv = maketable("window", 2000, "hanning")
outenv = maketable("curve", 2000, 0,1,0, indur,1,-8, outdur,0) * 200

NOISE(0, indur, amp=30000)
BUTTER(0, 0, indur, inenv, "bandpass", steep=2, bal=0, inch=0, pan=0.5, 
	bypass=0, cf, bw, ringdur, outenv)
