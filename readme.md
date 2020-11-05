To compile: 
	
	g++ -g -std=c++11 src/BackPropagation/backpropagation.cpp -o executable.out

To run:

	./executable.out
	
To see program options:
	
	./executable.out -h
	
To see the magic

	./executable.out -progress

For threaded - simultaneous execution:
	
	./execute.out -t [no. of threads]

Netowrk parameters can be altered from the 'parameters.txt' file
	
The project includes multiple scripts that:

	- compile code
	- send my a personal email
	- play a sound
	- runs a routine of multiple scripts to:
		> compile & execute program. 
		> Train the network and evaluate results. 
		> Notify me when routine is done
	
This project also includes a self-made library that dynamically prsents the progress of each step in-line on the console using ANSI escape codes.
