rtsetparams(44100, 2)
load("STRUM2")
load("JDELAY")

bus_config("STRUM2", "aux 0-1 out")
bus_config("JDELAY", "aux 0-1 in", "out 0-1")

totdur = 30

//----------------------------------------------------------------------------
env = maketable("line", 1000, 0,0, 1,1, 10,1, 50,0)
pitches = {6.119, 7.00, 7.02, 7.05, 7.07, 7.071, 7.08, 7.10,
	8.00, 8.02, 8.07, 8.072}
numpitches = len(pitches)

transpose = octpch(0.02)

srand(3)

notedur = 0.12
incr = 0.116

maxampdb = 92
minampdb = 65

maxsquish = 4
minsquish = 1

decay = 0.5

smearpct =  0.0012
dursmearpct =  0.2

for (start = 0; start < totdur; start += incr) {
	index = trand(numpitches)
	pitch = pitches[index]
	pitch = pchoct(octpch(pitch) + transpose)

	amp = ampdb(irand(minampdb, maxampdb))
	squish = irand(minsquish, maxsquish)
	smear = start * smearpct * random()
	dursmear = notedur * dursmearpct * random()

	STRUM2(start + smear, notedur + dursmear, amp * env, pitch,
		squish, decay, random())
}

//----------------------------------------------------------------------------
amp = 2.0
pitch = octpch(6.00)
deltime = 1 / cpsoct(pitch + transpose)
regen = 0.98
wetdry = 0.10
cutoff = 2000
ringdur = 5.0

atk = 4; dcy = 2
env = maketable("line", 1000, 0,0, atk,1, totdur-dcy,1, totdur,0)

JDELAY(st=0, insk=0, totdur, amp * env, deltime, regen, ringdur, cutoff,
			wetdry, inchan=0, pctleft=1)
JDELAY(st=0.02, insk=0, totdur, amp * env, deltime, regen, ringdur, cutoff,
			wetdry, inchan=1, pctleft=0)


// JGG, 5/28/00, rev. 12/6/05
