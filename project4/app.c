/**
 * CS342 Spring 2019 - Project 4
 * A simple application programs that creates, writes and reads files.
 * @author Yusuf Dalva - 21602867
 * @author Efe Acer - 21602217
 */

// Necessary include(s)
#include <stdio.h>
#include <unistd.h>

int main() {
	printf("Application started.\n");
	
	char byte_str[] = "BYTE"; // 4 bytes	

	char* file1 = "file1.txt";
	FILE* fp1 = fopen(file1, "w+b");
	fwrite(byte_str, 1, sizeof(byte_str) - 1, fp1);
	char str_in1[sizeof(byte_str)];
	fclose(fp1); // restore the pointer
	fp1 = fopen(file1, "r"); 
	if (fgets(str_in1, sizeof(byte_str), fp1)) {
		printf("File 1 content: %s\n", str_in1);
	}	

	char* file2 = "file2.txt";
	FILE* fp2 = fopen(file2, "w+b");
	char str2[] = "This is the second file created by the application.";
	for (int i = 0; i < 1000 * 5; i++) { // will fill up 5 pages
		fwrite(byte_str, 1, sizeof(byte_str) - 1, fp2);
	}
	fwrite(str2, 1, sizeof(byte_str) - 1, fp2);
	char str_in2[sizeof(byte_str) * 1000 * 5];
	fclose(fp2); // restore the pointer
	fp2 = fopen(file2, "r"); 
	if (fgets(str_in2, sizeof(byte_str) * 1000 * 5, fp2)) {
		printf("File 2 content: %s\n", str_in2);
	}

	char* file3 = "file3.txt";
	FILE* fp3 = fopen(file3, "w+b");
	for (int i = 0; i < 1000 * 10; i++) { // will fill up 10 pages
		fwrite(byte_str, 1, sizeof(byte_str) - 1, fp3);
	}
	char str_in3[sizeof(byte_str) * 1000 * 10];
	fclose(fp3); // restore the pointer
	fp3 = fopen(file3, "r"); 
	if (fgets(str_in3, sizeof(byte_str) * 1000 * 10, fp3) != NULL) {
		printf("File 3 content: %s\n", str_in3);
	}

	printf("process identifier: %d\n", getpid());
	printf("Application is infinite looping...(terminate yourself)\n");
	while(1);
	return 0;
}

