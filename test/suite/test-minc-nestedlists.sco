depth = 50

container = {}
mylist = {}
for (i = 0; i < depth; i += 1) {
	mylist = {"nest depth:", i + 1}
	container = {container, mylist}
}

print(container)

