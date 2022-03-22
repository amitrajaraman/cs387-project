#ifndef _TBL_H_
#define _TBL_H_
#include <stdbool.h>
#include <string>
#include <cstring>
#include <vector>

#define VARCHAR 1
#define INT     2
#define LONG    3

typedef char byte;

typedef struct {
    char *name;
    int  type;  // one of VARCHAR, INT, LONG
} ColumnDesc;

typedef struct {
    int numColumns;
    ColumnDesc **columns; // array of column descriptors
} Schema;

typedef struct {
    // UNIMPLEMENTED; 
    Schema *schema;
    int fd;
    int indexFd;
} Table ;

typedef int RecId;

int
Table_Open(std::string fname, Schema *schema, bool overwrite, Table **table);

int
Table_Insert(Table *t, byte *record, int len, RecId *rid);

int
Table_Get(Table *t, RecId rid, byte *record, int maxlen);

void
Table_Close(Table *);

typedef void (*ReadFunc)(void *callbackObj, RecId rid, byte *row, int len, std::vector<int> rowsToBePrinted);

void
Table_Scan(Table *tbl, void *callbackObj, ReadFunc callbackfn);

void
printAllRows(Table *tbl, void *callbackObj, ReadFunc callbackfn, std::vector<std::string> *);


#endif
