rtsetparams(22050, 2)
load("FMINST")
load("WAVETABLE")
reset(2000)
print_off()

makegen(1, 7, 1000, 0, 500, 1, 500, 0)
makegen(2, 10, 1000, 1)
makegen(3, 24, 1000, 0,1, 2,0)

freq = 8.00
for (start = 0; start < 20; start = start + 0.5) {
	FMINST(start, .5, 3000, freq, 179, 0, 10, random())
	freq = freq + 0.002
}

makegen(1, 24, 1000, 0, 1,  950, 0)
makegen(2, 10, 2000, 1, 0.3, 0.2)

srand(0)

for (start = 0; start < 20; start = start + 0.14) {
	freq = random() * 200 + 35
	for (i = 0; i < 3; i = i+1) {
		WAVETABLE(start, 0.4, 1500, freq, 0)
		WAVETABLE(start+random()*0.1, 0.4, 1500, freq+(random()*7), 1)
		if (start > 10) {
			makegen(2, 10, 2000, 1, random(), random(), random(), random(),
						random(), random(), random(), random(),
						random(), random(), random())
		}
		freq = freq + 125
	}
}

