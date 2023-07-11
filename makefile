default:
	g++ -g -o cli src/demo.cpp -lssl -lcrypto -pthread -std=c++17 -fsanitize=address
