/* Another test score for gen20, this one using new optional args for
   seed, min, and max.  Invoke with "CMIX -Q < gen20a.sco"   -JGG, 12/5/01
*/
min = -2
max = 2
count = 50
type = 0
/* 0: even, 1: low weighted linear, 2: high weighted linear,
   3: triangle linear, 4: gaussian, 5: cauchy */

makegen(1, 20, count, type, seed=0, min, max)
fdump(1)
mid = min + ((max - min) / 2)
low = high = 0
for (i = 0; i < count; i = i+1) {
   num = sampfunc(1, i)
   if (num < mid)
      low = low + 1
   else
      high = high + 1
}
str_num("mid: ", mid, ", low: ", low, ", high: ", high)
