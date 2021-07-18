// translation of SuperCollider's Dust helpfile example.
rtsetparams(44100, 2)
load("DUST")

density = maketable("expbrk", "nonorm", 20, 20000, 10, 2, 10, 2)

DUST(0, dur=10, amp=14000, density, imprange=0, seed=1, pan=0.5)
