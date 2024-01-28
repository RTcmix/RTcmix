
for (count = 0; count < 3; ++count)
{
	if ((count % 2) == 0) {
		auto_var = "world";
	}
	else {
		auto_var = "hello";
	}
	auto_var.print();	// This succeeds because enclosing for() block is overridden by internal if/else
}
