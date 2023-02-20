#include "md5.h"

#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <cryptopp/strciphr.h>

using namespace CryptoPP;

namespace Horizon {

Container::String md5(const Container::String &input) {
    CryptoPP::MD5 hash;
    std::string output;
    CryptoPP::StringSource(input.c_str(), true,
                           new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(output))));
    return Container::String{output};
}

} // namespace Horizon