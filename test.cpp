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

int main()
{
	// http get
	std::cerr << minicurl::get("http://httpbin.org/get") << '\n';
	
	// http get with query string
	std::cerr << minicurl::get("http://httpbin.org/get?testing=query") << '\n';
	
	// http get with header information (header always sent as a list)
	std::cerr << minicurl::get("http://httpbin.org/get", {"testing:header"}) << '\n';
	
	// http get with not-valued information in the header (note the semicolon instead of a colon)
	std::cerr << minicurl::get("http://httpbin.org/get", {"testing;"}) << '\n';
	
	// http get with multiple header information
	std::cerr << minicurl::get("http://httpbin.org/get", {"testing:header", "more:header", "still;"}) << '\n';
	
	// http post without payload (equivalent to get)
	std::cerr << minicurl::post("http://httpbin.org/get") << '\n';
	
	// http post with plain text as payload
	std::cerr << minicurl::post("http://httpbin.org/post", "tesing_payload") << '\n';
	
	// http post with stringfied json as payload
	std::cerr << minicurl::post("http://httpbin.org/post", "{testing:\"payload\"}") << '\n';
	
	// http post with payload and query string
	std::cerr << minicurl::post("http://httpbin.org/post?testing=query", "{testing:\"payload\"}") << '\n';
		
	// http post with payload and header information
	std::cerr << minicurl::post("http://httpbin.org/post", "{testing:\"payload\"}", {"testing:header"}) << '\n';
	
	// http post with payload and multiple header information
	std::cerr << minicurl::post("http://httpbin.org/post", "{testing:\"payload\"}", {"testing:header", "more:header", "still;"}) << '\n';
		
	return 0;
}
