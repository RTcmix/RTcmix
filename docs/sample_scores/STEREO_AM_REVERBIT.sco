rtsetparams(44100, 2)

// ftp://presto.music.virginia.edu/pub/rtcmix/snd/ah.snd
rtinput("/snd/Public_Sounds/vccm_old/ah.snd")
totdur = 30

/* -------------------------------------------------------------------------- */
load("STEREO")
bus_config("STEREO", "in 0", "aux 0-1 out")

minskip = 1.0; maxskip = 2.3
notedur = .07
increment = .17
start_smear = .10
mindb = -24; maxdb = 0

makegen(1, 4, 1000, 0,0,0, 1,1,-4, 10,0)
reset(10000)

// low-weighted linear random number table, for duration smear
makegen(3, 20, 100, 1, 1, notedur * .5, notedur * 4)
srand(4)

for (st = 0; st < totdur; st = st + increment) {
   start = st + irand(0, increment * start_smear)
   inskip = minskip + ((maxskip - minskip) * random())
   dur = notedur + sampfunci(3, random() * 100)
   db = mindb + ((maxdb - mindb) * random())
   pctleft = random()
   STEREO(start, inskip, dur, ampdb(db), pctleft)
}

/* -------------------------------------------------------------------------- */
load("AM")
bus_config("AM", "aux 0-1 in", "aux 2-3 out")

setline(0,1,1,1)

makegen(2, 10, 2000, 1)    // bipolar sine wave - for ring mod

amp = 12

freq = 780
AM(0, 0, totdur, amp, freq, 0, 1)
freq = freq * 1.2
AM(0, 0, totdur, amp, freq, 1, 0)

/* -------------------------------------------------------------------------- */
load("REVERBIT")
bus_config("REVERBIT", "aux 2-3 in", "out 0-1")
reset(10)

REVERBIT(0, 0, totdur, amp=1, revtime=.1, wet=.22, rcd=.01)

