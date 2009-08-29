/* START:
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
   p8 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

   FRET:
   p0 = start; p1 = dur; p2 = pitch(oct.pc); p3 = fundamental decay time;
   p4 = nyquist decay time; p5 = stereo spread [optional]
*/

rtsetparams(44100, 2)
load("STRUM")
makegen(2, 2, 7, 7.00, 7.02, 7.05, 7.07, 7.10, 8.00, 8.07)

srand(0)
for (st = 0; st < 15; st = st + 0.2) {
	pind = random() * 7
	pitch = sampfunc(2, pind)
	stereo = random()
	START(st, 0.05, pitch, 1.0, 0.1, 10000, 1,  stereo)
	FRET(st+0.05, 0.05, pitch+0.07, 1.0, 0.1, stereo)
	FRET(st+0.1, 0.05, pitch+0.04, 1.0, 0.1, stereo)
	FRET(st+0.15, 0.05, pitch+0.02, 1.0, 0.1, stereo)
}
