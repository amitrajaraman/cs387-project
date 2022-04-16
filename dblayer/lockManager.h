#ifndef _LOCKMANAGER
#define _LOCKMANAGER

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <semaphore> // need to install some "linux-headers" stuff, not there on WSL?

typedef struct {
	std::counting_semaphore xLock<1>;
	std::counting_semaphore sLock{0};
	int lockCount;
} lockObject;

typedef struct {
	map<std::string,lockObject> lockMap;
	std::mutex accessLock;
} lockManager;


/*
Each element of the vector has a table name and an int.
If the int is 0, an exclusive lock is being requested and if it is 1, a shared lock is being requested.
The entries are in lexicographic order (in the string) to prevent deadlocks
*/
vector<std::counting_semaphore> lockManager::getLocks(vector<pair<std::string,int>>);

#endif