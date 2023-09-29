links = 10

pfield = maketable("line", 100, 0,0, 1,1)

mylist = {}
for (i = 0; i < links; i += 1) {
	mylist[i] = pfield
	pfield = makefilter(pfield, "smooth", 10)
}
mylist[i] = pfield

print(mylist)

