.PHONY: node wallet

default:
	g++ -o server src/api.cpp -lhttpserver -std=c++17
	g++ -o client src/wallet.cpp  -std=c++17

node:
	g++ -o server src/api.cpp -lhttpserver -std=c++17

wallet:
	g++ -o client src/wallet.cpp -lssl -lcrypto  -std=c++17

