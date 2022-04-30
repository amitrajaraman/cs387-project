# Transaction Management

This is the Git repository which hosts the code for our CS387 project that deals with transaction management. 
The team consists of four members:
1. Amit Rajaraman - 190050015
2. Aman Yadav - 190050013
3. Akash Cherukuri - 190050009
4. Vigna Surapaneni - 190050121

## Running the code

Create files `client<i>_t<j>.txt` with queries for jth transaction for the ith client in the folder `testfiles`

* Perform `make` in each of the directories `pflayer`, `amlayer`, and `dblayer` in that order.
* From `dblayer`, run `./interface` to open the terminal.
* Or alternatively run `./run.sh`

It will save the outputs for `client<i>` in the file `client_<i>.output`. And if the transaction is aborted then the all the changes are not reflected in the database and the output file will not contain the output of the aborted transaction.

The types of queries that may be performed are as follows:
* `create table file <file_name> index <col_number>`. This creates a table from the csv file `<file_name>`, with the name of the table being the filename without the csv extension. The first row of the file is taken as the schema, and it must be of a form similar to `Country:varchar,Capital:varchar,Population:int`. The `<col_number>`th column is taken as the index of the table.
* `dump all <table_name>`. This prints all rows of the table.
* `dump all <table_name> where <condition>`. This prints all rows of the table where the given condition is true. The condition is of the form `<cmp> <num>`, where `<cmp>` is one of `lt`, `gt`, `neq`, `geq`, `leq`, and `eq`, and `<num>` is the number that is compared against the values of each row of the indexed column.
* Similar to the above two queries, we also have `dump <column_list> <table_name>` and `dump <column_list> <table_name> where <condition>`, where `column_list` is a comma-separated list of all the columns to be printed.
* `add constraint [eq/lt/gt/leq/geq/neq] <num> as <constraint_name> into <table_name>`, for adding a constraint with name <constraint_name> on the indexing column of <table_name>.
* `dump constraint <table_name>` to list all the constraints of a table.
Note that the query language is case-sensitive! So, with the schema example mentioned in the first bullet above, `dump Population` will work while `dump population` will not.
