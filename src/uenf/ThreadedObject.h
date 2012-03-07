#ifndef COWBOI_THREADEDOBJECT_H
#define COWBOI_THREADEDOBJECT_H


#include <uenf/Exceptions.h>


#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>


#pragma warning( disable : 4355)


  

namespace uenf
{


/*!
	This class represents an object with its own thread attached (like the QThread interface).
	To use it, you just derive this class and overwrite the "void run()" method. If you want a
	kind of "pause"-functionality, you could implement this in a derived class using a semaphore
	or something comparable in a base class.
*/
class ThreadedObject : boost::noncopyable
{
private:
  
  /*! Internal class construct to be able to use the boost thread interface. Needs to be 
      copyable because boost::thread copies its argument upon construction (which happens
      to be an object of this class)
  */
  class LaunchedThread
  {
  public:
    LaunchedThread(ThreadedObject * threadArg):thread(threadArg){}
    void operator()()
    {
      {
        boost::lock_guard<boost::mutex> guard(thread->launchedThreadPtrMutex);
        thread->launchedThreadPtr = this;
      }
      thread->runBarrier.wait();
      try
      {
        thread->run();
      }
      catch(...)  // TODO: more sophisticated exception handling here!
      {}
      { 
        boost::lock_guard<boost::mutex> guard(thread->launchedThreadPtrMutex);
        thread->launchedThreadPtr = 0;
      }
    }

    ThreadedObject * thread;
  };


public:  
  // some sleepy helper funcs
  static void mSleep(int milliSeconds) { boost::this_thread::sleep(boost::posix_time::milliseconds(milliSeconds)); }
  static void uSleep(int microSeconds) { boost::this_thread::sleep(boost::posix_time::microseconds(microSeconds)); }



public:
  ThreadedObject():stop(false), launchedThreadPtr(0), runBarrier(2){}
  ~ThreadedObject()
  {
    stopAndWaitForThreadToExit();
    {
      boost::lock_guard<boost::mutex> guard(threadMutex);
      thread.reset();
    }
  }

  void startThread()
  {
    boost::lock_guard<boost::mutex> guard(threadMutex); // thread manipulation needs to be protected
    
    { // check, if thread is running (then the pointer is set) with scoped mutex locking
      boost::lock_guard<boost::mutex> guard(launchedThreadPtrMutex);
      if(launchedThreadPtr) BOOST_THROW_EXCEPTION(ExceptionCode());
    }

    stop = false;
    LaunchedThread launchedThreadObject(this); // makin only local var, as it will be copied by the boost::thread anyway
    thread.reset(new boost::thread(launchedThreadObject));
    runBarrier.wait();  // we will not return, until the thread starts running
  }

  void stopThread()
  {
    stop = true;  // we do not lock this access, as the variable is protected anyway and thus accessible by subclasses
  }

  void joinThread() 
  {
    // not reentrant this is! (and must be protected against access of thread variable)
    boost::lock_guard<boost::mutex> guard(threadMutex);
    if(thread) thread->join();
  }

  bool isRunning()
  {
    boost::lock_guard<boost::mutex> guard(launchedThreadPtrMutex);
    return launchedThreadPtr != 0;
  }


  // convenience functions (kind of alias)
  void waitForThreadToExit() {joinThread();}
  void stopAndWaitForThreadToExit()
  {
    stopThread();
    joinThread();
  }

protected:
  /*! typical implementation is:
		
      void run()
	  {
		while(not stop)
		{
			... do something
		}
	  }

	 The stop-variable is already defined in this base class and gets set
	 via "stopThread()" safely.
  */
  virtual void run() = 0;

  // used to stop thread endless loop from the outside
  volatile bool stop; 

private:
  // we need this pointer, because the LaunchedThread object in startThread is copied
  // by boost and thus needs to be set from inside the LaunchedThread object, see its
  // operator()().
  LaunchedThread * launchedThreadPtr;
  boost::mutex launchedThreadPtrMutex;
  
  boost::scoped_ptr<boost::thread> thread;
  boost::mutex threadMutex;
  
  boost::barrier runBarrier;
};





} // end of namespace uenf


#endif