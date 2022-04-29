#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <queue>

class TransactionInstance {
    public:
    std::vector<std::string> queries;
    int done;
    std::string output;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int client_id;
    int trans_id;
    TransactionInstance();
};

class TransactionHandler {
    public:
    TransactionInstance txn;
    void addQuery(std::string query);
    void executeTransaction();
};