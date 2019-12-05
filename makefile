all:
	g++ -std=c++14 -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-result test.cpp -O3 -o test.exe