rtsetparams(44100, 2)
print_off()
load("ELL")

rtinput("impulse.snd")

outskip = 0
inskip = 0
dur = DUR()
ringdur = 3

p0 = 500
p1 = 2000
p2 = 5000
ripple = 45.0
attenuation = 90.0

srand(29467)
amp = 6.0

numbeats = 10
bpm = 100
notesperbeat = 4

tempo(0, bpm * notesperbeat)
numnotes = numbeats * notesperbeat

for (i = 0; i < numnotes; i = i + 1) {
   p0 = p0 + (rand() * 20)
   p1 = p1 + (rand() * 20)
   p2 = p2 + (rand() * 20)
   ripple = ripple + (rand() * 5)
   ellset(p0, p1, p2, ripple, attenuation)
   spread = random()
   outskip = tb(i)
   ELL(outskip, inskip, dur, amp * random(), ringdur, inchan=0, spread)
}

