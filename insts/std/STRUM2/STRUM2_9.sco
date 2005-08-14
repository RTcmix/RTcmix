// Translation of STRUM9.sco  -JGG

print_off()
rtsetparams(44100, 2)
load("STRUM2")

semitone = 1.0 / 12.0

pitch = 7.00

srand(1)

for (st = 0; st < 50; st += 0.08) {
	STRUM2(st, 1.0, 10000, pitch, 1, 1, random())
	dev = trunc((rand() * 2.1));
	//print(dev);
	dev = dev * semitone;
	opitch = octpch(pitch);
	pitch = pchoct(opitch + dev);
	if (pitch > 11)
	    pitch = pitch - 3
	if (pitch < 5)
	    pitch = pitch + 3
}

