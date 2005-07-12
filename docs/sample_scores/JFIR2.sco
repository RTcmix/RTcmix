print_off()
rtsetparams(44100, 2)
load("JFIR")

rtinput("../../snd/loocher.aiff")
inchan = 0
inskip = 0
filedur = DUR()

reset(4000)
env = maketable("line", 12000, 0,0, 1,1, 29,1, 30,0)

amp = 4.0
dur = 0.05
pan = 0.5
bypass = 0

cflist = { 500, 2000, 1000, 750, 3000, 180, 1700, 5000, 1400, 450, 900, 2200 }
numcf = len(cflist)

half_bandwidth_percent = 0.50

nyquist = 44100 / 2
order = 300


for (st = 0; st < filedur - dur; st += dur) {
	n = (st / filedur) * (numcf - 1)
	cf = cflist[trunc(n)]
	low = cf - (cf * half_bandwidth_percent)
	high = cf + (cf * half_bandwidth_percent)
	freqresp = maketable("line", 5000, 0,0, low,0, cf,1, high,0, nyquist,0)
	JFIR(st, inskip, dur, amp * env, order, inchan, pan, bypass, freqresp)
	inskip += dur
}


