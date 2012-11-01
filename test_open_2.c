#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>

int copyFile(const char *o_file, const char *c_file){
	  
	  char ch;
	  FILE *p,*q;
	  p=fopen(o_file,"r");
	  if(p==NULL){
		printf("cannot open %s",o_file);
		return 1;
	  }
	  q=fopen(c_file,"w");
	  if(q==NULL){
		printf("cannot open %s",file2);
		return 1;
	  }
	  while((ch=getc(p))!=EOF)
		  putc(ch,q);
	  printf("\nCOMPLETED");
	  fclose(p);
	  fclose(q);
	 return 0;
}


int test_fopen_fread(){
	FILE *myfile;
    
    struct timespec tp_start;
	if(clock_gettime(CLOCK_REALTIME, &tp_start) < 0){
		printf("clock_gettime() failed\n");
	}

    if(!(myfile=fopen("/nethome/hchan35/source_code/cs3210_proj2/README.md","r")))
    {
         fprintf(stderr,"Could not open file\n");
         return -1;
    }

	const char *myfile_s = "/nethome/hchan35/source_code/cs3210_proj2/README.md";
    const char *file2 = "/nethome/hchan35/source_code/cs3210_proj2/README(COPY).md";
    
    if(copyFile(myfile_s, file2)){
		fprintf(stderr,"Error copying file\n");
		return -1;
	}
	
	struct timespec tp_end;
	if(clock_gettime(CLOCK_REALTIME, &tp_end) < 0){
		printf("clock_gettime() failed\n");
	}
	//struct timespec tp_diff;
	
	long nano_seconds = (tp_end.tv_nsec - tp_start.tv_nsec);
	long seconds = (tp_end.tv_sec - tp_start.tv_sec);
	if (nano_seconds < 0) {
		nano_seconds += 1000000000;
		seconds --;
	}
	printf("%ld %ld", tp_end.tv_sec, tp_end.tv_nsec); 
	printf("Read: This test took  %ld.%ld seconds.\n", seconds, nano_seconds);
	
	return 0;
}

int test_mmap(){
    
    struct timespec tp_start;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_start) < 0){
		printf("clock_gettime() failed\n");
	}
	
	//implement the mmap
   	struct stat ss;
    int src = open("/nethome/hchan35/source_code/cs3210_proj2/README.md", O_RDONLY);

    fstat(src, &ss);

    int dest = open("/nethome/hchan35/source_code/cs3210_proj2/README(mmapCopy).md", O_RDWR | O_CREAT | O_TRUNC, ss.st_mode);

    void *dest_addr = mmap(NULL, ss.st_size, PROT_WRITE, MAP_SHARED, dest, 0);
    printf("dest is: %x\n", dest_addr);

    void *src_addr = mmap(dest_addr, ss.st_size, PROT_READ, MAP_PRIVATE | MAP_FIXED, src, 0);
    printf("src is: %x\n", src_addr);

    if (munmap(dest_addr, ss.st_size))
        printf("munmap failed");

    if (munmap(src_addr, ss.st_size))
        printf("munmap failed");

	struct timespec tp_end;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_end) < 0){
		printf("clock_gettime() failed\n");
	}
	//struct timespec tp_diff = diff(timespec start, timespec end);
	//std::cout << "This test took " << tp_diff.tv_sec << " seconds" << endl;
	
	long nano_seconds = (tp_end.tv_nsec - tp_start.tv_nsec);
	long seconds = (tp_end.tv_sec - tp_start.tv_sec);
	if (nano_seconds < 0) {
		nano_seconds += 1000000000;
		seconds --;
	}
	printf("%ld %ld", tp_end.tv_sec, tp_end.tv_nsec); 
	printf("Mmap: This test took  %ld.%ld seconds.\n", seconds, nano_seconds);
	
	return 0;
}

int main() {
	
	test_fopen_fread();
	test_mmap();
}