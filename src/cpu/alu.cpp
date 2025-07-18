#include <limits.h>
#include <stdarg.h>

#include "alu.h"
#include <cstdio>
Word ArithmaticLogicUnit::add(int total_addends, ...) 
{
    u_int64_t result = 0;
    va_list addends;

    va_start(addends, total_addends);
    for (int i = 0; i < total_addends; i++)
    {
        Word arg = va_arg(addends, Word);
        result += arg;
    }
    va_end(addends);

    if (result > UINT_MAX)
    
        carry_flag = true;
    else 
        carry_flag = false;

    // printf("%d \n", result);
    
    return result & UINT_MAX;
};

Word ArithmaticLogicUnit::subtract(int total_subtrahends, Word base, ...) 
{
    u_int64_t result = base;
    va_list addends;

    va_start(addends, total_subtrahends);
    for (int i = 0; i < total_subtrahends; i++)
    {
        Word arg = va_arg(addends, Word);
        result -= arg;
    }
    va_end(addends);

    if (result > UINT_MAX)
        carry_flag = false;
    else 
        carry_flag = true;

    return result & UINT_MAX;
};