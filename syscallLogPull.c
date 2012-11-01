#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define procfs_name "syslog"

int main() {

	FILE *procFile, *outputFile;
	char tempstring[4][1024];
	struct timeval tv;
	struct stat st;
	char filePath[1024];
	if(!(procFile=fopen("/proc/syslog","r")))
	{
		fprintf(stderr,"Could not open file\n");
		return -1;
	}
	
	if (stat("/var/log/sclog", &st) != 0) {
		mkdir("/var/log/sclog",0666);
	}
	
	if (stat("/var/log/sclog", &st) != 0) {
		fprintf(stderr, "Could not create directory on /var/log/sclog");
		return -1;
	}
	
	gettimeofday(&tv, NULL);
	sprintf(filePath, "/var/log/sclog/log_%ld", tv.tv_sec);
	if (!(outputFile=fopen(filePath,"w")))
	//if (!(outputFile=fopen("/var/log/sclog/log_"+tv.tv_sec,"w")))
	{
		fprintf(stderr,"Could not open file to dump\n");
		return -1;
	}
	
	
	
	fprintf(outputFile,"pid   \tsyscall_number\ttimestamp       \targ value\n");

	while(!feof(procFile))
	{
		fscanf(procFile,"%s",tempstring[0]); // pid
		
		fscanf(procFile,"%s",tempstring[1]); // syscall number
		
		fscanf(procFile,"%s",tempstring[2]); // timestamp
		
		fscanf(procFile,"%s", tempstring[3]); // arg value
		
		fprintf(outputFile,"%-6s\t%-14s\t%s\t%s\n", tempstring[0], tempstring[1], tempstring[2], tempstring[3]);
	}
	fclose(procFile);
	fclose(outputFile);
	
	//chmod("/var/log/sclog/log_"+tv.tv_sec, S_ISVTX | S_IRUSR | S_IWUSR);
	printf("---End---\n");

	return 0;
}
