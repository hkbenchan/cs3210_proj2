#include <stdio.h>
#include <time.h>
#include <iostream>

int main() {
	
	test_fopen_fread();
	test_mmap();
}

int test_fopen_fread(){
	FILE *myfile;
    
    struct timespec tp_start;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_start) < 0){
		printf("clock_gettime() failed\n");
	}

    if(!(myfile=fopen("/nethome/hchan35/source_code/cs3210_proj2/README.md","r")))
    {
         fprintf(stderr,"Could not open file\n");
         return -1;
    }

    const char *file2 = "/nethome/hchan35/source_code/cs3210_proj2/README(COPY).md";
    
    if(!copyFile(myFile, file2)){
		fprintf(stderr,"Error copying file\n");
	}
	
	struct timespec tp_end;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_end) < 0){
		printf("clock_gettime() failed\n");
	}
	struct timespec tp_diff = diff(timespec tp_start, timespec tp_end);
	std::cout << "This test took " << tp_diff.tv_sec << " seconds" << endl;
	
	return 0;
}

int test_mmap(){
    
    struct timespec tp_start;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_start) < 0){
		printf("clock_gettime() failed\n");
	}
	
	//implement the mmap
   	
	struct timespec tp_end;
	if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp_end) < 0){
		printf("clock_gettime() failed\n");
	}
	struct timespec tp_diff = diff(timespec start, timespec end);
	std::cout << "This test took " << tp_diff.tv_sec << " seconds" << endl;
	
	return 0;
}

int copyFile(const char *o_file, const char *c_file){
	  
	  char ch;
	  FILE *p,*q;
	  p=fopen(o_file,"r");
	  if(p==NULL){
		  printf("cannot open %s",o_file);
		  exit(0);
	  }
	  q=fopen(c_file,"w");
	  if(q==NULL){
		  printf("cannot open %s",file2);
		  exit(0);
	  }
	  while((ch=getc(p))!=EOF)
		  putc(ch,q);
	  printf("\nCOMPLETED");
	  fclose(p);
	  fclose(q);
	 return 0;
}
