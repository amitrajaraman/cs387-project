#include "lockManager.h"
#include <thread>

lockManager::getLocks(vector<pair<std::string,int>> requestedLocks) {
	accessLock.lock(); // is this needed?
	vector<counting_semaphore> semVec;
	for (auto tbl : requestedLocks) {		
		if(tbl.second == 0)
			semVec.push_back(lockMap[tbl.first].xLock);
		else
			semVec.push_back(lockMap[tbl.first].sLock);
	}
	accessLock.unlock(); // is this needed?
	return semVec;
}