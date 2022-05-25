rm ../*.db ../*.output
cp ./csvs/* ../
if [[ "$1" != "4" && "$1" != "5" ]]
then
	rm -r "../testfiles"
	cp -r "./testfiles$1" "../testfiles"
fi
cd ..
cd "../pflayer" && make
cd "../amlayer" && make
cd "../dblayer"
if [[ "$1" == "4" || "$1" == "5" ]]
then
	make
	rm -r "./testfiles"
	cp -r "./demo/testfiles$1_1" "./testfiles"
	./interface
	rm -r "./testfiles"
	cp -r "./demo/testfiles$1_2" "./testfiles"
	./interface
else
	make
	./interface
fi
cd demo

# 1: 1 client -- working of basic commands
# 2: 1 client -- working of constraints
# 3: 2 clients -- basic concurrency, x/s/$-locks
# 4: 2 clients -- isolation
# 5: 10 clients -- working of s-locks
# 6: 3 clients -- complicated transaction testcase