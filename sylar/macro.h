//常用宏封装

#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __11vm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif

//断言宏的封装
#define SYLAR_ASSERT(x) \
    if(!(x)) { \
        HR_LOG_ERROR(HR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << hr::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }



#define SYLAR_ASSERT2(x, w) \
    if(!(x)) { \
        HR_LOG_ERROR(HR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << hr::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif