


#include <Cowboi/Log.h>
#include <Cowboi/Exceptions.h>

#include <iostream>
#include <fstream>
#include <ciso646>


namespace Cowboi
{

namespace Log
{


Logger::Logger():severityMask(0xffffffff), minSeverity(minSeverityEnum)
{
  /* we dont do "LogDispatcher::addLogger(this);" here because:
      Derived classes should call
      "registerLogger()" in their constructors body when they are ready to receive log messages.
      We do not do this in this ctor, because then we do not have fully constructed
      instances of the derived class and could be in big trouble, if they receive messages
      before complete construction.
  */
}





Logger::~Logger()
{
  LogDispatcher::removeLogger(this);
}





void Logger::registerLogger()
{
  LogDispatcher::addLogger(this);
}






std::string Logger::getSeverityPrefix(Severity severity)
{
  switch(severity)
  {
    case debug:   return std::string(" DEBUG: "   );
    case info:    return std::string(" INFO: "    );
    case warning: return std::string(" WARNING: " );
    case error:   return std::string(" ERROR: "   );
    case fatal:   return std::string(" FATAL: "   );

    default:
      BOOST_THROW_EXCEPTION(Cowboi::ExceptionParameter());
  }
}






void StdCoutLogger::output( std::string const & message, Severity severity)
{
  if(isEnabled(severity))
  {
    std::cout << appName << getSeverityPrefix(severity) << message << std::endl;
  }
}





void StdCoutLogger::output( std::string const & message, unsigned int userCode)
{
  std::cout << appName << " USERCODE: " << userCode << ' ' << message << std::endl;
}











FileLogger::FileLogger(std::string const & fileName):file(new std::ofstream(fileName.c_str()))
{
  if(not (*file)) // could not open file?
    BOOST_THROW_EXCEPTION(ExceptionParameter());
  else
    registerLogger();
}



FileLogger::~FileLogger(){}



void FileLogger::output( std::string const & message, Severity severity)
{
  if(isEnabled(severity))
  {
    (*file) << getSeverityPrefix(severity) << message << std::endl;
  }
}




void FileLogger::output( std::string const & message, unsigned int userCode)
{
  (*file) << " USERCODE: " << userCode << ' ' << message << std::endl;
}










void LogDispatcher::addLogger(Logger * l)
{
  boost::unique_lock<boost::mutex> guard(*(getLoggerListMutex()));
  getLoggerList()->push_back(l);
}


// this should be a nothrow, as called in dtor
void LogDispatcher::removeLogger(Logger * l)
{
  boost::unique_lock<boost::mutex> guard(*(getLoggerListMutex()));
  getLoggerList()->remove(l);
}


void LogDispatcher::setSeverityMaskOnAllListeners(unsigned int mask)
{
  boost::unique_lock<boost::mutex> guard(*(getLoggerListMutex()));
  std::list<Logger *>::iterator it = getLoggerList()->begin();
  for(; it not_eq getLoggerList()->end(); ++it)
  {
    (*it)->setSeverityMask(mask);
  }
}


void LogDispatcher::setMinSeverityOnAllListeners (Severity minSev)
{
  boost::unique_lock<boost::mutex> guard(*(getLoggerListMutex()));
  std::list<Logger *>::iterator it = getLoggerList()->begin();
  for(; it not_eq getLoggerList()->end(); ++it)
  {
    (*it)->setMinSeverity(minSev);
  }
}


void LogDispatcher::callLoggers(std::string const & message, Severity severity)
{
  boost::unique_lock<boost::mutex> guard(*(getLoggerListMutex()));
  std::list<Logger *>::iterator it = getLoggerList()->begin();
  for(; it not_eq getLoggerList()->end(); ++it)
  {
    (*it)->output(message, severity);
  }
}


void LogDispatcher::callLoggers(std::string const & message, unsigned int userCode)
{
  boost::unique_lock<boost::mutex> guard(*(getLoggerListMutex()));
  std::list<Logger *>::iterator it = getLoggerList()->begin();
  for(; it not_eq getLoggerList()->end(); ++it)
  {
    (*it)->output(message, userCode);
  }
}




std::list<Logger *> * LogDispatcher::getLoggerList()
{
  // the static pointer is a wanted memory leak to keep up the logger list as long as possible
  // upon static dynamic destruction and additionally keep the code simple
  // ( see http://stackoverflow.com/questions/2373859/c-static-const-and-initialization-is-there-a-fiasco
  //   for details on what could be a solution with constant POD types )
  static std::list<Logger *> * loggerListPtr = 0; 
  if(not loggerListPtr)
  {
    loggerListPtr = new std::list<Logger *>;
  }
  return loggerListPtr;
}
  
  
boost::mutex * LogDispatcher::getLoggerListMutex()
{
  static boost::mutex * mutexPtr = 0; 
  if(not mutexPtr)
  {
    mutexPtr = new boost::mutex;
  }
  return mutexPtr;  
}




void log(std::string message, Severity severity)
{
  LogDispatcher::callLoggers(message, severity);
}

void log(std::string message, unsigned int userCode)
{
  LogDispatcher::callLoggers(message, userCode);
}





}  // end of namespace Log



}  // end of namespace Cowboi









// These operators are global to avoid name searching problems with the compiler

void operator<<(std::ostringstream & oss, Cowboi::Log::Sync const & logSync)
{
  Cowboi::Log::LogDispatcher::callLoggers(oss.str(), logSync.severity);
}

void operator<<(std::ostringstream & oss, Cowboi::Log::SyncUser const & logSyncUser)
{
  Cowboi::Log::LogDispatcher::callLoggers(oss.str(), logSyncUser.userCode);
}









