README

 * Artur Wojcik
 * NetID: awojci5
 * CS 361
 * UIC Fall 2017
 *

-= Optional Enhancements =-

-------------------------------------------------------------------------

Program prints scale on the picture on the left side and on the bottom 
of the picture. 

-------------------------------------------------------------------------

To build the program type make that will compile a code and creates 
executable files. To run the program type 

./Mandelbrot 

this will execute all 3 programs that are communicate each other 
through pipes, queues, and shared memory. 
Folder containe 3 files:

- mandelbrot-awojci5.cpp
- mandelCalc-awojci5.cpp
- mandelDisplay-awojci5.cpp

First program creates two child using fork(), additionally pipes,
shared memory and queues. After, successful execution sends user input 
to the mandelCalc child. 
Second program is responsible for processing information and 
store to the shared memory. 
Third program displays processed information from shared memory 
on the display. 

Files included: 
 
  - Mandelbrot-awojci5.cpp
  - mandelCalc-awojci5.cpp
  - mandelDisplay-awojci5.cpp

  - makefile
  - README.txt
  - sampleOutput.txt
  - samplerun.txt 

Total files 7 !!!



