////////////////////////////////////////////////////////////////////////////////
/// @brief simple arango importer
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2012 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Alexey Parhomenko
/// @author Copyright 2011-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Basics/FileUtils.h"
#include "DumpClient.h"

namespace triagens {
namespace arangoio {
namespace dump {

/**
 * Constructor
 */
DumpClient::DumpClient(triagens::httpclient::SimpleHttpClient * httpClient) :
	httpClient_(httpClient),
	httpResult_(0) {

}

/**
 * Destructor
 */
DumpClient::~DumpClient() {

	if (0 != httpResult_) {
		delete httpResult_;
	}

}

std::string DumpClient::getPath() throw (std::runtime_error) {

	if (path_.empty()) {
		throw std::runtime_error("Directory to save can't be empty.");
	}

	return path_;

}

void DumpClient::setPath(std::string path) throw (std::runtime_error) {

	if (!triagens::basics::FileUtils::exists(path)
		|| !triagens::basics::FileUtils::isDirectory(path)) {
		throw std::runtime_error("Path " + path + " isn't directory or can't exist");
	}

	path_ = path;

}

// Save data or metadata to file
void DumpClient::write(const std::string & url, const std::string & fileName) throw(std::runtime_error) {
	write(url, fileName, false);
}

void DumpClient::write(const std::string & url, const std::string & fileName, bool isMetaData) throw(std::runtime_error) {

	std::map<std::string, std::string> headerFields;

	if (0 != httpResult_) {
		delete httpResult_;
	}

	httpResult_ = httpClient_->request(
				triagens::rest::HttpRequest::HTTP_REQUEST_GET, url, 0, 0, headerFields);

	if (!httpResult_ || !httpResult_->isComplete()) {
		throw std::runtime_error("Can't send request to server.");
	}

	if (200 != httpResult_->getHttpReturnCode()) {
		throw std::runtime_error(httpResult_->getBody().str());
	}

	std::string file;
	file
		.append(getPath())
		.append("/")
		.append(fileName);

	if (isMetaData) {
		file.append(".metadata");
	}

	file.append(".json");

	std::ofstream stream (file.c_str());

	if (!stream.is_open()) {
		throw std::runtime_error("Can't write to file " + file);
	}

	if (!httpResult_->getBody().str().empty()) {
		stream << httpResult_->getBody().str().substr(0, httpResult_->getBody().str().size() - 26);
		stream << "}";
	}

	stream.close();
}

} /* namespace dump */
} /* namespace arangoio */
} /* namespace triagens */
