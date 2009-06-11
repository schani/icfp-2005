#include <stdio.h>
#include <stdarg.h>

#include "dprintf.h"

void
dprintf( const char *fmt, ... )
{
#ifndef NO_OUTPUT
    va_list ap;

    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end( ap );
#endif
}
