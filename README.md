# Minicurl

> This library depends on libcurl. To install the latter in your system, open the terminal and type:

	sudo apt-get install -y -f --install-suggests --install-recommends curl libcurl4-openssl-dev

> GCC 8 or later is required. To compile the test, open the terminal and type:

	g++ test.cpp -o test.out -O3 -march=native -std=c++2a -fopenmp -L/usr/lib/x86_64-linux-gnu -lcurl

Minicurl is a **very simple and limited** single header C++ wrapper around libcurl, intended to make the use of HTTP GET and HTTP POST easier (other types of REST requests were not implemented). Both methods are static and can be used anywhere in your code without the need to manually initialize or instantiate anything. Be aware that the first call of any of them will also call the function "curl_global_init", which is not thread-safe. Everything in minicurl is returned as std::string. Check "test.cpp" for examples.

*Copyright 2019 Jean Diogo (aka [Jango](mailto:jeandiogo@gmail.com))*
