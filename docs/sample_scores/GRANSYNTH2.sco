rtsetparams(44100, 2, 128)
load("GRANSYNTH")

dur = 30

amp = maketable("line", 1000, 0,0, 1,1, 2,0.5, 3,1, 4,0)
wave = maketable("wave", 2000, 1, .5, .3, .2, .1)
granenv = maketable("window", 2000, "hanning")
hoptime = maketable("line", "nonorm", 1000, 0,0.01, 1,0.002, 2,0.05)
hopjitter = 0.0001
mindur = .04
maxdur = .06
minamp = maxamp = 1
pitch = maketable("line", "nonorm", 1000, 0,6, 1,9)
transpcoll = maketable("literal", "nonorm", 0, 0, .02, .03, .05, .07, .10)
pitchjitter = 1

st = 0
bus_config("GRANSYNTH", "out 0")
GRANSYNTH(st, dur, amp*5000, wave, granenv, hoptime, hopjitter,
   mindur, maxdur, minamp, maxamp, pitch, transpcoll, pitchjitter)

bus_config("GRANSYNTH", "out 1")
st = st+0.14
pitch = pitch+0.002
GRANSYNTH(st, dur, amp*5000, wave, granenv, hoptime, hopjitter,
   mindur, maxdur, minamp, maxamp, pitch, transpcoll, pitchjitter)

