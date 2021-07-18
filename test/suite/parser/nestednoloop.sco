depth = 2

container = {}
list2 = {}
i = 0
list2 = {"nest depth:", i + 1}
container = {container, list2}
list2 = {"nest depth:", i + 2}
container = {container, list2}

print(container);


