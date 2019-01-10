# Minicurl

> Type *make* in the terminal to compile and run the test (GCC 8.0.0 or later is required).

> This library depends on *libcurl*. To install the latter in your system, open the terminal and type:

	sudo apt-get install -y -f --install-suggests --install-recommends curl libcurl4-openssl-dev

Minicurl is a very simple and limited single header C++ wrapper around *libcurl*, intended to make the use of HTTP GET and HTTP POST easier. Both methods were implemented as static methods and can be used anywhere in your code without the need to manually initialize or instantiate anything. However, note that an instance of CURL will be initialized on the first use of any of these methods, so the function *curl_global_init* will be called (as well as the function *curl_global_cleanup* will be called when the program ends). Since this may affect your code globally, we recommend you to use *minicurl* just in case all the REST requests your program does are gets and posts. Everything in *minicurl* is returned as std::string. Check the file *test.cpp* for examples.

*Copyright 2019 Jean Diogo (aka [Jango](mailto:jeandiogo@gmail.com))*
