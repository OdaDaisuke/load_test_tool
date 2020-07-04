build: main.cpp
	@clang++ -std=c++11 -o main main.cpp -lcurl -pthread
clean:
	@rm -f main
