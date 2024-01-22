#ifndef GPIES_EXCEPTION
#define GPIES_EXCEPTION


class GpiesException
{
    public:
    GpiesException::GpiesException(const char* message)
    : message(message) {}

    const char* message;
};


#endif // GPIES_EXCEPTION