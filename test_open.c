#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int main() {
	
	FILE *myfile;
    char tempstring[1024];
    
    struct timespec tp_start;
	if(clock_gettime(CLOCK_REALTIME, &tp_start) < 0){
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
	if(clock_gettime(CLOCK_REALTIME, &tp_end) < 0){
		printf("clock_gettime() failed\n");
	}
	//struct timespec tp_diff;
	
	long nano_seconds = (tp_end.tv_nsec - tp_start.tv_nsec);
	printf("%ld %ld", tp_end.tv_sec, tp_end.tv_nsec); 
	printf("This test took  %ld seconds.\n", nano_seconds);
	
	return 0;
}
