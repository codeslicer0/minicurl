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
	std::cout << "HTTP GET:\n\n" << minicurl::get("http://httpbin.org/get") << "\n\n";
	
	std::cout << "HTTP GET with query string:\n\n" << minicurl::get("http://httpbin.org/get?HELLO=WORLD") << "\n\n";
	
	std::cout << "HTTP GET with header information:\n\n" << minicurl::get("http://httpbin.org/get", {"HELLO:WORLD", "GOODBYE:WORLD"}) << "\n\n";
	
	std::cout << "HTTP GET with non-valued header information:\n\n" << minicurl::get("http://httpbin.org/get", {"HELLO:", "WORLD;", "GOODBYE"}) << "\n\n";
	
	std::cout << "HTTP POST with plain text as payload:\n\n" << minicurl::post("http://httpbin.org/post", "HELLO_WORLD") << "\n\n";
	
	std::cout << "HTTP POST with stringfied json as payload:\n\n" << minicurl::post("http://httpbin.org/post", "{HELLO:\"WORLD\"}", {"Content-type: application/json"}) << "\n\n";
	
	std::cout << "Getting header information:\n\n" << minicurl::get_header("http://httpbin.org/get") << "\n\n";
	
	std::cout << "Uploading a file to an address:\n\n" << minicurl::upload("https://httpbin.org/put", "README.md") << "\n\n";
	
	std::cout << "Downloading to an optionally specified file (returns filename if succeeded, empty string if failed):\n\n" << minicurl::download("http://httpbin.org/get", "DOWNLOADED.txt") << "\n\n";
	
	return 0;
}
