////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Minicurl
// Copyright 2019 Jean Diogo (aka Jango) <jeandiogo@gmail.com>
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// test.cpp
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "minicurl.hpp"

#include <iostream>
#include <chrono>

struct runtime
{
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::high_resolution_clock::now();
	~runtime() {std::cout << "Runtime: " << (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start)).count() << " ms\n";}
}
rt;

// macro to print the results
#define PUTS(x); {std::cerr << (x) << '\n';}

int main()
{
	// http get
	PUTS(curl::get("http://httpbin.org/get"));
	
	// http get with query string
	PUTS(curl::get("http://httpbin.org/get?testing=query"));
	
	// http get with header information
	PUTS(curl::get("http://httpbin.org/headers", "testing:header"));
	
	// http get with multiple header information (as a list of strings)
	PUTS(curl::get("http://httpbin.org/headers", {"testing:header", "more:header", "still;"}));
	
	// http post without payload (same as http get)
	PUTS(curl::post("http://httpbin.org/get"));
	
	// http post with payload
	PUTS(curl::post("http://httpbin.org/post", "tesing_payload"));
	
	// http post with a stringfied json as payload
	PUTS(curl::post("http://httpbin.org/post", "{testing:\"payload\"}"));
	
	// http post with payload and header information
	PUTS(curl::post("http://httpbin.org/post", "{testing:\"payload\"}", "testing:header"));
	
	// http post with payload and multiple header information	
	PUTS(curl::post("http://httpbin.org/post", "{testing:\"payload\"}", {"testing:header", "more:header", "still;"}));
	
	return 0;
}
