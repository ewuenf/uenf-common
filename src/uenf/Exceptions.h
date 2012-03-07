#ifndef COWBOI_EXCEPTIONS_H
#define COWBOI_EXCEPTIONS_H


// we use boost exception here and only define some special types
#include <boost/exception/all.hpp>
#include <string>




namespace Cowboi
{



// the following class and the typedef is right from the example on the boost website
class ExceptionBase: virtual public std::exception, virtual public boost::exception
{
public:
	~ExceptionBase() throw() {}
};

class TagCowboiException {};
typedef boost::error_info<TagCowboiException, std::string> ExceptionStringInfo;



// ################################################################################################
//
//
// general usage would be:
//
//  ... somewhere in your code
//  if(some evil)
//    BOOST_THROW_EXCEPTION(ExceptionBase());
//    ... or, alternatively:
//    BOOST_THROW_EXCEPTION(ExceptionBase() << ExceptionStringInfo("Would you like to know more?"));
//
//
//  ... and somewhere else catching it and producin output
//  catch( ExceptionBase & e )
//  {
//    std::cout << boost::diagnostic_information(e) << std::endl;
//    ... do something additionally, if possible
//  }
//
//  ... for more info, consider the boost exception webpages
//
//
//
//
// ################################################################################################
//
//
// some more specific exception classes follow (all virtual inheritance is important here!), to
// indicate more specific errors, that could then be handled by a more specific catch expressions like
//
//  catch(SpecificException & e)
//




//! This exception represents errors with input/output in general, for example with files.
class ExceptionIO : virtual public ExceptionBase
{
public:
  enum Spec{open, read, parse, write, access};

  /*! You can specify more in detail what went wrong (default is "open") and
      give a name identifying the ressource.
  */
  ExceptionIO(Spec detail = open, std::string const & name = "<Unknown>")
  {
    switch(detail)
    {
      case open:
        (*this) << (ExceptionStringInfo(std::string("Open failed with: ") + name));
        break;

      case read:
        (*this) << (ExceptionStringInfo(std::string("Read failed on: ") + name));
        break;

      case parse:
        (*this) << (ExceptionStringInfo(std::string("Parse error in: ") + name));
        break;

      case write:
        (*this) << (ExceptionStringInfo(std::string("Write failed on: ") + name));
        break;

      case access:
        (*this) << (ExceptionStringInfo(std::string("Access was wrong with: ") + name));
        break;

      default:  // we do not throw here (, although we could?)
        (*this) << (ExceptionStringInfo(std::string("Unknown IO error with name: ") + name));
        break;
    }
  }
  ~ExceptionIO() throw()
  {
  }
};








//! This exception represents errors with user given parameters, the parameter number can be specified.
class ExceptionParameter : virtual public ExceptionBase
{
public:
  // by default, the first parameter was wrong (for typical 1-parameter functions)
  ExceptionParameter(int parameterNrArg = 0):parameterNr(parameterNrArg)
  {
    (*this) << ExceptionStringInfo(std::string("Error in parameter nr. "))
            << boost::error_info<TagCowboiException, const int>(parameterNrArg);
  }
  const int parameterNr;

  ~ExceptionParameter() throw()
  {
  }
};







/*! This exception represents semantic, rather fatal errors, when we have something unexpected in the code,
    that we detect upon sanity checks (kind of an assertion).
*/
class ExceptionCode : virtual public ExceptionBase
{
public:
  ExceptionCode()
  {
    (*this) << (ExceptionStringInfo("Some programmer did write screwed code or this hardware sucks!"));
  }

  ExceptionCode(std::string const & message)
  {
    (*this) << (ExceptionStringInfo(std::string("Some programmer did write screwed code and gave this message: ") + message));
  }
  ~ExceptionCode() throw()
  {
  }
};




/*! This exception represents runtime errors related to an error state of the system (not an ExceptionCode!)
    like an error return of a system call.
*/
class ExceptionRuntime : virtual public ExceptionBase
{
public:
  ExceptionRuntime()
  {
    (*this) << (ExceptionStringInfo("A runtime error occured."));
  }

  ExceptionRuntime(std::string const & message)
  {
    (*this) << (ExceptionStringInfo(std::string("A runtime error occured, detail: ") + message));
  }
  ~ExceptionRuntime() throw()
  {
  }
};




} // end of namespace cowboi



#endif

