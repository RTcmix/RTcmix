/* Reverb for real-time mic input. */
set_option("full_duplex_on")
rtsetparams(44100, 2, 512)
load("REVERBIT")

rtinput("AUDIO", "MIC")

bus_config("REVERBIT", "in 0", "out 0-1")

dur = 60
amp = .9
revtime = 0.4
revpct = .8
rtchandel = .02
cf = 1000

setline(0,0, 1,1, 9,1, 10,0)

REVERBIT(st=0, insk=0, dur, amp, revtime, revpct, rtchandel, cf)

