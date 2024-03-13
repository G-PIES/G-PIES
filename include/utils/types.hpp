#ifndef TYPES_HPP
#define TYPES_HPP

#if defined(USE_METAL)
// metal does not support doubles 
#define GP_FLOAT float
#elif !defined(GP_FLOAT)
#define GP_FLOAT double
#endif

typedef GP_FLOAT gp_float;

#endif  // TYPES_HPP
