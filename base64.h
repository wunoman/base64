#ifndef _BASE64_H
#define _BASE64_H

#include <new.h>  // placement new operator support
#include <stdarg.h>
#include <time.h>

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

//------------------------------------------------------------------------------------------------

#define self ( *this )

//------------------------------------------------------------------------------------------------
#ifdef DEBUG
constexpr const auto DEBUG_MODE = true;
#else
constexpr const auto DEBUG_MODE = false;
#endif

//------------------------------------------------------------------------------------------------
namespace base64 {

using Buffer = std::vector<char>;
void encode( const char* p, int length, Buffer& outbuf, int needreturn = 1 );

void decode( const char* p, int length, Buffer& outbuf, std::uint64_t& phase,
             std::uint64_t& nValue );

};  // namespace base64

#endif
