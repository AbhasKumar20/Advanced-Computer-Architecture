*Programming Languages used: C++ and Python(for plotting)



					To generate Traces of a program(Part I):

Step1: $make obj-intel64/'trace generator program name '.so
step2: $gcc -O3 -static -pthread 'input program's name' -o 'executables's name'
Step3: $../../../pin -t obj-intel64/addrtrace.so -- ./'executables's name' 8

Example-
$make obj-intel64/addrtrace.so
$gcc -O3 -static -pthread prog1.c -o prog1
$$../../../pin -t obj-intel64/addrtrace.so -- ./prog1 8

Traces will be get generated in the file named as 'trace.out'.

					To generate CDF and Plot(Part II)

Generate CDF

Step1:$g++ AccessDistance.cpp -o AccessDistance
Step1:$./AccessDistance trace.out

*An output file names as 'CDF.out' will get created.

Generate plot.

Step1:$python Plotter.py

Plot will be displayed.

				To generate Cache Miss CDF and Plot(Part III)


Generate Miss trace

Step1:$g++ CacheLRU.cpp -o CacheLRU
Step1:$./CacheLRU trace.out

*An output file named as 'misstrace.out' will get generated.

Generate Access distance for Miss trace

Step1:$g++ AccessDistance.cpp -o AccessDistance
Step1:$./AccessDistance misstrace.out

*A output file named as 'CDF.out' will get created.

Generate plot.

Step1:$python Plotter.py

*Plot will be displayed.

					To get Sharing Profile(Part IV)

Step1:$g++ SharingProfile.cpp -o SharingProfile
Step1:$./SharingProfile trace.out

*output on the terminal.


