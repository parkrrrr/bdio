CPPFILES = $(wildcard *.cpp ../sqlitecpp/src/*.cpp)
CFILES = ../sqlitecpp/sqlite3/sqlite3.c
HFILES = $(wildcard *.h)
OBJFILES = $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) 
LDLIBS := -lstdc++ -lpthread -ldl

INC := -I../sqlitecpp/sqlite3 -I../sqlitecpp/include
override CFLAGS += $(INC)
override CXXFLAGS += -std=gnu\+\+17 $(INC)

#%.o: %.c | %.cpp
#	$(CXX) $(OFLAGS) $(IFLAGS) -c -o $@ $<

maptodb: $(OBJFILES) $(HFILES)
	$(CXX) $(CFLAGS) -o $@ $(OBJFILES) $(LDLIBS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJFILES) maptodb

