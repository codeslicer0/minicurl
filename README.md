# Minicurl

Minicurl is a **very simple and limited** header-only C++ wrapper around libcurl, intended to make easier the use of HTTP GET and HTTP POST, as well as file uploading and downloading. All methods were implemented as static member functions and they can be used anywhere in your code without the need to manually initialize or instantiate anything. Be aware that the first call to any of them will also call the function *curl_global_init*, which is not thread-safe. The results are always returned as std::string. Check *test.cpp* for examples.

> This library depends on libcurl. To install the latter in your system, open the terminal and type:

	sudo apt-get install -y -f --install-suggests curl libcurl4-openssl-dev

> To compile the test, open the terminal and enter the command below (GCC 8 or later is required):

	g++ test.cpp -std=c++17 -lcurl -o test.out

*Copyright 2019 Jean Diogo (aka [Jango](mailto:jeandiogo@gmail.com))*
