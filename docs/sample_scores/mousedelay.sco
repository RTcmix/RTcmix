// Test variable delay times with mouse interface.    -JGG, 2/21/05

rtsetparams(44100, 2, 128)
load("DELAY")

dur = 120

use_file_input = 0

// must use audio rate (or near audio rate) control signal when
// updating delay time
control_rate(44100)

if (use_file_input) {
   bus_config("MIX", "in 0-1", "aux 0 out")
   rtinput("../../snd/huhh.wav")
   inchan = 0
   inskip = 0.35
   inend = 0.45
   notedur = inend - inskip
   increment = notedur * 3
   amp = 0.5
   amp = maketable("line", "nonorm", 2000, 0,0, 1,amp, 4,amp, 5,0)
   end = dur - notedur
   for (st = 0; st < end; st += increment)
      MIX(st, inskip, notedur, amp, 0)
   amp = 4
}
else {
   load("WAVETABLE")
   bus_config("WAVETABLE", "aux 0 out")
   notedur = .2
   increment = .5
   wavet = maketable("wave", 5000, 1, .5, .3, .2, .1)
   amp = 8000
   amp = maketable("line", "nonorm", 2000, 0,0, 1,amp, 9,amp, 10,0)
   end = dur - notedur
   for (st = 0; st < end; st += increment)
      WAVETABLE(st, notedur, amp, freq=440, pan=0, wavet)
   inskip = inchan = 0
   amp = 1
}

env = maketable("line", 5000, 0,0, 1,1, 19,1, 20,0)
pan = makeconnection("mouse", "x", 1, 0, .5, lag=50, "pan")

ringdur = 2

// high lag makes smoother deltime changes, reducing clicks
deltime = makeconnection("mouse", "x", 0.0002, .5, .5, lag=90,
            "delay time", "", 5)

feedback = makeconnection("mouse", "y", 0, 1, 0, lag=20, "feedback")

bus_config("DELAY", "aux 0 in", "out 0-1")
DELAY(0, 0, dur, amp * env, deltime, feedback, ringdur, inchan, pan)


