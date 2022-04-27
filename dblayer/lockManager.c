#include "lockManager.h"

std::vector<sem_t*> lockManager::getLocks(std::vector<std::pair<std::string,int>> requestedLocks) {
	sem_wait(accessLock);
	std::vector<sem_t*> semVec;
	for (auto tbl : requestedLocks) {		
		if(tbl.second == 0)
			semVec.push_back(lockMap[tbl.first].xLock);
		else
			semVec.push_back(lockMap[tbl.first].sLock);
	}
	sem_post(accessLock);
	return semVec;
}