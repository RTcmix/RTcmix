double
m_system(p,n_args, pp)
float *p; short n_args; double *pp;
{
	int i;
	char *command;
	i = (int) pp[0];
	command = (char *) i;
	return system(command);
}
