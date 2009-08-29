/* START:
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
   p8 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

   BEND:
   p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc);
   p4 = gliss function; p5 = fundamental decay time; p6 = nyquist decay time;
   p7 = update times/sec; p8 = stereo spread [optional]
*/

rtsetparams(44100, 2)
load("STRUM")
makegen(3, 2, 7, 7.00, 7.02, 7.05, 7.07, 7.10, 8.00, 8.07)

srand(0)
makegen(2, 24, 1000, 0, 0, 0.2, 1, 2, 0)
for (st = 0; st < 15; st = st + 0.2) {
	pind = random() * 7
	pitch = sampfunc(3, pind)
	stereo = random()
	START(st, 0.0, pitch, 1.0, 0.1, 10000.0, 1, stereo)
	BEND(st+0, 1.0, pitch, pitch+0.02, 2, 1.0, 0.1, 100, stereo)
}
