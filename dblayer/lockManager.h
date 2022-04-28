#ifndef _LOCKMANAGER
#define _LOCKMANAGER

#include <string>
#include <map>
#include <vector>
#include <semaphore.h>
#include <pthread.h>

sem_t* make_semaphore(int);

class lockObject {
public:
	sem_t *xLock;
	sem_t *sLock;
	int xAcquired;
	int lockCount;

	lockObject();
};

class lockManager {
public:
	pthread_cond_t lmCond;
	pthread_mutex_t lmLock;
	std::map<std::string,lockObject> lockMap;
	pthread_mutex_t databaseLock;
	int dbLockAcquired;
	
	lockManager();

	/*
	Each element of the vector has a table name and an int.
	If the int is 0, an exclusive lock is being requested and if it is 1, a shared lock is being requested.
	The entries are in lexicographic order (in the string) to prevent deadlocks
	*/
	int getLocks(int, std::vector<std::pair<std::string,int>>);
	int releaseLocks(int, std::vector<std::pair<std::string,int>>);

};

#endif