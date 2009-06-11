#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include <string.h>

#define MALLOC_TYPE(t)        ({ t *x = (t*)malloc(sizeof(t)); assert(x != 0); memset(x, 0, sizeof(t)); x; })
#define xstrdup(s)            ({ char *__n = strdup((s)); assert(__n != 0); __n; })

#define	min(a,b)	(((a)<(b))?(a):(b))
#define	max(a,b)	(((a)>(b))?(a):(b))


#endif
