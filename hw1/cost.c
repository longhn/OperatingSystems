/**
 * This program performs some timing experiments on some of the commonly
 * used Linux system calls. It executes the system calls with the specified 
 * arguments and reports the execution time in microseconds.
 * @author Efe Acer
 * @version 1.0
 */
 
// Necessary imports to be able to run the system calls
#include <sys/time.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/stat.h>
#include <stdio.h>

// Function decleration(s)
unsigned long getCurrentTime();

int main() {
    printf("\nTime measurements for different system calls in microseconds follows:\n");
    
    // Preparation for getpid() measurements
    unsigned long getpidStart;
    unsigned long getpidEnd;
    int processID;

    // Get process ID
    getpidStart = getCurrentTime();
    processID = getpid();
    getpidEnd = getCurrentTime();
    printf("\nTime to execute getpid(): %ld\n", getpidEnd - getpidStart);
    printf("Process ID is: %d\n", processID);

    // Preparation for open() measurements
    int fileDescriptor;
    unsigned long openStart;
    unsigned long openEnd;

    // Create a .txt file that will be used to read bytes from
    openStart = getCurrentTime();
    fileDescriptor = open("read.txt", O_CREAT | O_RDWR | O_APPEND,  00700); // 00700 is for file owner permissions
    openEnd = getCurrentTime();
    printf("\nTime to execute open() to create a new .txt file is: %ld\n", openEnd - openStart);
    printf("The value of fileDescriptor is %d:\n", fileDescriptor); 
    
    // Preparation for write() measurements
    unsigned char bytes[100000];
    for (int i = 0; i < 100000; i++) {
        bytes[i] = '.';	
    }
    unsigned long writeStart;
    unsigned long writeEnd;
    int numWritten;

    // Write 100 bytes to read.txt
    writeStart = getCurrentTime();
    numWritten = write(fileDescriptor, &bytes, 100);
    writeEnd = getCurrentTime();
    printf("\nTime to execute write() for %d bytes: %ld\n", numWritten, (writeEnd - writeStart));

    // Write 1000 bytes to read.txt
    writeStart = getCurrentTime();
    numWritten = write(fileDescriptor, &bytes, 1000);
    writeEnd = getCurrentTime();
    printf("Time to execute write() for %d bytes: %ld\n", numWritten, (writeEnd - writeStart));

    // Write 10000 bytes to read.txt
    writeStart = getCurrentTime();
    numWritten = write(fileDescriptor, &bytes, 10000);
    writeEnd = getCurrentTime();
    printf("Time to execute write() for %d bytes: %ld\n", numWritten, (writeEnd - writeStart));

    // Write 100000 bytes to read.txt
    writeStart= getCurrentTime();
    numWritten = write(fileDescriptor, &bytes, 100000);
    writeEnd = getCurrentTime();
    printf("Time to execute write() for %d bytes: %ld\n", numWritten, (writeEnd - writeStart));
    
    // Restore the pointer that is used to read and write to the file
    close(fileDescriptor);
    fileDescriptor = open("read.txt", O_RDWR, 00700); // 00700 is for file owner permissions

    // Preparation for read() measurements
    unsigned char buffer[111100];
    unsigned long readStart;
    unsigned long readEnd;
    int numRead;	
	
    // Read 100 bytes from read.txt
    readStart = getCurrentTime();
    numRead = read(fileDescriptor, &buffer, 100);
    readEnd = getCurrentTime();
    printf("\nTime to execute read() for %d bytes: %ld\n", numRead, (readEnd - readStart));

    // Read 1000 bytes from read.txt
    readStart = getCurrentTime();
    numRead = read(fileDescriptor, &buffer, 1000);
    readEnd = getCurrentTime();
    printf("Time to execute read() for %d bytes: %ld\n", numRead, (readEnd - readStart));

    // Read 10000 bytes from read.txt
    readStart = getCurrentTime();
    numRead = read(fileDescriptor, &buffer, 10000);
    readEnd = getCurrentTime();
    printf("Time to execute read() for %d bytes: %ld\n", numRead, (readEnd - readStart));

    // Read 100000 bytes from read.txt
    readStart = getCurrentTime();
    numRead = read(fileDescriptor, &buffer, 100000);
    readEnd = getCurrentTime();
    printf("Time to execute read() for %d bytes: %ld\n", numRead, (readEnd - readStart));

    // Preparation for mkdir() measurements
    unsigned long mkdirStart;
    unsigned long mkdirEnd;
    int success;

    // Make a directory with all access permissions given
    mkdirStart = getCurrentTime();
    success = mkdir("allAccessPermissionsGiven", ACCESSPERMS);
    mkdirEnd = getCurrentTime();
    printf("\nTime to execute mkdir() giving all access permissions: %ld\n", mkdirEnd - mkdirStart);
    printf("Successful creation (0 is OK if -1 the directory probably exists): %d\n", success);
    
    // Make a directory with only read permissions given
    mkdirStart = getCurrentTime();
    success = mkdir("onlyReadPermissionsGiven", S_IRUSR | S_IRGRP | S_IROTH);
    mkdirEnd = getCurrentTime();
    printf("Time to execute mkdir() giving only read permissions: %ld\n", mkdirEnd - mkdirStart);
    printf("Successful creation (0 is OK if -1 the directory probably exists): %d\n", success);    

    return 0;
}

/**
 * Returns the current time in microseconds. The current 
 * time corresponds to the time elapsed from the starting 
 * point used by the gettimeofday() function.
 * @return currentTime: The current time in microseconds
 */
unsigned long getCurrentTime() {
    struct timeval timeValue;
    gettimeofday(&timeValue, NULL);
    unsigned long currentTime = timeValue.tv_usec; // microseconds part of the struct
    currentTime += timeValue.tv_sec * 1e6; // add the seconds part of the struct
    return currentTime;
}
