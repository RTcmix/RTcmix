rtsetparams(44100, 2)
load("STEREO")
load("ELL")

rtinput("../../snd/huhh.wav")
inchan = 0
inskip = 0
dur = DUR()

//----------------------------------------------
// unprocessed signal

amp = 2
STEREO(start=0, inskip, dur, amp, pan=.5)

//----------------------------------------------
// low-pass filter

pbcut = 400
sbcut = 800
ripple = .1
atten = 90

ellset(pbcut, sbcut, 0, ripple, atten)

amp = 4
ringdur = .1

ELL(start=dur+1, inskip, dur, amp, ringdur, inchan, pan=.5)

//----------------------------------------------
// high-pass filter

pbcut = 800
sbcut = 400
ripple = .1
atten = 90

ellset(pbcut, sbcut, 0, ripple, atten)

amp = 8
ringdur = .1

ELL(start=start+dur+1, inskip, dur, amp, ringdur, inchan, pan=.5)

