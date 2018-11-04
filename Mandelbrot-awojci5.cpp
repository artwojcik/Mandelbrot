/*
 * Mandelbrot-awojci5.cpp
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
 * This program collects inpput from user and sends those inputs to the next child.
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


//define for the sending end receiving through the pipes

#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1


struct msgQueue {
    long num;
    char msg[256];
};


using namespace std;

//global variables used to make sure that
//shared memory and queues are released when error occur
//end program terminates with perror

int shmID, msgqID, msgqID2;
int numPictures = 0;

const int LENGTH = sizeof(msgQueue) - sizeof(long);
pid_t mandelCalcChild, endCalcChild;
pid_t mandelDisplayChild, endDisplayChild;



//handler in case some program exit due to error than handler will kill
//other child and mandelbrot program with code -42
void sig_handler(int signal) {

    int status;
    int s2 ;
    if (waitpid(mandelCalcChild, &status, WNOHANG) > 0) {
        cout <<" Exit status for calc: " << WEXITSTATUS(status);

        if (WEXITSTATUS(status)<0){
            cout << "Error in Calc... !!! \nExit status for for Calc was " << WEXITSTATUS(status) -256 << endl;
            cout << "Total pictures processed: " << numPictures << endl;
            cout << "Exiting program with code -42 killing MandelDisplay\n\n    See yaaa!!!\n\n ";
            kill(mandelDisplayChild, 9);

            exit(-42);
        }

    }

    if (waitpid(mandelDisplayChild, &s2, WNOHANG) > 0) {
        cout<<" Exit status for Display "<< WEXITSTATUS(s2)<< "!!!display!";
        if(WEXITSTATUS(s2)<0){
            cout << "Error in Display... !!! \nExit status for for Display was " << WEXITSTATUS(s2)-256 << endl;
            cout << "Total pictures processed: " << numPictures << endl;
            cout << "Exiting program with code -42 killing MandelCalc\n\n    See yaaa!!!\n\n";
            kill(mandelCalcChild, 9);

            exit(-42);
        }
    }
}

//info about author class 
void info(){
    cout << endl;
    cout << "  ++++++++++++++++++++++++++++++\n";
    cout << "  +   Artur Wojcik             +\n";
    cout << "  +   NetID: awojci5           +\n";
    cout << "  +   CS 361 UIC Fall 2017     +\n";
    cout << "  +   11/14/2017               +\n";
    cout << "  +   Project 4 Mandelbrot     +\n";
    cout << "  ++++++++++++++++++++++++++++++\n\n";
}

//info about program how to use it 
void infoProgram (){
    cout << " Welcome to the Mandelbrot fractal processing\n";
    cout << " Program will display picture that is calculated \n";
    cout << " and displayed by two different processes using\n";
    cout << " interprocess communication (pipes, shared memory, queues\n";
    cout << " Lets get started !!!\n\n";
}

//function that will free resources at exit if any problem will happen
void clearResources() {

    if(shmctl(shmID, IPC_RMID, NULL)==0){
        cout << "*** Freeing shared memory status:    OK - FREE\n";
    }
    else {
        cout << "** Unable to free shared memory\n";
    }

    //removes message queues
    if (msgctl(msgqID, IPC_RMID, NULL)==0){
        cout << "*** Freeing message queue #1 status: OK - FREE\n";
    }
    else {
        cout << "*** Unable to free message queue #1 status: FAILED\n";
    }

    if(msgctl(msgqID2, IPC_RMID, NULL)==0){
        cout << "*** Freeing message queue #2 status: OK - FREE\n";
    }
    else {
        cout << "*** Unable to free message queue #2 status: FAILED\n";
    }
}

//handler for ctrl+c to free resources 
void ctrlcHandler(int signal){
    //clearResources(); 
    cout << endl;
    kill(mandelCalcChild, 9); 
    kill(mandelDisplayChild, 9);
    exit(1);
}

int main(int argc, char **argv) {

    //stdin variables scan for user input
    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;


    int status = 0;
    int pipeDisplay[2];
    int pipeCalc[2];
    char sharedMemory[20];
    char idQ1[20];
    char idQ2[20];
    FILE *writeToCalc = NULL;
    char fileName[256];

    msgQueue Q1;
    Q1.num = 1;
    msgQueue Q2;
    Q2.num = 1;




    //release resources if exited with failure
    atexit(clearResources);

    //message queue 1
    msgqID = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (msgqID < 0) {
        perror("Q1");
        exit(-1);
    }


    // int msgget(key_t key, int msgflg);
    //create message queues
    msgqID2 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (msgqID2 < 0) {
        perror("Q2");
        exit(-2);
    }

    //create shared memory
    shmID = shmget(IPC_PRIVATE, 40000, IPC_CREAT | 0600);

    if (shmID < 0) {
        perror("shmget");
        exit(-3);
    }

    //convert id for queues and shared memory to string
    sprintf(sharedMemory, "%d", shmID);
    sprintf(idQ1, "%d", msgqID);
    sprintf(idQ2, "%d", msgqID2);


    //create pipe to inter[process communication for 2 child
    if (pipe(pipeCalc) < 0) {
        perror("pipe");
        exit(-4);
    }
    if (pipe(pipeDisplay) < 0) {
        perror("pipe");
        exit(-5);
    }

    //display info about program and author 
    info();
    infoProgram();


    //set signal for ctrl+c and user1
    signal(SIGINT, ctrlcHandler);
    signal(SIGCHLD, sig_handler);



    //fork firs child
    mandelCalcChild = fork();


    //create Mandelcalc
    if (mandelCalcChild == 0) {

        //close unused ends of the pipes
        close(pipeCalc[WRITE]);
        close(pipeDisplay[READ]);

        //copy FD to STDIN/STDOUT
        dup2(pipeCalc[READ], STDIN);
        dup2(pipeDisplay[WRITE], STDOUT);

        //close original FD
        close(pipeDisplay[WRITE]);
        close(pipeCalc[READ]);


        if (execl("mandelCalc", "mandelCalc", sharedMemory, idQ1, NULL) == -1) {
            perror("command not found calc\n");
            exit(-6);
        }

    }

        //create Mandel display
    else {
        mandelDisplayChild = fork();
        if (mandelDisplayChild == 0) {

            //close not used end of the pipes pipe calc not used at all
            close(pipeCalc[WRITE]);
            close(pipeCalc[READ]);
            close(pipeDisplay[WRITE]);

            //coppy FD to stdin andclose original one
            dup2(pipeDisplay[READ], STDIN);
            close(pipeDisplay[READ]);

            if (execl("mandelDisplay", "mandelDisplay", sharedMemory, idQ1, idQ2, NULL) == -1) {
                perror("command not found display\n");
                exit(-7);
            }
        }


            //i am  parent
        else {

            //close unused sides of the pipes
            close(pipeCalc[READ]);
            close(pipeDisplay[READ]);
            close(pipeDisplay[WRITE]);

            //send a number to mandelCalc
            writeToCalc = fdopen(pipeCalc[WRITE], "w");


            while (true) {

                //read in values from user
                cout << "\nPlease enter the number of rows to display (0 to quit) > ";
                cin >> nRows; 
                if (nRows == 0 ){

                    break; 
                }

                cout << "Please enter the number of columns to display > ";
                cin >> nCols;

                if((nCols*nRows)>40000){
                    cout << "\nMake sure that number of rows times number of columns is < 40,000\n";
                    continue;
                }


                cout << "\nPlease enter the minimum X value > ";
                cin >> xMin;
                cout << "\nPlease enter the maximum X value > ";
                cin >> xMax;
                cout << "\nPlease enter the minimum Y value > ";
                cin >> yMin;
                cout << "\nPlease enter the maximum Y value > ";
                cin >> yMax;

                cout << "\nPlease enter the maximum number of iterations (max 100): ";
                cin >> maxIters; 

                //error checking for max iter in range 0 -100
                while (maxIters <= 0 || maxIters > 100){
                    cout << "Please enter the maximum number of iterations: ";
                    cin >> maxIters; 
                }

                cout << "Please enter the output file: ";
                cin >> fileName;

                //b. If user is not done yet
                //Write filename to message queue 2
                strcpy(Q2.msg, fileName);
                if (msgsnd(msgqID2, &Q2, LENGTH, 0) < 0) {
                    perror("snd in main file name  ");
                    exit(-9);
                }

                //Write xMin, xMax, yMin, yMax, nRows, nCols, and maxIters to pipe and flush
                fprintf(writeToCalc, "%lf %lf %lf %lf %d %d %d\n", xMin, xMax, yMin, yMax, maxIters, nRows, nCols);
                fflush(writeToCalc);

                //receive message from chicld waifor message
                while (true) {
                    int r = msgrcv(msgqID, &Q1, LENGTH, 1, IPC_NOWAIT);
                    if (r > 0) {
                        break;
                    }

                }
                //wait for messagee from child
                while (true) {
                    int r2 = msgrcv(msgqID, &Q1, LENGTH, 1, IPC_NOWAIT);

                    if (r2 > 0) {
                        break;
                    }

                }

                numPictures++;

            }


        }
    }



    //
    // wait for children calc and display
    // report exit status
    //
    kill(mandelCalcChild, SIGUSR1);
    //wait for children and report their status - calc
    endCalcChild = waitpid(mandelCalcChild, &status, WUNTRACED);

    // //check exit status end print
    // if (endCalcChild == -1) {
    //     perror("waitpid() failed");
    //     exit(EXIT_FAILURE);
   // }
    if (WIFEXITED(status)) {
        int es = WEXITSTATUS(status);
        printf("Exit status for mandelCalc from main was %d\n", es);
    }


    int status2;
    kill(mandelDisplayChild, SIGUSR1);

    //wait for display and report exit status
    endDisplayChild = waitpid(mandelDisplayChild, &status2, WUNTRACED);

    // if (endDisplayChild == -1) {
    //     perror("waitpid() failed");
    //     exit(EXIT_FAILURE);
    // } 
    if (WIFEXITED(status2)) {
        int es = WEXITSTATUS(status2);
        printf("Exit status for mandelDisplay from main was %d\n", es);
    }



    //deallocate shared memory from system b parent
    //clearResources();

    //close file descriptor that was used for pipe 
    if (fclose(writeToCalc) != 0) {
        //don't stop program at this point ignore fact 
        perror("fclose");
    }

    cout << "Total number of pictures processed:  " << numPictures << endl;

    cout << "\nHope you liked this.\n   !!! Exiting with 0 Errors ;-) !!! \n";


    return 0;
}



