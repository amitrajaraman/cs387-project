#include "client.h"

queue<TransactionInstance*> transaction_queue;

TransactionInstance::TransactionInstance() {
    lock = PTHREAD_MUTEX_INITIALIZER;
    cond = PTHREAD_COND_INITIALIZER;
    done = 0;
}

void TransactionHandler::addQuery(string query) {
    txn.queries.push_back(query);
}

void TransactionHandler::executeTransaction() {
    transaction_queue.push(&txn);
    pthread_mutex_lock(&txn.lock);
    while(txn.done == 0) {
        pthread_cond_wait(&txn.cond, &txn.lock);
    }
    pthread_mutex_unlock(&txn.lock);
}