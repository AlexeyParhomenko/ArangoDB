////////////////////////////////////////////////////////////////////////////////
/// @brief Thread
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
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2008-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Thread.h"

#include <errno.h>
#include <signal.h>

#include "Basics/ConditionLocker.h"
#include "Logger/Logger.h"

using namespace triagens::basics;

// -----------------------------------------------------------------------------
// --SECTION--                                            static private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief static started with access to the private variables
////////////////////////////////////////////////////////////////////////////////

void Thread::startThread (void* arg) {
  Thread * ptr = (Thread *) arg;

  ptr->runMe();
  ptr->cleanup();
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                             static public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the process id
////////////////////////////////////////////////////////////////////////////////

TRI_pid_t Thread::currentProcessId () {
  return TRI_CurrentProcessId();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the thread process id
////////////////////////////////////////////////////////////////////////////////

TRI_pid_t Thread::currentThreadProcessId () {
  return TRI_CurrentThreadProcessId();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the thread id
////////////////////////////////////////////////////////////////////////////////

TRI_tid_t Thread::currentThreadId () {
  return TRI_CurrentThreadId();
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief constructs a thread
////////////////////////////////////////////////////////////////////////////////

Thread::Thread (const string& name)
  : _name(name),
    _asynchronousCancelation(false),
    _thread(),
    _finishedCondition(0),
    _started(0),
    _running(0) {
  TRI_InitThread(&_thread);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief deletes the thread
////////////////////////////////////////////////////////////////////////////////

Thread::~Thread () {
  if (_running != 0) {
    LOGGER_WARNING << "forcefully shuting down thread '" << _name << "'";
    TRI_StopThread(&_thread);
  }

  TRI_DetachThread(&_thread);
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief getter for running
////////////////////////////////////////////////////////////////////////////////

bool Thread::isRunning () {
  return _running != 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a thread identifier
////////////////////////////////////////////////////////////////////////////////

intptr_t Thread::threadId () {
  return (intptr_t) _thread;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief starts the thread
////////////////////////////////////////////////////////////////////////////////

bool Thread::start (ConditionVariable * finishedCondition) {
  _finishedCondition = finishedCondition;

  if (_started != 0) {
    LOGGER_FATAL << "called started on an already started thread";
    return false;
  }

  _started = 1;

  string text = "[" + _name + "]";
  bool ok = TRI_StartThread(&_thread, text.c_str(), &startThread, this);

  if (! ok) {
    LOGGER_ERROR << "could not start thread '" << _name << "': " << strerror(errno);
  }

  return ok;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief stops the thread
////////////////////////////////////////////////////////////////////////////////

void Thread::stop () {
  if (_running != 0) {
    LOGGER_TRACE << "trying to cancel (aka stop) the thread " << _name;
    TRI_StopThread(&_thread);
  }
  else {
    LOGGER_DEBUG << "trying to cancel (aka stop) an already stopped thread " << _name;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief joins the thread
////////////////////////////////////////////////////////////////////////////////

void Thread::join () {
  TRI_JoinThread(&_thread);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief send signal to thread
////////////////////////////////////////////////////////////////////////////////

void Thread::sendSignal (int signal) {
  if (_running != 0) {
    TRI_SignalThread(&_thread, signal);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief allows asynchrounous cancelation
////////////////////////////////////////////////////////////////////////////////

void Thread::allowAsynchronousCancelation () {
  if (_started) {
    if (_running) {
      if (TRI_IsSelfThread(&_thread)) {
        LOGGER_DEBUG << "set asynchronous cancelation for " << _name;
        TRI_AllowCancelation();
      }
      else {
        LOGGER_ERROR << "cannot change cancelation type of an already running thread from the outside";
      }
    }
    else {
      LOGGER_WARNING << "thread has already stop, it is useless to change the cancelation type";
    }
  }
  else {
    _asynchronousCancelation = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Threading
/// @{
////////////////////////////////////////////////////////////////////////////////

void Thread::runMe () {
  if (_asynchronousCancelation) {
    LOGGER_DEBUG << "set asynchronous cancelation for " << _name;
    TRI_AllowCancelation();
  }

  _running = 1;

  try {
    run();
  }
  catch (...) {
    LOGGER_DEBUG << "caught exception on " << _name;
    _running = 0;

    if (_finishedCondition != 0) {
      CONDITION_LOCKER(locker, *_finishedCondition);
      locker.broadcast();
    }

    throw;
  }

  _running = 0;

  if (_finishedCondition != 0) {
    CONDITION_LOCKER(locker, *_finishedCondition);
    locker.broadcast();
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
