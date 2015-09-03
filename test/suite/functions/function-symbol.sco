float function()
{
	float private_sym;
	private_sym = 999;
	return 0;
}

x = private_sym;	// this needs to fail: function body symbols private
