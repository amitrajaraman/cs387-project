#ifndef _LOCKMANAGER
#define _LOCKMANAGER

#include <string>
#include <map>
#include <vector>
#include <semaphore.h>
#include <pthread.h>

sem_t* make_semaphore(int value)
{
    sem_t* sem = new sem_t;
    int n = sem_init(sem, 0, value);
	// if (n != 0) perror_exit("sem_init failed");
    return sem;
}


class lockObject {
public:
	sem_t *xLock;
	sem_t *sLock;
	int lockCount;

	lockObject() {
		this->xLock = make_semaphore(1);
		this->sLock = make_semaphore(-1);
	}
};

class lockManager {
public:
	std::map<std::string,lockObject> lockMap;
	sem_t *accessLock;

	lockManager() {
		this->accessLock = make_semaphore(1);
	}

	/*
	Each element of the vector has a table name and an int.
	If the int is 0, an exclusive lock is being requested and if it is 1, a shared lock is being requested.
	The entries are in lexicographic order (in the string) to prevent deadlocks
	*/
	std::vector<sem_t*> getLocks(std::vector<std::pair<std::string,int>>);

};

#endif