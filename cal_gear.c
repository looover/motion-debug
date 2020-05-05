
#include <stdio.h>


int main(int arg, char * argv[])
{
	int input = 1000;
	int output = 720;
	int z0 = ((double)output / (double)input * 10000);
	
	printf("z0 = %d\n", z0);

	int denominator = 0;
	int numerator = 0;

	int multiplier = 0;

	int multi_max = 17;
	int denom_max = 10000;
	int numerator_max = 10000;

	int cnt = 0;
	for(int pr = 0; pr < multi_max; pr++){
		for(int pa = 1; pa < 100; pa++){
			for(int pb = 1; pb< 100; pb++){
				double z = pb * 10000 / (pa * (1 << pr));
				if(z0 == (int)z){
					printf("index=%4d, pa=%5d, pr=%5d, pb=%2d, z=%f\n", cnt++, pa, pr, pb, z);
					if(cnt > 10){
						return 0;
					}
				}
			}
		}
	}
	printf("finished\n");

	return 0;
}
