////////////////////////////////////////////////////////////////////////////////
/// @brief Ahuacatl, query language functions 
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
/// @author Jan Steemann
/// @author Copyright 2012, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_DURHAM_AHUACATL_QUERY_FUNCTIONS_H
#define TRIAGENS_DURHAM_AHUACATL_QUERY_FUNCTIONS_H 1

#include "BasicsC/common.h"

#include "Ahuacatl/ahuacatl-ast-node.h"

#ifdef __cplusplus
extern "C" {
#endif

struct TRI_aql_context_s;
struct TRI_aql_field_access_s;
struct TRI_associative_pointer_s;

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Ahuacatl
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief query function data structure
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_aql_function_s {
  char* _externalName;
  char* _internalName;
  bool _isDeterministic;
  bool _isGroup;
  const char* _argPattern;
  size_t _minArgs;
  size_t _maxArgs;
  void (*optimise)(const TRI_aql_node_t* const, struct TRI_aql_context_s* const, struct TRI_aql_field_access_s*);
}
TRI_aql_function_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise the array with the function declarations 
////////////////////////////////////////////////////////////////////////////////

struct TRI_associative_pointer_s* TRI_InitialiseFunctionsAql (void);

////////////////////////////////////////////////////////////////////////////////
/// @brief free the array with the function declarations 
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeFunctionsAql (struct TRI_associative_pointer_s*);

////////////////////////////////////////////////////////////////////////////////
/// @brief return a function, looked up by its external name
////////////////////////////////////////////////////////////////////////////////

TRI_aql_function_t* TRI_GetByExternalNameFunctionAql (struct TRI_associative_pointer_s*,
                                                      const char* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief get internal function name for an external one
////////////////////////////////////////////////////////////////////////////////

const char* TRI_GetInternalNameFunctionAql (const TRI_aql_function_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief register a function name
////////////////////////////////////////////////////////////////////////////////

bool TRI_RegisterFunctionAql (struct TRI_associative_pointer_s*,
                              const char* const, 
                              const char* const, 
                              const bool,
                              const bool,
                              const char* const,
                              void (*)(const TRI_aql_node_t* const, struct TRI_aql_context_s* const, struct TRI_aql_field_access_s*));

////////////////////////////////////////////////////////////////////////////////
/// @brief check whether a function argument must be converted to another type
////////////////////////////////////////////////////////////////////////////////

bool TRI_ConvertParameterFunctionAql (const TRI_aql_function_t* const,
                                      const size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief validate the arguments passed to a function
////////////////////////////////////////////////////////////////////////////////

bool TRI_ValidateArgsFunctionAql (struct TRI_aql_context_s* const,
                                  const TRI_aql_function_t* const,
                                  const TRI_aql_node_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
