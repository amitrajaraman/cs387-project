#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <string>
#include <algorithm>
#include <experimental/filesystem>
#include <semaphore.h>
#include "lockManager.h"
#include "client.h"
extern "C" {
	#include "../pflayer/pf.h"
}

using directory_iterator = std::experimental::filesystem::directory_iterator;
extern 
int parse_query(std::string s, int *res);
int executeQuery(int i, std::vector<std::string>q, std::vector<std::string>col,std::vector<int>cond,int client_id, int a, int b, int *res, std::string &s);
extern std::queue<TransactionInstance*> transaction_queue;
extern pthread_mutex_t lock;		// Lock for the transaction_queue
extern std::string table;
extern int lock_type;
extern int qc;
extern std::vector<std::string> q;
extern std::vector<std::string> cols;
extern std::vector<int> cond;
extern std::string created_table;
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
	int t_max = 0;
	for (const auto &file : directory_iterator(path)) {
		int c, t;
		getCandT(file.path(), c, t);
		if(c == i) {
			t_max = (t > t_max) ? t : t_max;
		}
	}
	for(int j = 1; j <= t_max; j++) {
		TransactionHandler txnh;
		std::string f = "./testfiles/client" + std::to_string(i) + "_t" + std::to_string(j) + ".txt"; 
		std::fstream infile;
		infile.open(f, std::ios::in);
		if(infile.is_open()) {
			std::string line;
			while(getline(infile, line)) {
				txnh.addQuery(line);
			}
		}
		infile.close();
		txnh.txn.client_id = i;
		txnh.txn.trans_id  = j;
		std::cout << "Client " << i << " attempting to execute Transaction " << j << std::endl;
		txnh.executeTransaction();
		if(txnh.txn.done == -1)
			std::cout << "Client " << i << " has aborted Transaction " << j << std::endl;
		std::ofstream myfile;
		myfile.open ("client_" + std::to_string(i) + ".output", std::fstream::app);
		myfile << txnh.txn.output;
		myfile.close();
	}
	return NULL;
}
struct thread_args {
	std::vector<std::pair<std::string,int> > table_and_locks;
	std::vector<std::string> tables_original;
	std::vector<std::string> created_tables;
	std::vector<int> qcs;
	std::vector<std::vector<std::string> > qs;
	std::vector<std::vector<std::string> > colss;
	std::vector<std::vector<int> > conds;
	TransactionInstance* txn;
};

void* transaction_final_execution(void* _args) {
	//std:cout << "in final exec" << std::endl;
	struct thread_args *args = (struct thread_args *) _args;
	int client_id = args->txn->client_id;
	int trans_id = args->txn->trans_id;
	int k = lm.getLocks(client_id, args->table_and_locks);	// Acquire the locks in sorted order
	int res=1;

	//std:cout << "Client " << client_id  << " in final phase of exec" << std::endl;

	int copied_meta_data = 0;
    std::string output = "";
	for(int i = 0; i < args->table_and_locks.size(); i++) {
		std::string tbl = args->table_and_locks[i].first;

		// Copy metadata file if $-lock acquired (when table/contraints are being created)
		if(tbl=="$") {
			copied_meta_data = 1;
			std::string line;
			std::ifstream ini_file("meta_data.db");
			std::ofstream out_file("meta_data_" + std::to_string(client_id) + ".db");

			if(ini_file && out_file){
				while(getline(ini_file,line)){
					out_file << line << "\n";
				}		
			} else {
				//Something went wrong
				printf("Cannot read File\n");
				res = 0;
			}
			ini_file.close();
			out_file.close();
			//std:cout << "Meta data copied" << std::endl;
		}
	}

	// Make a local copy of the required tables
	for(int i = 0; i < args->table_and_locks.size(); i++) {
		std::string tbl = args->table_and_locks[i].first;
		int lock_type = args->table_and_locks[i].second;

		if(tbl!="$" && lock_type == 0) {
            //if the table will be created in the same transaction then do not create a copy
            int f = 0;
            for(int j = 0; j < args->created_tables.size(); j++) {
                std::string tbl1 = args->created_tables[j];
                if(tbl1 == tbl) {
                    f = 1;
                    break;
                }
            }
            if(f == 1) continue; 
			// Make a client's copy of a table when its X-lock has been acquired
			std::string line;
			std::ifstream ini_file(tbl + ".db");
			std::ofstream out_file(tbl + "_" + std::to_string(client_id) + ".db");
		
			if(ini_file && out_file){
				while(getline(ini_file,line)){
					out_file << line << "\n";
				}		
			} else {
				//Something went wrong
				printf("Cannot read File\n");
				res = 0;
			}
			ini_file.close();
			out_file.close();
			//std:cout << "Table file copied to " << tbl + "_" + std::to_string(client_id) + ".db" << std::endl;
		}
	}
	   
	for(int i = 0; i < args->qcs.size(); i++) {
		int copied_table = 0;
		for(int j = 0; j < args->table_and_locks.size(); j++) {
			std::string tbl = args->table_and_locks[j].first;
			int lock_type = args->table_and_locks[j].second;

			if(tbl!="$" && lock_type == 0) {
				if(tbl == args->tables_original[i]) {
					copied_table = 1;
					break;
				}
			}
		}
		for(int j = 0; j < args->created_tables.size(); j++) {
			std::string tbl = args->created_tables[j];
			if(tbl == args->tables_original[i]) {
				copied_table = 1;
				break;
			}
		}
		// std::cout << "Copied table " << copied_table << " Copied Metadata " << copied_meta_data << " Client Id " << client_id << std::endl;
		
		std::string output_temp;
		if(res == 1){
			executeQuery(args->qcs[i], args->qs[i], args->colss[i], args->conds[i], args->txn->client_id, copied_meta_data, copied_table, &res, output_temp);
			output = output + output_temp;
		}
		if(res == 1)
			std::cout << "A query of " << args->txn->client_id << " was executed completely" << std::endl;
		else{
			std::cout << "Query execution error in client " << args->txn->client_id << std::endl;
			break;
		}
	}

	// After the query has executed completely, copy back the modifed database and index from client to the main server
	for(int i = 0; i < args->table_and_locks.size(); i++) {
		std::string tbl = args->table_and_locks[i].first;
		int lock_type = args->table_and_locks[i].second;

		if(tbl!="$" && lock_type == 0) {
			// copy back table_<client_id>.db to table.db
			// do not copy here if it was also a created table in the same transaction
			int f = 0;
			for(int j = 0; j < args->created_tables.size(); j++) {
				std::string tbl1 = args->created_tables[j];
				if(tbl1 == tbl) {
					f = 1;
					break;
				}
			}
			if(f == 1) continue; 
			if(res == 1){
				std::string line;
				std::ifstream ini_file(tbl + "_" + std::to_string(client_id) + ".db");
				std::ofstream out_file(tbl + ".db");
				if(ini_file && out_file){
					while(getline(ini_file,line)){
						out_file << line << "\n";
					}		
				} else {
					//Something went wrong
					printf("Cannot read File\n");
				}
				ini_file.close();
				out_file.close();
			}

			//std:cout << "Copied local table back\n";
			
			std::string str = tbl + "_" + std::to_string(client_id) + ".db";
			int status = remove(str.c_str());
			std::cout << "Removed " << str << std::endl;
			if(status!=0)
				std::cout<<"\nError Occurred in deleting file!\n";
		}
	}

	// Update server's metadata if it was modified as well
	if(copied_meta_data) {
		if(res == 1){
			std::string line;
			std::ifstream ini_file1("meta_data_" + std::to_string(client_id) + ".db");
			std::ofstream out_file1("meta_data.db");

			if(ini_file1 && out_file1){
				while(getline(ini_file1,line)){
					out_file1 << line << "\n";
				}		
			} else {
				//Something went wrong
				printf("Cannot read File\n");
			}
			ini_file1.close();
			out_file1.close();
		}
		//std:cout << "Copied MetaData back\n";
		std::string str = "meta_data_" + std::to_string(client_id) + ".db";
		int status = remove(str.c_str());
		std::cout << "Removed " << str << std::endl;
		if(status!=0)
			std::cout<<"\nError Occurred in deleting file!\n";
	}
	for(int i = 0; i < args->created_tables.size(); i++) {
		// std::cout << args->created_tables[i] << std::endl;
		std::string old_file = args->created_tables[i] + "_" + std::to_string(client_id) + ".db";
		std::string new_file = args->created_tables[i] + ".db";
		if(res == 1) {
			std::ifstream ini_file1(old_file);
			std::ofstream out_file1(new_file);
			std::string line;
			if(ini_file1 && out_file1){
				while(getline(ini_file1,line)){
					out_file1 << line << "\n";
				}		
			} else {
				//Something went wrong
				printf("Cannot read File\n");
			}
			ini_file1.close();
			out_file1.close();
		}
		//std:cout << "Copied created table back\n";
		int status = remove(old_file.c_str());
		std::cout << "Removed " << old_file << std::endl;
		if(status!=0)
			std::cout<<"\nError Occurred in deleting file!\n";
	}

	// Release the locks, and signal the client thread that its transaction has finished execution
	k = lm.releaseLocks(client_id, args->table_and_locks);
	pthread_mutex_lock(&(args->txn->lock));
	args->txn->done = 1;
    args->txn->output = output;
	pthread_cond_signal(&(args->txn->cond));
	pthread_mutex_unlock(&(args->txn->lock));

	if(res == 1)
		std::cout << "Client " << client_id << " has finished transaction " << trans_id << std::endl;
	else
		std::cout << "Client " << client_id << " has aborted transaction " << trans_id << std::endl;
		

	return NULL;
}

void* server(void* d) {
	int l = *((int *)d);
	std::fstream file2 ("meta_data.db", std::fstream::in | std::fstream::out | std::fstream::app );
	file2.close();
	// we don't need this l anywhere though

	//std:cout << "Server tread has spawned" << std::endl;
	PF_Init();
	// The server thread never returns, it keeps checking if new transactions have been added to the queue 
	while(true) {
		// If the transaction queue is not empty, get the top-most txn and execute its queries
		pthread_mutex_lock(&lock);
		if(!transaction_queue.empty()) {
			TransactionInstance* txn = transaction_queue.front();
			transaction_queue.pop();
			pthread_mutex_unlock(&lock);
			std::vector<std::string> queries = txn->queries;  

			struct thread_args *args = new(struct thread_args);
			bool abort_trans = false;

			// Parse the queries to get information on required locks
			for(int i = 0; i < queries.size(); i++) {
				int res;
				parse_query(queries[i], &res);
				if (res == 0){
					std::cout << "Need to abort!" << std::endl;
					abort_trans = true;
				}
				else{
					std::string t = table;
					int lt = lock_type;		 // 0: X-Lock, 1: S-Lock, $: Database Locked
					int qc1 = qc;
					std::vector<std::string> q1 = q;
					std::vector<std::string> cols1 = cols;
					std::vector<int> cond1 = cond;
					if(created_table != "")
						args->created_tables.push_back(created_table);
					qc = -1;
					q.clear(); cols.clear(); cond.clear();
					table = "";
					created_table = "";
					if(t != "") {
						int found = 0;
						for(int j = 0; j < args->table_and_locks.size(); j++) {
							if(args->table_and_locks[j].first == t) {
								found = 1;
								if(args->table_and_locks[j].second > lt) {
									args->table_and_locks[j].second = lt;
								}
							}
						}
						if(found == 0) {
							args->table_and_locks.push_back(std::pair<std::string, int>(t, lt));
						}
					}
					args->tables_original.push_back(t);
					args->qcs.push_back(qc1);
					args->qs.push_back(q1);
					args->colss.push_back(cols1);
					args->conds.push_back(cond1);
				}
			}
			std::sort(args->created_tables.begin(), args->created_tables.end());
    		args->created_tables.erase(std::unique(args->created_tables.begin(), args->created_tables.end()), args->created_tables.end());

			if(!abort_trans){
				args->txn = txn;

				//std:cout << "transaction parsed" << std::endl;

				// Sort the locks required to avoid deadlocking
				std::sort(args->table_and_locks.begin(), args->table_and_locks.end());
				
				//std:cout << "locks sorted" << std::endl;

				// Create `transaction_final_execution` thread to execute parsed transactions
				//std:cout << "spawning final exec thread" << std::endl;
				pthread_t *p = (pthread_t *)malloc(sizeof(pthread_t));
				pthread_create(p, NULL, transaction_final_execution, (void *)args);
				//std:cout << "final exec spawned" << std::endl;
			}
			// Need to abort the transaction and exit
			else{
				std::cout << "Aborting transaction!" << std::endl;
				pthread_mutex_lock(&(txn->lock));
				txn->done = -1;
				pthread_cond_signal(&(txn->cond));
				pthread_mutex_unlock(&(txn->lock));
			}
		}
		else{
			pthread_mutex_unlock(&lock);
		}
	}
}


int main(int argc, char* argv[]) {

	int num_clients = 10;	// Set number of client threads
	int num_server = 1;	 // Number of server threads will always be 1

	int *client_thread_id;
	int *server_thread_id;

	// Spawn the client and server threads
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

	// Join the client threads once they have finished their execution
	for (i = 0; i < num_clients; i++)
	{
		pthread_join(client_thread[i], NULL);
		std::cout << "client " << i+1 << " has joined!" << std::endl;
	}
	pthread_cancel(server_thread[0]);
	return 0;

	// Join server thread when its function has finished execution
	// Server thread never joins as it is in a while(true) loop looking for new clients that might come in
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