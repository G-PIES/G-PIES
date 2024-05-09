#pragma once

#define STRINGIFY(a) #a

#ifdef __clanga__
  #define DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
  #define DIAGNOSTIC_DISABLE(diag) \
    _Pragma(STRINGIFY(clang diagnostic ignored diag))
  #define DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#elif __GNUC__
  #define DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
  #define DIAGNOSTIC_DISABLE(diag) \
    _Pragma(STRINGIFY(GCC diagnostic ignored diag))
  #define DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#else
  #define DIAGNOSTIC_PUSH
  #define DIAGNOSTIC_DISABLE(diag)
  #define DIAGNOSTIC_POP
#endif
