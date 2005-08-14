// Translation of STRUM3.sco  -JGG

rtsetparams(44100, 2)
load("STRUM2")

pitchtab = { 7.00, 7.02, 7.05, 7.07, 7.10, 8.00, 8.07 }
pitchtablen = len(pitchtab)

gliss = maketable("literal", "nonorm", "nointerp", 0,
	0, octpch(0.07), octpch(0.04), octpch(0.02), 0)
//gliss = makefilter(gliss, "smooth", 25)

amp = maketable("line", "nonorm", 1000, 0,0, 1,10000, 40,10000, 45,0)

squish = 1

srand(1)

totdur = 15
dur = 0.2

for (st = 0; st < totdur; st += dur) {
	pind = random() * pitchtablen
	pitch = pitchtab[trunc(pind)]
	pitch = octpch(pitch)
	freq = makeconverter(pitch + gliss, "cpsoct")
	pan = random()
	STRUM2(st, dur, amp, freq, squish, 1, pan)
}

