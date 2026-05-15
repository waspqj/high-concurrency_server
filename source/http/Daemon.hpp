#pragma once 

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

namespace DaemonModule
{
    #define DEV_NULL "/dev/null"
    #define ROOT "."

    void Daemon(int ischdr, int isclose)
    {   
        signal(SIGCHLD, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        
        if(fork() > 0)
            exit(1);
        
        setsid();

        if(ischdr)
            chdir(ROOT);
        
        if(isclose)
        {
            close(0);
            close(1);
            close(2);
        }
        else{
            int fd = open(DEV_NULL, O_RDWR);
            if(fd > 0)
            {
                dup2(fd, 0);
                dup2(fd, 1);
                dup2(fd, 2);
                close(fd);
            }
            
        }
    }
}
