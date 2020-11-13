#include "stack.h"

static bool isStackFull(const Stack *p)
{
    return p->top == p->size;
}

static bool isStackEmpty(const Stack *p)
{
    return p->top == 0;
}

// 判断范围
bool validateRange(Validator *pThis, int val)
{
    Range *pRange = (Range *)(pThis->pData);

    return (pRange->min <= val) && (val <= pRange->max);
}

// 判断后面的值是否比前面的大
bool validatePrevious(Validator *pThis, int val)
{
    PreviousValue *pPrevious = (PreviousValue *)pThis->pData;

    if (val < pPrevious->previousValue)
        return false;

    pPrevious->previousValue = val;

    return true;
}

bool validate(Validator *p, int val)
{
    if (!p)
        return true;

    return p->validate(p, val);
}

bool push(Stack *p, int val)
{
    if (!validate(p->pValidator, val) || isStackFull(p))
        return false;

    p->pBuf[p->top++] = val;

    return true;
}

bool pop(Stack *p, int *pRet)
{
    if (isStackEmpty(p))
        return false;

    *pRet = p->pBuf[--p->top];

    return true;
}
