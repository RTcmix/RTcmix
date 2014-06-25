rtsetparams(44100, 1)
load("./libMYWAVETABLE.so")
makegen(1, 7, 1000, 0, 50, 1, 900, 1, 50, 0)
makegen(2, 10, 1000, 1, 0.3, 0.2)

MYWAVETABLE(0, 3.5, 25000, 349)
