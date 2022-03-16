/* misc.c */
#include "pf.h"
#include "testam.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "am.h"
#include "aminternals.h"

/**********************************************************
pad end of str until up to length bytes with '\0'
Assume that str is terminated with '\0'
***********************************************************/
void
padstring(
    char *str,
    int length
)
{
    int i;

    for (i= strlen(str)+1, str= str+i; i < length; i++, str++) {
        *str = '\0';
    }
}


int
xAM_CreateIndex(
    char *fname,
    int indexno,
    char attrtype,
    int attrlen
)
{
    int errval;

    if ((errval=AM_CreateIndex(fname,indexno,attrtype,attrlen))!= AME_OK) {
        printf("AM_CreateIndex(%s,%d,'%c',%d) failed: %d\n",
               fname,indexno,attrtype,attrlen,errval);
        exit(1);
    }
    return(errval);
}

int
xAM_DestroyIndex(
    char *fname,
    int indexno
)
{
    int errval;

    if ((errval=AM_DestroyIndex(fname,indexno)) != AME_OK) {
        printf("AM_DestroyIndex(%s,%d) failed: %d\n",fname,indexno,errval);
        exit(1);
    }
    return(errval);
}


int
xAM_InsertEntry(
    int fd,
    char attrtype,
    int attrlen,
    char *val,
    RecIdType recid
)
{
    int errval;

    if ((errval=AM_InsertEntry(fd,attrtype,attrlen,val,recid))!=AME_OK) {
        printf("AM_InsertEntry(%d,'%c',%d,val,%d) failed: %d\n",
               fd,attrtype,attrlen,RecIdToInt(recid),errval);
        exit(1);
    }
    return(errval);
}


int
xAM_DeleteEntry(
    int fd,
    char attrtype,
    int attrlen,
    char *val,
    RecIdType recid
)
{
    int errval;

    if ((errval=AM_DeleteEntry(fd,attrtype,attrlen,val,recid))!= AME_OK) {
        printf("AM_DeleteEntry(%d,'%c',%d,val,%d) failed: %d\n",
               fd,attrtype,attrlen,RecIdToInt(recid),errval);
        exit(1);
    }
    return(errval);
}

int
xAM_OpenIndexScan(
    int fd,
    char attrtype,
    int  op,
    int attrlen,
    char *val
)
{
    int sd;


    if ((sd=AM_OpenIndexScan(fd,attrtype,attrlen,op,val))<0 ) {
        printf("AM_OpenIndexScan(%d,'%c',%d,%d,rec) failed: %d\n",
               fd,attrtype,attrlen,op,sd);
        exit(1);
    }
    return(sd);
}


RecIdType xAM_FindNextEntry(int sd)
{
    int errval;
    RecIdType recid;

    recid= AM_FindNextEntry(sd);
    if ((errval=RecIdToInt(recid))<AME_OK && errval != AME_EOF) {
        printf("AM_FindNextEntry(%d) failed: %d\n",sd,errval);
        exit(1);
    }
    return(recid);
}

int
xAM_CloseIndexScan(int sd)
{

    int errval;

    if ((errval=AM_CloseIndexScan(sd))!= AME_OK) {
        printf("AM_CloseIndexScan(%d) failed:%d\n",sd,errval);
        exit(1);
    }
    return(errval);
}

int
xPF_OpenFile(char *fname)
{
    int errval;

    if ((errval=PF_OpenFile(fname))<0) {
        printf("PF_OpenFile(%s) failed: %d\n", fname, errval);
        exit(1);
    }
    return(errval);
}

int
xPF_CloseFile(int fd)
{
    int errval;

    if ((errval=PF_CloseFile(fd))!= PFE_OK) {
        printf("PF_CloseFile(%d) failed: %d\n",fd,errval);
        exit(1);
    }

    return(errval);
}
