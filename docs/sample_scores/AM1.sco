rtsetparams(44100, 2)
load("AM")

rtinput("../../snd/input.wav")
inchan = 0
inskip = 0
dur = DUR()

amp = 1
env = maketable("line", 1000, 0,0, 2,1, 5,1, 7,0)

wavetable = maketable("wave", 1000, "sine")

AM(start=0, inskip, dur, amp=1, freq=14, inchan, pan=.7, wavetable)

start = dur + 0.15
AM(start, inskip, dur, amp=1, freq=187, inchan, pan=.3, wavetable)

