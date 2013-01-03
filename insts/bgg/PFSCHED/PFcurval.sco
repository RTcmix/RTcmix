rtsetparams(44100, 1)
load("PFSCHED")
load("WAVETABLE")

startenv = maketable("line", 1000, 0,0.0,  1,1.0, 2,0.5)
fadenv = maketable("line", "dynamic", 1000, 0,"curval", 1,0.0)
//fadenv = maketable("line", "dynamic", 1000, 0,"curval", 1,1.0, 2, "curval", 3,0)
//fadenv = maketable("line", 1000, 0,1, 1,0.0)

value = makeconnection("pfbus", 1, 0.0)

wave = maketable("wave", 1000, "sine")

// this doesn't work because PFSCHED now requires start time of 0.
// left in the repos to show how the "curval" token works in table construction
PFSCHED(0, 2.1, 1, startenv)
PFSCHED(3.5, 3.0, 1, fadenv, 1)
WAVETABLE(0, 777.0, 20000*value, 8.00, 0.5, wave)
