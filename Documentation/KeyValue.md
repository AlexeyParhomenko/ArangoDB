////////////////////////////////////////////////////////////////////////////////
/// @brief statements and cursors
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2012 triagens GmbH, Cologne, Germany
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
/// @author Achim Brandt
/// @author Copyright 2012, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @page KeyValue REST Interface for storing key-value pairs
///
/// @section KeyValueCreate Create a new key-value pair in the data store. 
///
/// Creates a new key-value pair with the given key, the data passed in content 
/// and optional attributes passed in the header.
/// If a key-value pair with the same key exists in the data store, the request 
/// returns an error. 
///
/// @anchor KeyValuePost
/// @REST{POST /_api/key/@FA{collection-name}/@FA{key}}
///
/// Optional Attributes:
/// - @LIT{x-voc-expires} Expiry-date for this key-value pair.
/// - @LIT{x-voc-extended} A JSON-object of one or more attributes to be attached 
///    to the key-value pair.
/// - TODO @LIT{binary data}
/// 
/// If the document was created successfully, then a @LIT{HTTP 201} is returned.
///
/// If the @FA{collection-name} is unknown, then a @LIT{HTTP 404} is
/// returned.
///
/// If the @FA{key} is used, then a @LIT{HTTP 404} is returned.
///
/// @EXAMPLES
///
/// Create a key-value pair
///
/// @verbinclude key-value_post-key-request
///
/// Response (Success) 
///
/// @verbinclude key-value_post-key-ok
///
/// Response (Failure) 
///
/// @verbinclude key-value_post-key-bad
///
///
///
/// @section KeyValueChange Creates or changes the value for a given key. 
///
/// Sets the value associated with key to the new data passed in content. 
///
/// @anchor KeyValuePut
/// @REST{PUT /_api/key/@FA{collection-name}/@FA{key}}
///
/// Optional parameter:
/// - @LIT{?create=1} When set to 1 database will create keys if they do not 
///   exist. When set to 0 (default) it will return an error if a key does not 
///   exist. 
///
/// If the document was updated successfully, then a @LIT{HTTP 202} is returned.
///
/// If the @FA{collection-name} is unknown, then a @LIT{HTTP 404} is
/// returned.
///
/// If the @FA{key} is not found and @LIT{?create=0}, then a @LIT{HTTP 404} is 
/// returned.
///
/// @EXAMPLES
///
/// Update a key-value pair
///
/// @verbinclude key-value_put-key-request
///
/// Response (Success) 
///
/// @verbinclude key-value_put-key-ok
///
/// Response (Failure) 
///
/// @verbinclude key-value_put-key-bad
///
///
///
/// @section KeyValueReturn Returns the value associated with a given key.
///
/// Returns the value associated with the key in content. The Attributes of
/// the key-value pair are returned in the header.
///
/// @anchor KeyValueGet
/// @REST{GET /_api/key/@FA{collection-name}/@FA{key}}
///
/// If the document was found, then a @LIT{HTTP 200} is returned.
///
/// If the @FA{collection-name} is unknown, then a @LIT{HTTP 404} is
/// returned.
///
/// If the @FA{key} is not found, then a @LIT{HTTP 404} is returned.
///
/// @EXAMPLES
///
/// Get a key-value pair
///
/// @verbinclude key-value_get-key-request
///
/// Response (Success) with attributes
///
/// @verbinclude key-value_get-key-ok
///
/// Response (Failure) 
///
/// @verbinclude key-value_get-key-bad
///
///
///
/// @section KeyValueRemove This function deletes a key-value pair from the data store. 
///
/// Deletes the key-value pair with key from the store.
/// Deletions cannot be undone!
///
/// @anchor KeyValueDelete
/// @REST{DELETE /_api/key/@FA{collection-name}/@FA{key}}
///
/// This function does not support any header parameters. 
/// 
/// If the document was found and deleted, then a @LIT{HTTP 200} is returned.
///
/// If the @FA{collection-name} is unknown, then a @LIT{HTTP 404} is
/// returned.
///
/// If the @FA{key} is not found, then a @LIT{HTTP 404} is returned.
///
/// @EXAMPLES
///
/// Delete a key-value pair
///
/// @verbinclude key-value_delete-key-request
///
/// Response (Success)
///
/// @verbinclude key-value_delete-key-ok
///
/// Response (Failure) 
///
/// @verbinclude key-value_delete-key-bad
///
///
///
/// @section KeyValueLookup This function returns all keys matching a given prefix. 
///
/// Retrieves all values that begin with key and returns them as a JSON-array in 
/// content. 
///
/// @anchor KeyValueSearch
/// @REST{GET /_api/keys/@FA{collection-name}/@FA{prefix}}
///
/// @FA{prefix} (=beginning of word) to search for. Note that any /-characters 
/// contained in key are considered part of the key and have no special meaning. 
///
/// TODO Optional parameter:
/// - @LIT{?filter=Filter Expression} Return only those keys that match the given 
///   filter expression (have certain attributes). Please note that the filter 
///   expression has to be URL-encoded
/// 
/// If the @FA{collection-name} is unknown, then a @LIT{HTTP 404} is
/// returned.
///
/// @EXAMPLES
///
/// Find keys by prefix
///
/// @verbinclude key-value_get-keys-request
///
/// Response (Success)
///
/// @verbinclude key-value_get-keys-ok
///
/// Response (Failure) 
///
/// @verbinclude key-value_get-keys-bad
///
////////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
