// Translation of STRUM2.sco  --JGG

rtsetparams(44100, 2)
load("STRUM2")

pitchtab = { 7.00, 7.02, 7.05, 7.07, 7.10, 8.00, 8.07 }
pitchtablen = len(pitchtab)

maxpitch = 0.02
gliss = maketable("line", "nonorm", 1000, 0,0, 0.2,octpch(maxpitch), 2,0)

srand(1)

for (st = 0; st < 15; st = st + 0.2) {
	pind = trunc(random() * pitchtablen)
	pitch = pitchtab[pind]
	pitch = octpch(pitch)
	freq = makeconverter(pitch + gliss, "cpsoct")
	stereo = random()
	STRUM2(st, 1.0, 10000, freq, 1, 1, stereo)
}

