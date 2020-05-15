rtsetparams(44100, 2)
load("WAVETABLE")

float play(float start, float durmult, float freqmult, float ampmult, handle env)
{
	amp = ampmult * env
	WAVETABLE(start, durmult * 15,  amp * 6,  freqmult * 35,   pan=1)
	WAVETABLE(start, durmult * 20,  amp * 20, freqmult * 82,   pan=0)
	WAVETABLE(start, durmult * 17,  amp * 15, freqmult * 82.4, pan=.9)
	WAVETABLE(start, durmult * 20,  amp * 20, freqmult * 165,  pan=.1)
	WAVETABLE(start, durmult * 15,  amp * 30, freqmult * 200,  pan=.8)
	WAVETABLE(start, durmult * 6,   amp * 20, freqmult * 342,  pan=.2)
	WAVETABLE(start, durmult * 5,   amp * 15, freqmult * 425,  pan=.7)
	WAVETABLE(start, durmult * 7,   amp * 20, freqmult * 500,  pan=.3)
	WAVETABLE(start, durmult * 4,   amp * 5,  freqmult * 895,  pan=.6)
	WAVETABLE(start, durmult * 2,   amp * 4,  freqmult * 1303, pan=.4)
	WAVETABLE(start, durmult * 1,   amp * 5,  freqmult * 1501, pan=.5)
	WAVETABLE(start, durmult * 4,   amp * 6,  freqmult * 1700, pan=.5)
	WAVETABLE(start, durmult * 1.5, amp * 4,  freqmult * 2200, pan=.5)
	return 0
}


slowenv = maketable("curve", 1000,  0,0,5, 3,1,-5, 10,0)
expenv = maketable("expbrk", 1000,  1, 1000, .0000000002)

play(0, durf=3, freqf=3, ampf=400, expenv)
play(2, durf=8, freqf=2.98, ampf=200, slowenv)
play(4, durf=6, freqf=1.2, ampf=300, slowenv)
play(5, durf=9, freqf=2.1, ampf=250, slowenv)
