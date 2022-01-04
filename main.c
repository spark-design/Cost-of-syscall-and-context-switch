// Name: Connor Sparkman
// netid: cps260
// 12/25/2021
// https://www.tutorialspoint.com/c_standard_library/time_h.htm
// https://www.usenix.org/legacy/events/expcs07/papers/2-li.pdf
// https://www.tutorialspoint.com/what-is-context-switching-in-operating-system
// https://man7.org/linux/man-pages/man2/sched_setaffinity.2.html
// https://man7.org/linux/man-pages/man0/sched.h.0p.html
// https://linuxhint.com/gettimeofday_c_language/
// https://man7.org/linux/man-pages/man2/settimeofday.2.html
// https://stackoverflow.com/questions/12214878/what-does-fd-represent-when-typing-int-fd-openfile
// https://pubs.opengroup.org/onlinepubs/7908799/xsh/systime.h.html
// https://www.gnu.org/software/libc/manual/html_node/CPU-Affinity.html
#define _GNU_SOURCE // Needed for cpu_set. GNU mentioned in 2.7
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> // used for gettimeofday
#include <sched.h> // pid_t
#include <unistd.h>
#include <fcntl.h> // figure 2.6

int main(int argc, char *argv[]) { // Saw argc and argv in fork file on canvas. Reviewed: http://crasseux.com/books/ctutorial/argc-and-argv.html
    // syscall 
    int fd = open("./syscall_contextswitch.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // Open introduced in 2.3. code taken from here. fd is file descriptor
    int times = 1000000;							            //  Info on open and its' flags: https://man7.org/linux/man-pages/man2/open.2.html
    struct timeval begin, terminate;									     
    gettimeofday(&begin, NULL); // gettimeofday recommended by textbook
    for (int j = 0; j < times; j++) 
    { read(fd, NULL, 0); }
    gettimeofday(&terminate, NULL);
    
    // cost of system call calculation
    printf("Cost of system call: %f ms\n", (float) (terminate.tv_sec * 1000000 + terminate.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec) / times); 
    // Found a solution on stack overflow to convert seconds to milliseconds and while I did not use the code, I accepted the idea and created solution using the times variable
    // link to solution looked at is found here: https://stackoverflow.com/questions/3756323/how-to-get-the-current-time-in-milliseconds-from-c-in-linux
    /* The tv argument is a struct timeval. tv_sec is number of seconds since epoch. tv.usec is additional microseconds after number of seconds since the epoch */
    
    close(fd);



    cpu_set_t cpu; // Info on these 3 macros along with sched_setaffinity available here: https://www.gnu.org/software/libc/manual/html_node/CPU-Affinity.html
    CPU_ZERO(&cpu);
    CPU_SET(0, &cpu);
    // contextswitch
    // Pipe code below modified from pipe files posted on canvas chapter 5 Interlude
    int pipe_1fd[2], pipe_2fd[2];  // check 1st and 2nd pipes
    if (pipe(pipe_1fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_2fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t rc = fork(); // pid_t from sched.h

    if (rc == -1) {
	fprintf(stderr, "fork failed\n");
	exit(1);
    } else if (rc == 0) { // child (new process)
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpu) == -1) { // sizeof first introduced in 2.1 and further explain in chapter 14 (popular in c++)
            exit(EXIT_FAILURE);
        }
	// read first pipe and write second
        for (int j = 0; j < times; j++) {
            read(pipe_1fd[0], NULL, 0);
            write(pipe_2fd[1], NULL, 0);
        }
    } else {  // parent goes down this path (main)
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpu) == -1) {
            exit(EXIT_FAILURE);
        }
	// gettimeofday at beginning
        gettimeofday(&begin, NULL);
        for (int j = 0; j < times; j++) {
        // write first pipe and read second
            write(pipe_1fd[1], NULL, 0);
            read(pipe_2fd[0], NULL, 0);
        }
        gettimeofday(&terminate, NULL);
        // gettimeofday at the termination
        
        // cost of context switch calculation. Explained for cost of system call.
        printf("Cost of context switch: %f ms\n", (float) (terminate.tv_sec * 1000000 + terminate.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec) / times);
    }
    return 0;
}
