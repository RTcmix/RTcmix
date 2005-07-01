rtsetparams(44100, 2, 512)
load("GRANULATE")

dur = 60
amp = ampdb(-9)
env = maketable("curve", 1000, 0,0,1, 2,1,0, 7,1,-1, 10,0)

fname = "../../snd/nucular.wav"
intab = maketable("soundfile", "nonorm", 0, fname)
filedur = 1.578957
numchans = 1
inchan = 0

inskip = 0.45
winstart = 0.1
winend = filedur - 0.1
wrap = 1

travrate = 0.01

envtab = maketable("window", 1000, "hanning")

injitter = 0.0
outjitter = 0.001
hoptime = 0.006
mindur = hoptime * 22
maxdur = mindur
minamp = maxamp = 1.0
transp = -0.07
transpcoll = 0
transpjitter = 0.0
seed = 1
minpan = 0.3
maxpan = 0.7

bus_config("GRANULATE", "out 0")
GRANULATE(0, inskip, dur, amp * env, intab, numchans, inchan, winstart, winend,
   wrap, travrate, envtab, hoptime, injitter, outjitter, mindur, maxdur,
   minamp, maxamp, transp, transpcoll, transpjitter, seed)
seed += 1
bus_config("GRANULATE", "out 1")
GRANULATE(0, inskip, dur, amp * env, intab, numchans, inchan, winstart, winend,
   wrap, travrate, envtab, hoptime, injitter, outjitter, mindur, maxdur,
   minamp, maxamp, transp, transpcoll, transpjitter, seed)

