#include <stdio.h>
#include <stdlib.h> // for exit()
#include <unistd.h> // for pipe, getpid() and int usleep(useconds_t usec); // microsleep
#include <fcntl.h> // for O_RDONLY
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait() or waitpid().
#include <string.h> // for strcmp()

int main(int argc, char *argv[]) {
    for(int i=0;i<argc;++i) {
        printf("%s ", argv[i]);
    }
    int fd = open(argv[1], O_RDONLY);
    if(fd==-1) {
        perror("File open error");
        return 0;
    }
    int p_counter=1, stat_loc, pipefd[2];
    pipe(pipefd); // [0]: read, [1]: write
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
    int pc_read;
    write(pipefd[1], &p_counter, sizeof(p_counter));
    sleep(1);
    do {
        read(pipefd[0], &pc_read, sizeof(pc_read));
        lseek(pipefd[0], -sizeof(pc_read), SEEK_CUR);
    } while(p_counter != pc_read);
    printf("%dth process, PID %d started.\nparent: %d,\nchild: %d\n", p_counter, pid, par_pid, child_pid);
    printf("Hi! I'm %dth process!\n", p_counter);
    
    sleep(1); // Intention: each process sleeps for 1s. Actual: as soon as the lower lseek once occurs, the read() occurs simultaneously => all get initiated at once.
    lseek(pipefd[0], sizeof(pc_read), SEEK_CUR);

    // Ending'
    if(child_pid==0 || wait(&stat_loc)==child_pid && stat_loc==0) {
        printf("%dth process, PID: %d is exiting...\n", p_counter, pid);
        exit(0);
    }

    return 0;
}