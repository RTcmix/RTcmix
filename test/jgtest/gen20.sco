print_off()
if (n_arg() < 2) {
   str_num("usage:  CMIX num_array_slots distribution_type < gen20.sco")
   str_num("  distribution_type:")
   str_num("    0=even, 1=low-weighted, 2=high-weighted, 3=triangle")
   str_num("    4=gaussian, 5=cauchy")
   exit(1)
}
slots = i_arg(0)
distribution = i_arg(1)
if (distribution < 0 || distribution > 5) {
   str_num("distribution_type must be one of:")
   str_num("0=even, 1=low-weighted, 2=high-weighted, 3=triangle, 4=gaussian, 5=cauchy")
   exit(1)
}
if (n_arg() > 2)
   seed = i_arg(2)
else
   seed = 0      /* take seed from microsecond clock */

/* -------------------------------------------------------------------------- */

if (distribution == 0)
   str_num("You're making an even distribution...")
else if (distribution == 1)
   str_num("You're making a low-weighted linear distribution...")
else if (distribution == 2)
   str_num("You're making a high-weighted linear distribution...")
else if (distribution == 3)
   str_num("You're making a triangle linear distribution...")
else if (distribution == 4)
   str_num("You're making a Gaussian distribution, das ist recht gut...")
else if (distribution == 5)
   str_num("You're making a Cauchy distribution, d'accord, c'est tout...")

if (seed)
   str_num("(using ", seed, " as a seed)")
else
   str_num("(consulting microsec counter for seed)")

makegen(1, 20, slots, distribution, seed)

for (i = 0; i < slots; i = i + 1) {
   num = sampfunc(1, i)
   str_num("slot ", i, " contains:  ", num)
}

/* fills a gen with random numbers b/t 0 and 1 --
* the argument is the distribution type:
* 0 = even distribution
* 1 = low weighted linear distribution
* 2 = high weighted linear distribution
* 3 = triangle linear distribution
* 4 = gaussian distribution
* 5 = cauchy distribution
*
* by r. luke dubois - CMC/CU, 1998.
*
* distribution equations adapted from dodge and jerse.
*
* test score by JG
*/
