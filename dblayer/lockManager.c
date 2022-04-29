#include "lockManager.h"

sem_t* make_semaphore(int value)
{
    sem_t* sem = new sem_t;
    int n = sem_init(sem, 0, value);
	// if (n != 0) perror_exit("sem_init failed");
    return sem;
}

lockObject::lockObject() {
	this->xLock = PTHREAD_MUTEX_INITIALIZER;
	this->sLock = make_semaphore(1);
	this->xAcquired = 0;
}

lockManager::lockManager() {
	this->lmLock = PTHREAD_MUTEX_INITIALIZER;
	this->lmCond = PTHREAD_COND_INITIALIZER;
	this->databaseLock = PTHREAD_MUTEX_INITIALIZER;
}

int lockManager::getLocks(int clientId, std::vector<std::pair<std::string,int>> requestedLocks) {
	//std:cout << "in getLocks()" << std::endl;
	
	pthread_mutex_lock(&lmLock);


	while(dbLockAcquired == 1)
		pthread_cond_wait(&lmCond, &lmLock);
	for (auto tbl : requestedLocks) {
		if(tbl.first != "$" && lockMap.find(tbl.first) == lockMap.end()) {
			lockObject lockObj;
			lockMap[tbl.first] = lockObj;
		}
	}
	if(requestedLocks[0].first == "$") {
		pthread_mutex_lock(&databaseLock);
		dbLockAcquired = 1;
		std::cout << "Acquired database lock" << std::endl;
		requestedLocks.clear();
		for (auto &pair : lockMap)
			requestedLocks.push_back(make_pair(pair.first,0));
	}
	for (auto tbl : requestedLocks) {
		if(tbl.second == 1) {
			while(lockMap[tbl.first].xAcquired != 0)
				pthread_cond_wait(&lmCond, &lmLock);
			sem_post(lockMap[tbl.first].sLock);
			++lockMap[tbl.first].lockCount;
			std::cout << "Acquired slock of " << tbl.first << std::endl;
		}
		else if(tbl.second == 0) {
			while(lockMap[tbl.first].lockCount > 0)
				pthread_cond_wait(&lmCond, &lmLock);
			pthread_mutex_lock(&lockMap[tbl.first].xLock);
			lockMap[tbl.first].xAcquired = 1;
			std::cout << "Acquired xlock of " << tbl.first << std::endl;
		}
		else
			return 1;
	}
	//std:cout << "attempting to unlock lm" << std::endl;
	pthread_mutex_unlock(&lmLock);
	//std:cout << "unlocked lm" << std::endl;
	return 0;
}

int lockManager::releaseLocks(int clientId, std::vector<std::pair<std::string,int>> requestedLocks) {
	//std:cout << "release locks" << std::endl;
	pthread_mutex_lock(&lmLock);
	//std:cout << "acqd lmlock" << std::endl;

	if(requestedLocks[0].first == "$") {
		pthread_mutex_unlock(&databaseLock);
		dbLockAcquired = 0;
		std::cout << "Released database lock" << std::endl;
		requestedLocks.clear();
		for (auto &pair : lockMap)
			requestedLocks.push_back(make_pair(pair.first,0));
	}
	for (auto tbl : requestedLocks) {
		if(tbl.second == 1) {
			sem_wait(lockMap[tbl.first].sLock);
			--lockMap[tbl.first].lockCount;
			std::cout << "Released slock of " << tbl.first << std::endl;
		}
		else if(tbl.second == 0) {
			pthread_mutex_unlock(&lockMap[tbl.first].xLock);
			lockMap[tbl.first].xAcquired = 0;
			std::cout << "Released xlock of " << tbl.first << std::endl;
		}
		else
			return 1;
	}

	pthread_cond_signal(&lmCond);
	pthread_mutex_unlock(&lmLock);
	return 0;
}