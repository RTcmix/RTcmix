float octcps(float);
float pchoct(float);

float pchcps(float cps)
{

  float oct,result;
  oct = octcps(cps);
  result = pchoct(oct);
  return(result);
}

