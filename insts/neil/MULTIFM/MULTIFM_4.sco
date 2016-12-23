rtsetparams(44100,2)
load("./libMULTIFM.so")
control_rate(20000)
srand(0.19)

wavet1 = maketable("wave", 1000, "sine")
wavet2 = maketable("wave", 1000, "tri9")

freqenv = maketable("line", 1000, 0,0.7, 1,1)

notes = { 5.04, 5.07, 5.08, 5.10, 6.02 }
inc = 0.5
i = 0
for (start = 0; start < 10; start += inc) {
	ampenv = maketable("line", 1000, 0,0, 1,1, irand(10,50),0)
	freq = cpspch(notes[i % len(notes)])
	if (irand(100) < 50) {
		ind2env = maketable("line", "nonorm", 1000, 0,irand(0.2,2), 1,irand(3,10))
	}
	else {
		ind2env = irand(0.2, 5)
	}
	MULTIFM(start, 0.5, 20000 * ampenv, 4, 0.5,
		freq, wavet1,
		freq * irand(0.99, 1.01), wavet2,
		freq * 32, wavet1,
		freq * 60, wavet1,
		1, 0, 1,
		1, 1, 1,
		2, 1, ind2env,
		3, 2, 0.5,
		4, 1, 0.05
	)
	if (irand(100) < 50)
		inc = 0.25
	else
		inc = 0.5
	i += 1
}
