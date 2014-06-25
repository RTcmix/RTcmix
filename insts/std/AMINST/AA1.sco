rtsetparams(44100, 1)
load("AMINST")

makegen(1, 24, 1000, 0,0, 0.1,1, 3.4,1, 3.5,0)
makegen(2, 24, 1000, 0,0, 1,1, 2,0)
makegen(3, 10, 1000, 1)
makegen(4, 10, 1000, 1)
AMINST(0, 3.5, 10000, 178, 315)

makegen(1, 24, 1000, 0,1, 1,0)
makegen(2, 24, 1000, 0,1, 1.0,0.2, 3.4,0)
AMINST(3.9, 3.4, 10000, cpspch(8.00), cpspch(8.02))
