load("/home/jwmatthys/Software/RTcmix/jfuncs/libjfuncs.so")

rtsetparams(44100,2)
print_off()
srand()
arr_size = 160

for (i=0; i<arr_size; i+=1)
{
	put_array(0,i,0)
}

for (i=0; i<10000; i+=1)
{
	randomValue = prob(0.5,0.7)
	index = trunc(randomValue*arr_size)
	oldval = get_array(0,index)
	put_array(0,index,oldval+1)
}

randtable = maketable("literal","nonorm",arr_size,get_array(0,0),get_array(0,1),get_array(0,2),get_array(0,3),get_array(0,4),get_array(0,5),get_array(0,6),get_array(0,7),get_array(0,8),get_array(0,9),
get_array(0,10),get_array(0,11),get_array(0,12),get_array(0,13),get_array(0,14),get_array(0,15),get_array(0,16),get_array(0,17),get_array(0,18),get_array(0,19),
get_array(0,20),get_array(0,21),get_array(0,22),get_array(0,23),get_array(0,24),get_array(0,25),get_array(0,26),get_array(0,27),get_array(0,28),get_array(0,29),
get_array(0,30),get_array(0,31),get_array(0,32),get_array(0,33),get_array(0,34),get_array(0,35),get_array(0,36),get_array(0,37),get_array(0,38),get_array(0,39),
get_array(0,40),get_array(0,41),get_array(0,42),get_array(0,43),get_array(0,44),get_array(0,45),get_array(0,46),get_array(0,47),get_array(0,48),get_array(0,49),
get_array(0,50),get_array(0,51),get_array(0,52),get_array(0,53),get_array(0,54),get_array(0,55),get_array(0,56),get_array(0,57),get_array(0,58),get_array(0,59),
get_array(0,60),get_array(0,61),get_array(0,62),get_array(0,63),get_array(0,64),get_array(0,65),get_array(0,66),get_array(0,67),get_array(0,68),get_array(0,69),
get_array(0,70),get_array(0,71),get_array(0,72),get_array(0,73),get_array(0,74),get_array(0,75),get_array(0,76),get_array(0,77),get_array(0,78),get_array(0,79),
get_array(0,80),get_array(0,81),get_array(0,82),get_array(0,83),get_array(0,84),get_array(0,85),get_array(0,86),get_array(0,87),get_array(0,88),get_array(0,89),
get_array(0,90),get_array(0,91),get_array(0,92),get_array(0,93),get_array(0,94),get_array(0,95),get_array(0,96),get_array(0,97),get_array(0,98),get_array(0,99),
get_array(0,100),get_array(0,101),get_array(0,102),get_array(0,103),get_array(0,104),get_array(0,105),get_array(0,106),get_array(0,107),get_array(0,108),get_array(0,109),
get_array(0,110),get_array(0,111),get_array(0,112),get_array(0,113),get_array(0,114),get_array(0,115),get_array(0,116),get_array(0,117),get_array(0,118),get_array(0,119),
get_array(0,120),get_array(0,121),get_array(0,122),get_array(0,123),get_array(0,124),get_array(0,125),get_array(0,126),get_array(0,127),get_array(0,128),get_array(0,129),
get_array(0,130),get_array(0,131),get_array(0,132),get_array(0,133),get_array(0,134),get_array(0,135),get_array(0,136),get_array(0,137),get_array(0,138),get_array(0,139),
get_array(0,140),get_array(0,141),get_array(0,142),get_array(0,143),get_array(0,144),get_array(0,145),get_array(0,146),get_array(0,147),get_array(0,148),get_array(0,149),
get_array(0,150),get_array(0,151),get_array(0,152),get_array(0,153),get_array(0,154),get_array(0,155),get_array(0,156),get_array(0,157),get_array(0,158),get_array(0,159))

plottable(randtable,15)
