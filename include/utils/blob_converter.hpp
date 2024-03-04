#ifndef BLOB_CONVERTER_HPP
#define BLOB_CONVERTER_HPP

#include <cstring>
#include <sstream>
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

    static std::string to_csv(const std::vector<gp_float>& vec) {
        std::stringstream ss;
        size_t len = vec.size();
        for (size_t i = 0; i < len - 1; ++i) {
            ss << vec[i] << ',';
        }
        ss << vec[len - 1];

        return ss.str();
    }

    static std::vector<gp_float> from_csv(const std::string& csv) {
        std::stringstream ss(csv);
        std::vector<gp_float> vec;

        while (ss.good()) {
            std::string str;
            getline(ss, str, ',');
            vec.push_back((gp_float)std::stod(str));
        }

        return vec;
    }
};

#endif  // BLOB_CONVERTER_HPP
