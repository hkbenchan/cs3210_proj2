#include <stdio.h>

#define procfs_name "syslog"

int main() {

	FILE *procFile, *outputFile;
	char tempstring[3][1024];

	if(!(procFile=fopen("/proc/syslog","r")))
	{
		fprintf(stderr,"Could not open file\n");
		return -1;
	}

	while(!feof(procFile))
	{
		fscanf(procFile,"%s",tempstring[0]);
		printf("%s ",tempstring[0]);
		
		fscanf(procFile,"%s",tempstring[1]);
		printf("%s ",tempstring[1]);
		
		fscanf(procFile,"%s",tempstring[2]);
		printf("%s\n",tempstring[2]);
		
	}
	printf("---End---\n");

	return 0;
}
