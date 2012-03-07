#ifndef COWBOI_LOG_H
#define COWBOI_LOG_H

#ifdef COWBOI_LOG_INFO
  #error "COWBOI_LOG_INFO already defined"
#else
  #define COWBOI_LOG_INFO (std::string("In function ") + std::string(__FUNCSIG__) + std::string(":\n"))
#endif

#include <sstream>
#include <string>
#include <list>
#include <iosfwd>
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>



// some forward decls for operator<<
namespace Cowboi
{
  namespace Log
  {
    class Sync;
    class SyncUser;
  }
}
void operator<<(std::ostringstream & oss, Cowboi::Log::Sync const & logSync);
void operator<<(std::ostringstream & oss, Cowboi::Log::SyncUser const & logSyncUser);






namespace Cowboi
{


/*!

  Logging Framework, thread-safe and easy to use and extend (see below).


  Example usage would be (assuming user is in namespace Cowboi):

  Log::log("some std::string", Log::info);

  or, with a user defined number (should ideally be an enum, too)

  Log::log("some std::string", 12345);

  If you want to do more complicated output with user defined formatted types, you
  can use an ostringstream:

    std::ostringstream oss;
    oss << "any output" << evenObjectsIfTheirOuputOpIsDefined;
    oss << Log::Sync(Log::fatal);




  The class Log::Logger can be subclassed to receive the log messages
  and do something special with em or, if you just want standard logging facilities,
  instantiate a Log::StdCoutLogger or a Log::FileLogger at the beginning of main,
  for example liek this:

  int main(int argc, char * argv[])
  {
    Log::StdCoutLogger stdCoutLogger("[MyAppName] ");
    Log::FileLogger fileLogger("MyAppName_logFile.txt");

    ...
  }
*/
namespace Log
{





enum Severity { minSeverityEnum    = (1 << 0),
                debug              = (1 << 1),
                info               = (1 << 2),
                warning            = (1 << 3),
                error              = (1 << 4),
                fatal              = (1 << 5),
                maxSeverityEnum    = (1 << 6)
              };




class Logger
{
public:
  /*! The effect of setting this is implementation defined, but standard behaviour
      would be setting this to Log::maxSeverity effectively turns off logging.
  */
  virtual void setMinSeverity (Severity minSev)   { minSeverity = minSev; }
  /*! The effect of setting this is implementation defined, but standard behaviour
      would be that first minSeverity is checked, then the mask.
  */
  virtual void setSeverityMask(unsigned int mask) { severityMask = mask;  }


  // a custom logger should override at least these
  virtual void output( std::string const & message, Severity severity    ){}
  virtual void output( std::string const & message, unsigned int userCode){}

protected:
  /*! This class is not meant to be instantiated directly. Derived classes should call
      "registerLogger()" in their constructors body when they are ready to receive log messages.
      We do not do this in this ctor, because then we do not have fully constructed
      instances of the derived class and could be in big trouble, if the receive messages
      before complete construction.
  */
  Logger(); 
  ~Logger();

  void registerLogger();

  bool isEnabled(Severity severity) const
  {
    return (severity >= minSeverity) && ((unsigned int)(severity) & severityMask);
  }

  std::string getSeverityPrefix(Severity severity);

protected:
  unsigned int severityMask;
  Severity minSeverity;
};








class StdCoutLogger : public Logger
{
public:
  StdCoutLogger(std::string const & applicationName):appName(applicationName){registerLogger();}

  void output( std::string const & message, Severity severity);
  void output( std::string const & message, unsigned int userCode);

private:
  std::string const appName;
};








class FileLogger : public Logger
{
public:
  FileLogger(std::string const & fileName);
  ~FileLogger();

  void output( std::string const & message, Severity severity);
  void output( std::string const & message, unsigned int userCode);

private:
  boost::scoped_ptr<std::ofstream> file;
};











class Sync
{
public:
  explicit Sync(Severity severityArg):severity(severityArg) {}
  Severity const severity;
};




class SyncUser
{
public:
  explicit SyncUser(unsigned int userCodeArg):userCode(userCodeArg) {}
  unsigned int const userCode;
};






class LogDispatcher
{
  friend class Logger;
  friend void ::operator<<(std::ostringstream & oss, Cowboi::Log::Sync const & logSync);
  friend void ::operator<<(std::ostringstream & oss, Cowboi::Log::SyncUser const & logSyncUser);
  friend void log(std::string message, Severity severity);
  friend void log(std::string message, unsigned int userCode);

  // all them statics that follow have to be thread safe

public:
  // convenience functions
  static void setSeverityMaskOnAllListeners(unsigned int mask);
  static void setMinSeverityOnAllListeners (Severity minSev);

private:
  static void addLogger(Logger * l);
  static void removeLogger(Logger * l); // this should be a nothrow, as called in dtor

  // these funcs do something like "foreach oL in outputListenersList oL->output(message, logSync->severity);"
  static void callLoggers(std::string const & message, Severity logSync);
  static void callLoggers(std::string const & message, unsigned int logSyncUser);

  static std::list<Logger *> * getLoggerList();
  static boost::mutex        * getLoggerListMutex();
};






void log(std::string message, Severity severity);
void log(std::string message, unsigned int userCode);




}  // end of namespace Log






}  // end of namespace Cowboi



#endif
