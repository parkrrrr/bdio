CPPFILES = $(wildcard *.cpp)
CFILES = ../sqlitecpp/sqlite3/sqlite3.c
HFILES = $(wildcard *.h)
OBJFILES = $(CPPFILES:.cpp=.o) 
LDLIBS := -lstdc++ -lpthread -ldl

INC := -I../sqlitecpp/sqlite3 -I../sqlitecpp/include
override CFLAGS += $(INC)
override CXXFLAGS += -std=gnu\+\+17 $(INC)

#%.o: %.c | %.cpp
#	$(CXX) $(OFLAGS) $(IFLAGS) -c -o $@ $<

maptodb: $(OBJFILES) $(HFILES) SBackup.o SDatabase.o SStatement.o SColumn.o SException.o STransaction.o sqlite3.o
	$(CXX) $(CFLAGS) -o $@ $(OBJFILES) sqlite3.o SBackup.o SDatabase.o SStatement.o \
		SColumn.o SException.o STransaction.o $(LDLIBS) $(LDFLAGS)

sqlite3.o: ../sqlitecpp/sqlite3/sqlite3.c
	$(CC) -c $(CFLAGS) -o $@ $<

SBackup.o: ../sqlitecpp/src/Backup.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

SDatabase.o: ../sqlitecpp/src/Database.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

SStatement.o: ../sqlitecpp/src/Statement.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

SColumn.o: ../sqlitecpp/src/Column.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

SException.o: ../sqlitecpp/src/Exception.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

STransaction.o: ../sqlitecpp/src/Transaction.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJFILES) maptodb S*.o sqlite3.o

