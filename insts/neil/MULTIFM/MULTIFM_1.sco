rtsetparams(44100,2)
load("./libMULTIFM.so")
tempo(0, 120)
control_rate(20000)

// an example of double modulator FM

ampenv = maketable("line", 1000, 0,0, 1,1, 100,0)

wavet1 = maketable("wave", 1000, "tri5")
wavet2 = maketable("wave", 1000, "sine")

freq = 400
for (start = 0; start < 10; start += 0.5) {
	MULTIFM(tb(start), 0.5, 10000 * ampenv, 3, 0.5,
		freq, wavet1,                         // oscil 1
		freq * (10 - start + 3.333), wavet2,  // oscil 2
		freq * (start + 3.001), wavet2,       // oscil 3
		1, 0, 1,     // oscil 1 to audio out (0) with amp 1
		2, 1, 1,     // oscil 2 modulates oscil 1 with index 1
		3, 1, 1)     // oscil 3 modulates oscil 1 with index 1
}
