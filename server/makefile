GXX = g++

all :  
	g++ -o server server.cpp ./common/*.cpp `mysql_config --cflags --libs`

.PHONY : clean
clean :
	rm -rf *.exe *.o log.log
