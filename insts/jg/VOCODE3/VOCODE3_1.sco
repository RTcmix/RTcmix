//set_option("play=off")
rtsetparams(44100, 2, 256)
load("VOCODE3")

//rtoutput("foo.wav", "float")
// CPU load: c. 70% running as root (without COMPLIMIT)
// with Obalance, it's 40% !!  Must be conversion to floats and inline next()

totdur = 60

rtinput("/snd/japan1/tvnews.aif")
rtinput("../../../snd/input.wav")

// modulator
bus_config("MIX", "in 0", "aux 1 out")
dur = DUR()
amp = 1
for (st = 0; st < totdur; st += dur)
	MIX(st, 0, dur, amp, 0)
totdur = st

// carrier
noise = 0
if (noise) {
	load("NOISE")
	load("EQ")
	bus_config("NOISE", "aux 2 out")
	bus_config("EQ", "aux 2 in", "aux 0 out")
	amp = 15000
	NOISE(0, totdur, amp, 0)
	EQ(0, 0, totdur, 1, "lowpass", 0, 0, bypass=0, cf=4000, q=0.5)
}
else {
	load("WAVETABLE")
	bus_config("WAVETABLE", "aux 0 out")
	amp = 10000
	pitch = 8.00
	wavet = maketable("wave", 20000, "buzz")
	wavet = maketable("random", 40, "even", 0, 1, seed=1)
	WAVETABLE(0, totdur, amp, pitch, 0, wavet)
	WAVETABLE(0, totdur, amp, pitch + 0.02, 0, wavet)
	WAVETABLE(0, totdur, amp, pitch + 0.021, 0, wavet)
	WAVETABLE(0, totdur, amp, pitch + 0.05, 0, wavet)
	WAVETABLE(0, totdur, amp, pitch + 0.07, 0, wavet)
	WAVETABLE(0, totdur, amp, pitch + 0.069, 0, wavet)
	WAVETABLE(0, totdur, amp, pitch + 1.00, 0, wavet)
}


// --------------------------------------------------------------------------
bus_config("VOCODE3", "aux 0-1 in", "aux 4-5 out")

env = maketable("line", 1000, 0,0, .1,1, totdur-.1,1, totdur,0)

if (1) {
	numfilts = 64
	numfilts = 32
	tab = {}
	oct = octpch(6.00)
	incr = octpch(0.025)
	for (i = 0; i < numfilts; i += 1) {
		tab[i] = oct
		oct += incr
	}
	modcf = maketable("literal", "nonorm", 0, tab)
}
else {
	modcf = maketable("literal", "nonorm", 0,
		octpch(7.00),
		octpch(7.05),
		octpch(7.07),
		octpch(8.00),
		octpch(8.02),
		octpch(8.06),
		octpch(8.09),
		octpch(9.00),
		octpch(9.04),
		octpch(9.11),
		octpch(10.06),
		octpch(11.01)
	)
}
numfilts = tablelen(modcf)

if (1) {
	//index = makeconnection("mouse", "x", min=0, max=numfilts, df=0, lag=50, "idx", "", 1)
	//value = makeconnection("mouse", "y", min=5, max=10, df=min, lag=50, "val")
	//modcf = modtable(modcf, "draw", "literal", index, value)

	index = makerandom("even", lfreq=2, min=0, max=1, seed=1)
	//index = makefilter(index, "smooth", lag=0)
	value = makerandom("even", lfreq=.5, min=7, max=12, seed=1)
	value = makefilter(value, "smooth", lag=80)
	//value = makemonitor(value, "display", "value")
	modcf = modtable(modcf, "draw", index, value)
}

carcf = modcf

if (1) {
	maplist = {}
	spray_init(1, numfilts, seed=1)
	for (i = 0; i < numfilts; i += 1)
		maplist[i] = get_spray(1)
	map = maketable("literal", "nonorm", numfilts, maplist)
	dumptable(map)
}
else
	map = 0

scale = 0
//scale = maketable("curve", "nonorm", numfilts, 0,1,-1, 1,0)
//scale = maketable("curve", "nonorm", numfilts, 0,0,1, 1,1)

modtransp = 0.0
//modtransp = makeconnection("mouse", "x", min=-1, max=3, dflt=0, lag=50, "mtrns")
cartransp = 1.0
//cartransp = makeconnection("mouse", "y", min=-2, max=2, dflt=0, lag=50, "ctrns")

modq = 10.0
carq = 400.0

amp = 1.0
//amp = makeconnection("mouse", "y", min=0, max=1, dflt=0, 0, "amp")

response = 0.001
//response = makeconnection("mouse", "x", min=0, max=2, dflt=0.01, 0, "resp")

hold = 0
//hold = makeconnection("mouse", "y", min=0, max=2, dflt=0, 0, "hold")
hold = makeLFO("square2", freq=10, min=0, max=3)
//hold = makerandom("even", freq=2, min=0, max=1.5, seed=1)

VOCODE3(0, 0, totdur, amp * env, modcf, carcf, map, scale,
			modtransp, cartransp, modq, carq, response, hold, pan=.9)
modtransp += 1
cartransp -= .03
hold = makeLFO("square2", freq=10, min=0, max=8)
VOCODE3(0, 0, totdur, amp * env, modcf, carcf, map, scale,
			modtransp, cartransp, modq, carq, response, hold, pan=.1)


// --------------------------------------------------------------------------
load("COMPLIMIT")
ingain = 0
outgain = 0
atk = 0.01
rel = 0.01
thresh = -1
ratio = 100
look = atk
win = 128
bus_config("COMPLIMIT", "aux 4 in", "out 0")
COMPLIMIT(0, 0, totdur, ingain, outgain, atk, rel, thresh, ratio, look, win)
bus_config("COMPLIMIT", "aux 5 in", "out 1")
COMPLIMIT(0, 0, totdur, ingain, outgain, atk, rel, thresh, ratio, look, win)

