rtsetparams(44100, 2)
load("./libMYSYNTH.so")

env = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0)

for (st = 0; st < 8; st = st + .10) {
   dur = .03 + (.06 * random())
   amp = 4000 + (rand() * 3000)
   MYSYNTH(st, dur, amp * env, pan=random())
}

