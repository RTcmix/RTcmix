set_option("record = on")
rtsetparams(44100, 2, 256)
load("MULTICOMB")
rtinput("AUDIO")

control_rate(10000)

srand(87)

amp = 0.01
env = maketable("line", 1000, 0,0, 1,1, 3,1, 5,0)

dur = 5
rvbtime = 2.5

for (start = 0; start < 30; start += 2.5) {
	low = (random() * 500.0) + 50.0
	hi = low + (random() * 300.0)
	MULTICOMB(start, 0, dur, amp * env, low, hi, rvbtime)
}

