#include <iostream>
#include <string>
int parse_query(std::string s);

int main(int argc, char* argv[]) {
    std::string s = "create table file data.csv index 2";
    parse_query(s);
    s = "dump all data";
    parse_query(s);
}