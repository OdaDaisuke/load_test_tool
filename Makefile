build: main.cpp
	@clang++ -std=c++11 -o load main.cpp -lcurl -pthread
clean:
	@rm -f load
