To compile use the compile script
	./compile.sh
	
To see program options:
	./a.out -h
	
To run a simple routine:
	./a.out [routine]
	
Avaliable routines:
	backpropagation
	kohonen
	
For threaded - simultaneous execution (Experimental):
	./a.out -t [no. of threads]

Network and other parameters can be altered from the 'parameters.txt' file
	
The project includes multiple scripts that:
	- compile code
	- send my a personal email
	- play a sound
	
This project also includes a self-made library that dynamically prsents the progress of each step in-line on the console using ANSI escape codes.

To see the magic
	./a.out -progress

