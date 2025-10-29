#pragma once

#include <string>
#include <flecs.h>

namespace ECSModule
{
    namespace Utils
    {

    inline int string_serialize(const flecs::serializer* s, const std::string* data) {
        if (!s || !data) return -1;                       
        ecs_string_t tmp = const_cast<char*>(data->c_str()); 
        return s->value(flecs::String, &tmp);             
    }

    inline void string_assign_string(std::string* dst, const char* value) {
        if (!dst) return;                                 
        if (value) *dst = value;                          
        else dst->clear();                                
    }

    inline void string_assign_null(std::string* dst) {
        if (dst) dst->clear();                            
    }
    }
}
