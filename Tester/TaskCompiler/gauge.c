#include <stdio.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <assert.h>
#include <math.h>
#include <time.h>

void die(const char* msg)
{
    fprintf(stderr, "-1\nError: %s\n", msg);
    exit(1);
}

const char* sig_text[] = {
    "Success",
    "Hangup",
    "Interrupted",
    "Quit",
    "ILL",
    "TRAP",
    "Aborted",
    "7",
    "Floating Point Exception",
    "Killed",
    "10",
    "Segmentation Fault"
};

#define ARG_PROC 1
#define ARG_TIME_LIMIT 2
#define ARG_MEM_LIMIT 3

int main(int argc, char* argv[])
{
    FILE *proc, *ofile;
    int pid, mem_limit;
    float time_limit;
    char old_dir[256];
    char* temp_dir;
    
    if (argc != 4)
        die("Incorrect number of arguments.");
    
    sscanf(argv[ARG_TIME_LIMIT], "%f", &time_limit);
    sscanf(argv[ARG_MEM_LIMIT], "%d", &mem_limit);

    // change to a temp directory
    temp_dir = tempnam("/tmp", "gauge");
    if (!temp_dir || mkdir(temp_dir, 0700))
        die("Failed to create temporary directory.");

    if (!getcwd(old_dir, 256) || chdir(temp_dir) == -1)
        die("Failed to change to temporary directory.");

    pid = fork();
    if (pid == -1) {
        die("Forking failed!");
    }
    
    if (pid == 0) {
        char command[256];
        struct rlimit rlim;

        // time limit
        rlim.rlim_cur = (int)ceil(time_limit+0.1);
        rlim.rlim_max = (int)ceil(time_limit+0.1);
        assert(setrlimit(RLIMIT_CPU, &rlim) != -1);
        
        // memory limit
        rlim.rlim_cur = mem_limit*1024*1024;
        rlim.rlim_max = mem_limit*1024*1024;
        assert(setrlimit(RLIMIT_AS, &rlim) != -1);

        if (argv[ARG_PROC][0] == '/')
            strcpy(command, argv[ARG_PROC]);
        else
            sprintf(command, "%s/%s", old_dir, argv[ARG_PROC]);
        
		if (execl(command, argv[ARG_PROC], 0) == -1) {
			perror("execl");
			exit(1);
		}
    } else {
        int status, ret, max_real_secs = (int)ceil(time_limit)*2;
        long ticks_per_sec = sysconf(_SC_CLK_TCK);
        struct tms t;
        time_t start = time(0);
        
        while ((ret = waitpid(pid, &status, WNOHANG)) == 0) {
            if ((time(0) - start) > max_real_secs)
                die("Killed because it seemed to have hung.");
            usleep(500000);
        }

        if (ret < 0)
            die("Failed to wait for child process.");

        // restore working dir & delete temp dir
        chdir(old_dir);
        rmdir(temp_dir);
        
        times(&t);
        fprintf(stderr, "%.2f\n", ((float)(t.tms_cutime + t.tms_cstime))/ticks_per_sec);
        
        if ((float)(t.tms_cutime + t.tms_cstime) > time_limit*ticks_per_sec) {
            fprintf(stderr, "Exceeded time limit of %.2f seconds.\n", time_limit);
            return 1;
        }
        
        if (WIFEXITED(status))
            fprintf(stderr, "success\n");
        else {
            if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);
                
                if (sig < 0 || sig > 11)
                    fprintf(stderr, "Terminated (%d).\n", sig);
                else {
                    fprintf(stderr, "Terminated (%s).\n", sig_text[sig]);
                    /*if (sig == 9) {
                        fprintf(stderr, "Hit hard memory usage limit of %d MegaBytes.\n", mem_limit);
                    }*/
                }
            }
            else
                fprintf(stderr, "Unknown error (%d)!\n", status);
            return 1;
        }
    }
    
    return 0;
}
