load("sgran")

system("rm -f sgran1.wav")
system("sfcreate -t wav -f sgran1.wav")
output("sgran1.wav")

setline(0,0, 1,1, 9,1, 10,0)
makegen(2, 7, 1000, 0, 50, 1, 950, 0)
makegen(3, 7, 1000, 1, 400, 0.1, 600, 0.4)
makegen(4, 7, 1000, 0, 400, 0.1, 600, 0.9)
makegen(5, 7, 1000, 0, 1000, 1)
makegen(6, 10, 1000, 1, 0.1, 0.3)
makegen(8, 7, 1000, 0, 500, 1, 500, 0)

sgran(0, 3.5, 3000,
 0.1, 0.005,
 0, 0.5, 0.9, 0.2,
 0.9, 0.5, 0.1, 0.7,
 0.9, 0.2, 0.9, 0.5,
 0.9, 0.5, 0.1, 0.7,
 0.1, 0.7, 0.9, 0.2,
 0.5, 0.9, 0.2, 0.9,
 200, 900, 500, 0.1,
 2000, 1000, 1500, 0.5)

system("rescale -r sgran1.wav")


/*
   sgran:
      0              start time of group
      1              duration of group
      2              amplitude
      3              beginning grain rate (time in seconds btw. grains)
      4              ending grain rate

      amount of variation in rate: (percentage of grain rate)
      5-8            beg: lo, average, hi, tightness (0-1, is 0-100%)
      9-12           end: lo, average, hi, tightness (0-1, is 0-100%)

      average duration:
      13-16          starting lo, average, hi, tightness
      17-20          ending lo, average, hi, tightness

      location:
      21-24          starting lo, average, hi, tightness
      25-28          ending lo, average, hi, tightness

      pitch band:
      29-32          starting lo, average, hi, tightness
                     (if p29 < 0, noise is the input)
      33-36          ending lo, average, hi, tightness

      37             random seed (integer) [optional]

                 *       *       *

   functions: (stt variation changes are linear)

      1              overall envelope (or setline)

      shape of change (usually linear for all shapes):
      2              grain density
      3              grain duration
      4              grain location
      5              grain frequency

      6              oscillator waveform

      8              grain envelope (note: was function 1 in Mara's version)
*/

