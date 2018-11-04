/*
 * mandelDisplay-awojci5.cpp
 *
 * Artur Wojcik
 * NetID: awojci5
 * CS 361
 * UIC Fall 2017
 *
 *
 * Mandelbrot program to create "pictures" of fractal.
 * Program use fork to create to separate processes that comunicates with parent through
 * pipes and message queues and shared memory.
 * If any problem occure then program terminates processes at exits
 * This program process the output that was enter by user in mandelbrot prints info from
 * that was stored into shared memory by mandelCalc display output
 *
 *
 */


#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <iomanip>

using namespace std;

const int nColors = 15;
const char colors[nColors] = {'.', '-', '~', ':', '+',
                              '*', '%', 'O', '8', '&',
                              '?', '$', '@', '#', 'X'};//, where nColors = 15

struct msgQueue {
  long int num;
  char msg[256];
};

const int LENGTH = sizeof(msgQueue) - sizeof(long);

int numPictures = 0;

void sig_handler(int status) {
    exit(numPictures);
}

//prints numbers on the side and spaces 
void printSideSpace(int nRows, double yMax, double yMin, int r ){
    if(r==nRows-1){
        cout << endl<< endl;
        cout << setw(7) << fixed << right << setprecision(2) << yMax; 
    }
    else if (r==0) {
        cout << setw(7) << fixed << right << setprecision(2)<< yMin;
    }
    else{
        cout <<setw(7) << fixed << right << " ";
    }
}

//prints numbers below the picture
void printBottomSpace(int nCols, double xMin, double xMax){
    cout << setw(7) << fixed << right <<" ";
    cout << setw(7) << fixed << left << xMin ; 
    cout << setw(nCols -7) << fixed << right << xMax;
    cout << endl; 
}


int main(int argc, char **argv) {

    //stdin variables scan for user input
    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;

//exit(-67);
    int shmID;
    int msgqID;
    int msgqID2;
    int *data;  //attached shared meory
    FILE *file;

    msgQueue Q1;
    msgQueue Q2;
    Q1.num = 1;
    Q2.num = 1;


    //set dignal
    signal(SIGUSR1, sig_handler);



    //command line arguments if not more or less exit
    if (argc <= 3 || argc >= 5) {
        perror("Arguments\n");
        exit(-21);
    }

    //parse command line argument if negative exit
    shmID = atoi(argv[1]);
    if (shmID < 0) {
        perror("argv[1]");
        exit(-22);
    }
    msgqID = atoi(argv[2]);
    if (msgqID < 0) {
        perror("argv[2]");
        exit(-23);
    }
    msgqID2 = atoi(argv[3]);
    if (msgqID2 < 0) {
        perror("argv[3]");
        exit(-24);
    }

    //should go to the child cpp
    data = (int *) shmat(shmID, NULL, 0);

    if (data == (int *) -1) {
        perror("shmat");
        exit(-25);
    }


    while (true) {

        //a. read xMin, xMax, yMin, yMax, nRows, nCols, and maxIters from stdin
        fscanf(stdin, "%lf %lf %lf %lf %d %d %d", &xMin, &xMax, &yMin, &yMax, &maxIters, &nRows, &nCols);
        fflush(stdout);
        fflush(stdin);


        while (true) {
            //b. Read filename from message queue 2 and open file. ( If fail, don't save to file. )
            int m = msgrcv(msgqID2, &Q2, LENGTH, 1, IPC_NOWAIT);
            if (m > 0) {
                break;
            }

        }

        //open file to write if fail just continue and dont save
        file = fopen(Q2.msg, "w");
        if (file == NULL) {
            perror("open file");
        }

        int n;
        for (int r = nRows - 1; r >= 0; r--) {

            printSideSpace( nRows, yMax, yMin, r );

            for (int c = 0; c < nCols; ++c) {
                n = *(data + r * nCols + c);

                if (n < 0) {
                    cout << " ";
                    if (file != NULL) {
                        fprintf(file, "%d ", -1);
                    }
                } else {
                    cout << colors[n % nColors];
                    if (file != NULL) {
                        fprintf(file, "%d ", n);
                    }
                }

            }
            cout << '\n';
            if (file != NULL) {
                fprintf(file, "%c", '\n');
            }
        }

        printBottomSpace(nCols, xMin, xMax); 

        //close file after done
        if (file != NULL) {
            fclose(file);
        }


        numPictures++;

        fflush(stdout);
        fflush(stdin);


        //send msg to main
        strcpy(Q1.msg, "done DISPLAY!!!");


        if (msgsnd(msgqID, &Q1, LENGTH, IPC_NOWAIT) < 0) {
            perror("snd Q1 invalid in display");
            exit(-26);
        }

    }

    //return number of picture processed
    return numPictures;
}


//hh.txt -2 2 -1.5 1.5 100 50 80
//______________________________________________________________________________________________________________________
