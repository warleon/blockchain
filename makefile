.PHONY: node wallet

nodeFlags =  -o server -lhttpserver -std=c++17
walletFlags = -o wallet -lssl -lcrypto  -std=c++17
toyFlags = -o test -lssl -lcrypto  -std=c++17


default:
	g++ -o server src/api.cpp -lhttpserver -std=c++17
	g++ -o client src/wallet.cpp  -std=c++17

node:
	g++ src/api.cpp $(nodeFlags)

wallet:
	g++ src/wallet.cpp $(walletFlags)

toy:
	g++ src/getprivatekey.cpp $(toyFlags)

