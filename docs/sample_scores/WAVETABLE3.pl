# This is a translation of sample_scos_1.0/WAVETABLE3.sco into Perl.   -JGG

use RT;

rtsetparams(44100, 2);
load("WAVETABLE");
print_off();
makegen(1, 24, 1000, 0, 1,  950, 0);
makegen(2, 10, 1000, 1, 0.3, 0.2);

srand(0.35);

for ($start = 0; $start < 7; $start += 0.14) {
   $freq = (random() * 200) + 35;
   for ($i = 0; $i < 3; $i += 1) {
      WAVETABLE($start, 0.4, 1500, $freq, 0);
      WAVETABLE($start + random()*0.1, 0.4, 1500, $freq + (random()*7), 1);
      if ($start > 3.5) {
         makegen(2, 10, 1000, 1, random(), random(), random(), random(),
                  random(), random(), random(), random(), random(), random(),
                  random());
      }
      $freq += 125;
   }
}
