set_option("record = on")
rtsetparams(44100, 2, 512)
load("DELAY")
rtinput("AUDIO")

DELAY(start=0, 0, dur=14, amp=1, deltime=.078, fdbk=0.8, ring=3.5, 0, pan=0.1)
DELAY(start=7, 0, dur=10, amp=1, deltime=.415, fdbk=0.5, ring=3, 0, pan=0.9)

