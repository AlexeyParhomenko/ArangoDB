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

DumpClient::DumpClient (triagens::httpclient::SimpleHttpClient * httpClient) :
    httpClient_(httpClient), 
    httpResult_(0), 
    rewriteExistsPath_(false) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the dump client
////////////////////////////////////////////////////////////////////////////////

DumpClient::~DumpClient () {

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
///
/// system collections may be ignored
////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> DumpClient::getCollections (const bool includeSystemCollections) 
  throw (std::runtime_error) {

  std::string url("/_api/collection");

  if (! includeSystemCollections) {
    // we can ignore all system collections
    url.append("/?excludeSystem=true");
  }

  std::vector<std::string> collections;

  sendRequest(url, 
              triagens::rest::HttpRequest::HTTP_REQUEST_GET,
              0);

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

    // this will free the json struct with all sub-elements
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
  }

  return collections;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the output path
////////////////////////////////////////////////////////////////////////////////

std::string DumpClient::getPath () const throw (std::runtime_error) {

  if (path_.empty()) {
    throw std::runtime_error("Output path must not be empty.");
  }

  return path_;

}

////////////////////////////////////////////////////////////////////////////////
/// @brief return whether it is allowed to overwrite files in the output path
////////////////////////////////////////////////////////////////////////////////

bool DumpClient::isRewriteExistsPath () {
  return rewriteExistsPath_;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief set the output path
///
/// this will try to create the output path directory if it does not yet exist.
/// if creating the directory fails, or the directory already exists and we
/// disallow overwriting data, an exception will be thrown
////////////////////////////////////////////////////////////////////////////////

void DumpClient::setPath (std::string path) throw (std::runtime_error) {

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

void DumpClient::setRewriteExistsPath (bool isRewrite) {
  rewriteExistsPath_ = isRewrite;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fetch collection metadata from the server and write it to an
/// output file
///
/// collection metadata is fetched with two API calls:
/// - properties: /_api/collection/.../properties
/// - indexes:    /_api/index?collection=...
/// both results are written as a JSON-array into the output file
////////////////////////////////////////////////////////////////////////////////

void DumpClient::dumpMetadata (const std::string & collection) throw (std::runtime_error) {
  std::string url;
  std::string body;

  // fetch collection properties
  url.append("/_api/collection/").append(collection).append("/properties");
  // sendRequest() will throw if HTTP return code is != 20x 
  sendRequest(url, 
              triagens::rest::HttpRequest::HTTP_REQUEST_GET,
              0);
    
  body = httpResult_->getBody().str();
  TRI_json_t* properties = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE, body.c_str());

  if (0 == properties) {
    throw std::runtime_error("Out of memory");
  }

  // remove transfer attributes we're not interested in
  TRI_DeleteArrayJson(TRI_UNKNOWN_MEM_ZONE, properties, "error");
  TRI_DeleteArrayJson(TRI_UNKNOWN_MEM_ZONE, properties, "code");
  
  // fetch collection index data
  url.clear();
  url.append("/_api/index/?collection=").append(collection);
  // sendRequest() will throw if HTTP return code is != 20x 
  sendRequest(url, 
              triagens::rest::HttpRequest::HTTP_REQUEST_GET,
              0);
  
  body = httpResult_->getBody().str();
  TRI_json_t* indexes = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE, body.c_str());
  if (0 == indexes) {
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, properties);
    throw std::runtime_error("Out of memory");
  }
  
  // remove transfer attributes we're not interested in
  TRI_DeleteArrayJson(TRI_UNKNOWN_MEM_ZONE, indexes, "error");
  TRI_DeleteArrayJson(TRI_UNKNOWN_MEM_ZONE, indexes, "code");


  // assemble overall result
  TRI_json_t* json = TRI_CreateArrayJson(TRI_UNKNOWN_MEM_ZONE);
  if (0 == json) {
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, properties);
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, indexes);
    throw std::runtime_error("Out of memory");
  }

  TRI_Insert3ArrayJson(TRI_UNKNOWN_MEM_ZONE, json, "properties", properties);
  TRI_Insert3ArrayJson(TRI_UNKNOWN_MEM_ZONE, json, "indexes", indexes);

  if (! TRI_SaveJson(getFilename(collection, true).c_str(), json)) {
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
    throw std::runtime_error("Cannot save metadata for collection '" + collection + "'");
  }

  TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fetch collection data from the server and write it to an output file
///
/// the actual data may be fetched with multiple HTTP requests:
/// - PUT /_api/simple/all to fetch initial data
/// - PUT /_api/cursor to fetch remaining data until there is no more
/// - DELETE /_api/cursor to clean up
////////////////////////////////////////////////////////////////////////////////

void DumpClient::dumpData (const std::string & collection) throw (std::runtime_error) {
  triagens::basics::StringBuffer sb(TRI_UNKNOWN_MEM_ZONE);

  // initialise output file
  std::ofstream stream(getFilename(collection, false).c_str());
  if (! stream.is_open()) {
    throw std::runtime_error("Can't save data for collection '" + collection + "'");
  }

  // fetch collection data

  // all collection names have been validated before
  // so we don't need to care about escaping them
  const std::string requestBody = "{\"collection\":\"" + collection + "\",\"batchSize\":2000}";

  // sendRequest() will throw if HTTP return code is != 20x 
  sendRequest("/_api/simple/all",
              triagens::rest::HttpRequest::HTTP_REQUEST_PUT,
              &requestBody);

  std::string responseBody;
  std::string cursorUrl;

  // after the initial request to /_api/simple/all, there might be
  // subsequent requests to /_api/cursor to fetch more data from the server
  while (true) {
    responseBody = httpResult_->getBody().str();

    TRI_json_t* json = TRI_JsonString(TRI_UNKNOWN_MEM_ZONE, responseBody.c_str());

    if (0 == json) {
      throw std::runtime_error("Out of memory");
    }

    // look for the "result" attribute
    // this attribute contains the actual documents
    TRI_json_t * result = TRI_LookupArrayJson(json, "result");
    if (0 == result || TRI_JSON_LIST != result->_type) {
      TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
      throw std::runtime_error("Got malformed JSON from server.");
    }

    // look for the "hasMore" attribute
    // the value indicates whether we can fetch more data from the server
    TRI_json_t* hasMore = TRI_LookupArrayJson(json, "hasMore");
    if (0 == hasMore || TRI_JSON_BOOLEAN != hasMore->_type) {
      TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
      throw std::runtime_error("Got malformed JSON from server.");
    }

    const bool hasMoreData = hasMore->_value._boolean;
    
    if (hasMoreData) {
      // the server indicated that there are more documents, it should also
      // have returned a cursor id in the "id" attribute
      TRI_json_t* id = TRI_LookupArrayJson(json, "id");
      if (0 == id || TRI_JSON_STRING != id->_type) {
        TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
        throw std::runtime_error("Got malformed JSON from server.");
      }

      if (cursorUrl == "") {
        cursorUrl.append("/_api/cursor/");
        cursorUrl.append(id->_value._string.data);
      }
    }

    // dump all documents into the outstream
    for (size_t i = 0; i < result->_value._objects._length; ++i) {
      TRI_json_t* current = (TRI_json_t*) TRI_AtVector(&result->_value._objects, i);

      sb.clear();
      if (TRI_ERROR_NO_ERROR != TRI_StringifyJson(sb.stringBuffer(), current)) {
        TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
        throw std::runtime_error("Out of memory.");
      }

      stream << sb.c_str() << std::endl;
    }
    
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
    
    if (! hasMoreData) {
      // "hasMore" is false. that means we have reached the end of the data
      break;
    }
    
    // fetch the next documents
    // sendRequest() will throw if HTTP return code is != 20x 
    sendRequest(cursorUrl, 
                triagens::rest::HttpRequest::HTTP_REQUEST_PUT,
                0);
  }


  if (cursorUrl != "") {
    // clean up server-side cursor
    sendRequest(cursorUrl, 
                triagens::rest::HttpRequest::HTTP_REQUEST_DELETE,
                0);
  }
    
  
  // close output file
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return an absolute filename for a collection
////////////////////////////////////////////////////////////////////////////////

std::string DumpClient::getFilename (const std::string& collection,
                                     const bool isMetaData) const {
  std::string file;
  file.append(getPath()).append(TRI_DIR_SEPARATOR_STR).append(collection);

  if (isMetaData) {
    file.append(".metadata");
  }

  file.append(".json");

  return file;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief send request to server and get result
////////////////////////////////////////////////////////////////////////////////

void DumpClient::sendRequest (const std::string & url, 
                              const triagens::rest::HttpRequest::HttpRequestType type,
                              std::string const* body) throw (std::runtime_error) {

  std::map<std::string, std::string> headerFields;

  if (0 != httpResult_) {
    delete httpResult_;
  }


  if (0 != body) {
    // we have a body to send (e.g. HTTP POST, HTTP PUT)
    httpResult_ = httpClient_->request(type, url, body->c_str(), body->size(), headerFields);
  }
  else {
    // we do not have a body to send (e.g. HTTP GET)
    httpResult_ = httpClient_->request(type, url, 0, 0, headerFields);
  }

  if (! httpResult_ || ! httpResult_->isComplete()) {
    throw std::runtime_error("Can't send request to server.");
  }

  if (! httpResult_->is20xResponse()) {

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
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
