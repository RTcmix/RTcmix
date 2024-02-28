depth = 50

container = {}
mylist = {}
for (i = 0; i < depth; i += 1) {
	pfield = maketable("line", 100, 0,0, 1,1)
	mylist = {"nest depth:", i + 1, pfield}
	container = {container, mylist}
}

print(container)

