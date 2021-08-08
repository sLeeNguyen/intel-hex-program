#ifndef HEXFILEEXCEPTION_H
#define HEXFILEEXCEPTION_H

#include <exception>
#include <string>

class HexFileException : public std::exception
{
protected:
  /** Error message.
   */
  std::string _msg;

public:
  /** Constructor (C strings).
   *  @param message C-style string error message.
   *                 The string contents are copied upon construction.
   *                 Hence, responsibility for deleting the char* lies
   *                 with the caller. 
   */
  explicit HexFileException(const char* message) : _msg(message) {}

  /** Constructor (C++ STL strings).
   *  @param message The error message.
   */
  explicit HexFileException(const std::string& message) : _msg(message) {}

  virtual ~HexFileException() noexcept {}

  /** Returns a pointer to the (constant) error description.
   *  @return A pointer to a const char*. The underlying memory
   *          is in posession of the Exception object. Callers must
   *          not attempt to free the memory.
   */
  virtual const char *what() const noexcept
  {
    return _msg.c_str();
  }
};

#endif