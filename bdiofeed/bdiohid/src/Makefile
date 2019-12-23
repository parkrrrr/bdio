CPPFILES = $(wildcard *.cpp sqlitecpp/src/*.cpp)
CFILES = sqlitecpp/sqlite3/sqlite3.c
OBJFILES = $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) 
LDLIBS := -lstdc++ -lbrlapi

INC := -Iinclude -Isqlitecpp/sqlite3 -Isqlitecpp/include
override CFLAGS += $(INC)
override CXXFLAGS += -std=gnu\+\+17 $(INC)

#%.o: %.c | %.cpp
#	$(CXX) $(OFLAGS) $(IFLAGS) -c -o $@ $<

bdiohid: $(OBJFILES)
	$(CXX) $(CFLAGS) -o $@ $(OBJFILES) $(LDLIBS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJFILES) bdiohid

