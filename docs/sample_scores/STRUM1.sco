/* START:
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
   p8 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]
*/

rtsetparams(44100, 2)
load("STRUM")
makegen(2, 2, 7, 7.00, 7.02, 7.05, 7.07, 7.10, 8.00, 8.07)

srand(0)
for (st = 0; st < 15; st = st + 0.1) {
	pind = random() * 7
	pitch = sampfunc(2, pind)
	START(st, 1.0, pitch, 1.0, 0.1, 10000.0, 1, random())
}
