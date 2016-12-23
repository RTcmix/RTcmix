rtsetparams(44100, 2)
load("./libHENON.so")

x = maketable("line", "nonorm", 1000, 0,1.3, 2,1.5)
HENON(0,10,10000,x,0.3,0.25,0.25)
