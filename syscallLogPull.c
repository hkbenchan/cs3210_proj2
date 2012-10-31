#include <stdio.h>

#define procfs_name 

int main() {
	
	FILE *procFile, *outputFile;
    char tempstring[1024];

    if(!(procFile=fopen("/proc/syslog","r")))
    {
         fprintf(stderr,"Could not open file\n");
         return -1;
    }

    while(!feof(procFile))
    {
         fscanf(procFile,"%s",tempstring);
         printf("%s",tempstring);
    }
	printf("\n");
	
	return 0;
}
