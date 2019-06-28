#ifndef __TINY_PRINTF_H__
#define __TINY_PRINTF_H__

int tiny_printf( const char *format, ... );
int vsnprintf( char *apBuf, size_t aMaxLen, const char *apFmt, va_list args );
int snprintf( char *apBuf, size_t aMaxLen, const char *apFmt, ... );
int sprintf( char *apBuf, const char *apFmt, ... );
int vsprintf( char *apBuf, const char *apFmt, va_list args );
const char *mkSize (unsigned long long aSize, char *apBuf, int aLen);

#endif // __TINY_PRINTF_H__
