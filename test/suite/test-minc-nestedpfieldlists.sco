depth = 50

container = {}
list = {}
for (i = 0; i < depth; i += 1) {
	pfield = maketable("line", 100, 0,0, 1,1)
	list = {"nest depth:", i + 1, pfield}
	container = {container, list}
}

print(container)

