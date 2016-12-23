rtsetparams(44100, 2)
load("./libHENON.so")
srand()

for (start = 0; start < 15; start += 0.75) {
	a = irand(1.2, 1.35)
	b = irand(0.25, 0.35)
	x = irand(0.5, 1.5)
	y = irand(0.5, 1.5)
	HENON(start, 0.75, 10000, a, b, x, y)
}

