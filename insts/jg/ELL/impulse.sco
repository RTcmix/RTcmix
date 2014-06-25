load("numtest")
system("rm -f impulse.snd")
system("sfcreate -r 44100 -c 1 -t sun impulse.snd")
output("impulse.snd")
numtest(0, 1/44100, 32767)           /* one sample */
system("sfshrink .25 impulse.snd")   /* extend with zeros */
