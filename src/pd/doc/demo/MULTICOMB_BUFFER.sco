rtinput("demo-array")

amp = maketable("line", 1000, 0,0, 0.5,1, 4.0,1, 4.3,0) * 0.1

srand()

MULTICOMB(0, 0, dur=3.57, amp, cpspch(6.02), cpspch(9.05), rvbtime=2)
