set_option("full_duplex_on")  /* must do this before rtsetparams */
rtsetparams(44100, 2, 256)
load("AM")

rtinput("AUDIO", "MIC", 2)

makegen(1, 24, 1000, 0,0, 0.1,1, 0.2,1, 0.3,0)
makegen(2, 10, 1000, 1)

for(start = 0; start < 15.0; start = start + 0.1) {
        freq = random() * 400.0
        AM(start, 0, 0.3, 1, freq, 0, random())
        }
