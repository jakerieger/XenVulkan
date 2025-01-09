// Author: Jake Rieger
// Created: 12/12/24.
//

#pragma once

#include "Types.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <future>
#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd
    #define PATH_SEPARATOR '\\'
#else
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
#endif

namespace x {
    namespace Filesystem {
        class FileReader {
        public:
            static std::vector<u8> ReadAllBytes(const str& path);
            static str ReadAllText(const str& path);
            static std::vector<str> ReadAllLines(const str& path);
            static std::vector<u8> ReadBlock(const str& path, size_t size, u64 offset = 0);
            static size_t QueryFileSize(const str& path);
        };

        class FileWriter {
        public:
            static bool WriteAllBytes(const str& path, const std::vector<u8>& data);
            static bool WriteAllText(const str& path, const str& text);
            static bool WriteAllLines(const str& path, const std::vector<str>& lines);
            static bool WriteBlock(const str& path, const std::vector<u8>& data, u64 offset = 0);
        };

        class AsyncFileReader {
        public:
            static std::future<std::vector<u8>> ReadAllBytes(const str& path);
            static std::future<str> ReadAllText(const str& path);
            static std::future<std::vector<str>> ReadAllLines(const str& path);
            static std::future<std::vector<u8>>
            ReadBlock(const str& path, size_t size, u64 offset = 0);

        private:
            template<typename Func>
            static auto runAsync(Func&& func) -> std::future<decltype(func())> {
                using ReturnType = decltype(func());
                auto task =
                  std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
                std::future<ReturnType> future = task->get_future();
                std::thread([task]() { (*task)(); }).detach();
                return future;
            }
        };

        class AsyncFileWriter {
        public:
            static std::future<bool> WriteAllBytes(const str& path, const std::vector<u8>& data);
            static std::future<bool> WriteAllText(const str& path, const str& text);
            static std::future<bool> WriteAllLines(const str& path, const std::vector<str>& lines);
            static std::future<bool>
            WriteBlock(const str& path, const std::vector<u8>& data, u64 offset = 0);

        private:
            template<typename Func>
            static auto runAsync(Func&& func) -> std::future<decltype(func())> {
                using ReturnType = decltype(func());
                auto task =
                  std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
                std::future<ReturnType> future = task->get_future();
                std::thread([task]() { (*task)(); }).detach();
                return future;
            }
        };

        // class StreamReader {};
        //
        // class StreamWriter {};

        class Path {
        public:
            explicit Path(const str& path) : path(Normalize(path)) {}
            static Path Current();

            [[nodiscard]] Path Parent() const;
            [[nodiscard]] bool Exists() const;
            [[nodiscard]] bool IsFile() const;
            [[nodiscard]] bool IsDirectory() const;
            [[nodiscard]] bool HasExtension() const;
            [[nodiscard]] str Extension() const;
            [[nodiscard]] Path ReplaceExtension(const str& ext) const;
            [[nodiscard]] Path Join(const str& subPath) const;
            [[nodiscard]] Path operator/(const str& subPath) const;
            [[nodiscard]] str Str() const;
            [[nodiscard]] const char* CStr() const;
            [[nodiscard]] bool operator==(const Path& other) const;

            bool Create() const;
            bool CreateAll() const;

        private:
            str path;
            static str Join(const str& lhs, const str& rhs);
            static str Normalize(const str& rawPath);
        };
    };  // namespace Filesystem
}  // namespace x
