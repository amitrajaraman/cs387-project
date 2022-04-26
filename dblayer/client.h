#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <queue>

using namespace std;


class TransactionInstance {
    public:
    vector<string> queries;
    int done;
    string output;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    TransactionInstance();
};

class TransactionHandler {
    public:
    TransactionInstance txn;
    void addQuery(string query);
    void executeTransaction();
};