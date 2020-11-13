#ifndef __STACK_H_
#define __STACK_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cpluscplus
extern "C"
{
#endif

    typedef struct Validator
    {
        bool (*const validate)(struct Validator *pThis, int val);
        void *const pData;
    } Validator; // 检验数据结构体

    typedef struct
    {
        const int min;
        const int max;
    } Range; // 范围

    typedef struct
    {
        int previousValue;
    } PreviousValue; // 前一个值

    typedef struct
    {
        int top;
        const size_t size;
        int *const pBuf;
        Validator *const pValidator;
    } Stack; // 栈

    bool validateRange(Validator *pThis, int val);    // 范围验证
    bool validatePrevious(Validator *pThis, int val); // 递增验证

    bool push(Stack *p, int val);
    bool pop(Stack *p, int *pRet);

// 结构体初始化宏
#define newStack(buf)                        \
    {                                        \
        0, sizeof(buf) / sizeof(int), (buf), \
            NULL                             \
    }

#define rangeValidator(pRange) \
    {                          \
        validateRange,         \
            pRange             \
    }

#define previousValidator(pPrevious) \
    {                                \
        validatePrevious             \
            pPrevious                \
    }

#define newStackWithValidator(buf, pValidator) \
    {                                          \
        0, sizeof(buf) / sizeof(int), (buf),   \
            pValidator                         \
    }

#ifdef __cpluscplus
}
#endif

#endif
