rtsetparams(44100, 2)
load("./libSGRAN2.so")

        /* Args:
                p0: outskip
                p1: dur
                p2: amp*
                p3: rateLow (seconds before new grain)*
                p4: rateMid*
                p5: rateHigh*
                p6: rateTight*
                p7: durLow (length of grain in seconds)*
                p8: durMid*
                p9: durHigh*
                p10: durTight*
                p11: freqLow*
                p12: freqMid*
                p13: freqHigh*
                p14: freqTight*
		p15: panLow (0 - 1.0)*
		p16: panMid*
		p17: panHigh*
		p18: panTight*
                p19: wavetable**
                p20: grainEnv** 
                p21: grainLimit=1500 (optional)
		
                * may recieve pfield values
		** must be passed pfield maketables.
        */

outskip = 0
dur = 15

amp = maketable("line", 1000, 0, 0, 8, 0.8, 16, 1, 17, 0)

ratelo = 0.004
ratemid = 0.005
ratehi = 0.007
rateti = maketable("line", "nonorm", 200, 0, 8, 1, 0.2)

durlo = 0.01
durmid = 0.05
durhi = 0.08
durti = 0.1

freqlo = maketable("line", "nonorm", 200, 0, 400, 1, 200)
freqmid = maketable("line", "nonorm", 200, 0, 430, 1, 350, 2, 600)
freqhi = maketable("line", "nonorm", 200, 0, 440, 1, 460, 2, 800)
freqti = maketable("line", "nonorm", 200, 0, 6, 1, 0.2)

panlo = 0
panmid = maketable("line", "nonorm", 200, 0, 0.1, 1, 0.1, 2, 0.5)
panhi = maketable("line", "nonorm", 200, 0, 0.2, 1, 0.5, 2, 1)
panti = 0.4

wave = maketable("wave", 1000, "square")
env = maketable("window", 1000, "hanning")

SGRAN2(outskip, dur, 800 * amp, ratelo, ratemid, ratehi, rateti, durlo, durmid, durhi, durti, 
freqlo, freqmid, freqhi, freqti, panlo, panmid, panhi, panti, wave, env)


