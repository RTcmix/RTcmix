rtsetparams(44100, 2)
load("JDELAY")

rtinput("astereo.snd")

outskip = 0
inskip = 1
indur = DUR() - inskip
amp = 1.0
deltime = 1/cpspch(7.02)
feedback = .980
ringdowndur = 2
percent_wet = 0.5

setline(0,0, .01,1, indur/1.1,1, indur,0)

cutoff = 4000
JDELAY(outskip, inskip, indur, amp, deltime, feedback, ringdowndur,
       cutoff, percent_wet, inchan=0, spread=1)
JDELAY(outskip, inskip, indur, amp, deltime, feedback, ringdowndur,
       cutoff, percent_wet, inchan=1, spread=0)

