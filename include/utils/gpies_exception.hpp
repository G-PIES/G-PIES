#ifndef GPIES_EXCEPTION
#define GPIES_EXCEPTION

#include <string>

class GpiesException {
 public:
  explicit GpiesException(const std::string &message) : message(message) {}

  std::string message;
};

#endif  // GPIES_EXCEPTION
