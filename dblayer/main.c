#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "client.h"

using std::__fs::filesystem::directory_iterator;
extern 
int parse_query(std::string s);
extern std::queue<TransactionInstance*> transaction_queue;
extern std::string table;
extern int lock_type;
extern int qc;
extern std::vector<std::string> q;
extern std::vector<std::string> cols;
extern std::vector<int> cond;

void getCandT(std::string s, int &c, int &t) {
    std::string r = s.substr(12);
    r = r.substr(0, r.size()-4);
    size_t i = r.find("_");
    c = stoi(r.substr(6, i - 6));
    t = stoi(r.substr(i + 2));
}

void* client(void* d) {
    int i = *((int *)d);
    // scan all the transactions from the files 
    std::string path = "./testfiles";
    for (const auto &file : directory_iterator(path)) {
        int c, t;
        getCandT(file.path(), c, t);
        if(c == i) {
            TransactionHandler txnh;
            std::fstream infile;
            infile.open(file.path(), std::ios::in);
            if(infile.is_open()) {
                std::string line;
                while(getline(infile, line)) {
                    txnh.addQuery(line);
                }
            }
            infile.close();
            txnh.txn.client_id = i;
            txnh.executeTransaction();
        }
    }
}
struct thread_args {
    std::vector<std::string> tables;
    std::vector<int> table_locks;
    std::vector<int> qcs;
    std::vector<std::vector<std::string> > qs;
    std::vector<std::vector<std::string> > colss;
    std::vector<std::vector<int> > conds;
};

void* server(void* d) {
    int i = *((int *)d);
    // we don't need this i anywhere though
    while(true) {
        if(!transaction_queue.empty()) {
            TransactionInstance* txn = transaction_queue.front();
            transaction_queue.pop();

            std::vector<std::string> queries = txn->queries;
            struct thread_args *args = new(struct thread_args);
            for(int i = 0; i < queries.size(); i++) {
                parse_query(queries[i]);
                std::string t = table;
                int lt = lock_type;
                int qc1 = qc;
                std::vector<std::string> q1 = q;
	            std::vector<std::string> cols1 = cols;
	            std::vector<int> cond1 = cond;
                if(table != "") {
                    int found = 0;
                    for(int j = 0; j < args->tables.size(); j++) {
                        if(args->tables[j] == t) {
                            found = 1;
                            if(args->table_locks[j] > lt) {
                                args->table_locks[j] = lt;
                            }
                        }
                    }
                    if(found == 1) {
                        args->tables.push_back(t);
                        args->table_locks.push_back(lt);
                    }
                }
                args->qcs.push_back(qc1);
                args->qs.push_back(q1);
                args->colss.push_back(cols1);
                args->conds.push_back(cond1);
            }


        }
    }
}


void* transaction_final_execution(void* args) {

}

int main(int argc, char* argv[]) {

    int num_clients = 2;
    int num_server = 1;
    int *client_thread_id;
    int *server_thread_id;
    pthread_t *client_thread;
    pthread_t *server_thread;
    client_thread_id = (int *)malloc(sizeof(int) * num_clients);
    client_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_clients);
    server_thread_id = (int *)malloc(sizeof(int) * num_server);
    server_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_server);
    int i;
    for (i = 0; i < num_clients; i++) {
        client_thread_id[i] = i + 1;
    }

    for(i = 0; i < num_server; i++) {
        server_thread_id[i] = i + 1;
    }

    for (i = 0; i < num_clients; i++) {
        pthread_create(&client_thread[i], NULL, client, (void *)&client_thread_id[i]);
    }

    for (i = 0; i < num_server; i++) {
        pthread_create(&server_thread[i], NULL, server, (void *)&server_thread_id[i]);
    }

    for (i = 0; i < num_clients; i++)
    {
        pthread_join(client_thread[i], NULL);
        printf("client %d joined\n", i);
    }

    for (i = 0; i < num_server; i++)
    {
        pthread_join(server_thread[i], NULL);
        printf("server joined\n");
    }
    std::cout << "Everything is done. But we won't ever reach here :( \n";
    std::string s = "create table file data.csv index 2";
    parse_query(s);
    s = "dump all data";
    parse_query(s);
    parse_query("help");
}