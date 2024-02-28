list ll;

ll[3] = 333;

ll[1] = 111;

list returnNullList()
{
	list nullList;
	return nullList;
}

l = returnNullList();
l[2] = 222;

l2 = { returnNullList() };

nullList = l2[0];

nullList[3] = 333;

