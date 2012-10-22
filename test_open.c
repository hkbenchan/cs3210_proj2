#include <stdio.h>

int main() {
	
	FILE *myfile;
    char tempstring[1024];

    if(!(myfile=fopen("~/source_code/cs3210_proj2/README.md","r")))
    {
         fprintf(stderr,"Could not open file");
         return -1;
    }

    while(!feof(myfile))
    {
         fscanf(myfile,"%s",tempstring);
         printf("%s",tempstring);
    }
	
	return 0;
}
