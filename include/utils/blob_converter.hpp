#ifndef BLOB_CONVERTER_HPP
#define BLOB_CONVERTER_HPP

#include <string>
#include <sstream>
#include <vector>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <boost/iostreams/filtering_streambuf.hpp>
#pragma GCC diagnostic pop
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

#include "types.hpp"


class BlobConverter {
 public:
  static std::vector<char> to_blob(const std::vector <gp_float> &vec) {
    if (vec.empty())
      return {};

    std::stringstream data;
    data.write(reinterpret_cast<const char *>(vec.data()),
               vec.size() * sizeof(gp_float));

    std::stringstream data_comp;
    boost::iostreams::filtering_streambuf <boost::iostreams::output> out;
    out.push(boost::iostreams::bzip2_compressor());
    out.push(data_comp);
    boost::iostreams::copy(data, out); // NOLINT(build/include_what_you_use)

    std::string str_comp = data_comp.str();
    std::vector<char> blob(str_comp.begin(), str_comp.end());
    return blob;
  }

  static std::vector <gp_float> from_blob(const std::vector<char> &blob) {
    if (blob.empty())
      return {};

    std::string str_comp(blob.begin(), blob.end());
    std::stringstream data_comp(str_comp);

    std::stringstream data;
    boost::iostreams::filtering_streambuf <boost::iostreams::input> in;
    in.push(boost::iostreams::bzip2_decompressor());
    in.push(data_comp);
    boost::iostreams::copy(in, data); // NOLINT(build/include_what_you_use)

    std::vector <gp_float> vec(data.str().size() / sizeof(gp_float));
    data.read(reinterpret_cast<char *>(vec.data()), data.str().size());
    return vec;
  }
};

#endif  // BLOB_CONVERTER_HPP
