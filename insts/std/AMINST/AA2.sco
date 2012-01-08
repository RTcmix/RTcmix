rtsetparams(44100, 1)
load("AMINST")

AMINST(0, 3.5, 10000, 178, 315)

modenv = maketable("line", 1000, 0,0, 1,1, 2,0)
AMINST(3.9, 3.4, 10000, cpspch(8.00), cpspch(8.02), 0, modenv)
