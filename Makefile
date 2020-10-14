COMPILER         = g++
#COMPILER         = clang++
OPTIMIZATION_OPT = -O1
OPTIONS          =  -w -std=c++0x $(OPTIMIZATION_OPT) -g -o 
LINKER_OPT       =  -L/usr/lib64/ -lcrypto
INCLUDE_OPT	 = 	-I./include
SRC_FILE	 = ./*.cpp
BIN_DIR		 = .
TARGET		 = ./TestBWTSolution ./TestNewSolution ./TestSuffixTreeSolution

#BUILD_LIST = $(BIN_DIR)/$(TARGET) $(BIN_DIR)/$(TARGET)

all: $(TARGET) 

./TestBWTSolution: ./TestBWTSolution.cpp ./AES.cpp ./include/*
	$(COMPILER) $(OPTIONS) ./TestBWTSolution ./TestBWTSolution.cpp ./AES.cpp  $(INCLUDE_OPT) $(LINKER_OPT) 

./TestNewSolution: ./TestNewSolution.cpp ./AES.cpp ./include/*
	$(COMPILER) $(OPTIONS) ./TestNewSolution ./TestNewSolution.cpp ./AES.cpp  $(INCLUDE_OPT) $(LINKER_OPT) 

./TestSuffixTreeSolution: ./TestSuffixTreeSolution.cpp ./AES.cpp ./include/*
	$(COMPILER) $(OPTIONS) ./TestSuffixTreeSolution ./TestSuffixTreeSolution.cpp ./AES.cpp  $(INCLUDE_OPT) $(LINKER_OPT) 

strip_bin :
	strip -s $(TARGET)

clean:
	rm -f $(BIN_DIR)/$(TARGET) 

