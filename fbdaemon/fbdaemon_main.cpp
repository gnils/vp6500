#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "camera.hpp"
#include "rpr_redirect.hpp"
#include "fbwrapper.hpp"
#include "log.hpp"

void ex_program(int sig);
void cleanup();

FramebufferWrapper* fbWrapper = NULL;

int main(int argc, const char* argv[])
{
    signal(SIGINT, ex_program); // register clean shutdown function
    logDebug("starting fbdaemon");

    fbWrapper = new FramebufferWrapper();

    unsigned short* fb = fbWrapper->open();

    fbWrapper->backup();

    unsigned int x,y;

    bool binaryMode = true;

    if(argc == 2 && strcmp("--debug", argv[1]) == 0) {
        binaryMode = false;
    }

/*
    for(x=0; x<FramebufferWrapper::FRAMEBUFFER_WIDTH; x++) {
        for(y=0; y<FramebufferWrapper::FRAMEBUFFER_HEIGHT; y++) {
            short pixel = fb[y*FramebufferWrapper::FRAMEBUFFER_HEIGHT+x];
            logInfo("pixel at [%d, %d] is %04X", x,y,pixel);
        }
    }
*/
    if(binaryMode) {
        time_t start = time(NULL);
        const int runs = 100;

        //for(int c=0; c<runs; c++) {
        while(true) {
            for(int y = 0; y < FramebufferWrapper::FRAMEBUFFER_HEIGHT; y++) {
                fwrite(&fb[FramebufferWrapper::FRAMEBUFFER_WIDTH * y], 2, FramebufferWrapper::FRAMEBUFFER_WIDTH, stdout);
            }
        }

        logInfo("%d runs in %d seconds", runs, time(NULL) - start);
    } else {
        while(true) {
            for(int i = 0; i < FramebufferWrapper::FRAMEBUFFER_WIDTH; i++) {
                for(int j = 0; j < FramebufferWrapper::FRAMEBUFFER_HEIGHT; j++) {
                    printf("%04x", fb[FramebufferWrapper::FRAMEBUFFER_WIDTH*j + i]);
                }
            }
        }
    }


    cleanup();
    return 0;
}

void cleanup() {
    fbWrapper->restore();
    fbWrapper->close();

    delete fbWrapper;
    fbWrapper = NULL;

    logDebug("exiting fbdaemon");
    exit(0);
}

void ex_program(int sig) {
    logWarn("Catched signal: %d. Cleaning up ...", sig);
    signal(SIGINT, SIG_DFL);
    cleanup();
}

