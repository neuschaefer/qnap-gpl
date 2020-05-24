#include <cmath>
#include <iomanip>
#include <json/writer.h>
#include <json/msgpack.h>

namespace Json { namespace Msgpack {
    void JSON_API pack(std::ostream& osb, Value& val) {
        convert_t t;
        
        unsigned char* ptr;
        Value::Members members;
        
        switch (val.type()) {
            case nullValue:
                osb << (unsigned char)0xc0;
                break;
            
            case intValue:
            case longValue:
                t._long = val.asLong();
                if (t._long < 0) {
                    if (t._long >= -32) { // negative fixnum
                        osb << (unsigned char)(0xe0 + (t._long + 32));
                    } else if (t._long > -0x80) { // int8
                        osb << (unsigned char)0xd0
                            << (unsigned char)(t._long + 0x100);
                    } else if (t._long > -0x8000) { // int16
                        short tmp = t._long;
                        ptr = (unsigned char*) &tmp;
                        osb << (unsigned char)0xd1
                            << (unsigned char)ptr[1]
                            << (unsigned char)ptr[0];
                    } else if (t._long > -0x80000000) { // int32
                        Int tmp = t._long;
                        ptr = (unsigned char*) &tmp;
                        osb << (unsigned char)0xd2
                            << (unsigned char)ptr[3]
                            << (unsigned char)ptr[2]
                            << (unsigned char)ptr[1]
                            << (unsigned char)ptr[0];
                    } else { // int64
                        ptr = (unsigned char*) &t._long;
                        osb << (unsigned char)0xd3
                            << (unsigned char)ptr[7]
                            << (unsigned char)ptr[6]
                            << (unsigned char)ptr[5]
                            << (unsigned char)ptr[4]
                            << (unsigned char)ptr[3]
                            << (unsigned char)ptr[2]
                            << (unsigned char)ptr[1]
                            << (unsigned char)ptr[0];
                    }
                    break;
                }
            
            case uintValue:
            case ulongValue:
                t._ulong = val.asULong();
                if (t._ulong < 0x80) { // positive fixnum
                    osb << (unsigned char)(t._ulong & 0xff);
                } else if (t._ulong < 0x100) { // uint 8
                    osb << (unsigned char)0xcc
                        << (unsigned char)(t._ulong);
                } else if (t._ulong < 0x10000) { // uint 16
                    unsigned short tmp = t._ulong;
                    ptr = (unsigned char*) &tmp;
                    osb << (unsigned char)0xcd
                        << (unsigned char)ptr[1]
                        << (unsigned char)ptr[0];
                } else if (t._ulong < 0x1000000) { // uint 32
                    UInt tmp = t._ulong;
                    ptr = (unsigned char*) &tmp;
                    osb << (unsigned char)0xce
                        << (unsigned char)ptr[3]
                        << (unsigned char)ptr[2]
                        << (unsigned char)ptr[1]
                        << (unsigned char)ptr[0];
                } else {
                    ptr = (unsigned char*) &t._ulong;
                    osb << (unsigned char)0xcf
                        << (unsigned char)ptr[7]
                        << (unsigned char)ptr[6]
                        << (unsigned char)ptr[5]
                        << (unsigned char)ptr[4]
                        << (unsigned char)ptr[3]
                        << (unsigned char)ptr[2]
                        << (unsigned char)ptr[1]
                        << (unsigned char)ptr[0];
                }
                break;
            
            case realValue:
                t._double = val.asDouble();
                ptr = (unsigned char*) &t._double;
                
                osb << (unsigned char)0xcb
                    << (unsigned char)ptr[7]
                    << (unsigned char)ptr[6]
                    << (unsigned char)ptr[5]
                    << (unsigned char)ptr[4]
                    << (unsigned char)ptr[3]
                    << (unsigned char)ptr[2]
                    << (unsigned char)ptr[1]
                    << (unsigned char)ptr[0];
                break;
            
            case stringValue:
                t._ulong = val.size();
                if (t._ulong < 32) {
                    osb << (unsigned char)(0xa0 + t._ulong)
                        << val.asCString();
                } else if (t._ulong < 0x10000) {
                    osb << (unsigned char)0xda
                        << (unsigned char)(t._ulong >>  8)
                        << (unsigned char)(t._ulong >>  0)
                        << val.asCString();
                } else /*if (t._ulong < 0x100000000)*/ {
                    osb << (unsigned char)0xdb
                        << (unsigned char)(t._ulong >> 24)
                        << (unsigned char)(t._ulong >> 16)
                        << (unsigned char)(t._ulong >>  8)
                        << (unsigned char)(t._ulong >>  0)
                        << val.asCString();
                }
                break;
            
            case booleanValue:
                if (val.asBool()) {
                    osb << (unsigned char)0xc3;
                } else {
                    osb << (unsigned char)0xc2;
                }
                break;
            
            case arrayValue:
                t._ulong = val.size();
                if (t._ulong < 16) {
                    osb << (unsigned char)(0x90 + t._ulong);
                } else if (t._ulong < 0x10000) {
                    osb << (unsigned char)0xdc
                        << (unsigned char)(t._ulong >> 8)
                        << (unsigned char)(t._ulong);
                } else if (t._ulong < 0xffffffff) {
                    osb << (unsigned char)0xdd
                        << (unsigned char)(t._ulong >> 24)
                        << (unsigned char)(t._ulong >> 16)
                        << (unsigned char)(t._ulong >>  8)
                        << (unsigned char)(t._ulong >>  0);
                }
                for (ULong i = 0 ; i < t._ulong ; ++i) {
                    pack(osb, val[i]);
                }
                break;
            
            case objectValue:
                t._ulong = val.size();
                if (t._ulong < 16) {
                    osb << (unsigned char)(0x80 + t._ulong);
                } else if (t._ulong < 0x10000) {
                    osb << (unsigned char)0xde
                        << (unsigned char)(t._ulong >> 8)
                        << (unsigned char)(t._ulong);
                } else if (t._ulong <= 0xffffffff) {
                    osb << (unsigned char)0xdf
                        << (unsigned char)(t._ulong >> 24)
                        << (unsigned char)(t._ulong >> 16)
                        << (unsigned char)(t._ulong >>  8)
                        << (unsigned char)(t._ulong >>  0);
                }
                
                members = val.getMemberNames();
                
                for ( Value::Members::iterator it = members.begin(); it != members.end(); ++it) {
                    Value name = *it;
                    pack(osb, name);
                    pack(osb, val[*it]);
                }
                break;
        }
    }
    
    void JSON_API unpack(std::istream& isb, Value& val) {
        unsigned char type;
        Long num = 0;
        
        unsigned char* ptr;
        
        char* buffer = NULL;
        
        type = isb.get();
        
        if (type >= 0xe0) { // Negative FixNum (111x xxxx) (-32 ~ -1)
            val = type - 0x100;
            return;
        }
        if (type < 0xc0) {
            if (type < 0x80) { // Positive FixNum (0xxx xxxx) (0 ~ 127)
                val = type;
                return;
            }
            if (type < 0x90) { // FixMap (1000 xxxx)
                num  = type - 0x80;
                type = 0x80;
            } else if (type < 0xa0) { // FixArray (1001 xxxx)
                num  = type - 0x90;
                type = 0x90;
            } else /*if (type < 0xc0)*/ { // FixRaw (101x xxxx)
                num  = type - 0xa0;
                type = 0xa0;
            }
        }
        
        switch (type) {
        case 0xc0: // null
            break;
        
        case 0xc2: // false
            val = false;
            break;
        
        case 0xc3: // true
            val = true;
            break;
        
        case 0xca: { // float
            float tmp;
            ptr = (unsigned char*) &tmp;
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xcb: { // double
            double tmp;
            ptr = (unsigned char*) &tmp;
            ptr[7] = isb.get();
            ptr[6] = isb.get();
            ptr[5] = isb.get();
            ptr[4] = isb.get();
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xcf: {
            ULong tmp;
            ptr = (unsigned char*) &tmp;
            ptr[7] = isb.get();
            ptr[6] = isb.get();
            ptr[5] = isb.get();
            ptr[4] = isb.get();
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xce: {
            UInt tmp;
            ptr = (unsigned char*) &tmp;
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xcd: {
            unsigned short tmp;
            ptr = (unsigned char*) &tmp;
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xcc:
            val = (unsigned char)(
                (isb.get() <<  0)
            );
            break;
        
        case 0xd3: {
            Long tmp;
            ptr = (unsigned char*) &tmp;
            ptr[7] = isb.get();
            ptr[6] = isb.get();
            ptr[5] = isb.get();
            ptr[4] = isb.get();
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xd2: {
            Int tmp;
            ptr = (unsigned char*) &tmp;
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xd1: {
            short tmp;
            ptr = (unsigned char*) &tmp;
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            val = tmp;
        } break;
        
        case 0xd0:
            val = (char)(
                (isb.get() <<  0)
            );
            break;
        
        // string
        case 0xdb:
            num += (UInt)(
                (isb.get() <<  24) +
                (isb.get() <<  16)
            );
            
        case 0xda:
            num += (UInt)(
                (isb.get() <<  8) +
                (isb.get() <<  0)
            );
            
        case 0xa0:
            buffer = new char[num];
            isb.read(buffer, num);
            val = std::string(buffer, num);
            delete [] buffer;
            break;
        
        // map
        case 0xdf:
            num += (UInt)(
                (isb.get() <<  24) +
                (isb.get() <<  16)
            );
        case 0xde:
            num += (UInt)(
                (isb.get() <<  8) +
                (isb.get() <<  0)
            );
        case 0x80:
            while (num--) {
                Value k, v;
                unpack(isb, k);
                unpack(isb, v);
                val[k.asCString()] = v;
            }
            break;
        
        case 0xdd:
            num += (UInt)(
                (isb.get() <<  24) +
                (isb.get() <<  16)
            );
            
        case 0xdc:
            num += (UInt)(
                (isb.get() <<  8) +
                (isb.get() <<  0)
            );
            
        case 0x90:
            val = arrayValue;
            while (num--) {
                Value v;
                unpack(isb, v);
                val.append(v);
            }
            break;
        }
    }
    
    //void JSON_API unpack(std::istream& isb, std::stringstream& ss) {
    void JSON_API unpack(std::istream& isb, std::ostream& ss) {
        unsigned char type;
        Long num = 0;
        
        unsigned char* ptr;
        
        char* buffer = NULL;
        
        type = isb.get();
        
        if (type >= 0xe0) { // Negative FixNum (111x xxxx) (-32 ~ -1)
            ss << (int)(type - 0x100);
            return;
        }
        
        if (type < 0xc0) {
            if (type < 0x80) { // Positive FixNum (0xxx xxxx) (0 ~ 127)
                ss << (int)type;
                return;
            }
            if (type < 0x90) { // FixMap (1000 xxxx)
                num  = type - 0x80;
                type = 0x80;
            } else if (type < 0xa0) { // FixArray (1001 xxxx)
                num  = type - 0x90;
                type = 0x90;
            } else /*if (type < 0xc0)*/ { // FixRaw (101x xxxx)
                num  = type - 0xa0;
                type = 0xa0;
            }
        }
        
        switch (type) {
        case 0xc0: // null
            ss << "null";
            break;
        
        case 0xc2: // false
            ss << "false";
            break;
        
        case 0xc3: // true
            ss << "true";
            break;
        
        case 0xca: { // float
            float tmp;
            ptr = (unsigned char*) &tmp;
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << std::fixed << std::setprecision(1) << tmp;
        } break;
        
        case 0xcb: { // double
            double tmp;
            ptr = (unsigned char*) &tmp;
            ptr[7] = isb.get();
            ptr[6] = isb.get();
            ptr[5] = isb.get();
            ptr[4] = isb.get();
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << std::fixed << std::setprecision(2) << tmp;
        } break;
        
        case 0xcf: {
            ULong tmp;
            ptr = (unsigned char*) &tmp;
            ptr[7] = isb.get();
            ptr[6] = isb.get();
            ptr[5] = isb.get();
            ptr[4] = isb.get();
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << tmp;
        } break;
        
        case 0xce: {
            UInt tmp;
            ptr = (unsigned char*) &tmp;
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << tmp;
        } break;
        
        case 0xcd: {
            unsigned short tmp;
            ptr = (unsigned char*) &tmp;
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << tmp;
        } break;
        
        case 0xcc:
            ss << (unsigned int)(
                (isb.get() <<  0)
            );
            break;
        
        case 0xd3: {
            Long tmp;
            ptr = (unsigned char*) &tmp;
            ptr[7] = isb.get();
            ptr[6] = isb.get();
            ptr[5] = isb.get();
            ptr[4] = isb.get();
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << tmp;
        } break;
        
        case 0xd2: {
            Int tmp;
            ptr = (unsigned char*) &tmp;
            ptr[3] = isb.get();
            ptr[2] = isb.get();
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << tmp;
        } break;
        
        case 0xd1: {
            short tmp;
            ptr = (unsigned char*) &tmp;
            ptr[1] = isb.get();
            ptr[0] = isb.get();
            ss << tmp;
        } break;
        
        case 0xd0:
            ss << (int)(
                (isb.get() <<  0)
            );
            break;
        
        // string
        case 0xdb:
            num += (UInt)(
                (isb.get() <<  24) +
                (isb.get() <<  16)
            );
            
        case 0xda:
            num += (UInt)(
                (isb.get() <<  8) +
                (isb.get() <<  0)
            );
            
        case 0xa0:
            buffer = new char[num + 1];
            isb.read(buffer, num);
            buffer[num] = '\0';
            ss << valueToQuotedString(buffer);
            delete [] buffer;
            break;
        
        // map
        case 0xdf:
            num += (UInt)(
                (isb.get() <<  24) +
                (isb.get() <<  16)
            );
        case 0xde:
            num += (UInt)(
                (isb.get() <<  8) +
                (isb.get() <<  0)
            );
        case 0x80:
            ss << "{";
            while (num--) {
                Value k;
                unpack(isb, k);
                ss << valueToQuotedString(k.asCString()) << ":";
                unpack(isb, ss);
                if (num > 0) {
                    ss << ",";
                }
            }
            ss << "}";
            break;
        
        // array
        case 0xdd:
            num += (UInt)(
                (isb.get() <<  24) +
                (isb.get() <<  16)
            );
            
        case 0xdc:
            num += (UInt)(
                (isb.get() <<  8) +
                (isb.get() <<  0)
            );
            
        case 0x90:
            ss << "[";
            while (num--) {
                unpack(isb, ss);
                if (num > 0) {
                    ss << ",";
                }
            }
            ss << "]";
            break;
        }
    }
} }
