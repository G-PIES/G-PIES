#ifndef BLOB_CONVERTER_HPP
#define BLOB_CONVERTER_HPP

#include <cstring>
#include <vector>

#include "types.hpp"

// TODO: Consider adding compression/decompression
class BlobConverter {
 public:
  static std::vector<char> to_blob(const std::vector<gp_float>& vec) {
    std::vector<char> blob(vec.size() * sizeof(gp_float));
    std::memcpy(blob.data(), vec.data(), blob.size());
    return blob;
  }

  static std::vector<gp_float> from_blob(const std::vector<char>& blob) {
    std::vector<gp_float> vec(blob.size() / sizeof(gp_float));
    std::memcpy(vec.data(), blob.data(), blob.size());
    return vec;
  }
};

#endif  // BLOB_CONVERTER_HPP
