// Translation of STRUM1.sco  -JGG

rtsetparams(44100, 2)
load("STRUM2")

pitchtab = { 7.00, 7.02, 7.05, 7.07, 7.10, 8.00, 8.07 }
pitchtablen = len(pitchtab)

srand(4)

for (st = 0; st < 15; st += 0.1) {
	pind = trunc(random() * pitchtablen)
	pitch = pitchtab[pind]
	STRUM2(st, 1.0, 10000, pitch, 1, 1.0, random())
}

