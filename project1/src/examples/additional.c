#include <stdio.h>
#include <syscall.h>

int main(int argc,char**argv)
{
	if(argc==5){
	int a,b,c,d;
	a=atoi(argv[1]);
	b=atoi(argv[2]);
	c=atoi(argv[3]);
	d=atoi(argv[4]);
	printf("%d th fibonnaci number is %d\nmax is %d\n",a,fibonacci(a),max_of_four_int(a,b,c,d));
	}
	return EXIT_SUCCESS;
}
