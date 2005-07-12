set_option("record = on")  // must do this before rtsetparams
rtsetparams(44100, 2, 256)
load("REVERBIT")

rtinput("AUDIO")

bus_config("REVERBIT", "in 0", "out 0-1")

dur = 60
amp = 0.9
revtime = 0.4
revpct = 0.8
rtchandel = 0.02
cf = 1000

env = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

REVERBIT(st=0, insk=0, dur, amp * env, revtime, revpct, rtchandel, cf)

