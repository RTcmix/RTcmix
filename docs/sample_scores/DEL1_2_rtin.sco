// see DEL11.sco for usage comments

set_option("record=on")  // must do this before rtsetparams
rtsetparams(44100, 2, 512)
load("DEL1")
rtinput("AUDIO", "MIC")

dur = 17
amp = maketable("line", "nonorm", 1000, 0,0, 1,1, 16,1, 17,0)
rtchandel = 4.3
rtchanamp = 1

DEL1(0, 0, dur, amp, rtchandel, rtchanamp)

