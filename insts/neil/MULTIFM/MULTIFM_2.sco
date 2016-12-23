rtsetparams(44100, 2)
load("./libMULTIFM.so")
control_rate(20000)

ampenv = maketable("curve", 1000, 0,0,3, 1,1,-5, 500,0)

wavet1 = maketable("wave", 1000, "sine")
wavet2 = maketable("curve", 500, 0,0,-5, 1,1,5, 2,0,-5, 3,-1,5, 4,0,-5, 8,0)
wavet2 = makefilter(wavet2, "smooth", 100)

notes = { 7.00, 7.02, 7.04, 7.07, 7.096 }
fbenv = maketable("line", 1000, 0,0, 1,1)
i = 0
for (start = 0; start < 15; start += 0.2) {
	index = trand(len(notes))
	freq = cpspch(notes[index])
	MULTIFM(start, 1, irand(2000, 20000) * ampenv, 4, irand(0.3, 0.8),
		freq, wavet1,
		freq * 2.01, wavet2,
		freq * 3.7, wavet1,
		freq * 5, wavet1,
		4, 3, 1,
		4, 2, 0.6,
		3, 1, irand(0.3, 0.8),
		2, 1, 0.8,
		1, 0, 0.8
	)
	i += 1
}

ind2env = maketable("line", "nonorm", 1000, 0,0.5, 1,1.5, 2,0.3)

freq = maketable("linestep", "nonorm", 1000, 0,9.00, 1,9.02, 2,9.06, 3,9.04, 4,9.07, 7,9.096, 8,9.07, 11,9.07)
freq = makeconverter(freq, "cpspch")
freq = makefilter(freq,"smooth",70,cpspch(9.00))

ampenv = maketable("line", 1000, 0,0, 0.4,1, 4,1, 5,0)

MULTIFM(2, 12, 6000 * ampenv, 4, 0.4,
	freq, wavet1,
	freq * 2.333, wavet1,
	freq * 4.01, wavet1,
	freq, wavet1,
	4, 3, 0.12,
	3, 1, 0.5,
	2, 1, ind2env,
	1, 0, 1
)

