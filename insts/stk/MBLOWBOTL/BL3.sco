rtsetparams(44100, 2)
load("./libMBLOWBOTL.so")

//makegen(1, 24, 1000, 0,1, 1,0)
amp = maketable("line", 1000, 0,0,1,1,2,0)
breath = maketable("line", 1000, 0,0,1,1,2,0)
MBLOWBOTL(0, 3.5, amp*20000.0, 349.0, 0.3, 0.5)

freq = maketable("line", "nonorm", 1000, 0,278.0, 1, 520.0, 4,250.0)
noiseamp = makeLFO("saw", 5, 1.0)
pan = makeLFO("sine", 1, 0.0, 1.0)
MBLOWBOTL(4, 3.5, 20000.0, freq, noiseamp, 0.8, pan, breath)
