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

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
 
class minicurl
{
	static auto & get_singleton()
	{
		static minicurl singleton;
		return singleton;
	}
	
	struct chunk
	{
		std::size_t size;
		char * data;
		
		friend auto swap(chunk & x, chunk & y)
		{
			using std::swap;
			swap(x.size, y.size);
			swap(x.data, y.data);
		}
		
		chunk(std::size_t s = 0) : size(s)
		{
			data = (char *) malloc(sizeof(char) * (size + 1));
			if(data)
			{
				data[size] = '\0';
			}
			else
			{
				size = 0;
			}
		}
		
		chunk(chunk const & other) : size(other.size)
		{
			data = (char *) malloc(sizeof(char) * (size + 1));
			if(data)
			{
				memcpy(data, other.data, size);
				data[size] = '\0';
			}
			else
			{
				size = 0;
			}
		}
		
		chunk(chunk && other) : chunk()
		{
			swap(*this, other);
		}
		
		chunk & operator=(chunk other)
		{
			swap(*this, other);
			return *this;
		}
		
		~chunk()
		{
			if(data)
			{
				free(data);
				data = nullptr;
			}
			size = 0;
		}
		
		auto to_string()
		{
			std::string content = "";
			if(size && data)
			{
				content = std::string(data, size);
			}
			return content;
		}
	};
	
	struct package
	{
		std::size_t status = 0;
		chunk header;
		chunk content;
		
		friend auto swap(package & x, package & y)
		{
			using std::swap;
			swap(x.status, y.status);
			swap(x.header, y.header);
			swap(x.content, y.content);
		}
		
		package()
		{
		}
		
		package(std::size_t s, chunk const & h, chunk const & c) : status(s), header(h), content(c)
		{
		}
		
		package(package const & other) : status(other.status), header(other.header), content(other.content)
		{
		}
		
		package(package && other) : package()
		{
			swap(*this, other);
		}
		
		auto & operator=(package other)
		{
			swap(*this, other);
			return *this;
		}
		
		~package()
		{
			status = 0;
		}
	};
	
	static auto split(std::string const & data, std::string const & delimiters = " \n\t")
	{
		std::vector<std::string> tokens;
		auto const size = data.size();
		if(size)
		{
			std::size_t current = 0;
			for(std::size_t i = 0; i < size; ++i)
			{
				if(strchr(delimiters.c_str(), data[i]))
				{
					tokens.emplace_back(&(data[current]), (i - current));
					current = i + 1;
				}
			}
			tokens.emplace_back(&(data[current]), (size - current));
		}
		return tokens;
	}
	
	static auto write_function(void * buffer, std::size_t size, std::size_t count, void * stream)
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
		else
		{
			realsize = 0;
		}
		return realsize;
	}
	
	auto fetch(std::string const & url, std::string const & payload, std::string const & filename, std::vector<std::string> const & headers)
	{
		std::size_t status = 0;
		chunk header;
		chunk content;
		
		if(url.size())
		{
			CURL * curl = curl_easy_init();
			if(curl)
			{
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
				curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_function);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &content);
				curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *) &header);
				
				if(payload.size())
				{
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
				}
				
				FILE * file = nullptr;
				if(filename.size() && (file = fopen(filename.c_str(), "rb")))
				{
					struct stat file_stat;
					if(fstat(fileno(file), &file_stat) == 0)
					{
						curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
						curl_easy_setopt(curl, CURLOPT_READDATA, file);
						curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, file_stat.st_size);
					}
				}
				
				struct curl_slist * header_list = nullptr;
				for(auto h : headers)
				{
					if(h.size())
					{
						auto tokens = split(h, ":");
						if((tokens.size() == 1 || (tokens.size() == 2 && tokens[1].empty())) && tokens[0].back() != ';')
						{
							h = tokens.front() + ";";
						}
						header_list = curl_slist_append(header_list, h.c_str());
					}
				}
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
				
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
				if(curl_easy_perform(curl) == CURLE_OK)
				{
					long status_code = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
					status = static_cast<std::size_t>(status_code);
				}
				
				if(file)
				{
					fclose(file);
				}
				if(header_list)
				{
					curl_slist_free_all(header_list);
				}
				curl_easy_cleanup(curl);
			}
		}
		
		return package(status, header, content);
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
		return get_singleton().fetch(url, "", "", headers).content.to_string();
	}
	
	static auto get_header(std::string const & url, std::vector<std::string> const & headers = {})
	{
		return get_singleton().fetch(url, "", "", headers).header.to_string();
	}
	
	static auto post(std::string const & url, std::string const & payload, std::vector<std::string> const & headers = {"Content-Type: text/plain"})
	{
		return get_singleton().fetch(url, payload, "", headers).content.to_string();
	}
	
	static auto upload(std::string const & url, std::string const & filename, std::vector<std::string> const & headers = {"Content-Type: text/plain"})
	{
		return get_singleton().fetch(url, "", filename, headers).content.to_string();
	}
	
	static auto download(std::string const & url, std::string const & filename = "", std::vector<std::string> const & headers = {})
	{
		if(url.size())
		{
			auto confirmed_filename = filename.size() ? filename : split(url, "/").back();
			auto file = std::fstream(confirmed_filename, std::fstream::out | std::fstream::binary);
			if(file.good())
			{
				auto chunk = get_singleton().fetch(url, "", "", headers).content;
				if(chunk.size > 1)
				{
					file << chunk.data << std::flush;
				}
				file.close();
				return confirmed_filename;
			}
		}
		return std::string("");
	}
};

#endif
