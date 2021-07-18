// Sonification of the 3x+1 algorithm, a number theory game
// that proceeds like this:
// pick a positive integer >= 1
// A) if it's 1, stop
// B) if it's even, divide by 2
// C) if it's odd, multiply it by 3 and add 1
// repeat A-B-C until it stops:
// eg 7->22->11->34->17->52->26->13->40->20->10->5->16->8->4->2->1
// -- (c) Joel Matthys

bus_config ("MMODALBAR", "out 0-1")
bus_config ("WAVETABLE", "aux 0-1 out")
bus_config ("COMBIT", "aux 0-1 in", "aux 2-3 out")
bus_config ("FREEVERB", "aux 2-3 in", "out 0-1")

srand()
reset(44100)

venv = maketable ("line", 1000, 0,1, 9,1, 10,0)
FREEVERB (0, 0, 120, 0.8*venv, 0.9, 0.03, 3, 70, 20, 60, 100)

env = maketable ("line",1000,0,0,1,1,2,0.8,3,0.8,8,0)
wav = maketable ("wave", 1000, "buzz")
cfreq = makeLFO ("sine", 0.009, 20,10000)
pan = makeLFO ("sine", 0.07, 0,1)
outdelay = makeLFO ("sine", 0.27, 0,4)

COMBIT (outdelay, 0, 120, 0.1*venv, cfreq, 0.5, 0, pan)

startTime = 0
count = 1
for (ii = 1; ii < 50; ii += 2)
{
	swellLen = count / 4.0
	swellPitch = cpspch(3)*count
	WAVETABLE(startTime,swellLen, 10000*env, swellPitch, 0.5, wav)

	count = 1
	startval = ii
	while (startval > 1)
	{
		count += 1
		if (mod(startval,2)==0)
			startval = startval / 2
		else
			startval = (3 * startval) + 1
		pitch = cpspch (5 + (startval-1)/400)
		while (pitch > 10000)
		{
			pitch = pitch / 2
		}
		hardness = min(count/100,1)
		MMODALBAR(startTime,0.5,20000,pitch,hardness,random(),mod(startval,2))
		startTime += 0.1
	}
	startTime += 0.3
}
