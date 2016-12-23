rtsetparams(44100, 2)
load("./libMULTIFM.so")
load("GVERB")
bus_config("MULTIFM", "aux 0-1 out")
bus_config("GVERB", "aux 0-1 in", "out 0-1")

wavet = maketable("wave", 1000, "sine")

srand(3)

ampenv = maketable("line", "nonorm", 1000, 0,0, 1,1, 2,0)
ampenv4 = maketable("line", "nonorm", 1000, 0,4, 1,0)
ampenv6 = maketable("line", "nonorm", 1000, 0,0, 1,5, 2,0, 3,0)
amplfo5 = makeLFO("tri", 1.7, 0.5, 1)

fbenv = maketable("line", 1000, 0,0, 1,1)
start = 0
for (i = 0; i < 3; i += 1) {
	freq = irand(300, 600)
	freq2env = maketable("line", "nonorm", 1000, 0,irand(1.5), 1,irand(1.5))
	MULTIFM(start, 5, 10000 * ampenv, 6, 0.5,
		freq, wavet,
		freq * 3 * freq2env, wavet,
		freq * 1.01, wavet,
		freq * 1.999, wavet,
		freq * 5.333, wavet,
		freq * 8, wavet,

		// dx7 algorithm 15 (ish)
		6, 4, ampenv6,
		5, 4, amplfo5,
		5, (i+1)*2, 2,
		4, 3, ampenv4,
		3, 0, 1,
		2, 1, 2,
		1, 0, 0.8
	)
	start += 7
}

GVERB(0,0,20,1,200,15,0.5,1,-50,-10,0,10)
