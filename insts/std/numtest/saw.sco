/* numtest: write a test pattern into a soundfile
 
   p0 = start time
   p1 = duration
   p2 = target number
   p3 = channel 1 target number [optional]
   p4 = increment (If non-zero, don't write a single nunmber; instead, start
        from zero, add <increment> to successive samples until reaching the
        target number, return to zero, then do it over and over, creating
        an upward sawtooth wave.) [optional]
*/
load("numtest")
system("sfcreate -c 2 -f -t next saw.snd")
output("saw.snd")
numtest(0, 10, 10000, 5000, 10)
