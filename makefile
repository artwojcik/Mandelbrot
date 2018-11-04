DEBUG_FLAGS = -g -Werror -Wall -Wextra -Wno-unused-parameter

all: Mandelbrot-awojci5.cpp
	g++ -Wall -std=c++11 -o Mandelbrot Mandelbrot-awojci5.cpp
	g++ -Wall -std=c++11 -o mandelCalc mandelCalc-awojci5.cpp
	g++ -Wall -std=c++11 -o mandelDisplay mandelDisplay-awojci5.cpp
	rm -f *.o *.s *.i

#debug: preprocess compile assemble
#	g++ $(DEBUG_FLAGS) -o HW4__debug mandelDisplay-awojci5.o mandelCalc-awojci5.o Mandelbrot-awojci5.o

#preprocess: Mandelbrot-awojci5.cpp mandelCalc-awojci5.cpp mandelDisplay-awojci5.cpp
#	g++ -std=c++11 -E Mandelbrot-awojci5.cpp -o Mandelbrot-awojci5.i
#	g++ -std=c++11 -E mandelCalc-awojci5.cpp -o mandelCalc-awojci5.i
#	g++ -std=c++11 -E mandelDisplay-awojci5.cpp -o mandelDisplay-awojci5.i

#compile: preprocess
#	g++ -std=c++11 -S Mandelbrot-awojci5.i
#	g++ -std=c++11 -S mandelCalc-awojci5.i
#	g++ -std=c++11 -S mandelDisplay-awojci5.i
	
#assemble: compile
#	g++ -std=c++11 -c Mandelbrot-awojci5.s
#	g++ -std=c++11 -c mandelCalc-awojci5.s
#	g++ -std=c++11 -c mandelDisplay-awojci5.s

clean:
	rm -f *.o *.s *.i

run:
	./Mandelbrot

