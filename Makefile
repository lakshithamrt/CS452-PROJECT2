run: read.cpp 
	g++ read.cpp -lglut -lGLU -lGL -lGLEW -g 
	
clean: 
	rm -f *.out *~ run
	
