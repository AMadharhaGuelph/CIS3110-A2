#ifndef HEADERFILE_H
#define HEADERFILE_H

/* INCLUDES */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/* CONSTANTS */
#define BUFFER_SIZE 1024
#define MAX_FILES 100
#define MAX_ALPHABETS 26

/* FUNCTION PROTOTYPES */
void sigchld_handler(int sig);
int *calculateHistogram(char *fileData, int fileSize);

#endif