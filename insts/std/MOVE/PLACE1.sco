/* PLACE -- Doug Scott's room simulation instrument
*
* space (dist_to_front, dist_to_right, -dist_to_back, -dist_to_left, height, abs_fact, rvbtime)
*
* mikes(mike_angle, pattern_factor)
* mikes_off()
*
* PLACE(inskip, outskip, dur, amp, distance_to_sound, angle_of_sound, dist_between_mikes, 1.0)
*
*/

set_option("full_duplex_on")
rtsetparams(44100, 2)
load("PLACE")
rtinput("AUDIO")

space(400,400,-400,-400,400,8.,10.)
PLACE(0,0,14.3,5,10,20,-1,1.)
