#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>

#include "becomedaemon.h"

int become_daemon()
{
    switch(fork()){
        case -1: return -1;
        case 0: break;
        default: _exit(EXIT_SUCCESS);
    }

    if(setsid() == -1)
        return -1;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);    

    switch(fork()){
        case -1: return -1;
        case 0: break;
        default: _exit(EXIT_SUCCESS);
    }


    return 0;
}