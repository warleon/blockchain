default:
	g++ -g -o cli src/demo.cpp -lssl -lcrypto -std=c++17 -fsanitize=address
