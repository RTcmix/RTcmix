rtsetparams(44100, 2)
load("JCHOR")

rtinput("../../snd/huhh.wav")
inchan = 0

outdur = 16
inskip = 0.01
indur = 0.20
maintain_dur = 1
transposition = 0.02
nvoices = 60
minamp = 0.1
maxamp = 1.0
minwait = 0.00
maxwait = 0.60
seed = 0.9371

amp = 6.0
env = maketable("line", 1000, 0,0, 1,1, outdur-3,1, outdur,0)

grainenv = maketable("window", 1000, "hanning")

reset(2000)

outskip = 0
JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed, inchan, amp * env, grainenv)

outskip = 2
outdur -= outskip
amp = 2.0
indur = 0.90
transposition = -0.10
maxamp = 0.5
maxwait = 1.0
seed = 0.2353
JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed, inchan, amp * env, grainenv)

