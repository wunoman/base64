#include "base64.h"

namespace base64 {

//------------------------------------------------------------------------------------------------
void encode( const char* p, int length, Buffer& outbuf, int needreturn ) {
  constexpr auto LINE_WIDTH = 76;
  const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned char Tmp[ 4 ] = { 0 };
  auto LineLength { 0 };
  auto pIn { p };
  for ( int i = 0; i < ( int ) ( length / 3 ); i++ ) {
    Tmp[ 1 ] = *pIn++;
    Tmp[ 2 ] = *pIn++;
    Tmp[ 3 ] = *pIn++;
    outbuf.push_back( EncodeTable[ Tmp[ 1 ] >> 2 ] );
    outbuf.push_back( EncodeTable[ ( ( Tmp[ 1 ] << 4 ) | ( Tmp[ 2 ] >> 4 ) ) & 0x3F ] );
    outbuf.push_back( EncodeTable[ ( ( Tmp[ 2 ] << 2 ) | ( Tmp[ 3 ] >> 6 ) ) & 0x3F ] );
    outbuf.push_back( EncodeTable[ Tmp[ 3 ] & 0x3F ] );
    if ( LineLength += 4, LineLength == LINE_WIDTH && needreturn ) {
      outbuf.push_back( '\r' );
      outbuf.push_back( '\n' );
      LineLength = 0;
    }
  }
  int Mod { length % 3 };
  if ( 1 == Mod ) {
    Tmp[ 1 ] = *pIn++;
    outbuf.push_back( EncodeTable[ ( Tmp[ 1 ] & 0xFC ) >> 2 ] );
    outbuf.push_back( EncodeTable[ ( ( Tmp[ 1 ] & 0x03 ) << 4 ) ] );
    outbuf.push_back( '=' );
    outbuf.push_back( '=' );
  } else if ( 2 == Mod ) {
    Tmp[ 1 ] = *pIn++;
    Tmp[ 2 ] = *pIn++;
    outbuf.push_back( EncodeTable[ ( Tmp[ 1 ] & 0xFC ) >> 2 ] );
    outbuf.push_back(
        EncodeTable[ ( ( Tmp[ 1 ] & 0x03 ) << 4 ) | ( ( Tmp[ 2 ] & 0xF0 ) >> 4 ) ] );
    outbuf.push_back( EncodeTable[ ( ( Tmp[ 2 ] & 0x0F ) << 2 ) ] );
    outbuf.push_back( '=' );
  }
}

//------------------------------------------------------------------------------------------------
// 编码规则
// 1. 3个字符变成4个字符,每个字符两位高位为0, 不够3倍数的长度用'＝'号补齐,
// 2. 76个字符加一个换行符,
// 3. 将64个(每个字符只有6位有效,其表达范围为64,0－63)数字用一个符号表来表示,
// 4. 0-25=='A'-'Z'; 26-51=='a'-'z'; 52-61=='0'-'9', 62=='+'; 63=='/'.
void decode( const char* p, int length, Buffer& outbuf, std::uint64_t& phase,
             std::uint64_t& nValue ) {
  // 反查表方式,用65填充非符号表索引对应的值,
  // 64用于标识补齐符号'='的ASCII做为下标在表中对应的值
  const char DecodeTable[] = {
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,  //  0-13
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,  // 14-27
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,  // 28-42
    65, 65, 65, 65,
    62,  // '+'==43
    65, 65, 65,
    63,                                      // '/'==47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  // '0'-'9'==48-57
    65, 65, 65, 64, 65, 65, 65,              // '='==61
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  // 'A'-'Z' == 65-90
    65, 65, 65, 65, 65, 65, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  // 'a'-'z' == 97-122
  };
  auto ucp { reinterpret_cast<const unsigned char*>( p ) };
  auto end { ucp + length };
  // auto nValue { 0 };
  // auto phase { 0 };
  while ( ucp < end ) {
    const unsigned char ch { *ucp++ };
    // 无效的下标或是无效的表字符(没有提供错误检测,允许参杂非表字符存在)
    if ( ch > 'z' || DecodeTable[ ch ] == 65 ) {
      continue;
    }
    if ( '=' == ch ) {  // ch=='=', ch==61, DecodeTable[ch]==64
      switch ( ( phase + 1 ) % 4 ) {
        case 0:
        case 3:
          ++phase;  // 补齐4个为一节的计数
      }
      continue;
    }
    switch ( ( ++phase ) % 4 ) {
      case 1: {
        nValue = DecodeTable[ ch ] << 18;
        break;
      }
      case 2: {
        nValue += DecodeTable[ ch ] << 12;
        outbuf.push_back( ( char ) ( ( nValue & 0x00FF0000 ) >> 16 ) );
        break;
      }
      case 3: {
        nValue += DecodeTable[ ch ] << 6;
        outbuf.push_back( ( char ) ( ( nValue & 0x0000FF00 ) >> 8 ) );
        break;
      }
      case 0: {
        nValue += DecodeTable[ ch ];
        outbuf.push_back( ( char ) ( nValue & 0x000000FF ) );
        break;
      }
    }  // switch
  }    // while
}

//------------------------------------------------------------------------------------------------
enum OP {
  op_undefined = 0,
  op_encode = 1,
  op_decode = 2,
};

struct Cargo {
  OP op = OP::op_undefined;
  std::string inputfile;
  std::string input;  // string
  std::string outputfile;
  fstream ifs;
  fstream ofs;
  bool needreturn = false;
  bool outtofile = false;
  std::uint64_t block_size = 3 * 1216;  // 76 times
};
};  // namespace base64

using namespace base64;
//------------------------------------------------------------------------------------------------
bool premise( base64::Cargo& cargo ) {
  auto ret = true;
  do {
    if ( cargo.op == OP::op_undefined ) {
      std::cout << "using -encode or -decode parameter \n";
      ret = false;
      break;
    }
    // allow read from console
    //if ( cargo.inputfile.empty() && cargo.input.empty() ) {
    //  std::cout << "using -input or -inputfile parameter \n";
    //  ret = false;
    //  break;
    //}
    if ( !cargo.inputfile.empty() ) {
      cargo.ifs.open( cargo.inputfile, ios::binary | ios::in );
      ret = cargo.ifs.is_open();
      if ( !ret ) {
        std::cout << "can't open inputfile \n";
        cargo.ifs.close();
        break;
      }
    }
    if ( !cargo.outputfile.empty() ) {
      cargo.ofs.open( cargo.outputfile, ios::binary | ios::out );
      ret = cargo.ofs.is_open();
      if ( !ret ) {
        std::cout << "can't open outputfile \n";
        cargo.ofs.close();
        break;
      }
    }
  } while ( 0 );
  return ret;
}

void write_and_resize( Cargo& cargo, Buffer& outbuffer ) {
  auto output_sz = outbuffer.size();
  if ( output_sz > 0 ) {
    if ( cargo.outtofile ) {
      cargo.ofs.write( outbuffer.data(), output_sz );
    } else {
      std::cout.write( outbuffer.data(), output_sz );
    }
    outbuffer.resize( 0 );
  }
}

void decode_it( Cargo& cargo ) {
  Buffer outbuffer;
  std::uint64_t phase { 0 };
  std::uint64_t nValue { 0 };
  if ( cargo.ifs.is_open() ) {
    auto& ifs = cargo.ifs;
    char buffer[ cargo.block_size ];
    auto readbuffer = &buffer[ 0 ];
    while ( !ifs.eof() ) {
      ifs.read( readbuffer, 1 << 10 );
      auto gcount = ifs.gcount();
      if ( gcount > 0 ) {
        decode( readbuffer, gcount, outbuffer, phase, nValue );
        write_and_resize( cargo, outbuffer );
      }
    }
  } else if ( !cargo.input.empty() ) {
    decode( cargo.input.data(), cargo.input.size(), outbuffer, phase, nValue );
    write_and_resize( cargo, outbuffer );
  } else {
    while ( true ) {
      char buffer[ cargo.block_size ];
      std::cin.read( &buffer[ 0 ], cargo.block_size );
      auto readbuffer = &buffer[ 0 ];
      auto gcount = std::cin.gcount();
      if ( gcount > 0 ) {
        decode( readbuffer, gcount, outbuffer, phase, nValue );
        write_and_resize( cargo, outbuffer );
      }
      // ctrl+d
      if ( std::cin.eof() ) break;
    }
  }
}

void encode_it( Cargo& cargo ) {
  Buffer outbuffer;
  if ( cargo.ifs.is_open() ) {
    auto& ifs = cargo.ifs;
    char buffer[ cargo.block_size ];
    auto readbuffer = &buffer[ 0 ];
    while ( !ifs.eof() ) {
      ifs.read( readbuffer, cargo.block_size );
      auto gcount = ifs.gcount();
      if ( gcount > 0 ) {
        encode( readbuffer, gcount, outbuffer, cargo.needreturn );
        write_and_resize( cargo, outbuffer );
      }
    }
  } else if ( !cargo.input.empty() ) {
    encode( cargo.input.data(), cargo.input.size(), outbuffer, cargo.needreturn );
    write_and_resize( cargo, outbuffer );
  } else {
    while ( true ) {
      char buffer[ cargo.block_size ];
      std::cin.read( &buffer[ 0 ], cargo.block_size );
      auto readbuffer = &buffer[ 0 ];
      auto gcount = std::cin.gcount();
      if ( gcount > 0 ) {
        encode( readbuffer, gcount, outbuffer, cargo.needreturn );
        write_and_resize( cargo, outbuffer );
      }
      // ctrl+d
      if ( std::cin.eof() ) break;
    }
  }
}

void handle( base64::Cargo& cargo ) {
  if ( premise( cargo ) ) {
    cargo.outtofile = cargo.ofs.is_open();
    if ( cargo.op == OP::op_encode ) {
      encode_it( cargo );
    } else if ( cargo.op == OP::op_decode ) {
      decode_it( cargo );
    }
    if ( cargo.ifs.is_open() ) {
      cargo.ifs.close();
    }
    if ( cargo.ofs.is_open() ) {
      cargo.ofs.close();
    }
  }
}

void process_param( int argc, const char** argv, Cargo& cargo ) {
  for ( auto i = 1; i < argc; ++i ) {
    if ( 0 == strcmp( argv[ i ], "-encode" ) ) {
      cargo.op = OP::op_encode;
    }
    if ( 0 == strcmp( argv[ i ], "-decode" ) ) {
      cargo.op = OP::op_decode;
    }
    if ( 0 == strcmp( argv[ i ], "-input" ) && i + 1 < argc ) {
      cargo.input = argv[ i + 1 ];
    }
    if ( 0 == strcmp( argv[ i ], "-inputfile" ) && i + 1 < argc ) {
      cargo.inputfile = argv[ i + 1 ];
    }
    if ( 0 == strcmp( argv[ i ], "-outputfile" ) && i + 1 < argc ) {
      cargo.outputfile = argv[ i + 1 ];
    }
    if ( 0 == strcmp( argv[ i ], "-needreturn" ) ) {  // used on encode
      cargo.needreturn = true;
    }
  }
}

void usage() {
  std::cout << R"(
base64 Encode and Decode tool (20220915)
usage: base64.exe <options>
    -encode                   : encode input
    -decode                   : decode input
    -input <string>
    -inputfile <filename>     : if input and inputfile do not provided read from std::cin
    -outputfile <filename>    : if not provided output to std::cout
    -needreturn               : when encode one line have 76 chars
  )";
}

//------------------------------------------------------------------------------------------------
int main( int argc, const char** argv ) {
  if ( argc == 1 ) {
    usage();
  } else {
    base64::Cargo cargo;
    process_param( argc, argv, cargo );
    handle( cargo );
  }
  return 0;
}

