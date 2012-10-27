#include <stdio.h>

int main() {
	
	FILE *myfile;
    char tempstring[1024];
    
    struct timespec tp_start;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_start) < 0){
		printf("clock_gettime() failed\n");
	}

    if(!(myfile=fopen("/nethome/hchan35/source_code/cs3210_proj2/README.md","r")))
    {
         fprintf(stderr,"Could not open file\n");
         return -1;
    }

    while(!feof(myfile))
    {
         fscanf(myfile,"%s",tempstring);
         printf("%s",tempstring);
    }
	printf("\n");
	
	struct timespec tp_end;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_end) < 0){
		printf("clock_gettime() failed\n");
	}
	struct timespec tp_diff = diff(timespec start, timespec end);
	std::cout << "This test took " << tp_diff.tv_sec << " seconds" << endl;
	
	return 0;
}
