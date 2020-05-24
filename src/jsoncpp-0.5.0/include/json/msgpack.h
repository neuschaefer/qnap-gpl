#ifndef JSON_MSGPACK_H_INCLUDED
#define JSON_MSGPACK_H_INCLUDED

#include <cmath>
#include <sstream>
#include <iostream>

#include "value.h"
#include "writer.h"

namespace Json { namespace Msgpack {
    
    typedef union convert_s {
        Long _long;
        ULong _ulong;
        double _double;
    } convert_t ;
    
    void JSON_API pack(std::ostream& osb, Value& val);
    void JSON_API unpack(std::istream& isb, Value& val);
    //void JSON_API unpack(std::istream& isb, std::stringstream& ss);
    void JSON_API unpack(std::istream& isb, std::ostream& ss);
} }

#endif
