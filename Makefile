CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		src/BuiltInCmds.o src/Executor.o src/Runtime.o src/Utils.o src/Command.o src/OopShell.o src/Scanner.o 

LIBS =

TARGET =	OopShell

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
