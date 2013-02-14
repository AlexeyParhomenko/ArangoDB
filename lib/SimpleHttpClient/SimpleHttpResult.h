////////////////////////////////////////////////////////////////////////////////
/// @brief http request result
///
/// @file
///
/// DISCLAIMER
///
/// Copyright by triAGENS GmbH - All rights reserved.
///
/// The Programs (which include both the software and documentation)
/// contain proprietary information of triAGENS GmbH; they are
/// provided under a license agreement containing restrictions on use and
/// disclosure and are also protected by copyright, patent and other
/// intellectual and industrial property laws. Reverse engineering,
/// disassembly or decompilation of the Programs, except to the extent
/// required to obtain interoperability with other independently created
/// software or as specified by law, is prohibited.
///
/// The Programs are not intended for use in any nuclear, aviation, mass
/// transit, medical, or other inherently dangerous applications. It shall
/// be the licensee's responsibility to take all appropriate fail-safe,
/// backup, redundancy, and other measures to ensure the safe use of such
/// applications if the Programs are used for such purposes, and triAGENS
/// GmbH disclaims liability for any damages caused by such use of
/// the Programs.
///
/// This software is the confidential and proprietary information of
/// triAGENS GmbH. You shall not disclose such confidential and
/// proprietary information and shall use it only in accordance with the
/// terms of the license agreement you entered into with triAGENS GmbH.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2008-2011, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_SIMPLE_HTTP_CLIENT_SIMPLE_HTTP_RESULT_H
#define TRIAGENS_SIMPLE_HTTP_CLIENT_SIMPLE_HTTP_RESULT_H 1

#include <Basics/Common.h>

#include <map>
#include <string>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
/// @brief class for storing a request result
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace httpclient {

    class SimpleHttpResult {
    private:
      SimpleHttpResult (SimpleHttpResult const&);
      SimpleHttpResult& operator= (SimpleHttpResult const&);
      
    public:

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief result types
      ////////////////////////////////////////////////////////////////////////////////

      enum resultTypes {
        COMPLETE = 0,
        COULD_NOT_CONNECT,
        WRITE_ERROR,
        READ_ERROR,
        UNKNOWN
      };
      
      ////////////////////////////////////////////////////////////////////////////////
      /// @brief http response codes
      ////////////////////////////////////////////////////////////////////////////////

      enum http_status_codes {
        HTTP_STATUS_OK = 200,
        HTTP_STATUS_CREATED = 201,
        HTTP_STATUS_ACCEPTED = 202,
        HTTP_STATUS_PARTIAL = 203,
        HTTP_STATUS_NO_RESPONSE = 204,

        HTTP_STATUS_MOVED_PERMANENTLY = 301,
        HTTP_STATUS_FOUND = 302,
        HTTP_STATUS_SEE_OTHER = 303,
        HTTP_STATUS_NOT_MODIFIED = 304,
        HTTP_STATUS_TEMPORARY_REDIRECT = 307,

        HTTP_STATUS_BAD = 400,
        HTTP_STATUS_UNAUTHORIZED = 401,
        HTTP_STATUS_PAYMENT = 402,
        HTTP_STATUS_FORBIDDEN = 403,
        HTTP_STATUS_NOT_FOUND = 404,
        HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
        HTTP_STATUS_UNPROCESSABLE_ENTITY = 422,

        HTTP_STATUS_SERVER_ERROR = 500,
        HTTP_STATUS_NOT_IMPLEMENTED = 501,
        HTTP_STATUS_BAD_GATEWAY = 502,
        HTTP_STATUS_SERVICE_UNAVAILABLE = 503 // Retry later
      };


    public:
      ////////////////////////////////////////////////////////////////////////////////
      /// @brief constructor
      ////////////////////////////////////////////////////////////////////////////////

      SimpleHttpResult ();

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief destructor
      ////////////////////////////////////////////////////////////////////////////////

      ~SimpleHttpResult ();

    public:
      ////////////////////////////////////////////////////////////////////////////////
      /// @brief clear result values
      ////////////////////////////////////////////////////////////////////////////////

      void clear ();
      
      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns whether the response is a 20x response
      ////////////////////////////////////////////////////////////////////////////////

      bool is20xResponse () const {
        return (_returnCode >= 200 && _returnCode <= 209);
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns the http return code
      ////////////////////////////////////////////////////////////////////////////////

      int getHttpReturnCode () const {
        return _returnCode;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief sets the http return code
      ////////////////////////////////////////////////////////////////////////////////

      void setHttpReturnCode (int returnCode) {
        this->_returnCode = returnCode;
      }
      
      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns the http return message
      ////////////////////////////////////////////////////////////////////////////////

      string getHttpReturnMessage () const {
        return _returnMessage;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief sets the http return message
      ////////////////////////////////////////////////////////////////////////////////

      void setHttpReturnMessage (const string& message) {
        this->_returnMessage = message;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns the content length
      ////////////////////////////////////////////////////////////////////////////////

      size_t getContentLength () const {
        return _contentLength;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief sets the content length
      ////////////////////////////////////////////////////////////////////////////////

      void setContentLength (size_t len) {
        _contentLength = len;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns the http body
      ////////////////////////////////////////////////////////////////////////////////

      std::stringstream& getBody ();

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns the request result type
      ////////////////////////////////////////////////////////////////////////////////

      enum resultTypes getResultType () const {
        return _requestResultType;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns true if result type == OK
      ////////////////////////////////////////////////////////////////////////////////

      bool isComplete () const {
        return _requestResultType == COMPLETE;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns true if "transfer-encoding: chunked"
      ////////////////////////////////////////////////////////////////////////////////

      bool isChunked () const {
        return _chunked;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief sets the request result type
      ////////////////////////////////////////////////////////////////////////////////

      void setResultType (enum resultTypes requestResultType) {
        this->_requestResultType = requestResultType;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief returns a message for the result type
      ////////////////////////////////////////////////////////////////////////////////

      std::string getResultTypeMessage ();

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief add header field
      ////////////////////////////////////////////////////////////////////////////////

      void addHeaderField (std::string const& line);

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief add header field
      ////////////////////////////////////////////////////////////////////////////////

      void addHeaderField (std::string const& key, std::string const& value);

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief get X-VOC-* header fields
      ////////////////////////////////////////////////////////////////////////////////

      const std::map<std::string, std::string>& getHeaderFields () {
        return _headerFields;
      }

      ////////////////////////////////////////////////////////////////////////////////
      /// @brief get content type header field
      ////////////////////////////////////////////////////////////////////////////////

      const std::string getContentType (const bool partial);


    private:

      // header informtion
      int _returnCode;
      string _returnMessage;
      size_t _contentLength;
      bool _chunked;

      // body content
      std::stringstream _resultBody;

      // request result type
      enum resultTypes _requestResultType;

      // header fields
      std::map<std::string, std::string> _headerFields;
    };
  }
}
#endif

