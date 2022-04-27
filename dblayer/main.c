#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "client.h"

using std::__fs::filesystem::directory_iterator;
extern 
int parse_query(std::string s);
extern std::queue<TransactionInstance*> transaction_queue;

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
            txnh.executeTransaction();
        }
    }
}

void* server(void* d) {
    int i = *((int *)d);

}

int main(int argc, char* argv[]) {

    // int num_clients = 2;
    // int num_server = 1;
    // int *client_thread_id;
    // int *server_thread_id;
    // pthread_t *client_thread;
    // pthread_t *server_thread;
    // client_thread_id = (int *)malloc(sizeof(int) * num_clients);
    // client_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_clients);
    // server_thread_id = (int *)malloc(sizeof(int) * num_server);
    // server_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_server);
    // int i;
    // for (i = 0; i < num_clients; i++) {
    //     client_thread_id[i] = i + 1;
    // }

    // for(i = 0; i < num_server; i++) {
    //     server_thread_id[i] = i + 1;
    // }

    // for (i = 0; i < num_clients; i++) {
    //     pthread_create(&client_thread[i], NULL, client, (void *)&client_thread_id[i]);
    // }

    // for (i = 0; i < num_server; i++) {
    //     pthread_create(&server_thread[i], NULL, server, (void *)&server_thread_id[i]);
    // }

    // for (i = 0; i < num_clients; i++)
    // {
    //     pthread_join(client_thread[i], NULL);
    //     printf("client %d joined\n", i);
    // }

    // for (i = 0; i < num_server; i++)
    // {
    //     pthread_join(server_thread[i], NULL);
    //     printf("server joined\n");
    // }
    // std::cout << "Everything is done. But we won't ever reach here :( \n";
    std::string s = "create table file data.csv index 2";
    parse_query(s);
    s = "dump all data";
    parse_query(s);
    parse_query("help");
}