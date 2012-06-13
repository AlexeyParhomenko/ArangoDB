////////////////////////////////////////////////////////////////////////////////
/// @brief interface of a job dispatcher
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2011 triagens GmbH, Cologne, Germany
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
/// @author Dr. Frank Celler
/// @author Martin Schoenert
/// @author Copyright 2009-2011, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_FYN_REST_DISPATCHER_H
#define TRIAGENS_FYN_REST_DISPATCHER_H 1

#include <Basics/Common.h>

////////////////////////////////////////////////////////////////////////////////
/// @defgroup Dispatcher Job Dispatcher
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace rest {
    class Job;
    class DispatcherQueue;
    class DispatcherThread;

    ////////////////////////////////////////////////////////////////////////////////
    /// @ingroup Dispatcher
    /// @brief interface of a job dispatcher
    ////////////////////////////////////////////////////////////////////////////////

    class Dispatcher {
      private:
        Dispatcher (Dispatcher const&);
        Dispatcher& operator= (Dispatcher const&);

      public:

        /////////////////////////////////////////////////////////////////////////
        /// @brief constructor
        /////////////////////////////////////////////////////////////////////////

        Dispatcher () {
        }

        /////////////////////////////////////////////////////////////////////////
        /// @brief destructor
        /////////////////////////////////////////////////////////////////////////

        virtual ~Dispatcher () {
        }

      public:

        /////////////////////////////////////////////////////////////////////////
        /// @brief is the dispatcher still running
        /////////////////////////////////////////////////////////////////////////

        virtual bool isRunning () = 0;

        /////////////////////////////////////////////////////////////////////////
        /// @brief adds a new queue
        /////////////////////////////////////////////////////////////////////////

        virtual void addQueue (string const& name, size_t nrThreads) = 0;

        /////////////////////////////////////////////////////////////////////////
        /// @brief adds a new job
        ///
        /// The method is called from the scheduler to add a new job request.
        /// It returns immediately (i.e. without waiting for the job to finish).
        /// When the job is finished the scheduler will be awoken and
        /// the scheduler will write the response over the network to
        /// the caller.
        /////////////////////////////////////////////////////////////////////////

        virtual bool addJob (Job*) = 0;

        /////////////////////////////////////////////////////////////////////////
        /// @brief start the dispatcher
        /////////////////////////////////////////////////////////////////////////

        virtual bool start () = 0;

        /////////////////////////////////////////////////////////////////////////
        /// @brief begins shutdown process
        /////////////////////////////////////////////////////////////////////////

        virtual void beginShutdown () = 0;

        /////////////////////////////////////////////////////////////////////////
        /// @brief reports status of dispatcher queues
        /////////////////////////////////////////////////////////////////////////

        virtual void reportStatus () = 0;
    };
  }
}

#endif