#include "client.h"

std::queue<TransactionInstance*> transaction_queue;
pthread_mutex_t lock;

TransactionInstance::TransactionInstance() {
    lock = PTHREAD_MUTEX_INITIALIZER;
    cond = PTHREAD_COND_INITIALIZER;
    done = 0;
}

void TransactionHandler::addQuery(std::string query) {
    txn.queries.push_back(query);
}

void TransactionHandler::executeTransaction() {
    pthread_mutex_lock(&lock);
    transaction_queue.push(&txn);
    pthread_mutex_unlock(&lock);
    pthread_mutex_lock(&txn.lock);
    while(txn.done == 0) {
        // std::cout << "Client About to wait for txn to complete \n";
        pthread_cond_wait(&txn.cond, &txn.lock);
    }
    pthread_mutex_unlock(&txn.lock);
    
}