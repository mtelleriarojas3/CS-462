#include "includes.h"


//PRINT MD5
void printMD5(char *fileName){
	string file = fileName;
	string md5file = "md5sum "+file;
	const char *actualmd5 = md5file.c_str();
	
	cout<<"\nMD5:\n";

	system(actualmd5);
	cout<<"\n";
}

//GET FILE
FILE *get_file(char *fileName) {
    const char *method = "rb";

    FILE *fp = NULL;
    while (!strlen(fileName) || !fp) {        
        fp = fopen(fileName, method);
    }
    return fp;
}

//GET FILE SIZE
int get_file_Size (FILE *fp) {
    
    if (fseek(fp, 0, SEEK_END)) {
        printf("Error: Unable to find end of file\n");
        exit(1);
    }
    int size = ftell(fp);
    //printf("File size: %li (bytes)\n", size);
    // Back to beginning of file;
    fseek(fp, 0, SEEK_SET);
    return size;
}
