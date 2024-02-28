max_depth = 3;

list function(float depth)
{
	if (depth < max_depth) {
		return function(depth+1);
	}
	return { depth };
}

mylist = function(0);

