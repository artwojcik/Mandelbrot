/*
 * mandelCalc-awojci5.cpp
 *
 *
 * Artur Wojcik
 * NetID: awojci5
 * CS 361
 * UIC Fall 2017
 *
 * Mandelbrot program to create "pictures" of fractal.
 * Program use fork to create to separate processes that comunicats with parent through
 * pipes and message queues and shared memory.
 * If any problem occure then program terminates processes at exits
 * This program process the inpput from user and saves to the shared memory that was created
 * by parent so the mandelDisplay can output on the screen
 *
 *
 */
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/msg.h>

using namespace std;


struct msgQueue {
    long num;
    char msg[256];
};
int numPictures = 0;

void sig_handler(int status) {
    exit(numPictures);
}


int main(int argc, char **argv) {

    //stdin variables scan for user input
    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;
//exit(-69);

    int shmID;
    int msgqID;
    int *data;  //attached shared
    msgQueue Q1;

    Q1.num = 1;


    //set dignal
    signal(SIGUSR1, sig_handler);



    //command line arguments if not more or less exit
    if (argc < 3 || argc >= 4) {
        perror("Arguments\n");
        exit(-11);
    }

    //parse command line argument if negative exit
    shmID = atoi(argv[1]);
    if (shmID < 0) {
        perror("argv[1]");
        exit(-12);//190155386
    }
    msgqID = atoi(argv[2]);
    if (msgqID < 0) {
        perror("argv[2]");
        exit(-13);
    }


    //should go to the child cpp
    data = (int *) shmat(shmID, NULL, 0);

    if (data == (int *) -1) {
        perror("shmat");
        exit(-14);
    }


    while (true) {


        fscanf(stdin, "%lf %lf %lf %lf %d %d %d", &xMin, &xMax, &yMin, &yMax, &maxIters, &nRows, &nCols);
        fflush(stdin);

        //mandelrbot agorithm 
        double deltaX = (xMax - xMin) / (nCols - 1);
        double deltaY = (yMax - yMin) / (nRows - 1);
        for (int r = 0; r < nRows; ++r) {

            double Cy = yMin + r * deltaY;
            for (int c = 0; c < nCols; ++c) {

                double Cx = xMin + c * deltaX;
                double Zx = 0.0;
                double Zy = 0.0;
                int n;
                for (n = 0; n < maxIters; n++) {
                    if (Zx * Zx + Zy * Zy >= 4.0) {
                        break;
                    }
                    double Zx_next = Zx * Zx - Zy * Zy + Cx;
                    double Zy_next = 2.0 * Zx * Zy + Cy;
                    Zx = Zx_next;
                    Zy = Zy_next;
                }
                if (n >= maxIters) {
                    *(data + r * nCols + c) = -1;
                } else {
                    *(data + r * nCols + c) = n;
                }
            }
        }

        numPictures++;

        fprintf(stdout, "%lf %lf %lf %lf %d %d %d\n", xMin, xMax, yMin, yMax, maxIters, nRows, nCols);

        fflush(stdin);
        fflush(stdout);

        //send msg to main
        //int msgsnd(int msqid, const void *msgp, size_t msgsz,int msgflg);
        strcpy(Q1.msg, "done CALC!!!");

        int len = sizeof(Q1) - sizeof(long);//use send and receive change every where

        if (msgsnd(msgqID, &Q1, len, 0) < 0) {
            perror("snd invalid in calc");
            exit(-15);
        }


    }

    return numPictures;
}

//______________________________________________________________________________________________________________________

