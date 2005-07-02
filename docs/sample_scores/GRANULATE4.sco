rtsetparams(44100, 2, 512)
load("GRANULATE")

dur = 360
amp = ampdb(-12)

fname = "../../snd/huhh.wav"
intab = maketable("soundfile", "nonorm", 0, fname)
filedur = 2.32
numchans = 1
inchan = 0

inskip = 0.5
winstart = 0.2
winend = filedur - 0.2
wrap = 1

travrate = makeconnection("mouse", "x", min=-2, max=2, dflt=0, lag=30, "rate")

envtab = maketable("window", 1000, "hanning")

injitter = 0.0
outjitter = 0.001
hoptime = 0.006
mindur = hoptime * 15
maxdur = mindur
minamp = maxamp = 1.0

transp = makeconnection("mouse", "y", min=-2, max=2, dflt=0, lag=20,
                                                         "transp", "oct")
// constrain to semitones
transp = makefilter(transp, "quantize", 1 / 12)

transpcoll = 0
transpjitter = 0.0

seed = 1
minpan = 0.2
maxpan = 0.8

bus_config("GRANULATE", "out 0")
GRANULATE(0, inskip, dur, amp, intab, numchans, inchan, winstart, winend,
   wrap, travrate, envtab, hoptime, injitter, outjitter, mindur, maxdur,
   minamp, maxamp, transp, transpcoll, transpjitter, seed)
seed += 1
bus_config("GRANULATE", "out 1")
GRANULATE(0, inskip, dur, amp, intab, numchans, inchan, winstart, winend,
   wrap, travrate, envtab, hoptime, injitter, outjitter, mindur, maxdur,
   minamp, maxamp, transp, transpcoll, transpjitter, seed)

