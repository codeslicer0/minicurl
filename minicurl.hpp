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
// minicurl.hpp
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MINICURL_HPP
#define MINICURL_HPP

#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <curl/curl.h>
	
class minicurl
{
	static auto & get_singleton()
	{
		static minicurl singleton;
		return singleton;
	}
	
	struct chunk
	{
		std::size_t status;
		std::size_t size;
		char * data;
		
		friend void swap(chunk & x, chunk & y)
		{
			using std::swap;
			swap(x.status, y.status);
			swap(x.size, y.size);
			swap(x.data, y.data);
		}
		
		chunk(std::size_t s = 0) : status(0), size(s)
		{
			data = (char *) malloc(sizeof(char) * (size + 1));
			if(data) data[size] = '\0';
			else size = 0;
		}
		
		chunk(chunk const & c) : status(c.status), size(c.size)
		{
			data = (char *) malloc(sizeof(char) * (size + 1));
			if(data)
			{
				memcpy(data, c.data, size);
				data[size] = '\0';
			}
			else size = 0;
		}
		
		chunk(chunk && c) : chunk() {swap(*this, c);}
		chunk & operator=(chunk c) {swap(*this, c); return *this;}
		~chunk() {if(data) free(data);}
		
		auto to_string()
		{
			auto content = std::string("");
			if(size) content = std::string(data, size);
			return content;
		}
	};
	
	static std::size_t write_function(void * buffer, std::size_t size, std::size_t count, void * stream)
	{
		std::size_t realsize = size * count;
		chunk * memory = (chunk *) stream;
		char * aux = (char *) realloc(memory->data, memory->size + realsize + 1);
		if(aux)
		{
			memory->data = aux;
			memcpy(&(memory->data[memory->size]), buffer, realsize);
			memory->size += realsize;
			memory->data[memory->size] = 0;
		}
		else realsize = 0;
		return realsize;
	}
	
	auto fetch(std::string const & url, std::string const & payload = "", std::vector<std::string> const & headers = {})
	{
		chunk response;
		CURL * curl = curl_easy_init();
		if(curl)
		{
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			if(payload.size()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
			struct curl_slist * header = nullptr;
			for(auto const & h : headers) if(h.size()) header = curl_slist_append(header, h.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
			if(curl_easy_perform(curl) == CURLE_OK)
			{
				long status = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
				response.status = static_cast<std::size_t>(status / 3);
			}
			if(header) curl_slist_free_all(header);
			curl_easy_cleanup(curl);
		}
		return response;
	}
	
	minicurl()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	
	public:
	
	~minicurl()
	{
		curl_global_cleanup();
	}
	
	minicurl(minicurl const &) = delete;
	minicurl(minicurl &&) = delete;
	minicurl & operator=(minicurl) = delete;
	
	static auto get(std::string const & url, std::vector<std::string> const & headers = {})
	{
		return get_singleton().fetch(url, "", headers).to_string();
	}
	
	static auto post(std::string const & url, std::string const & payload = "", std::vector<std::string> const & headers = {})
	{
		return get_singleton().fetch(url, payload, headers).to_string();
	}
};

#endif
