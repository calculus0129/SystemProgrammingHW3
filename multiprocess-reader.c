#include <stdio.h>
#include <stdlib.h> // for exit()
#include <unistd.h> // for pipe, getpid() and int usleep(useconds_t usec); // microsleep, int read(int fd, void *buffer, size_t bytes);
//#include <fcntl.h> // for O_RDONLY
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait() (or waitpid()).
//#include <string.h> // for strcmp()

int main(int argc, char *argv[]) {
    enum signal{ // signal codes for the PIPE. OK: previous processes succeeded in reading. MYEOF: previous process got EOF.
        OK,
        MYEOF,
    };
    enum signal pread=OK;
    int p_counter=1, stat_loc, pipefd[6][2]; // p_counter indicate the n of the nth process.
    // Make 5 pipes to activate/inactivate each 5 process.
    pipe(pipefd[1]); // [0]: read, [1]: write
    pipe(pipefd[2]); // [0]: read, [1]: write
    pipe(pipefd[3]); // [0]: read, [1]: write
    pipe(pipefd[4]); // [0]: read, [1]: write
    pipe(pipefd[5]); // [0]: read, [1]: write
    pid_t par_pid=0, pid=getpid(), child_pid = fork();
    while(child_pid==0 && p_counter<4) { // if it's a child process born from p_counter-th process.
        par_pid=pid;
        pid=getpid();
        p_counter++;
        child_pid = fork();
        usleep(1000); // For better synchronization.
    }
    if(p_counter==4 && child_pid==0) { // Child of 4th process, the 5th process.
        par_pid=pid;
        pid=getpid();
        p_counter++;
        usleep(1000); // For better synchronization.
    }
    // FILE pointer initialization has to be done AFTER the fork
    // => file pointer now becomes independent of the other process.
    FILE * fp = fopen(argv[1], "rt");
    int i;
    char newline[258]; // Assumed that a newline would be at most 256 letters.
    // Process starting message.
    printf("\n%dth process, PID %d started.\nparent: %d,\nchild: %d\n", p_counter, pid, par_pid, child_pid);
    printf("Hi! I'm %dth process!\n", p_counter);
    i=p_counter;
    // Initially read (p_counter-1) lines.
    // e.g. p_counter=5 => Initially read 4 lines.
    while(--i) {
        fgets(newline, 258, fp);
    }
    // Wait until all the starting messages appear.
    sleep(1);

    // Done ONLY ONCE by the 5th process.
    if(p_counter==5) {
        puts("\nActivate first process!\n");
        write(pipefd[1][1], &pread, sizeof(OK)); // Activate first process.
    }
    
    // Flag for continuing the reading.
    int flag=1;
    while(flag) {
        while(read(pipefd[p_counter][0], &pread, sizeof(pread))==0); // Waiting for new pipe reading.
        switch(pread) {
            case OK: // Prior process could read the data.
            if(fgets(newline, 258, fp)!=NULL) { // succeed in reading the next line.
                printf("[%d] %d: %s", p_counter, pid, newline);
                write(pipefd[p_counter%5+1][1], &pread, sizeof(OK));
                i=4; // skip the next 4 lines.
                while(i--) fgets(newline, 258, fp);
                break;
            } else { // current fseek position is EOF.
                printf("[%d] %d: Read all data!\n", p_counter, pid);
                pread=MYEOF;
            }
            case MYEOF: // Prior process could not read the data.
            write(pipefd[p_counter%5+1][1], &pread, sizeof(MYEOF));
            flag=0;
        }
    }
    // Ending. child_pid==0 only for the 5th process.
    // Therefore the 5th process exits first, and then the parents exit subsequentially.
    if(child_pid==0 || wait(&stat_loc)==child_pid && stat_loc==0) {
        printf("%dth process, PID: %d is exiting...\n", p_counter, pid);
        exit(0);
    }

    return 0;
}