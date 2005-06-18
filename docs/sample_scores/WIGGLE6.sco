rtsetparams(44100, 2)
load("WIGGLE")

dur = 14
amp = 10000
pitch = 6.00
mod_depth_type = 2      /* mod index */

setline(0,0, 1,1, 2,1, 5,0)

/* This score uses negative values for mod. freq, which are interpreted
   as C:M ratios where C = 1.  E.g., first note has 1:1 moving to 1:1.05;
   2nd note has 1:2 moving to 1:2.2.
*/
makegen(2, 10, 2000, 1)                /* car waveform */
makegen(3, 18, 10, 0,0, 1,0)           /* car gliss */
makegen(4, 10, 2000, 1)                /* mod waveform */
makegen(5, 18, 10, 0,-1, 1,-1.05)      /* mod freq */
makegen(6, 18, 10, 0,30, 1,1)          /* mod depth */
makegen(8, 18, 10, 0,0, 1,0)           /* pan */

WIGGLE(st=0.00, dur, amp, pitch, mod_depth_type)

makegen(5, 18, 10, 0,-2, 1,-2.2)       /* mod freq */
makegen(6, 18, 10, 0,20, 1,1)          /* mod depth */
makegen(8, 18, 10, 0,1, 1,1)           /* pan */
WIGGLE(st=0.01, dur, amp, pitch, mod_depth_type)

