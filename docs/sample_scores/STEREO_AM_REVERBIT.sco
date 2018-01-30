rtsetparams(44100, 2)

rtinput("../../snd/huhh.wav")
totdur = 20

// --------------------------------------------------------------------------
load("STEREO")
bus_config("STEREO", "in 0", "aux 0-1 out")

minskip = 1.0
maxskip = 2.3
notedur = 0.07
increment = 0.17
start_smear = 0.10
mindb = -24
maxdb = -3

env = maketable("curve", 1000, 0,0,0, 1,1,-6, 10,0)
reset(10000)

// low-weighted linear random number table, for duration smear
mindur = notedur * 0.5
maxdur = notedur * 2.0
smeartable = maketable("random", "nonorm", 100, "low", mindur, maxdur, seed=1)

// seed random number generator for calls to irand and random in loop
srand(2)

for (st = 0; st < totdur; st = st + increment) {
   start = st + irand(0, increment * start_smear)
   inskip = minskip + ((maxskip - minskip) * random())
   dur = notedur + samptable(smeartable, random() * 100)
   db = mindb + ((maxdb - mindb) * random())
   pan = random()
   STEREO(start, inskip, dur, ampdb(db) * env, pan)
}

// --------------------------------------------------------------------------
load("AM")
bus_config("AM", "aux 0-1 in", "aux 2-3 out")

// bipolar sine wave - for ring mod
wavetable = maketable("wave", 2000, "sine")

amp = 6

rfreq = 4
minfreq = 400
maxfreq = 1200
seed = 1
freq = makerandom("even", rfreq, minfreq, maxfreq, seed)

AM(0, 0, totdur, amp, freq, 0, 1, wavetable)

minfreq = 400
maxfreq = 1200
seed = 2
freq = makerandom("even", rfreq, minfreq, maxfreq, seed)

AM(0, 0, totdur, amp, freq, 1, 0, wavetable)

// --------------------------------------------------------------------------
load("REVERBIT")
bus_config("REVERBIT", "aux 2-3 in", "out 0-1")

REVERBIT(0, 0, totdur, amp=1, revtime=.1, wet=.3, rtchandel=.02)

