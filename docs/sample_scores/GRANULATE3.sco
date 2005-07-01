rtsetparams(44100, 2, 512)
load("GRANULATE")

dur = 360
amp = ampdb(-9)

fname = "../../snd/nucular.wav"
intab = maketable("soundfile", "nonorm", 0, fname)
numchans = 1
inchan = 0

inskip = 0.45
winstart = 0.01
winend = filedur(fname) - 0.01
wrap = 1

travrate = makeconnection("mouse", "x", min=-1, max=1, dflt=0, lag=30,
                          "traversal rate", "", 6)

envtab = maketable("window", 1000, "hanning")

injitter = 0.0
outjitter = 0.001

density = makeconnection("mouse", "y", min=5, max=120, dflt=10, lag=20,
                         "density", "grains/sec")
hoptime = 1 / density

mindur = hoptime * 10   // Since hoptime is dynamic, so is mindur and maxdur.
maxdur = mindur
minamp = maxamp = 1.0

transpcoll = maketable("literal", "nonorm", 0,  0.00, 0.02, 0.03, 0.05, 0.07)
transp = octpch(-0.03)
transpjitter = octpch(0.09)

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

