GCC        = gcc
EXE_1      = tcpserver
EXE_2      = tcpclient
OBJ_1      = $(EXE_1).o
OBJ_2      = $(EXE_2).o
SOURCE_1   = $(EXE_1).c
SOURCE_2   = $(EXE_2).c


default: $(SOURCE_1) $(SOURCE_2)
	$(GCC) -c -o $(OBJ_1) $(SOURCE_1) -pthread
	$(GCC) $(OBJ_1) -o $(EXE_1) -pthread
	$(GCC) -c -o $(OBJ_2) $(SOURCE_2) -pthread
	$(GCC) $(OBJ_2) -o $(EXE_2) -pthread
clean:
	rm -rf *.o $(EXE_1) $(EXE_2) 
	
	