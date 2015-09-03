count = 3   // < 3 runs without error

float func(float n)
{
   y = 0 
   if (n < 1)
      return n
   x = func(n - 1)
   return x + y   // comment out "+ y": no error
}  

for (i = 0; i < count; i += 1) {
   func(i)
}  
