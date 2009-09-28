// This Shepard Scale score is derived from one by Kaori Asada
// (original score: http://cm50.art.tamagawa.ac.jp/~takaoka/ShepardTone.sco).
// She wrote this as a student of Akira Takaoka at Tamagawa University.
// (modified by JG to loop through notes rather than spelling them all out)

rtsetparams(44100, 2)
load("WAVETABLE")

count = 8	// how many times?
notedur = 0.25
gapdur = 0.05
amp = 2000
dopan = true

// ---------------------------------------------
lowoctave = 4
hioctave = 15

notes = {
//   emphoctave emphamp
	{ 8,         2.000 },	// C
	{ 8,         1.833 },	// C#
	{ 8,         1.667 },	// D
	{ 8,         1.500 },	// D#
	{ 8,         1.333 },	// E
	{ 8,         1.167 },	// F
	{ 8,         1.000 },	// F#
	{ 7,         1.167 },	// G
	{ 7,         1.333 },	// G#
	{ 7,         1.500 },	// A
	{ 7,         1.667 },	// A#
	{ 7,         1.833 }	// B
}
numnotes = len(notes)

waveform = maketable("wave", 1000, "sine")
ampenv = maketable("line", 1000, 0,1, 1,0)

if (dopan) {
	panincr = 1 / numnotes
	pan = 0
}
else {
	panincr = 0
	pan = 0.5
}

start = 0
for (n = 0; n < count; n += 1) {
	for (i = 0; i < numnotes; i += 1) {
		pc = i * 0.01
		note = notes[i]
		emphoct = note[0]
		emphamp = note[1]
		for (j = lowoctave; j < hioctave; j += 1) {
			if (j != emphoct) {
				pitch = j + pc
				WAVETABLE(start, notedur, amp * ampenv, pitch, pan, waveform)
			}
		}
		pitch = emphoct + pc
		WAVETABLE(start, notedur, amp * emphamp * ampenv, pitch, pan, waveform)
		pan += panincr
		if (pan <= 0 || pan >= 1)
			panincr *= -1	// go back the other way
		start += notedur + gapdur
	}
}

