set_option("record=on")
rtsetparams(44100, 2, 1024)
load("COMBIT")
rtinput("AUDIO")

dur = 3.5
amp = 0.2

COMBIT(start=0, 0, dur, amp, freq=cpspch(7.09), rvbtime=.5, 0, pan=0)
COMBIT(start=0.2, 0, dur, amp, freq=cpspch(7.07), rvbtime=.5, 0, pan=1)

