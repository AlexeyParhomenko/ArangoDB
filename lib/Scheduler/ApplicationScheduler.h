////////////////////////////////////////////////////////////////////////////////
/// @brief application server scheduler implementation
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2012 triAGENS GmbH, Cologne, Germany
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
/// @author Copyright 2009-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_SCHEDULER_APPLICATION_SCHEDULER_H
#define TRIAGENS_SCHEDULER_APPLICATION_SCHEDULER_H 1

#include "ApplicationServer/ApplicationFeature.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

namespace triagens {
  namespace rest {
    class ApplicationServer;
    class Scheduler;
    class SignalTask;
    class Task;

// -----------------------------------------------------------------------------
// --SECTION--                                        class ApplicationScheduler
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief application server scheduler implementation
////////////////////////////////////////////////////////////////////////////////

    class ApplicationScheduler : public ApplicationFeature {
      private:
        ApplicationScheduler (ApplicationScheduler const&);
        ApplicationScheduler& operator= (ApplicationScheduler const&);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

        ApplicationScheduler (ApplicationServer*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

        ~ApplicationScheduler ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief allows a multi scheduler to be build
////////////////////////////////////////////////////////////////////////////////

        void allowMultiScheduler (bool value = true);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the scheduler
////////////////////////////////////////////////////////////////////////////////

        Scheduler* scheduler () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief builds the scheduler
////////////////////////////////////////////////////////////////////////////////

        void buildScheduler ();

////////////////////////////////////////////////////////////////////////////////
/// @brief builds the scheduler reporter
////////////////////////////////////////////////////////////////////////////////

        void buildSchedulerReporter ();

////////////////////////////////////////////////////////////////////////////////
/// @brief quits on control-c signal
////////////////////////////////////////////////////////////////////////////////

        void buildControlCHandler ();

////////////////////////////////////////////////////////////////////////////////
/// @brief installs a signal handler
////////////////////////////////////////////////////////////////////////////////

        void installSignalHandler (SignalTask*);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns true, if address reuse is allowed
////////////////////////////////////////////////////////////////////////////////

        bool addressReuseAllowed ();

////////////////////////////////////////////////////////////////////////////////
/// @brief register a new task
////////////////////////////////////////////////////////////////////////////////

        void registerTask (Task*);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        ApplicationFeature methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void setupOptions (map<string, basics::ProgramOptionsDescription>&);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool parsePhase1 (basics::ProgramOptions&);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool parsePhase2 (basics::ProgramOptions&);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool start ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool isRunning ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void beginShutdown ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void shutdown ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief adjusts the file descriptor limits
////////////////////////////////////////////////////////////////////////////////

        void adjustFileDescriptors ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief application server
////////////////////////////////////////////////////////////////////////////////

        ApplicationServer* _applicationServer;

////////////////////////////////////////////////////////////////////////////////
/// @brief scheduler
////////////////////////////////////////////////////////////////////////////////

        Scheduler* _scheduler;

////////////////////////////////////////////////////////////////////////////////
/// @brief task list
////////////////////////////////////////////////////////////////////////////////

        vector<Task*> _tasks;

////////////////////////////////////////////////////////////////////////////////
/// @brief intervall for reports
////////////////////////////////////////////////////////////////////////////////

        double _reportIntervall;

////////////////////////////////////////////////////////////////////////////////
/// @brief is a multi-threaded scheduler allowed
////////////////////////////////////////////////////////////////////////////////

        bool _multiSchedulerAllowed;

////////////////////////////////////////////////////////////////////////////////
/// @brief number of scheduler threads
///
/// @CMDOPT{--scheduler.threads @CA{arg}}
///
/// An integer argument which sets the number of threads to use in the IO
/// scheduler. The default is 1.
////////////////////////////////////////////////////////////////////////////////

        uint32_t _nrSchedulerThreads;

////////////////////////////////////////////////////////////////////////////////
/// @brief scheduler backend
///
/// @CMDOPT{--scheduler.backend @CA{arg}}
///
/// The I/O method used by the event handler. The default (if this option is
/// not specified) is to try all recommended backends. This is platform
/// specific. See libev for further details and the meaning of select, poll
/// and epoll.
////////////////////////////////////////////////////////////////////////////////

        uint32_t _backend;

////////////////////////////////////////////////////////////////////////////////
/// @brief allow port to be reused
////////////////////////////////////////////////////////////////////////////////

        bool _reuseAddress;

////////////////////////////////////////////////////////////////////////////////
/// @brief minimum number of file descriptors
////////////////////////////////////////////////////////////////////////////////

        uint32_t _descriptorMinimum;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}\\)"
// End:
