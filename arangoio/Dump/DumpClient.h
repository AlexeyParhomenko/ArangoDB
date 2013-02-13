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

#ifndef DUMPCLIENT_H_
#define DUMPCLIENT_H_

#include <stdexcept>
#include <map>
#include "SimpleHttpClient/SimpleHttpClient.h"
#include "SimpleHttpClient/SimpleHttpResult.h"

namespace triagens {
namespace arangoio {
namespace dump {

class DumpClient {
public:

  /**
   * Constructor
   */
  DumpClient(triagens::httpclient::SimpleHttpClient * httpClient);

  /**
   * Destructor
   */
  virtual ~DumpClient();

  /**
   * Return names collections
   */
  std::vector<std::string> getCollections() throw (std::runtime_error);

  /**
   * Return path to save
   */
  std::string getPath() throw (std::runtime_error);

  bool isRewriteExistsPath();

  /**
   * Set path to save
   */
  void setPath(std::string path) throw (std::runtime_error);

  void setRewriteExistsPath(bool isRewrite);

  /**
   * Write dump
   */
  void write(const std::string & url, const std::string & fileName)
      throw (std::runtime_error);
  void write(const std::string & url, const std::string & fileName,
      bool isMetaData) throw (std::runtime_error);

protected:

  triagens::httpclient::SimpleHttpClient * httpClient_;
  triagens::httpclient::SimpleHttpResult * httpResult_;bool rewriteExistsPath_;
  std::string path_;

  void sendRequest(const std::string & url) throw (std::runtime_error);

};

} /* namespace dump */
} /* namespace arangoio */
} /* namespace triagens */
#endif /* DUMPCLIENT_H_ */
