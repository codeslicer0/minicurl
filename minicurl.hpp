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

#ifndef WINDOWS

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

#else

#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <string>
#include <vector>

#endif

class minicurl
{
	static minicurl& get_singleton()
	{
		static minicurl singleton;
		return singleton;
	}
	
	struct chunk
	{
		std::size_t size;
		char * data;

		friend void swap(chunk & x, chunk & y)
		{
			using std::swap;
			swap(x.size, y.size);
			swap(x.data, y.data);
		}

		chunk(std::size_t s = 0) : size(s)
		{
			data = (char *)malloc(sizeof(char) * (size + 1));
			if (data)
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
			data = (char *)malloc(sizeof(char) * (size + 1));
			if (data)
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
			if (data)
			{
				free(data);
				data = nullptr;
			}
			size = 0;
		}

		void save(std::iostream& file)
		{
			// persist data as is, without the null terminator
			file.write(data, size);
			file.flush();
		}
		
		std::string to_string() const
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
		
		// response not found
#pragma region Error Handling
		bool isValid() const
		{
			return !isEmpty() && !hasErrors();
		}
		
		bool hasErrors() const
		{
			std::string response = content.to_string();
			return isNotFound(response) || isNotAuthorized(response);
		}

		bool isNotFound() const
		{
			return isNotFound(content.to_string());
		}

		bool isEmpty() const
		{
			return content.size == 0;
		}

		static bool isNotFound(const std::string& response)
		{
			return response.find("<title>404 ") != -1;
		}

		bool isNotAuthorized() const
		{
			return isNotAuthorized(content.to_string());
		}

		// response not authorized
		static bool isNotAuthorized(const std::string& response)
		{
			return response.find("<title>403 ") != -1;
		}
#pragma endregion

		void save(std::iostream& file)
		{
			content.save(file);
		}
		
		friend void swap(package & x, package & y)
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
		
		package& operator=(package other)
		{
			swap(*this, other);
			return *this;
		}
		
		~package()
		{
			status = 0;
		}
	};
	
	static std::vector<std::string> split(std::string const & data, std::string const & delimiters = " \n\t")
	{
		std::vector<std::string> tokens;
		size_t const size = data.size();
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

	static size_t debug_function(CURL * Handle, curl_infotype DebugInfoType, char * DebugInfo, size_t DebugInfoSize, void* UserData)
	{
		switch (DebugInfoType)
		{
		case CURLINFO_TEXT:
		{
			// in this case DebugInfo is a C string (see http://curl.haxx.se/libcurl/c/debug.html)
			// C string is not null terminated:  https://curl.haxx.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html

			// Truncate at 1023 characters. This is just an arbitrary number based on a buffer size seen in
			// the libcurl code.
			DebugInfoSize = min(DebugInfoSize, (size_t)1023);

			// Calculate the actual length of the string due to incorrect use of snprintf() in lib/vtls/openssl.c.
			char* FoundNulPtr = (char*)memchr(DebugInfo, 0, DebugInfoSize);
			int CalculatedSize = FoundNulPtr != nullptr ? FoundNulPtr - DebugInfo : DebugInfoSize;

			std::string DebugText(DebugInfo, CalculatedSize);

			//findAndReplaceAll(DebugText, "\n", "");
			//findAndReplaceAll(DebugText, "\r", "");

			std::cout << DebugText << "\n";
		}
		break;

		case CURLINFO_HEADER_IN:
			std::cout << "Received header (" << DebugInfoSize << "bytes)\n";
			break;

		case CURLINFO_HEADER_OUT:
		{
			// C string is not null terminated:  https://curl.haxx.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html

			// Scan for \r\n\r\n.  According to some code in tool_cb_dbg.c, special processing is needed for
			// CURLINFO_HEADER_OUT blocks when containing both headers and data (which may be binary).
			//
			// Truncate at 1023 characters. This is just an arbitrary number based on a buffer size seen in
			// the libcurl code.
			int RecalculatedSize = min(DebugInfoSize, (size_t)1023);
			for (int Index = 0; Index <= RecalculatedSize - 4; ++Index)
			{
				if (DebugInfo[Index] == '\r' && DebugInfo[Index + 1] == '\n'
					&& DebugInfo[Index + 2] == '\r' && DebugInfo[Index + 3] == '\n')
				{
					RecalculatedSize = Index;
					break;
				}
			}

			// As lib/http.c states that CURLINFO_HEADER_OUT may contain binary data, only print it if
			// the header data is readable.
			bool bIsPrintable = true;
			for (int Index = 0; Index < RecalculatedSize; ++Index)
			{
				unsigned char Ch = DebugInfo[Index];
				if (!isprint(Ch) && !isspace(Ch))
				{
					bIsPrintable = false;
					break;
				}
			}

			if (bIsPrintable)
			{
				std::string DebugText(DebugInfo, RecalculatedSize);

				//findAndReplaceAll(DebugText, "\n", "");
				//findAndReplaceAll(DebugText, "\r", "");

				std::cout << "Sent header (" << RecalculatedSize << " bytes) - " << DebugText << "\n";
			}
			else
			{
				std::cout << "Sent header (" << RecalculatedSize << " bytes) - contains binary data\n";
			}
		}
		break;

		case CURLINFO_DATA_IN:
			std::cout << "Received data (" << DebugInfoSize << " bytes)\n";
			break;

		case CURLINFO_DATA_OUT:
			std::cout << "Sent data (" << DebugInfoSize << " bytes)\n";
			break;

		case CURLINFO_SSL_DATA_IN:
			std::cout << "Received SSL data (" << DebugInfoSize << " bytes)\n";
			break;

		case CURLINFO_SSL_DATA_OUT:
			std::cout << "Sent SSL data (" << DebugInfoSize << " bytes)\n";
			break;

		default:
			std::cerr << "DebugCallback: Unknown DebugInfoType=" << (int)DebugInfoType << "(DebugInfoSize: " << DebugInfoSize << " bytes)\n";
			break;
		}

		return DebugInfoSize;
	}
	
	static size_t write_function(void * buffer, std::size_t size, std::size_t count, void * stream)
	{
		std::size_t realsize = size * count;
		chunk * memory = (chunk *) stream;
		char * aux = (char *) realloc(memory->data, memory->size + realsize);
		if(aux)
		{
			// save stream as is without adding the null terminator
			memory->data = aux;
			memcpy(memory->data + memory->size, buffer, realsize);
			memory->size += realsize;
		}
		else
		{
			realsize = 0;
		}
		return realsize;
	}
	
	// trim from start (in place)
	static inline void ltrim(std::string &s) 
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) 
		{
			return !std::isspace(ch) && ch != '\r' && ch != '\n';
		}));
	}

	// trim from end (in place)
	static inline void rtrim(std::string &s) 
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) 
		{
			return !std::isspace(ch) && ch != '\r' && ch != '\n';
		}).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(std::string &s) 
	{
		ltrim(s);
		rtrim(s);
	}

	static std::size_t get_content_length(const std::string& header)
	{
		std::string key, val;
		std::istringstream iss(header);

		while (std::getline(std::getline(iss, key, ':') >> std::ws, val))
		{
			trim(val);
			if (key == "Content-Length")
			{
				return atol(val.c_str());
			}
		}

		return 0;
	}
	
	package fetch(std::string const & url, std::string const & payload, std::string const & filename, bool save_to_disk, std::vector<std::string> const & headers)
	{
		std::size_t status = 0;
		chunk header;
		chunk content;
		
		if(url.size())
		{
			CURL * curl = curl_easy_init();
			if(curl)
			{
				// download file handle, if we are saving directly to disk
				FILE * save_file = nullptr;

				// upload file handle
				FILE * upload_file = nullptr;

				// make read timeout small because we are supporting retries
				curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000);

				// save large files directly to disk
				if (save_to_disk)
				{
					if (filename.size() && (save_file = fopen(filename.c_str(), "ab")))
					{
						// write curl response to file
						curl_easy_setopt(curl, CURLOPT_FILE, save_file);
						curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
					}
				}
				else
				{
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&content);
				}

				curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_function);
				curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *) &header);

				// Always setup the debug function to allow for activity to be tracked
				curl_easy_setopt(curl, CURLOPT_DEBUGDATA, this);
				curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_function);
				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
				
				if(payload.size())
				{
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
				}
				
				if (!save_to_disk)
				{
					if (filename.size() && (upload_file = fopen(filename.c_str(), "rb")))
					{
						struct stat file_stat;
						if (fstat(fileno(upload_file), &file_stat) == 0)
						{
							curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
							curl_easy_setopt(curl, CURLOPT_READDATA, upload_file);
							curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, file_stat.st_size);
						}
					}
				}
				
				bool bHasContentLength = false;
				struct curl_slist * header_list = nullptr;
				for(std::string h : headers)
				{
					if(h.size())
					{
						auto tokens = split(h, ":");
						if((tokens.size() == 1 || (tokens.size() == 2 && tokens[1].empty())) && tokens[0].back() != ';')
						{
							if (tokens.front() == "Content-Length")
								bHasContentLength = true;

							h = tokens.front() + ";";
						}
						header_list = curl_slist_append(header_list, h.c_str());
					}
				}

				// content-length should be present http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.4
				if (bHasContentLength)
					header_list = curl_slist_append(header_list, "Content-Length: -1");

				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
				
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

				CURLcode res = curl_easy_perform(curl);
				while (res = CURLE_PARTIAL_FILE)
				{
					std::size_t max_length = get_content_length(header.to_string());
					std::size_t file_size;

					if (save_to_disk)
					{
						fflush(save_file);

						// get the number of bytes written and try again
						fseek(save_file, 0L, SEEK_END);
						file_size = ftell(save_file);
					}
					else
					{
						file_size = content.size;
					}
					
					if (max_length <= file_size)
					{
						res = CURLE_OK;
						break;
					}
					curl_easy_setopt(curl, CURLOPT_RESUME_FROM, file_size);
					res = curl_easy_perform(curl);
				}
				
				if(res == CURLE_OK)
				{
					long status_code = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
					status = static_cast<std::size_t>(status_code);
				}
				
				if(save_file)
				{
					fclose(save_file);
				}

				if (upload_file)
				{
					fclose(upload_file);
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
	
	static std::string get(std::string const & url, std::vector<std::string> const & headers = {})
	{
		return get_singleton().fetch(url, "", "", false, headers).content.to_string();
	}
	
	static std::string get_header(std::string const & url, std::vector<std::string> const & headers = {})
	{
		return get_singleton().fetch(url, "", "", false, headers).header.to_string();
	}
	
	static std::string post(std::string const & url, std::string const & payload, std::vector<std::string> const & headers = {"Content-Type: text/plain"})
	{
		return get_singleton().fetch(url, payload, "", false, headers).content.to_string();
	}
	
	static std::string upload(std::string const & url, std::string const & filename, std::vector<std::string> const & headers = {"Content-Type: text/plain"})
	{
		return get_singleton().fetch(url, "", filename, false, headers).content.to_string();
	}

	static bool file_exists(const std::string& name) 
	{
		std::ifstream f(name.c_str());
		return f.good();
	}
	
	static std::string download(std::string const & url, std::string const & filename = "", bool save_to_disk=false, std::vector<std::string> const & headers = {})
	{
		if(url.size())
		{
			std::string confirmed_filename;
			
			// if saving directly to disk, the filename will be interpreted as the download file location path
			if (save_to_disk)
				confirmed_filename = filename.size() ? filename : split(url, "/").back();

			// combile url and filename to get the full url path
			package result = get_singleton().fetch(url, "", confirmed_filename, save_to_disk, headers);
			if (!result.isValid())
			{
				// report errors
				if (result.isNotFound())
				{
					std::cerr << "File not found: " << url << "\n";
				}
				else if (result.isNotAuthorized())
				{
					std::cerr << "Access not authorized: " << url << "\n";
				}
				else
				{
					std::cerr << "No data returned: " << url << "\n";
				}

				return std::string("");
			}
			else
			{
				if (save_to_disk)
				{
					// just check for the existence of the file
					if (file_exists(confirmed_filename))
						return confirmed_filename;
				}
				else
				{
					std::string confirmed_filename = filename.size() ? filename : split(url, "/").back();
					std::fstream file = std::fstream(confirmed_filename, std::fstream::out | std::fstream::binary);
					if (file.good())
					{
						// save file
						result.save(file);

						file.close();
						return confirmed_filename;
					}
				}
			}
		}

		return std::string("");
	}
};

#endif
