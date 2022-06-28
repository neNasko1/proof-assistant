# proof-assistant
Compact proof assistant

See example code in code.txt, which proves that $(a+b)^2=(a^2+a\*b\*2+b^2)$ in less than 1 second.

The tool currently supports:
* expressions can contain free variables - these are variables which can match any other expression, they should be prefixed with \*
* rule \<expression\> \<expression\>: to specify that the first expression can be swapped for the second one
* rule both \<expression\> \<expression\>: to specify that the two expressions can be swapped interchangeably
* equal \<expression\> \<expression\>: to check if the first expression can be reduced to the second one only using the rules defined above this line
* apply \<expression\>: print all expression, which are equivalent to the given expression

Compile and run the program with:
```
$ make
$ ./proof-assist code.txt
```
