mylist = { 1, 2, 3, 4 };

x = mylist.len();

if (x != 4)
{
	error("method 'len' on list failed");
}

if (mylist.contains(3) != true)
{
	error("method 'contains' on list failed");
}

if (mylist.index(2) != 1)
{
	error("method 'index' on list failed");
}

if (mylist.type() != "list")
{
	error("method 'type' on list failed");
}
