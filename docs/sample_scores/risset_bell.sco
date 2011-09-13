// Risset bell, after Dodge 2nd ed, p 105  -JG
rtsetparams(44100, 2)
load("WAVETABLE")

// exponential amplitude envelope ("F2" in Dodge)
env = maketable("expbrk", 1000,  1, 1000, .0000000002)

// just a sine wave (try extra harmonics for more complex bell sound)
wavet = maketable("wave", 5000, "sine")

durscale = 3
notes = {	// these are mine, not Risset's or Dodge's
//   start dur  amp    freq
	{  0,    5,  4000,  1000 },
	{  3,    8,  5000,  1200 },
	{  6,   12,  3000,   800 },
	{ 10,    8,  2000,   914 },
	{ 10.2,  8,  1000,   576 },
	{ 10.6,  8,  2000,   709 },
	{ 11.2,  8,  2000,   934 },
	{ 12,    8,  4000,  1348 },
	{ 13,    8,  2000,   572 },
	{ 14.2,  8,  3000,  1102 },
	{ 15.6,  8,  1500,   626 },
	{ 17.2,  8,  2500,   895 },
	{ 19,    8,  3500,   525 },
	{ 21.5, 15,  5500,   220 }
}
numnotes = len(notes)

for (i = 0; i < numnotes; i += 1) {
	note = notes[i]
	start = note[0]
	dur = note[1] * durscale
	amp = note[2] * env
	freq = note[3]

	// these factors straight outta Dodge; I panned everything
   WAVETABLE(start, dur, amp, freq * .56, pan=1, wavet)
   WAVETABLE(start, dur * .9, amp * .67, freq * .56 + 1, pan=0, wavet)
   WAVETABLE(start, dur * .65, amp, freq * .92, pan=.9, wavet)
   WAVETABLE(start, dur * .55, amp * 1.8, freq * .92 + 1.7, pan=.1, wavet)
   WAVETABLE(start, dur * .325, amp * 2.67, freq * 1.19, pan=.8, wavet)
   WAVETABLE(start, dur * .35, amp * 1.67, freq * 1.7, pan=.2, wavet)
   WAVETABLE(start, dur * .25, amp * 1.46, freq * 2, pan=.7, wavet)
   WAVETABLE(start, dur * .2, amp * 1.33, freq * 2.74, pan=.3, wavet)
   WAVETABLE(start, dur * .15, amp * 1.33, freq * 3, pan=.6, wavet)
   WAVETABLE(start, dur * .1, amp, freq * 3.76, pan=.4, wavet)
   WAVETABLE(start, dur * .075, amp * 1.33, freq * 4.07, pan=.5, wavet)
}

