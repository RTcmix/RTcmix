rtsetparams(44100, 1)

openfile("/home/jg/daytrip/snd/sub/dash.wav")
inskip = 1.0
dur = 4.0
amp = 1.0
inchan = 0

soundin(outskip=0, inskip, dur, amp, inchan)

