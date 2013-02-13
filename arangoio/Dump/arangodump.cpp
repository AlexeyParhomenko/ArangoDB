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
#include <fstream>
#include <vector>

#include "build.h"

#include "Basics/FileUtils.h"
#include "Basics/ProgramOptions.h"
#include "Basics/ProgramOptionsDescription.h"
#include "BasicsC/init.h"
#include "BasicsC/logging.h"
#include "Rest/Endpoint.h"
#include "Rest/InitialiseRest.h"
#include "Rest/HttpRequest.h"
#include "SimpleHttpClient/GeneralClientConnection.h"
#include "SimpleHttpClient/SimpleHttpClient.h"

#include "DumpClient.h"

using namespace triagens::arangoio::dump;

typedef struct ProgramOptions {
	double        connectionTimeout;
	bool          disableAuthentication;
	std::string   endpoint;
	bool          isRewriteExistsPath;
	bool          isWriteData;
	bool          isWriteMetaData;
	std::string   pathToSave;
	std::string   password;
	double        requestTimeout;
	std::string   username;
}
ProgramOptions;

std::string displayUsage(char* argv[]) {

	std::stringstream error;
	error
		<< "Usage: "
		<< argv[0]
		<< " [OPTIONS] collection"
		<< std::endl
		<< "For more options, use "
		<< argv[0]
		<< " --help";

	return error.str();

}

// Parse input program options
void parseOptions(int argc, char* argv[], ProgramOptions * params) throw (std::runtime_error) {

	triagens::basics::ProgramOptions options;
	triagens::basics::ProgramOptionsDescription description("STANDARD options");
	description
		("data", &params->isWriteData, "Export database data.")
		("create-collection", &params->isWriteMetaData, "Export database meta-data.")
		("path", &params->pathToSave, "Path to save data or meta-data.")
		("force", &params->isRewriteExistsPath, "Set permission to rewrite data in current path.")
		("help", "Display this help message and exit.");

	triagens::basics::ProgramOptionsDescription clientOptions("CLIENT options");

	clientOptions
		("server.disable-authentication", &params->disableAuthentication, "disable authentication")
		("server.endpoint", &params->endpoint, "endpoint to connect to, use 'none' to start without a server")
		("server.username", &params->username, "username to use when connecting")
		("server.password", &params->password, "password to use when connecting (leave empty for prompt)")
		("server.connect-timeout", &params->connectionTimeout, "connect timeout in seconds")
		("server.request-timeout", &params->requestTimeout, "request timeout in seconds");

	description(clientOptions, false);

	if (!options.parse(description, argc, argv)) {
		throw std::runtime_error("Cannot parse command line: " + options.lastError());
	}

	// check for help
	std::set<std::string> help = options.needHelp("help");

	if (!help.empty()) {
		throw std::runtime_error(description.usage(help));
	}

	if (1 == argc) {
		throw std::runtime_error(displayUsage(argv));
	}

	if (!params->isWriteData && !params->isWriteMetaData) {
		throw std::runtime_error("You choose do nothing for dump.");
	}
}

////////////////////////////////////////////////////////////////////////////////
/// @brief main
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	TRIAGENS_C_INITIALISE(argc, argv);
//	TRIAGENS_REST_INITIALISE(argc, argv);

	TRI_InitialiseLogging(false);

	int err = 0;

	// options by default
	ProgramOptions params;
	params.connectionTimeout      = 3.0;
	params.disableAuthentication  = false;
	params.endpoint               = triagens::rest::Endpoint::getDefaultEndpoint();
	params.isRewriteExistsPath    = false;
	params.isWriteData            = true;
	params.isWriteMetaData        = true;
	params.requestTimeout         = 300.0;
	params.pathToSave
		.append(triagens::basics::FileUtils::currentDirectory(&err))
		.append(TRI_DIR_SEPARATOR_STR)
		.append("dump");

	// Initialize pointers
	triagens::rest::Endpoint * endpoint                         = NULL;
	triagens::httpclient::GeneralClientConnection * connection  = NULL;
	triagens::httpclient::SimpleHttpClient * httpClient         = NULL;
	DumpClient * dumpClient                                     = NULL;

	try {

		parseOptions(argc, argv, &params);

		std::string collection = argv[argc - 1];

		if (collection.substr(0, 1) == "-") {
			throw std::runtime_error("Last parameter should be collection name.");
		}

		endpoint   = triagens::rest::Endpoint::clientFactory(params.endpoint);
		connection = triagens::httpclient::GeneralClientConnection::factory(
				endpoint, params.connectionTimeout, params.requestTimeout, 2); // 2 - is ArangoClient::DEFAULT_RETRIES

		if (connection == 0) {
			throw std::runtime_error("Can't give connection because out of memory.");
		}

		httpClient = new triagens::httpclient::SimpleHttpClient(connection, params.requestTimeout, false);
		httpClient->setUserNamePassword("/", params.username, params.password);

		dumpClient = new DumpClient(httpClient);
		dumpClient->setRewriteExistsPath(params.isRewriteExistsPath);
		dumpClient->setPath(params.pathToSave);

		std::vector<std::string> collectionsOnServer, collectionsToDump;
		std::vector<std::string>::iterator it;

		collectionsOnServer = dumpClient->getCollections();

		// Check collections from user
		for (int i = argc - 1; i > 0 ; i--) {

			collection = argv[i];

			if (collection.substr(0, 1) == "-") {
				break;
			}

			if ((argc != 2 && i - 1 < 1)
					|| std::string(argv[i - 1]).substr(0, 1) == "-") {
				break;
			}

			it = std::find(collectionsOnServer.begin(), collectionsOnServer.end(), collection);

			if (it == collectionsOnServer.end()) {
				throw std::runtime_error("Collection " + collection + " not found on server.");
			}

			collectionsToDump.push_back(collection);

		}

		if (collectionsToDump.size() == 0) {
			collectionsToDump = collectionsOnServer;
		}

		std::string url;

		// Start to save data
		for (it = collectionsToDump.begin(); it != collectionsToDump.end(); it++) {

			collection = (*it);

			try {

				std::cout << "Dumping collection: " << collection << std::endl;

				if (params.isWriteData) {

					std::cout << "   data...";

					url.clear();
					url
						.append("/_api/index/?collection=")
						.append(collection);

					dumpClient->write(url, collection);

					std::cout << " -> successful!" << std::endl;

				}

				if (params.isWriteMetaData) {

					std::cout << "   metadata...";

					url.clear();
					url
						.append("/_api/collection/")
						.append(collection)
						.append("/properties");

					dumpClient->write(url, collection, true);

					std::cout << " -> successful!" << std::endl;

				}

			}
			catch (std::runtime_error & e) {
				std::cout << e.what() << std::endl;
			}
		}

	}
	catch (std::exception & e) {
		std::cout << std::endl << e.what() << std::endl << std::endl;
	}

	// Delete pointers

	if (NULL != dumpClient) {
		delete dumpClient;
	}

	if (NULL != httpClient) {
		delete httpClient;
	}

	if (NULL != connection) {

		if (connection->isConnected()) {
			connection->disconnect();
		}

		delete connection;
	}

	if (NULL != endpoint) {
		delete endpoint;
	}

//	TRIAGENS_REST_SHUTDOWN;
	TRIAGENS_C_SHUTDOWN;

	return EXIT_SUCCESS;

}
