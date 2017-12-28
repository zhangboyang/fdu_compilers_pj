#pragma once


#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cctype>

#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <memory>
#include <functional>

typedef int32_t data_off_t;
#define panic() abort()

#include "minijavac.h"
#include "astnode.h"
#include "codegen.h"

static inline data_off_t ROUNDUP(data_off_t a, data_off_t b)
{
	return (a + b - 1) / b * b;
}