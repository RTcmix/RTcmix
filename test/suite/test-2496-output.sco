set_option("audio_off", "clobber_on")
rtsetparams(96000, nchan = 6)
rtoutput("2496out.aif", "24")

load("WAVETABLE")
bus_config("WAVETABLE", "aux 0 out")
totdur = 6
makegen(2, 10, 2000, 1)
WAVETABLE(0, totdur, 10000, 440)

dur = totdur / nchan
st = 0
bus_config("MIX", "aux 0 in", "out 0")
MIX(st, 0, dur, 1, 0)
st = st + dur
bus_config("MIX", "aux 0 in", "out 1")
MIX(st, 0, dur, 1, 0)
st = st + dur
bus_config("MIX", "aux 0 in", "out 2")
MIX(st, 0, dur, 1, 0)
st = st + dur
bus_config("MIX", "aux 0 in", "out 3")
MIX(st, 0, dur, 1, 0)
st = st + dur
bus_config("MIX", "aux 0 in", "out 4")
MIX(st, 0, dur, 1, 0)
st = st + dur
bus_config("MIX", "aux 0 in", "out 5")
MIX(st, 0, dur, 1, 0)

