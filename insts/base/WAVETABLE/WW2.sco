load("WAVETABLE")
rtsetparams(44100,2)
/* uncomment the following line to get rid of all the screen output */
/* print_off() */
makegen(1, 24, 1000, 0,0, 0.01,1, 0.1,0.2, 0.4, 0)
makegen(2, 10, 1000, 1, 0.3, 0.2)

reset(20000)
start = 0.0
freq = 149.0

for (i = 0; i < 100; i = i+1) {
	WAVETABLE(start, 0.4, 5000, freq, 0)
	WAVETABLE(start+random()*0.07, 0.4, 5000, freq+random()*2, 1)
	start = start + 0.1
	makegen(2, 10, 1000, 1, random(), random(), random(), random(), random(), random())
	}
