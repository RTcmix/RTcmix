load("FMINST")
rtsetparams(44100, 2)
/* print_off() */
makegen(1, 7, 1000, 0, 500, 1, 500, 0)
makegen(2, 10, 1000, 1)
makegen(3, 24, 1000, 0,1, 2,0)

freq = 8.00
for (start = 0; start < 60; start = start + 0.1) {
	FMINST(start, .5, 1000, freq, 179, 0, 10, random())
	freq = freq + 0.002
}
