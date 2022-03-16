#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tbl.h"
#include "codec.h"
extern "C" {
	#include "../pflayer/pf.h"
}

#define SLOT_COUNT_OFFSET 2
#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(EXIT_FAILURE);}}

int  getLen(int slot, byte *pageBuf); // UNIMPLEMENTED;
int  getNumSlots(byte *pageBuf); // UNIMPLEMENTED;
void setNumSlots(byte *pageBuf, int nslots); // UNIMPLEMENTED;
int  getNthSlotOffset(int slot, char* pageBuf); // UNIMPLEMENTED;

int getNumSlots(byte *pageBuf) {
	byte tempBuf[4];
	memcpy(tempBuf, pageBuf, sizeof(int)); // copy the first 4 bytes of pageBuf (which has numSlots) into tempBuf
	return DecodeInt(tempBuf);
}

void setNumSlots(byte *pageBuf, int nslots) {
	byte tempBuf[4];
	int oldslots = getNumSlots(pageBuf); // get the old number of slots
	EncodeInt(nslots, tempBuf);
	memcpy(pageBuf, tempBuf, sizeof(int)); // store the new number of slots into the first 4 bytes of pageBuf
	EncodeInt(-1, tempBuf);
	for (int i = oldslots; i < nslots; ++i)
		memcpy(pageBuf+(i+2)*sizeof(int), tempBuf, sizeof(int)); // sizeof(int)
		// create nslots - oldslots new entries in the array storing the pointers
		// this is always 1 in the other functions since we create a single slot precisely when we need one
}

int getNthSlotOffset(int slot, char *pageBuf) {
	int nslots = getNumSlots(pageBuf);
	if(slot >= nslots)
		return -1;
	byte tempBuf[4];
	memcpy(tempBuf, pageBuf+(2+slot)*sizeof(int), sizeof(int));
	// just store the int at the given position into tempBuf
	// the first two positions are numSlots and freeSpace, and the next nslots positions are pointers to the start of the corresponding sltos
	return DecodeInt(tempBuf);
}

int getLen(int slot, byte *pageBuf) {
	int slotOffset = getNthSlotOffset(slot, pageBuf);
	int prevSlotOffset = PF_PAGE_SIZE+1;
	if(slot > 0)
		prevSlotOffset = getNthSlotOffset(slot-1, pageBuf); // just find the distance from the offset of the current slot to the offset of the previous slot
	return (prevSlotOffset - slotOffset);
}

int getFreePtr(byte *pageBuf) {
	byte tempBuf[sizeof(int)];
	memcpy(tempBuf, pageBuf+sizeof(int), sizeof(int)); // get the pointer to the free space in the buffer (this is the second int in the byte*)
	return DecodeInt(tempBuf);
}


/**
   Opens a paged file, creating one if it doesn't exist, and optionally
   overwriting it.
   Returns 0 on success and a negative error code otherwise.
   If successful, it returns an initialized Table*.
 */
int
Table_Open(std::string dbname, Schema *schema, bool overwrite, Table **ptable)
{
	// UNIMPLEMENTED;
	PF_Init();
	FILE *fname = fopen(&dbname[0], "r");
	if(fname) { // If the file exists...
		fclose(fname);
		if(overwrite)
			PF_DestroyFile(&dbname[0]); // ...delete the file if the overwrite flag is 1.
		else {
			// In the case where overwrite is 0 and we are using a database that is already present, just open the file
			// Create a new table and store the relevant information into it
			Table *newTable = (Table*) malloc(sizeof(Table)); 
			newTable->schema = schema;
			newTable->fd = PF_OpenFile(&dbname[0]);
			*ptable = newTable;
			return 0;
		}
	}

	int errval = PF_CreateFile(&dbname[0]);
	if(errval < 0) // Create a new file using the pflayer.
		return errval;
	
	// Create a new table and store the relevant information into it
	Table *newTable = (Table*) malloc(sizeof(Table));
	newTable->schema = schema;
	newTable->fd = PF_OpenFile(&dbname[0]);

	*ptable = newTable;
	return 0;

	// Initialize PF, create PF file,
	// allocate Table structure  and initialize and return via ptable
	// The Table structure only stores the schema. The current functionality
	// does not really need the schema, because we are only concentrating
	// on record storage.
}

void
Table_Close(Table *tbl) {
	// UNIMPLEMENTED;
	int pgnum = -1;
	char *pagebuf;
	int pfe_status = PF_GetFirstPage(tbl->fd, &pgnum, &pagebuf);;
	// unfix all pages
	while(pfe_status == PFE_OK) {
		PF_UnfixPage(tbl->fd, pgnum, FALSE); // there are no dirty files since we immediately write them when inserting
		pfe_status = PF_GetNextPage(tbl->fd, &pgnum, &pagebuf);
	}
	// close the file
	PF_CloseFile(tbl->fd);
	// Unfix any dirty pages, close file.
}


int
Table_Insert(Table *tbl, byte *record, int len, RecId *rid) {
	// UNIMPLEMENTED;
	
	int fd = tbl->fd;
	byte* pgbuf;

	// printf("Length %d\nRecord: ", len);
	// char tempArr[999];
	// DecodeCString(record, tempArr, 999);
	// printf("%s\n", tempArr);


	int pgnum = -1;
	int res = PF_GetFirstPage(fd, &pgnum, &pgbuf); // store the first page into pgbuf
	while(true) {
		// if we have reached eof (so there is no page at the given position), unfix the old page and allocate a new page
		if(res == PFE_EOF) {
			byte tempBuf[999];
			PF_UnfixPage(fd, pgnum, TRUE);
			PF_AllocPage(fd, &pgnum, &pgbuf);
			// the next 4 lines just initialize the beginning of the page as one with 0 slots
			EncodeInt(0, tempBuf);
			memcpy(pgbuf, tempBuf, sizeof(int));
			EncodeInt(PF_PAGE_SIZE, tempBuf);
			memcpy(pgbuf + sizeof(int), tempBuf, sizeof(int));
			setNumSlots(pgbuf, 1);
			break;
		}
		if(res == PFE_OK) {
			// find the amount of free space that would be left after we create a new slot entry (note the +3 instead of +2!)
			int freeSpace = (getFreePtr(pgbuf) - (getNumSlots(pgbuf) + 3)*sizeof(int));
			// if this is enough to store the given record, increase the number of slots by 1
			if(freeSpace >= len) {
				setNumSlots(pgbuf, getNumSlots(pgbuf)+1);
				break;
			}
			// if the space is not enough, unfix this page and move to the next page
			else {
				PF_UnfixPage(fd, pgnum, TRUE);
				res = PF_GetNextPage(fd, &pgnum, &pgbuf);
			}
		}
		else {
			return res;
		}
	}

	// get all the relevant information
	byte tempBuf[4];
	int nslots = getNumSlots(pgbuf);
	int slot = nslots-1;
	int offset = getNthSlotOffset(slot, pgbuf);
	int freePtr = getFreePtr(pgbuf);
	// printf("nslots: %d  freeptr: %d\n", nslots, freePtr);
	nslots = getNumSlots(pgbuf);
	freePtr -= len; // this is the place we would start writing the data
	// just copy all the data in record into the calculated position
	memcpy(pgbuf + freePtr, record, len);
	// change the array of slot pointers appropriately
	EncodeInt(freePtr, tempBuf);
	memcpy(pgbuf + (sizeof(int)*(2+slot)), tempBuf, sizeof(int));
	// change the freeptr position appropriately
	EncodeInt(freePtr, tempBuf);
	memcpy(pgbuf + sizeof(int), tempBuf, sizeof(int));

	// unfix the page and set rid
	PF_UnfixPage(fd, pgnum, TRUE);
	*rid = slot | (pgnum << 16);

	return 0;

	// Allocate a fresh page if len is not enough for remaining space
	// Get the next free slot on page, and copy record in the free
	// space
	// Update slot and free space index information on top of page.
}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(EXIT_FAILURE);}}

/*
  Given an rid, fill in the record (but at most maxlen bytes).
  Returns the number of bytes copied.
 */
int
Table_Get(Table *tbl, RecId rid, byte *record, int maxlen) {
	int slot = rid & 0xFFFF;
	int pageNum = rid >> 16;

	// UNIMPLEMENTED;

	// get the page
	char *pgbuf;
	int fd = tbl->fd;
	PF_GetThisPage(fd, pageNum, &pgbuf);
	// get the slot
	if(slot > getNumSlots(pgbuf))
		return -1;
	int offset = getNthSlotOffset(slot, pgbuf);
	if(offset == -1)
		return -1;
	int length = getLen(slot, pgbuf);
	if(maxlen < length)
		length = maxlen;
	// store the slot into record
	memcpy(record, pgbuf+offset, length); // maxlen < slot_size?

	// In the page get the slot offset of the record, and
	// memcpy bytes into the record supplied.
	// Unfix the page
	return length; // return size of record
}

void
Table_Scan(Table *tbl, void *callbackObj, ReadFunc callbackfn) {

	char *pgbuf;
	int pgnum;
	byte record[999];
	int fd = tbl->fd;
	// get the first page
	int res = PF_GetFirstPage(fd, &pgnum, &pgbuf);
	while(res == PFE_OK) {
		// iterate through each of the slots within the page
		int numSlots = getNumSlots(pgbuf);
		for (int slot = 0; slot < numSlots; ++slot) {
			int offset = getNthSlotOffset(slot,pgbuf);
			int length = getLen(slot, pgbuf);
			memcpy(record, pgbuf+offset, length);
			// call the function on the given thing
			callbackfn(callbackObj, (pgnum << 16) | slot, record, length);
		}
		PF_UnfixPage(fd, pgnum, FALSE);
		// iterate through the pages one by one
		res = PF_GetNextPage(fd, &pgnum, &pgbuf);
	}

	return;

	// For each page obtained using PF_GetFirstPage and PF_GetNextPage
	//    for each record in that page,
	//          callbackfn(callbackObj, rid, record, recordLen)
}