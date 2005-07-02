set_option("record=on")
rtsetparams(44100, 1, 256)
load("AM")
rtinput("AUDIO")

amp = 1
env = maketable("line", 1000, 0,0, 2,1, 5,1, 7,0)
wavetable = maketable("wave", 1000, "sine")

AM(start=0, inskip=0, dur=7, amp * env, freq=14, 0, 0, wavetable)
AM(start=8, inskip=0, dur=7, amp * env, freq=187, 0, 0, wavetable)

