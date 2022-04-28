#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <semaphore.h>
#include "lockManager.h"
#include "client.h"

using std::__fs::filesystem::directory_iterator;
extern 
int parse_query(std::string s);
int executeQuery(int i, std::vector<std::string>q, std::vector<std::string>col,std::vector<int>cond,int client_id);
extern std::queue<TransactionInstance*> transaction_queue;
extern std::string table;
extern int lock_type;
extern int qc;
extern std::vector<std::string> q;
extern std::vector<std::string> cols;
extern std::vector<int> cond;
lockManager lm;

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
    std::vector<std::pair<std::string,int> > table_and_locks;
    std::vector<int> qcs;
    std::vector<std::vector<std::string> > qs;
    std::vector<std::vector<std::string> > colss;
    std::vector<std::vector<int> > conds;
    TransactionInstance* txn;
};

void* transaction_final_execution(void* _args) {
    struct thread_args *args = (struct thread_args *) _args;
    int client_id = args->txn->client_id;
    int k = lm.getLocks(client_id, args->table_and_locks);
    // making a local copy of the required tables
    for(int i = 0; i < args->table_and_locks.size(); i++) {
        std::string tbl = args->table_and_locks[i].first;
        int lock_type = args->table_and_locks[i].second;
        if(tbl!="$" && lock_type == 0) {
            // copying tbl.db to tbl_<client_id>.db
            std::string line;
            std::ifstream ini_file(tbl + ".db");
            std::ofstream out_file(tbl + "_" + std::to_string(client_id) + ".db");
        
            if(ini_file && out_file){
                while(getline(ini_file,line)){
                    out_file << line << "\n";
                }        
            } else {
                //Something went wrong
                printf("Cannot read File");
            }
            ini_file.close();
            out_file.close();
        }
        if(tbl=="$") {
            std::string line;
            std::ifstream ini_file("meta_data.db");
            std::ofstream out_file("meta_data_" + std::to_string(client_id) + ".db");
        
            if(ini_file && out_file){
                while(getline(ini_file,line)){
                    out_file << line << "\n";
                }        
            } else {
                //Something went wrong
                printf("Cannot read File");
            }
            ini_file.close();
            out_file.close();
        }
        for(int i = 0; i < args->qcs.size(); i++) {
            executeQuery(args->qcs[i], args->qs[i], args->colss[i], args->conds[i], args->txn->client_id);
        }
        for(int i = 0; i < args->table_and_locks.size(); i++) {
            if(args->table_and_locks[i].first != "$") {
                // copy back table_<client_id>.db to table.db
                std::string line;
                std::ifstream ini_file(tbl + "_" + std::to_string(client_id) + ".db");
                std::ofstream out_file(tbl + ".db");
            
                if(ini_file && out_file){
                    while(getline(ini_file,line)){
                        out_file << line << "\n";
                    }        
                } else {
                    //Something went wrong
                    printf("Cannot read File");
                }
                ini_file.close();
                out_file.close();
            } else {
                // copy back meta_data_<client_id>.db to meta_data.db
                std::string line;
                std::ifstream ini_file("meta_data_" + std::to_string(client_id) + ".db");
                std::ofstream out_file("meta_data.db");
            
                if(ini_file && out_file){
                    while(getline(ini_file,line)){
                        out_file << line << "\n";
                    }        
                } else {
                    //Something went wrong
                    printf("Cannot read File");
                }
                ini_file.close();
                out_file.close();
            }
        }
    }
    k = lm.releaseLocks(client_id, args->table_and_locks);
}

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
                qc = -1;
                q.clear(); cols.clear(); cond.clear();
                if(table != "") {
                    int found = 0;
                    for(int j = 0; j < args->table_and_locks.size(); j++) {
                        if(args->table_and_locks[j].first == t) {
                            found = 1;
                            if(args->table_and_locks[j].second < lt) {
                                args->table_and_locks[j].second = lt;
                            }
                        }
                    }
                    if(found == 0) {
                        args->table_and_locks.push_back(std::pair<std::string, int>(t, lt));
                    }
                }
                args->qcs.push_back(qc1);
                args->qs.push_back(q1);
                args->colss.push_back(cols1);
                args->conds.push_back(cond1);
            }
            args->txn = txn;
            std::sort(args->table_and_locks.begin(), args->table_and_locks.end());
            pthread_t *p = (pthread_t *)malloc(sizeof(pthread_t));
            pthread_create(p, NULL, transaction_final_execution, (void *)args);
        }
    }
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
    // std::string s = "create table file data.csv index 2";
    // parse_query(s);
    // s = "dump all data";
    // parse_query(s);
    // parse_query("help");
}