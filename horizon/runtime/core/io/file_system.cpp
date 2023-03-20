/* Copyright (c) 2019-2020, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file_system.h"

#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb_image_write.h>

#include "runtime/core/log/log.h"

namespace Horizon::fs {

namespace path {

static std::filesystem::path external_storage_directory;

static std::filesystem::path temp_directory;

static const std::filesystem::path &get_external_storage_directory() { return external_storage_directory; }

static const std::filesystem::path &get_temp_directory() { return temp_directory; }

const Container::HashMap<Type, std::filesystem::path> relative_paths = {
    {Type::Assets, "assets/"},    {Type::Shaders, "shaders/"},
    {Type::Storage, "output/"},   {Type::Screenshots, "output/images/"},
    {Type::Logs, "output/logs/"}, {Type::Graphs, "output/graphs/"}};

const std::filesystem::path get(const Type type, const std::filesystem::path &file) {
    assert(relative_paths.size() == Type::TotalRelativePathTypes &&
           "Not all paths are defined in filesystem, please check that each enum is specified");

    // Check for special cases first
    if (type == Type::WorkingDir) {
        return get_external_storage_directory();
    } else if (type == Type::Temp) {
        return get_temp_directory();
    }

    // Check for relative paths
    auto it = relative_paths.find(type);

    // FIXME(hylu): remove exception
    if (relative_paths.size() < Type::TotalRelativePathTypes) {
        throw std::runtime_error("Platform hasn't initialized the paths correctly");
    } else if (it == relative_paths.end()) {
        throw std::runtime_error("Path enum doesn't exist, or wasn't specified in the path map");
    } else if (it->second.empty()) {
        throw std::runtime_error("Path was found, but it is empty");
    }

    auto path = get_external_storage_directory() / it->second;

    //if (fs::is_directory(path)!=false) {
    //    create_path(get_external_storage_directory(), it->second);
    //}

    return path / file;
}

} // namespace path

bool is_directory(const std::filesystem::path &path) { return std::filesystem::is_directory(path); }

void create_directory(const std::filesystem::path &path) {
    if (fs::is_directory(path) != false) {
        std::filesystem::create_directory(path);
    }
}

bool is_file(const std::filesystem::path &filename) { return std::filesystem::is_regular_file(filename); }

//void create_path(const std::filesystem::path &root, const std::filesystem::path &path) {
//    auto str = path.string();
//    for (auto it = str.begin(); it != str.end(); ++it) {
//        it = std::find(it, path.string().end(), '/');
//        auto dir_path = (str.begin(), it);
//        std::filesystem::path out_path = root / dir_path;
//        fs::create_directory(out_path);
//    }
//}

Container::String read_text_file(const std::filesystem::path filename) {
    Container::Array<Container::String> data;

    std::ifstream file;

    file.open(filename, std::ios::in);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file : {}", filename.string());
        return {};
    }

    return Container::String{(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())};
}

Container::Array<u8> read_binary_file(const std::filesystem::path &filename, const uint32_t count) {
    Container::Array<u8> data;

    std::ifstream file;

    file.open(filename, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file : {}", filename.string());
        return {};
    }

    uint64_t read_count = count;
    if (count == 0) {
        file.seekg(0, std::ios::end);
        read_count = static_cast<uint64_t>(file.tellg());
        file.seekg(0, std::ios::beg);
    }

    data.resize(static_cast<size_t>(read_count));
    file.read(reinterpret_cast<char *>(data.data()), read_count);
    file.close();

    return data;
}

void write_text_file(const std::filesystem::path filename, void *data, u64 size) {
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file : {}", filename.string());
        return;
    }

    file.write(reinterpret_cast<const char *>(data), size);
    file.close();
}

static void write_binary_file(const Container::Array<u8> &data, const std::filesystem::path &filename,
                              const uint32_t count) {
    std::ofstream file;

    file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file : {}", filename.string());
        return;
    }

    uint64_t write_count = count;
    if (count == 0) {
        write_count = data.size();
    }

    file.write(reinterpret_cast<const char *>(data.data()), write_count);
    file.close();
}

Container::Array<u8> read_asset(const std::filesystem::path &filename, const uint32_t count) {
    return read_binary_file(path::get(path::Type::Assets) / filename, count);
}

Container::String read_shader(const std::filesystem::path &filename) {
    return read_text_file(path::get(path::Type::Shaders) / filename);
}

Container::Array<u8> read_shader_binary(const std::filesystem::path &filename) {
    return read_binary_file(path::get(path::Type::Shaders) / filename, 0);
}

Container::Array<u8> read_temp(const std::filesystem::path &filename, const uint32_t count) {
    return read_binary_file(path::get(path::Type::Temp) / filename, count);
}

void write_temp(const Container::Array<u8> &data, const std::filesystem::path &filename, const uint32_t count) {
    write_binary_file(data, path::get(path::Type::Temp) / filename, count);
}

//void write_image(const u8 *data, const std::filesystem::path &filename, const uint32_t width, const uint32_t height,
//                 const uint32_t components, const uint32_t row_stride) {
//    stbi_write_png((path::get(path::Type::Screenshots) + filename + ".png").c_str(), width, height, components, data,
//                   row_stride);
//}

bool write_json(nlohmann::json &data, const std::filesystem::path &filename) {
    std::stringstream json;

    try {
        // Whitespace needed as last character is overwritten on android causing the json to be corrupt
        json << data << " ";
    } catch (std::exception &e) {
        // JSON dump errors
        LOG_ERROR(e.what());
        return false;
    }

    if (!nlohmann::json::accept(json.str())) {
        LOG_ERROR("Invalid JSON string");
        return false;
    }

    std::ofstream out_stream;
    out_stream.open(fs::path::get(fs::path::Type::Graphs) / filename, std::ios::out | std::ios::trunc);

    if (out_stream.good()) {
        out_stream << json.str();
    } else {
        LOG_ERROR("Could not load JSON file {}", filename.string());
        return false;
    }

    out_stream.close();
    return true;
}

} // namespace Horizon::fs