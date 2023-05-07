#include <stdio.h>
#include <stdlib.h> // for exit()
#include <unistd.h> // for pipe, getpid() and int usleep(useconds_t usec); // microsleep, int read(int fd, void *buffer, size_t bytes);
#include <fcntl.h> // for O_RDONLY
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait() or waitpid().
#include <string.h> // for strcmp()

int main(int argc, char *argv[]) {
    enum signal{ // signal codes for the PIPE
        ZERO,
        OK,
        MYEOF,
    }signals;
    enum signal pread=OK;
    /*for(int i=0;i<argc;++i) {
        printf("%s ", argv[i]);
    }*/
    int p_counter=1, stat_loc, pipefd[6][2];
    pipe(pipefd[1]); // [0]: read, [1]: write
    pipe(pipefd[2]); // [0]: read, [1]: write
    pipe(pipefd[3]); // [0]: read, [1]: write
    pipe(pipefd[4]); // [0]: read, [1]: write
    pipe(pipefd[5]); // [0]: read, [1]: write
    pid_t par_pid=0, pid=getpid(), child_pid = fork();
    while(child_pid==0 && p_counter<4) {
        par_pid=pid;
        pid=getpid();
        p_counter++;
        child_pid = fork();
        usleep(19000); // For better synchronization.
    }
    if(p_counter==4 && child_pid==0) { // 5th process.
        par_pid=pid;
        pid=getpid();
        p_counter++;
        usleep(19000); // For better synchronization.
    }
    // THIS HAS TO BE DONE AFTER THE FORK => INDEPENDENT
    FILE * fp = fopen(argv[1], "rt"); // read file
    int i;
    char newline[257];
    printf("\n%dth process, PID %d started.\nparent: %d,\nchild: %d\n", p_counter, pid, par_pid, child_pid);
    printf("Hi! I'm %dth process!\n", p_counter);
    i=p_counter;
    while(--i) {
        fgets(newline, 257, fp);
    }
    sleep(1);
    // Done ONLY ONCE by the 5th process.
    if(p_counter==5) {
        puts("\nActivate first process!\n");
        write(pipefd[1][1], &pread, sizeof(OK)); // Activate first process.
    }
    
    int flag=1;
    while(flag) {
        while(read(pipefd[p_counter][0], &pread, sizeof(pread))==0); // Waiting for new pipe reading.
        switch(pread) {
            case ZERO: // No reading is given...
                break;
            case OK:
            if(fgets(newline, 257, fp)!=NULL) {
                printf("[%d] %d: %s", p_counter, pid, newline);
                write(pipefd[p_counter%5+1][1], &pread, sizeof(OK));
                i=4; // skip 4 lines
                while(i--) fgets(newline, 257, fp);
                break;
            } else {
                printf("[%d] %d: Read all data!\n", p_counter, pid);
                pread=MYEOF;
            }
            case MYEOF:
            write(pipefd[p_counter%5+1][1], &pread, sizeof(MYEOF));
            flag=0;
        }
    }
    // Ending
    if(child_pid==0 || wait(&stat_loc)==child_pid && stat_loc==0) {
        printf("%dth process, PID: %d is exiting...\n", p_counter, pid);
        exit(0);
    }

    return 0;
}