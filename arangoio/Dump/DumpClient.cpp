////////////////////////////////////////////////////////////////////////////////
/// @brief simple arango collection data & metadata exporter
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
/// @author Copyright 2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <fstream>

#include "Basics/FileUtils.h"
#include "BasicsC/json.h"

#include "DumpClient.h"

using namespace triagens::arangoio::dump;

// -----------------------------------------------------------------------------
// --SECTION--                                                        DumpClient
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors & destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDump
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create the dump client
////////////////////////////////////////////////////////////////////////////////

DumpClient::DumpClient(triagens::httpclient::SimpleHttpClient * httpClient) :
    httpClient_(httpClient), 
    httpResult_(0), 
    rewriteExistsPath_(false) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the dump client
////////////////////////////////////////////////////////////////////////////////

DumpClient::~DumpClient() {

  if (0 != httpResult_) {
    delete httpResult_;
  }

}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDump
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief fetch the names of all collections from server and return them
////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> DumpClient::getCollections() throw (std::runtime_error) {

  std::vector<std::string> collections;

  sendRequest("/_api/collection");

  TRI_json_t * json = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE,
      httpResult_->getBody().str().c_str());

  if (json) {

    TRI_json_t * cols = TRI_LookupArrayJson(json, "collections");

    if (TRI_JSON_LIST == cols->_type) {

      TRI_json_t * col;
      TRI_json_t * name;

      for (size_t i = 0, n = cols->_value._objects._length; i < n; i++) {

        col = (TRI_json_t*) TRI_AtVector(&cols->_value._objects, i);

        if (TRI_JSON_ARRAY != col->_type) {
          continue;
        }

        name = TRI_LookupArrayJson(col, "name");

        if (TRI_JSON_STRING != name->_type) {
          continue;
        }

        collections.push_back(name->_value._string.data);

      }

    }

  }

  // this will free the json struct with all sub-elements
  TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);

  return collections;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the output path
///
/// note: this will throw an exception if the path is empty
////////////////////////////////////////////////////////////////////////////////

std::string DumpClient::getPath() throw (std::runtime_error) {

  if (path_.empty()) {
    throw std::runtime_error("Output path must not be empty.");
  }

  return path_;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief return whether it is allowed to overwrite files in the output path
////////////////////////////////////////////////////////////////////////////////

bool DumpClient::isRewriteExistsPath() {
  return rewriteExistsPath_;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief set the output path
///
/// this will try to create the output path directory if it does not yet exist.
/// if creating the directory fails, or the directory already exists and we
/// disallow overwriting data, an exception will be thrown
////////////////////////////////////////////////////////////////////////////////

void DumpClient::setPath(std::string path) throw (std::runtime_error) {

  if (! triagens::basics::FileUtils::exists(path)) {
    // output path does not yet exist

    int err = 0;

    // try to create it
    if (! triagens::basics::FileUtils::createDirectory(path, 0700, &err)) {
      throw std::runtime_error("Can't create directory '" + path + "'");
    }

  }
  else {
    // output path already exists

    if (! isRewriteExistsPath()) {
      // not allowed to overwrite data in output path
      throw std::runtime_error(
          "Output directory '" + path
              + "' already exists. Please choose another directory to save the dump.");
    }

  }

  // check if the output path is a directory that we can write to

  if (! triagens::basics::FileUtils::isDirectory(path)) {
    throw std::runtime_error("Output path '" + path + "' is not a directory.");
  }
  else if (! triagens::basics::FileUtils::isWritable(path)) {
    throw std::runtime_error("Output path '" + path + "' is not writeable.");
  }
  
  // if we get here, the output path should be a valid directory that we can write into

  path_ = path;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the flag whether overwriting existing files in the output
/// path is allowed
////////////////////////////////////////////////////////////////////////////////

void DumpClient::setRewriteExistsPath(bool isRewrite) {
  rewriteExistsPath_ = isRewrite;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief send request to server and get result
////////////////////////////////////////////////////////////////////////////////

void DumpClient::sendRequest(const std::string & url) throw (std::runtime_error) {

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

    std::string error = httpResult_->getBody().str();

    // Parse error
    TRI_json_t * json = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE, error.c_str());

    if (json) {

      // get the error message. This returns a pointer, not a copy
      TRI_json_t * errorMessage = TRI_LookupArrayJson(json, "errorMessage");
      if (errorMessage) {
        if (errorMessage->_type == TRI_JSON_STRING) {
          error = std::string(errorMessage->_value._string.data,
              errorMessage->_value._string.length);
        }
      }

    }

    // this will free the json struct will a sub-elements
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);

    throw std::runtime_error(error);
  }

}

////////////////////////////////////////////////////////////////////////////////
/// @brief save data or metadata to file
/// just dispatches work to other write() method
////////////////////////////////////////////////////////////////////////////////

void DumpClient::write(const std::string & url, const std::string & fileName)
    throw (std::runtime_error) {
  write(url, fileName, false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief save data or metadata to file
/// this method does the actual work:
/// - fetch data from URL
/// - save data into file
////////////////////////////////////////////////////////////////////////////////

void DumpClient::write(const std::string & url, const std::string & fileName,
    bool isMetaData) throw (std::runtime_error) {

  sendRequest(url);

  std::string file;
  file.append(getPath()).append(TRI_DIR_SEPARATOR_STR).append(fileName);

  if (isMetaData) {
    file.append(".metadata");
  }

  file.append(".json");

  std::ofstream stream(file.c_str());

  if (! stream.is_open()) {
    throw std::runtime_error("Can't write to file '" + file + "'");
  }

  if (! httpResult_->getBody().str().empty()) {
    stream
        << httpResult_->getBody().str().substr(0,
            httpResult_->getBody().str().size() - 26);
    stream << "}";
  }

  stream.close();
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
