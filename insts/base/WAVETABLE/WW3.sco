load("WAVETABLE")
rtsetparams(22050, 2)
print_off()
makegen(1, 10, 1000, 1, 0.3, 0.2)
makegen(2, 24, 1000, 0, 1,  950, 0)

srand(0.35)

for (start = 0; start < 7; start = start + 0.14) {
freq = random() * 200 + 35
	for (i = 0; i < 3; i = i+1) {
		WAVETABLE(start, 0.4, 1500, freq, 0)
		WAVETABLE(start+random()*0.1, 0.4, 1500, freq+(random()*7), 1)
		if (start > 3.5) {
			makegen(1, 10, 1000, 1, random(), random(),random(),random(),random(),random(),random(),random(),random(),random(),random())
			}
		freq = freq + 125
		}
}
