rtsetparams(44100, 1);
rtinput("../../../snd/conga.snd");

float count;

while (count < 5) {
	outskip = count * 1.2;
	MIX(outskip, 0, DUR(), 1, 0);
	if (count == 6) {
		++count;
	}
	count += 1;
}

print("last line getting executed");

