## About
Stochastic granular synthesizers for RTcmix version 4.5 based on the SGRANR instrument, with added pfield control, improved performance, and additional buffer features.  SGRAN2 works with periodic waveforms while STGRAN2 works with a provided soundfile or live input signal.

## Examples
### SGRAN2

https://user-images.githubusercontent.com/69212477/147691785-44a433a8-5641-47cd-8736-3a59bc73df5a.mp4

https://user-images.githubusercontent.com/69212477/147691891-53d72308-b080-4f00-8393-49e684ce733b.mp4

### STGRAN2

https://user-images.githubusercontent.com/69212477/148407993-227b08c9-a545-46cc-b253-875c567a8963.mp4

https://user-images.githubusercontent.com/69212477/148408034-002d62c7-b3ef-4b4c-9067-4aa3a63c321d.mp4



## Usage

Make sure the package.conf points to the appropriate RTcmix makefile.conf before building, then `make`

Both instruments rely on Dr. Helmuth's `prob` function, which takes four floating point parameters: `low`, `mid`, `high` and `tight`.  Calling this function returns a stochastically chosen value based on a distribution centered around `mid` with upper and lower bounds at `low` and `high`.  The `tight` value determines how closely the distribution clusters at `mid`.  `tight` of 1 will be an even distribution, with more than one being closer to the `mid` value, and less than one spreading towards the `low` and `high` bounds.

Every time a new grain spawns, multiple `prob` functions run to generate properties of that grain.  These include the time until the next grain, the duration of this grain, the frequency/transposition of this grain, and the panning of this grain.

SGRAN2 creates grains from a user provided periodic wavefornm.

STGRAN2 works with a provided audio file or realtime audio source.  Grain start points are chosen randomly between the present and "buffer start size" (p20) seconds ago.  High p20 values result in the smearing of short impulses to long lasting clouds.  Extreme transpositions may be ignored so grains don't move "into the future", or go too far into the past.

See [TRANS usage notes](http://rtcmix.org/reference/instruments/TRANS.php#usage_notes) regarding dynamically updating STGRAN2 transposition values.

Both apply a user provided windowing function for each grain.

See the included scorefiles.

### SGRAN2

Args:  

    - p0: outskip  

    - p1: duration

    - p2: amplitude*  

    - p3-6: rate values (seconds before the next grain grain)* 

    - p7-10: duration values (length of grain in seconds)*

    - p11-14: pitch values (Hz or oct.pc)*

    - p15-18: pan values(0 - 1.0)* 

    - p19: synthesis waveform**  

    - p20: grain amplitude envelope**  

    - p21: maximum concurrent grains [optional; default is 1500]
    
\* may receive a reference to a pfield handle  

\*\* must receive a reference to a pfield maketable handle  


### STGRAN2

Args:

    - p0: outskip 
    
    - p1: inskip 

    - p2: dur  

    - p3: amp* 

    - p4-7: rate values (seconds before the next grain grain)* 

    - p8-11: duration values (length of grain in seconds)*

    - p12-15: transposition values (oct.pc)*

    - p16-19: pan values(0 - 1.0)*  

    - p20: grain amplitude envelope**

    - p21: size of the buffer used to choose grain start points [optional; default is 1]*

    - p22: maximum concurrent grains [optional; default is 1500]
    
    - p23: input channel [optional; default is 0]
    
\* may receive a reference to a pfield handle  

\*\* must receive a reference to a pfield maketable handle
