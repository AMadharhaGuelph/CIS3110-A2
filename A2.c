#include "header.h"

/* GLOBAL VARIABLES */
int pipes[MAX_FILES][2];
int pids[MAX_FILES];
int numChildren = 0;
int numTerminated = 0;

int main(int argc, char *argv[]) {
    // Error check: no file arguments
    if(argc == 1) {
        printf("Error: No input files provided.\n");
        exit(EXIT_FAILURE);
    }

    // Error check: too many file arguments
    if(argc > MAX_FILES) {
        printf("Error: Too many input files provided.\n");
        exit(EXIT_FAILURE);
    }

    // Register a signal handler for SIGCHLD and error check
    if(signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        printf("Error: Registering SIGCHLD.\n");
        exit(EXIT_FAILURE);
    }
   
    // Loop through files
    int pid;
    for(int i = 0; i < (argc-1); i++) {
        // Create pipe and error check
        int pipeResponse = pipe(pipes[i]);
        if(pipeResponse < 0) {
            printf("Error: Creating pipe.\n");
            exit(EXIT_FAILURE);
        }

        // Fork child process and 
        pid = fork();
        if(pid == -1) {     // Error check
            printf("Error: fork().\n");
            exit(EXIT_FAILURE);
        } 
        else if(pid == 0) {     // Child process
            // Check SIG case
            close(pipes[i][0]);
            if(strcmp(argv[i+1], "SIG") != 0) {
                int *histogram;
                int bytes;
                int fileSize;
                int fileDescriptor;
                char *fileData;

                // Read file into memory 
                fileDescriptor = open(argv[i+1], O_RDONLY);
                
                if(fileDescriptor < 0) {
                    printf("Error: Opening file.\n");
                    close(pipes[i][1]);
                    exit(EXIT_FAILURE);
                }

                fileSize = lseek(fileDescriptor, 0, SEEK_END);
                fileData = malloc(fileSize);
                lseek(fileDescriptor, 0, SEEK_SET);

                while((bytes = read(fileDescriptor, fileData, fileSize)) > 0) {
                    if(bytes < 0) {
                        printf("Error while reading file.\n");
                    }
                }
                close(fileDescriptor);

                // Calculate Histogram and write Histogram to pipe
                histogram = calculateHistogram(fileData, fileSize);
                write(pipes[i][1], histogram, MAX_ALPHABETS * sizeof(int));
                free(histogram);
                printf("Child %d successfully written %s\'s histogram to parent using pipe %d.\n", getpid(), argv[i+1], i);

                // Child goes to sleep
                printf("Child %d going to sleep for %d seconds.\n", getpid(), (10 + 2*i));
                sleep(10 + 2*i);

                // Close write end of pipe
                close(pipes[i][1]);
                free(fileData);

                exit(EXIT_SUCCESS);
            }
        }
        else {
            close(pipes[i][1]);
            numChildren++;
            pids[i] = pid;
            if(strcmp(argv[i+1], "SIG") == 0) {
                kill(pid, SIGINT);
            }
        }        
    }

    while(numTerminated < numChildren) {
        sleep(1);
    }

    return 0;
}

void sigchld_handler(int sig) {
    int child_status;
    int characterCounts[MAX_ALPHABETS];
    char filename[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    pid_t child_pid;

    while((child_pid = waitpid(-1, &child_status, WNOHANG)) > 0) {
        printf("Parent caught SIGCHLD from child process %d.\n", child_pid);
        numTerminated++;

        if(WIFEXITED(child_status)) {     // Exited normally
            // Find correct pipe
            int curPipe;
            for(int i = 0; i < MAX_FILES; i++) {
                if(child_pid == pids[i]) {
                    curPipe = i;
                }
            }

            // Read histogram from pipe into memory 
            read(pipes[curPipe][0], characterCounts, sizeof(int) * MAX_ALPHABETS);
            close(pipes[curPipe][0]);
            printf("Parent read histogram from pipe %d ", curPipe);
            curPipe++;

            // Open file to write to
            sprintf(filename, "file%d.hist", child_pid);
            printf("and saved to file %s.\n", filename);
            int fd = open(filename, O_CREAT | O_WRONLY, 0644);
            
            if(fd == -1) {
                printf("Error: cannot open file %s.\n", filename);
                exit(EXIT_FAILURE);
            }

            // Write character counts to file
            for (char letter = 'a'; letter <= 'z'; letter++) {
                sprintf(line, "%c=%d\n", (char) letter, characterCounts[letter-'a']); 
                write(fd, line, strlen(line));
            } 
            close(fd);

            printf("Child %d terminated normally.\n", child_pid);
        } 
        else if(WIFSIGNALED(child_status)) { 
            printf("Child %d terminated abnormally.\n", child_pid);
        }
    }

    // Reinstall hanlder
    if(signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        printf("Error: Registering SIGCHLD.\n");
        exit(EXIT_FAILURE);
    }
}

int *calculateHistogram(char *fileData, int fileSize) {
    // Calculate Histogram
    int *histogram;
    char c;

    histogram = calloc(MAX_ALPHABETS, sizeof(int));

    for(int i = 0; i < fileSize; i++) {
        c = fileData[i];

        if(isalpha(c)) {
            printf("%c", c);
            c = tolower(fileData[i]);
            histogram[c-'a']++;
        }
    }
    printf("\n");

    return histogram;
}