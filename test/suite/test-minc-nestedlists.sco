depth = 50

container = {}
list = {}
for (i = 0; i < depth; i += 1) {
	list = {"nest depth:", i + 1}
	container = {container, list}
}

print(container)

