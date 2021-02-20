/******************************************************************************
 *
 * Copyright (C) 1997-2015 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#include "qcstring.h"
#include "qgstring.h"

#include <qstring.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
//#include <qregexp.h>
#include <qdatastream.h>

QCString &QCString::sprintf( const char *format, ... )
{
  va_list ap;
  va_start( ap, format );
  const int minlen=256;
  int l = length();
  if (l<minlen) { resize(minlen); l=minlen; }
  int n=vsnprintf( rawData(), l, format, ap);
  if (n<0) n=l;
  resize(n+1);
  va_end( ap );
  return *this;
}

int QCString::find( char c, int index, bool cs ) const
{
  if (index<0 || index>=(int)length()) return -1; // index outside string
  const char *pos;
  if (cs)
  {
    pos = strchr(data()+index,c);
  }
  else
  {
    pos = data()+index;
    c = tolower((unsigned char)c);
    while (*pos && tolower((unsigned char)*pos)!=c) pos++;
    if (!*pos && c) pos=0; // not found
  }
  return pos ? (int)(pos - data()) : -1;
}

int QCString::find( const char *str, int index, bool cs ) const
{
  int l = length();
  if (index<0 || index>=l) return -1; // index outside string
  if (!str)  return -1;               // no string to search for
  if (!*str) return index;           // empty string matching at index
  const char *pos;
  if (cs) // case sensitive
  {
    pos = strstr(data()+index,str);
  }
  else // case insensitive
  {
    pos = data();
    int len = qstrlen(str);
    while (*pos)
    {
      if (qstrnicmp(pos,str,len)==0) break;
      pos++;
    }
    if (!*pos) pos = 0; // not found
  }
  return pos ? (int)(pos - data()) : -1;
}

int QCString::find( const QCString &str, int index, bool cs ) const
{
  return find(str.data(),index,cs);
}

#if 0
int QCString::find( const QRegExp &rx, int index ) const
{
  if ( index < 0 )
    index += length();
  return rx.match( data(), index );
}
#endif

int QCString::findRev( char c, int index, bool cs) const
{
  const char *b = data();
  const char *pos;
  int len = length();
  if (len==0) return -1; // empty string
  if (index<0) // start from end
  {
    if (cs)
    {
      pos = strrchr(b,c);
      return pos ? (int)(pos - b) : -1;
    }
    index=len;
  }
  else if (index>len) // bad index
  {
    return -1;
  }
  pos = b+index;
  if (cs)
  {
    while ( pos>=b && *pos!=c) pos--;
  }
  else
  {
    c = tolower((unsigned char)c);
    while ( pos>=b && tolower((unsigned char)*pos)!=c) pos--;
  }
  return pos>=b ? (int)(pos - b) : -1;
}

int QCString::findRev( const char *str, int index, bool cs) const
{
  int slen = qstrlen(str);
  int len = length();
  if (index<0) index = len-slen; // start from end
  else if (index>len) return -1; // bad index
  else if (index+slen>len) index=len-slen; // str would be too long
  if (index<0) return -1; // no match possible
  const char *pos = data()+index;
  if (cs) // case sensitive
  {
    for (int i=index; i>=0; i--) if (qstrncmp(pos--,str,slen)==0) return i;
  }
  else // case insensitive
  {
    for (int i=index; i>=0; i--) if (qstrnicmp(pos,str,slen)==0) return i;
  }
  return -1;
}

#if 0
int QCString::findRev( const QRegExp &rx, int index ) const
{
  if ( index < 0 )			// neg index ==> start from end
    index += length();
  if ( (uint)index > length() )		// bad index
    return -1;
  while( index >= 0 ) {
    if ( rx.match( data(), index ) == index )
      return index;
    index--;
  }
  return -1;
}
#endif

int QCString::contains( char c, bool cs ) const
{
  if (length()==0) return 0;
  int count=0;
  const char *pos = data();
  if (cs)
  {
    while (*pos) if (*pos++ == c) count++;
  }
  else
  {
    c = tolower((unsigned char)c);
    while (*pos)
    {
      if (tolower((unsigned char)*pos)==c) count++;
      pos++;
    }
  }
  return count;
}

int QCString::contains( const char *str, bool cs ) const
{
  if (str==0 || length()==0) return 0;
  int count=0;
  const char *pos = data();
  int len = qstrlen(str);
  while (*pos)
  {
    if (cs)
    {
      if (qstrncmp(pos,str,len)==0) count++;
    }
    else
    {
      if (qstrnicmp(pos,str,len)==0) count++;
    }
    pos++;
  }
  return count;
}

#if 0
int QCString::contains( const QRegExp &rx ) const
{
  if ( isEmpty() )
    return rx.match( data() ) < 0 ? 0 : 1;
  int count = 0;
  int index = -1;
  int len = length();
  while ( index < len-1 ) {			// count overlapping matches
    index = rx.match( data(), index+1 );
    if ( index < 0 )
      break;
    count++;
  }
  return count;
}
#endif


QCString QCString::simplifyWhiteSpace() const
{
  if ( isEmpty() )                            // nothing to do
    return *this;

  QCString result( length()+1 );
  const char *from  = data();
  char *to    = result.rawData();
  char *first = to;
  while ( TRUE )
  {
    while ( *from && isspace((uchar) *from) )
      from++;
    while ( *from && !isspace((uchar)*from) )
      *to++ = *from++;
    if ( *from )
      *to++ = 0x20;                       // ' '
    else
      break;
  }
  if ( to > first && *(to-1) == 0x20 )
    to--;
  *to = '\0';
  result.resize( (int)(to - result.data()) + 1 );
  return result;
}

QCString &QCString::replace( uint index, uint len, const char *s)
{
  remove( index, len );
  insert( index, s );
  return *this;
}

#if 0
QCString &QCString::replace( const QRegExp &rx, const char *str )
{
  if ( isEmpty() )
    return *this;
  int index = 0;
  int slen  = qstrlen(str);
  int len;
  while ( index < (int)length() ) {
    index = rx.match( data(), index, &len, FALSE );
    if ( index >= 0 ) {
      replace( index, len, str );
      index += slen;
      if ( !len )
        break;	// Avoid infinite loop on 0-length matches, e.g. [a-z]*
    }
    else
      break;
  }
  return *this;
}
#endif

static bool ok_in_base( char c, int base )
{
    if ( base <= 10 )
	return c>='0' && c<='9' && (c-'0') < base;
    else
	return (c>='0' && c<='9') ||
               (c >= 'a' && c < char('a'+base-10)) ||
               (c >= 'A' && c < char('A'+base-10));
}

short QCString::toShort(bool *ok, int base) const
{
  long v = toLong( ok, base );
  if ( ok && *ok && (v < -32768 || v > 32767) ) {
    *ok = FALSE;
    v = 0;
  }
  return (short)v;
}

ushort QCString::toUShort(bool *ok,int base) const
{
  ulong v = toULong( ok, base );
  if ( ok && *ok && (v > 65535) ) {
    *ok = FALSE;
    v = 0;
  }
  return (ushort)v;
}

int QCString::toInt(bool *ok, int base) const
{
  return (int)toLong( ok, base );
}

uint QCString::toUInt(bool *ok,int base) const
{
  return (uint)toULong( ok, base );
}


long QCString::toLong(bool *ok,int base) const
{
  const char *p = data();
  long val=0;
  int l = length();
  const long max_mult = INT_MAX / base;
  bool is_ok = FALSE;
  int neg = 0;
  if ( !p )
    goto bye;
  while ( l && isspace(*p) )			// skip leading space
    l--,p++;
  if ( l && *p == '-' ) {
    l--;
    p++;
    neg = 1;
  } else if ( *p == '+' ) {
    l--;
    p++;
  }

  // NOTE: toULong() code is similar
  if ( !l || !ok_in_base(*p,base) )
    goto bye;
  while ( l && ok_in_base(*p,base) ) {
    l--;
    int dv;
    if ( *p>='0' && *p<='9' ) {
      dv = *p-'0';
    } else {
      if ( *p >= 'a' && *p <= 'z' )
        dv = *p - 'a' + 10;
      else
        dv = *p - 'A' + 10;
    }
    if ( val > max_mult || (val == max_mult && dv > (INT_MAX%base)+neg) )
      goto bye;
    val = base*val + dv;
    p++;
  }
  if ( neg )
    val = -val;
  while ( l && isspace(*p) )			// skip trailing space
    l--,p++;
  if ( !l )
    is_ok = TRUE;
bye:
  if ( ok )
    *ok = is_ok;
  return is_ok ? val : 0;
}

ulong QCString::toULong(bool *ok,int base) const
{
  const char *p = data();
  ulong val=0;
  int l = length();
  const ulong max_mult = 429496729;		// UINT_MAX/10, rounded down
  bool is_ok = FALSE;
  if ( !p )
    goto bye;
  while ( l && isspace(*p) )			// skip leading space
    l--,p++;
  if ( *p == '+' )
    l--,p++;

  // NOTE: toLong() code is similar
  if ( !l || !ok_in_base(*p,base) )
    goto bye;
  while ( l && ok_in_base(*p,base) ) {
    l--;
    uint dv;
    if ( *p>='0' && *p<='9' ) {
      dv = *p-'0';
    } else {
      if ( *p >= 'a' && *p <= 'z' )
        dv = *p - 'a' + 10;
      else
        dv = *p - 'A' + 10;
    }
    if ( val > max_mult || (val == max_mult && dv > (UINT_MAX%base)) )
      goto bye;
    val = base*val + dv;
    p++;
  }

  while ( l && isspace(*p) )			// skip trailing space
    l--,p++;
  if ( !l )
    is_ok = TRUE;
bye:
  if ( ok )
    *ok = is_ok;
  return is_ok ? val : 0;
}

uint64 QCString::toUInt64(bool *ok,int base) const
{
  const char *p = data();
  uint64 val=0;
  int l = length();
  const uint64 max_mult = 1844674407370955161ULL;  // ULLONG_MAX/10, rounded down
  bool is_ok = FALSE;
  if ( !p )
    goto bye;
  while ( l && isspace(*p) )		 	   // skip leading space
    l--,p++;
  if ( *p == '+' )
    l--,p++;

  // NOTE: toULong() code is similar
  if ( !l || !ok_in_base(*p,base) )
    goto bye;
  while ( l && ok_in_base(*p,base) ) {
    l--;
    uint dv;
    if ( *p>='0' && *p<='9' ) {
      dv = *p-'0';
    } else {
      if ( *p >= 'a' && *p <= 'z' )
        dv = *p - 'a' + 10;
      else
        dv = *p - 'A' + 10;
    }
    if ( val > max_mult || (val == max_mult && dv > (ULLONG_MAX%base)) )
      goto bye;
    val = base*val + dv;
    p++;
  }

  while ( l && isspace(*p) )			// skip trailing space
    l--,p++;
  if ( !l )
    is_ok = TRUE;
bye:
  if ( ok )
    *ok = is_ok;
  return is_ok ? val : 0;
}

//-------------------------------------------------

void *qmemmove( void *dst, const void *src, uint len )
{
    char *d;
    char *s;
    if ( dst > src ) {
	d = (char *)dst + len - 1;
	s = (char *)src + len - 1;
	while ( len-- )
	    *d-- = *s--;
    } else if ( dst < src ) {
	d = (char *)dst;
	s = (char *)src;
	while ( len-- )
	    *d++ = *s++;
    }
    return dst;
}

char *qstrdup( const char *str )
{
    if ( !str )
	return 0;
    char *dst = new char[qstrlen(str)+1];
    CHECK_PTR( dst );
    return strcpy( dst, str );
}

char *qstrncpy( char *dst, const char *src, uint len )
{
    if ( !src )
	return 0;
    strncpy( dst, src, len );
    if ( len > 0 )
	dst[len-1] = '\0';
    return dst;
}

int qstricmp( const char *str1, const char *str2 )
{
    const uchar *s1 = (const uchar *)str1;
    const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return s1 == s2 ? 0 : (int)(s2 - s1);
    for ( ; !(res = (c=tolower(*s1)) - tolower(*s2)); s1++, s2++ )
	if ( !c )				// strings are equal
	    break;
    return res;
}

int qstrnicmp( const char *str1, const char *str2, uint len )
{
    const uchar *s1 = (const uchar *)str1;
    const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return (int)(s2 - s1);
    for ( ; len--; s1++, s2++ ) {
	if ( (res = (c=tolower(*s1)) - tolower(*s2)) )
	    return res;
	if ( !c )				// strings are equal
	    break;
    }
    return 0;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<( QDataStream &s, const QByteArray &a )
{
    return s.writeBytes( a.data(), a.size() );
}

QDataStream &operator>>( QDataStream &s, QByteArray &a )
{
    Q_UINT32 len;
    s >> len;					// read size of array
    if ( len == 0 || s.eof() ) {		// end of file reached
	a.resize( 0 );
	return s;
    }
    if ( !a.resize( (uint)len ) ) {		// resize array
#if defined(CHECK_NULL)
	qWarning( "QDataStream: Not enough memory to read QByteArray" );
#endif
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( a.data(), (uint)len );
    return s;
}

QDataStream &operator<<( QDataStream &s, const QCString &str )
{
    return s.writeBytes( str.data(), str.size() );
}

QDataStream &operator>>( QDataStream &s, QCString &str )
{
    Q_UINT32 len;
    s >> len;					// read size of string
    if ( len == 0 || s.eof() ) {		// end of file reached
	str.resize( 0 );
	return s;
    }
    if ( !str.resize( (uint)len )) {// resize string
#if defined(CHECK_NULL)
	qWarning( "QDataStream: Not enough memory to read QCString" );
#endif
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( str.rawData(), (uint)len );
    return s;
}

#endif //QT_NO_DATASTREAM

inline QCString operator+( const QCString &s1, const QGString &s2 )
{
  return s1.str()+s2.data();
}

inline QCString operator+( const QGString &s1, const QCString &s2 )
{
  return s1.data()+s2.str();
}

/// substitute all occurrences of \a src in \a s by \a dst
QCString substitute(const QCString &s,const QCString &src,const QCString &dst)
{
  if (s.isEmpty() || src.isEmpty()) return s;
  const char *p, *q;
  int srcLen = src.length();
  int dstLen = dst.length();
  int resLen;
  if (srcLen!=dstLen)
  {
    int count;
    for (count=0, p=s.data(); (q=strstr(p,src))!=0; p=q+srcLen) count++;
    resLen = s.length()+count*(dstLen-srcLen);
  }
  else // result has same size as s
  {
    resLen = s.length();
  }
  QCString result(resLen+1);
  char *r;
  for (r=result.rawData(), p=s; (q=strstr(p,src))!=0; p=q+srcLen)
  {
    int l = (int)(q-p);
    memcpy(r,p,l);
    r+=l;

    if (dst) memcpy(r,dst,dstLen);
    r+=dstLen;
  }
  if (r)
  {
    qstrcpy(r,p);
  }
  //printf("substitute(%s,%s,%s)->%s\n",s,src,dst,result.data());
  return result;
}


/// substitute all occurrences of \a src in \a s by \a dst, but skip
/// each consecutive sequence of \a src where the number consecutive
/// \a src matches \a skip_seq; if \a skip_seq is negative, skip any
/// number of consecutive \a src
QCString substitute(const QCString &s,const QCString &src,const QCString &dst,int skip_seq)
{
  if (s.isEmpty() || src.isEmpty()) return s;
  const char *p, *q;
  int srcLen = src.length();
  int dstLen = dst.length();
  int resLen;
  if (srcLen!=dstLen)
  {
    int count;
    for (count=0, p=s.data(); (q=strstr(p,src))!=0; p=q+srcLen) count++;
    resLen = s.length()+count*(dstLen-srcLen);
  }
  else // result has same size as s
  {
    resLen = s.length();
  }
  QCString result(resLen+1);
  char *r;
  for (r=result.rawData(), p=s; (q=strstr(p,src))!=0; p=q+srcLen)
  {
    // search a consecutive sequence of src
    int seq = 0, skip = 0;
    if (skip_seq)
    {
      for (const char *n=q+srcLen; qstrncmp(n,src,srcLen)==0; seq=1+skip, n+=srcLen)
        ++skip; // number of consecutive src after the current one

      // verify the allowed number of consecutive src to skip
      if (skip_seq > 0 && skip_seq != seq)
        seq = skip = 0;
    }

    // skip a consecutive sequence of src when necessary
    int l = (int)((q + seq * srcLen)-p);
    memcpy(r,p,l);
    r+=l;

    if (skip)
    {
      // skip only the consecutive src found after the current one
      q += skip * srcLen;
      // the next loop will skip the current src, aka (p=q+srcLen)
      continue;
    }

    if (dst) memcpy(r,dst,dstLen);
    r+=dstLen;
  }
  qstrcpy(r,p);
  result.resize((int)strlen(result.data())+1);
  //printf("substitute(%s,%s,%s)->%s\n",s,src,dst,result.data());
  return result;
}

