rtsetparams(44100, 2)
load("JCHOR")

/* -------------------------------------------------------------------------- */
rtinput("your_very_own.snd")
outskip = 0
outdur = 9
inskip = 0.00
indur = 0.20
maintain_dur = 1
transposition = -0.05
nvoices = 10
minamp = 0.1
maxamp = 0.5
minwait = 0.00
maxwait = 0.60
seed = 0.9371

setline(0,0, .5,1, outdur/8,1, outdur,0)
makegen(2, 7, 1000, 0, 10, 1, 990, 0)

JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

/* -------------------------------------------------------------------------- */
outskip = 1
outdur = 10
inskip = 0.60
indur = 0.35
transposition = -0.051
nvoices = 5
maxamp = 1.0
maxwait = 0.40
seed = 0.2353

setline(0,0, outdur/2,1, outdur,0)

JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

transposition = -0.049
seed = 0.9147
JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

