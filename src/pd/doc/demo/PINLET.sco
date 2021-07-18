dur = 60
amp = 25000
carfreq = makeconnection("inlet",0,440);
modfreq = makeconnection("inlet",1,440);
minindex = 0
maxindex = 10

env = maketable("line", 1000, 0, 0, 3.5,1, 7,0)
wavetable = maketable("wave", 1000, "sine")
guide = makeconnection("inlet",2,0)
control_rate(44100)
FMINST(0, dur, amp * env, carfreq, modfreq, minindex, maxindex, pan=0.5, wavetable, guide)
