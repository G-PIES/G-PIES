#ifndef GPIES_EXCEPTION
#define GPIES_EXCEPTION


class GpiesException
{
    public:
    explicit GpiesException(const std::string& message)
    : message(message) {}

    std::string message;
};


#endif // GPIES_EXCEPTION