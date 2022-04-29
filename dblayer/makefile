CPP=g++
BISON = bison
FLEX = flex
TGT = interface
SCAN = regex.l
PARSE = parser.y
CFLAGS = -g
OBJS = scan.o parse.tab.o
OBJ = tbl.o codec.o util.o client.o lockManager.o ../pflayer/pflayer.a ../amlayer/amlayer.a

all: loaddb $(TGT)

$(TGT): $(OBJS) $(OBJ)
	$(CPP) $(OBJ) dumpdb.o loaddb.o parse.tab.c scan.c main.c -o $(TGT) -std=c++17
	# $(CPP) $(OBJ) dumpdb.o loaddb.o parse.tab.c scan.c main.c -o $(TGT) -std=c++17 -pthread -lstdc++fs

dumpdb : dumpdb.o $(OBJ) ../pflayer/pflayer.a ../amlayer/amlayer.a
	$(CPP) -c $(CFLAGS) -o dumpdb dumpdb.o $(OBJ) 

loaddb : loaddb.o $(OBJ)
	$(CPP) -c $(CFLAGS) -o loaddb loaddb.o $(OBJ)

loaddb.o : loaddb.c tbl.h codec.h util.h
	$(CPP) -c $(CFLAGS) loaddb.c

dumpdb.o : dumpdb.c tbl.h codec.h util.h
	$(CPP) -c $(CFLAGS) dumpdb.c

tbl.o : tbl.c tbl.h
	$(CPP) -c $(CFLAGS) tbl.c

codec.o: codec.h codec.c
	$(CPP) -c $(CFLAGS) codec.c

util.o: util.h util.c tbl.h
	$(CPP) -c $(CFLAGS) util.c

%.o: %.c
	$(CPP) --std=c++0x $(CFLAGS) -c $<

scan.c: $(SCAN) parse.tab.h
	$(FLEX) -l --yylineno -o scan.c $(SCAN)

parse.tab.c parse.tab.h: dumpdb.o $(PARSE)
	$(BISON) -b parse -dv $(PARSE)

clean:
	rm  -f $(TGT) *.tab.c *.tab.h scan.c *.output *.o *.a a.out* *~ *.db* *db