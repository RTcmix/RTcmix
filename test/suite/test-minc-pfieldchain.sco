links = 10

pfield = maketable("line", 100, 0,0, 1,1)

list = {}
for (i = 0; i < links; i += 1) {
	list[i] = pfield
	pfield = makefilter(pfield, "smooth", 10)
}
list[i] = pfield

print(list)

