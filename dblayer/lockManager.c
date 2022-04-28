#include "lockManager.h"

sem_t* make_semaphore(int value)
{
    sem_t* sem = new sem_t;
    int n = sem_init(sem, 0, value);
	// if (n != 0) perror_exit("sem_init failed");
    return sem;
}

lockObject::lockObject() {
	this->xLock = make_semaphore(1);
	this->sLock = make_semaphore(-1);
	this->xAcquired = 0;
}

lockManager::lockManager() {
	this->lmLock = PTHREAD_MUTEX_INITIALIZER;
	this->lmCond = PTHREAD_COND_INITIALIZER;
	this->databaseLock = PTHREAD_MUTEX_INITIALIZER;
}

int lockManager::getLocks(int clientId, std::vector<std::pair<std::string,int>> requestedLocks) {
	pthread_mutex_lock(&lmLock);
	std::vector<sem_t*> semVec;
	while(dbLockAcquired == 1)
		pthread_cond_wait(&lmCond, &lmLock);
	for (auto tbl : requestedLocks) {
		if(tbl.first == "$") {
			pthread_mutex_lock(&databaseLock);
			dbLockAcquired = 1;
		}
		else {
			if(tbl.second == 1) {
				while(lockMap[tbl.first].xAcquired != 0)
					pthread_cond_wait(&lmCond, &lmLock);
				sem_wait(lockMap[tbl.first].sLock);
			}
			else if(tbl.second == 0) {
				while(lockMap[tbl.first].lockCount > 0)
					pthread_cond_wait(&lmCond, &lmLock);
				sem_wait(lockMap[tbl.first].xLock);
				lockMap[tbl.first].xAcquired = 1;
			}
			else
				return 1;
		}
	}
	pthread_mutex_unlock(&lmLock);
	return 0;
}

int lockManager::releaseLocks(int clientId, std::vector<std::pair<std::string,int>> requestedLocks) {
	pthread_mutex_lock(&lmLock);
	for (auto tbl : requestedLocks) {
		if(tbl.first == "$") {
			pthread_mutex_unlock(&databaseLock);
			dbLockAcquired = 0;
		}
		else {
			if(tbl.second == 1) {
				sem_post(lockMap[tbl.first].sLock);
				--lockMap[tbl.first].lockCount;
			}
			else if(tbl.second == 0) {
				sem_post(lockMap[tbl.first].xLock);
				lockMap[tbl.first].xAcquired = 0;
			}
			else
				return 1;
		}
	}
	pthread_cond_signal(&lmCond);
	pthread_mutex_unlock(&lmLock);
	return 0;
}