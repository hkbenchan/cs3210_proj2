#include <stdio.h>

#define procfs_name "syslog"

int main() {

	FILE *procFile, *outputFile;
	char tempstring[4][1024];

	if(!(procFile=fopen("/proc/syslog","r")))
	{
		fprintf(stderr,"Could not open file\n");
		return -1;
	}

	if (!(outputFile=fopen("/var/log/syscall.log","w")))
	{
		fprintf(stderr,"Could not open file to dump\n");
		return -1;
	}
	fprintf(outputFile,"pid   \tsyscall number\ttimestamp       \tsyscall name\n");

	while(!feof(procFile))
	{
		fscanf(procFile,"%s",tempstring[0]); // pid
		
		fscanf(procFile,"%s",tempstring[1]); // syscall number
		
		fscanf(procFile,"%s",tempstring[2]); // timestamp
		
		fscanf(procFile,"%s", tempstring[3]); // sys_call_name
		
		fprintf(outputFile,"%.6s\t%.14s\t%s\t%s\n", tempstring[0], tempstring[1], tempstring[2],tempstring[3]);
	}
	fclose(procFile);
	fclose(outputFile);
	printf("---End---\n");

	return 0;
}
