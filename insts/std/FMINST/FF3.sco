load("FMINST")
rtsetparams(44100, 1)
print_off()
makegen(1, 7, 1000, 0, 500, 1, 500, 0)
makegen(2, 10, 1000, 1)
makegen(3, 24, 1000, 0,1, 2,0)

nfms = 1
for (start = 0; start < 60; start = start + 1.5) {
	freq = 8.00
	for (n = 0; n < nfms; n = n + 1) {
		FMINST(start, 1.5, 1000, freq, 179, 0, 10)
		freq = freq + 0.02
		}
	nfms = nfms + 1
}
