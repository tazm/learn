TARGET := gtest
CFLAGS :=
CXXFLAGS := -DDEBUG -g -Wall  -I../ -I../include/
LDFLAGS := -L../lib -lpthread
LIBS    := ../lib/gtest_main.a 

CPP_SOURCES := ../common/stringprintf.cpp ../common/win_linux.cpp ../common/getip.cpp
CPP_SOURCES += $(wildcard *.cpp)
CPPOBJS := $(CPP_SOURCES:.cpp=.o)
C_SOURCES := $(wildcard *.c)
COBJS := $(C_SOURCES:.c=.o)

$(TARGET) : $(COBJS) $(CPPOBJS) 
	$(CXX) $(LDFLAGS) -o $(TARGET)  $(COBJS) $(CPPOBJS) $(LIBS)

clean:
	-rm -f  $(COBJS) $(CPPOBJS) $(TARGET) *.o *.d
