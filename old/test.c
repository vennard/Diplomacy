#include <stdio.h>
#include <stdlib.h>

int main() {
	printf("start testing");
	int x;
	FILE *fp,*pp;

	fp = fopen("/sys/kernel/ece453_digidiplo/tmrmins","w");
	printf("fp - %p!\r\n",fp);
	if (fp == NULL) {
		printf("FAILED TO OPEN FILE\r\n");
		return 1;
	}
	fprintf(fp, "%x",32);
	fclose(fp);
	printf("wrote out to tmrmins");

	pp = fopen("/sys/kernel/ece453_digidiplo/tmrstart","w");
	if (pp == NULL) {
		printf("FAILED TO OPEN FILE\r\n");
		return 1;
	}
	fprintf(pp, "%i",1);
	fclose(pp);
	printf("wrote out to tmrstart");
/*	
	fp = fopen("/sys/kernel/ece453_digidiplo/tmrtimeup","r");
	if (fp == NULL) printf("file doesn't exist");
	sleep(1);
	ch = fgetc(fp);
	printf("Result is %c\r\n",ch);
	fclose(fp);
	fp = fopen("/sys/kernel/ece453_digidiplo/tmrmins","w");
	if (fp == NULL) printf("failed to open file");
	fprintf(fp, "11",11);
	fclose(fp);
	
	sleep(1);
	fp = fopen("/sys/kernel/ece453_digidiplo/tmrstart","w");
	if (fp == NULL) printf("failed to open file");
	fprintf(fp, "%c",1);
	printf("wrote out to tmrstart");
	fclose(fp);
*/
	return 0;
}
