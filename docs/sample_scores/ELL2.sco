rtsetparams(44100, 2)
load("ELL")

rtinput("../../snd/conga.snd")
inchan = 0
inskip = 0
dur = DUR()

ripple = 20
atten = 90
ringdur = .2

env = maketable("line", 1000, 0,0, .01,1, dur/2,1, dur,0)

srand(9)

for (start = 0; start < 15; start = start + .12) {
   pbcut = 400 + (rand() * 200)
   sbcut = 900 + (rand() * 200)
   ellset(pbcut, sbcut, 0, ripple, atten)
   amp = .5
   pan = random()
   st = start + (random() * .01)
   ELL(st, inskip, dur, amp * env, ringdur, inchan, pan)

   pbcut = 900 + (rand() * 200)
   sbcut = 400 + (rand() * 200)
   ellset(pbcut, sbcut, 0, ripple, atten)
   amp = .23
   pan = random()
   st = start + (random() * .01)
   ELL(st, inskip, dur, amp * env, ringdur, inchan, pan)
}

