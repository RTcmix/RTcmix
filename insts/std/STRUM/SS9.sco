/* p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
*/

rtsetparams(44100, 2)
makegen(1, 2, 7, 0)
7.00 7.02 7.05 7.07 7.10 8.00 8.07

semitone = 1.0/12.0;

pitch = 7.00;

srand(0.314)
for (st = 0; st < 50; st = st + 0.08) {
	START(st, 1.0, pitch, 1.0, 0.1, 10000.0, 1, random())
	dev = trunc((rand() * 2.1));
	print(dev);
	dev = dev * semitone;
	opitch = octpch(pitch);
	pitch = pchoct(opitch + dev);
	if (pitch > 11)
	    pitch = pitch - 3
	if (pitch < 5)
	    pitch = pitch + 3
}
