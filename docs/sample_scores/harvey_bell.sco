// Simple playback of additive synth bell sound in Jonathan Harvey's
// Mortuos Plango, Vivos Voco. Data taken from the article by Michael
// Clarke in Mary Simoni's book, p. 116.
rtsetparams(44100, 1)
load("WAVETABLE")

partials = {
	// freq, amp (dBFS)
	{ 130, -10 },
	{ 260, -2 },
	{ 310, 0 },
	{ 390, -22 },
	{ 521, -3 },
	{ 683, -18 },
	{ 697, -10 },
	{ 781, 0 },
	{ 864, -25 },
	{ 975, -30 },
	{ 1080, -18 },
	{ 1168, -30 },
	{ 1215, -25 },
	{ 1236, -20 },
	{ 1285, -30 },
	{ 1336, -27 },
	{ 1412, -4 },
	{ 1487, -28 },
	{ 1541, -28 },
	{ 1739, -28 },
	{ 1828, -30 },
	{ 2091, -26 },
	{ 2151, -10 },
	{ 2950, -24 }
}
numpartials = len(partials)

dur = 20
durincr = -0.4	// subtract from duration of successive higher partials
maxdb = 74
do_gliss = 0
srand(1)

env = maketable("expbrk", 1000,  1, 1000, .0000002)

WAVETABLE(0, dur, ampdb(maxdb) * env, freq=347)	// strike tone

for (i = 0; i < numpartials; i += 1) {
	thispartial = partials[i]
	freq = thispartial[0]
	db = thispartial[1]
	if (do_gliss) {
		maxfactor = irand(0.8, 1.3)
		gliss = maketable("line", "nonorm", 1000, 0,1, 1,maxfactor)
	}
	else
		gliss = 1
	WAVETABLE(0, dur, ampdb(maxdb + db) * env, freq * gliss)
	dur += durincr
}

