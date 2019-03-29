/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "index_pack.h"
#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "gtest/gtest.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {
namespace {

/// \brief Stores files as an in-memory map for testing.
class InMemoryIndexPackFilesystem : public IndexPackFilesystem {
 public:
  bool AddFileContent(DataKind data_kind, WriteCallback callback,
                      std::string* error_text) override {
    google::protobuf::string file_data;
    std::string file_name;
    {
      google::protobuf::io::StringOutputStream stream(&file_data);
      auto result = callback(&stream, &file_name, error_text);
      if (!result) {
        return result;
      }
    }
    files_[data_kind].emplace(file_name, file_data);
    return true;
  }

  bool ReadFileContent(DataKind data_kind, const std::string& file_name,
                       ReadCallback callback,
                       std::string* error_text) override {
    auto record = files_[data_kind].find(file_name);
    if (record == files_[data_kind].end()) {
      *error_text = "file not found";
      return false;
    }
    // Use a weird block size to shake out errors.
    google::protobuf::io::ArrayInputStream stream(record->second.data(),
                                                  record->second.size(), 3);
    return callback(&stream, error_text);
  }

  bool ScanFiles(DataKind data_kind, ScanCallback callback,
                 std::string* error_text) override {
    for (auto pair : files_[data_kind]) {
      if (!callback(pair.first)) {
        break;
      }
    }
    return true;
  }

  OpenMode open_mode() const override { return OpenMode::kReadWrite; }

  /// Maps from data kinds to (maps from hashes to file content).
  std::map<DataKind, std::map<std::string, std::string>> files_;
};

TEST(IndexPack, AddCompilationUnit) {
  auto filesystem = absl::make_unique<InMemoryIndexPackFilesystem>();
  IndexPack pack(std::move(filesystem));
  kythe::proto::CompilationUnit unit;
  std::string error_text;
  EXPECT_TRUE(pack.AddCompilationUnit(unit, &error_text));
  EXPECT_TRUE(pack.AddCompilationUnit(unit, &error_text));
}

TEST(IndexPack, AddFileData) {
  auto filesystem = absl::make_unique<InMemoryIndexPackFilesystem>();
  IndexPack pack(std::move(filesystem));
  kythe::proto::FileData file_data;
  file_data.set_content("test");
  std::string error_text;
  EXPECT_TRUE(pack.AddFileData(file_data, &error_text));
  EXPECT_TRUE(pack.AddFileData(file_data, &error_text));
}

TEST(IndexPack, ScanData) {
  auto filesystem = absl::make_unique<InMemoryIndexPackFilesystem>();
  IndexPack pack(std::move(filesystem));
  std::string error_text;
  kythe::proto::FileData file_data;
  file_data.set_content("data1");
  kythe::proto::CompilationUnit unit_a;
  unit_a.set_output_key("unit_a");
  kythe::proto::CompilationUnit unit_b;
  unit_b.set_output_key("unit_b");
  ASSERT_TRUE(pack.AddCompilationUnit(unit_a, &error_text));
  ASSERT_TRUE(pack.AddCompilationUnit(unit_b, &error_text));
  ASSERT_TRUE(pack.AddFileData(file_data, &error_text));
  ASSERT_TRUE(pack.AddFileData(file_data, &error_text));
  bool file_matched = false, file_had_errors = false, multiple_files = false;
  EXPECT_TRUE(pack.ScanData(
      IndexPackFilesystem::DataKind::kFileData,
      [&pack, &file_matched, &multiple_files,
       &file_had_errors](const std::string& data_hash) {
        if (file_matched) {
          multiple_files = true;
        } else {
          std::string file_content;
          if (pack.ReadFileData(data_hash, &file_content) &&
              file_content == "data1") {
            file_matched = true;
          } else {
            file_had_errors = true;
          }
        }
        return true;
      },
      &error_text));
  EXPECT_TRUE(file_matched);
  EXPECT_FALSE(multiple_files);
  EXPECT_FALSE(file_had_errors);

  bool unit_a_ok = false;
  bool unit_b_ok = false;
  bool unit_errors = false;
  EXPECT_TRUE(pack.ScanData(
      IndexPackFilesystem::DataKind::kCompilationUnit,
      [&pack, &unit_a_ok, &unit_b_ok,
       &unit_errors](const std::string& unit_hash) {
        kythe::proto::CompilationUnit unit;
        std::string error_text;
        if (!pack.ReadCompilationUnit(unit_hash, &unit, &error_text) ||
            !error_text.empty() || unit.output_key().empty()) {
          unit_errors = true;
        } else if (unit.output_key() == "unit_a" && !unit_a_ok) {
          unit_a_ok = true;
        } else if (unit.output_key() == "unit_b" && !unit_b_ok) {
          unit_b_ok = true;
        } else {
          unit_errors = true;
        }
        return true;
      },
      &error_text));
  EXPECT_TRUE(unit_a_ok);
  EXPECT_TRUE(unit_b_ok);
  EXPECT_FALSE(unit_errors);
}

TEST(IndexPack, ReadFileData) {
  auto filesystem = absl::make_unique<InMemoryIndexPackFilesystem>();
  filesystem->files_[IndexPackFilesystem::DataKind::kFileData]["fakehash"] =
      "test";
  IndexPack pack(std::move(filesystem));
  kythe::proto::FileData file_data;
  std::string file_content;
  EXPECT_TRUE(pack.ReadFileData("fakehash", &file_content));
  EXPECT_EQ("test", file_content);
  file_content.clear();
  EXPECT_FALSE(pack.ReadFileData("notafile", &file_content));
  EXPECT_FALSE(file_content.empty());
}

TEST(IndexPack, ReadCompilationUnit) {
  auto filesystem = absl::make_unique<InMemoryIndexPackFilesystem>();
  filesystem
      ->files_[IndexPackFilesystem::DataKind::kCompilationUnit]["fakehash"] =
      "{\"format\":\"kythe\",\"content\":{\"output_key\":\"test\"}}";
  IndexPack pack(std::move(filesystem));
  kythe::proto::CompilationUnit unit;
  std::string error_text;
  EXPECT_TRUE(pack.ReadCompilationUnit("fakehash", &unit, &error_text));
  EXPECT_TRUE(error_text.empty());
  EXPECT_EQ("test", unit.output_key());
  EXPECT_FALSE(pack.ReadCompilationUnit("notafile", &unit, &error_text));
  EXPECT_FALSE(error_text.empty());
}

TEST(IndexPack, ReadCompilationUnitBadVersion) {
  auto filesystem = absl::make_unique<InMemoryIndexPackFilesystem>();
  filesystem
      ->files_[IndexPackFilesystem::DataKind::kCompilationUnit]["fakehash"] =
      "{\"format\":\"wrong\",\"content\":{\"output_key\":\"test\"}}";
  IndexPack pack(std::move(filesystem));
  kythe::proto::CompilationUnit unit;
  std::string error_text;
  EXPECT_FALSE(pack.ReadCompilationUnit("fakehash", &unit, &error_text));
  EXPECT_FALSE(error_text.empty());
}

// Some arbitrary but well-formed sha values.
static const char kData1Sha[] =
    "5b41362bc82b7f3d56edc5a306db22105707d01ff4819e26faef9724a2d406c9";
static const char kData2Sha[] =
    "d98cf53e0c8b77c14a96358d5b69584225b4bb9026423cbc2f7b0161894c402c";
static const char kUnit1Sha[] =
    "a7f4909446e1cc8286ceae47afe6ccb698af99769be9c5b4a7fa0aa7cc37667f";
static const char kUnit2Sha[] =
    "0efab319cc113a698ba482dd00dab8c25ee5537c2f974dc8cdc1c2fffdad46c7";

/// \brief Handles the creation and destruction of temporary files and
/// directories.
class TemporaryFilesystem {
 public:
  TemporaryFilesystem() {
    CHECK_EQ(
        0,
        llvm::sys::fs::createUniqueDirectory("index_pack_test", root_).value());
    directories_to_remove_.insert(root_.str());
  }

  ~TemporaryFilesystem() { Cleanup(); }

  /// \brief Attempts to clean up test directories.
  bool Cleanup() {
    // Do the best we can to clean up the temporary files we've made.
    std::error_code err;
    for (const auto& file : files_to_remove_) {
      err = llvm::sys::fs::remove(file);
      if (err.value()) {
        LOG(WARNING) << "Couldn't remove " << file << ": " << err.message();
      }
    }
    files_to_remove_.clear();
    // ::remove refuses to remove non-empty directories and we'd like to avoid
    // recursive traversal out of fear of being bitten by symlinks.
    bool progress = true;
    while (progress) {
      progress = false;
      for (const auto& dir : directories_to_remove_) {
        auto result = llvm::sys::fs::remove(llvm::Twine(dir));
        if (!result) {
          directories_to_remove_.erase(dir);
          progress = true;
          break;
        }
      }
    }
    return directories_to_remove_.empty();
  }

  /// \brief Attempts to then returns whether the directory at
  /// `relative_path` was removed. Fails if the directory is non-empty.
  bool RemoveDirectoryIfExists(const std::string& relative_path) {
    llvm::SmallString<512> full_path(root_);
    llvm::sys::path::append(full_path, relative_path);
    return !llvm::sys::fs::remove(llvm::Twine(full_path));
  }

  /// \brief Attempts to then returns whether the file file_name in directory
  /// file_dir was removed.
  bool RemoveFileIfExists(const std::string& file_dir,
                          const std::string& file_name) {
    llvm::SmallString<512> full_path(root_);
    llvm::sys::path::append(full_path, file_dir, file_name);
    return !llvm::sys::fs::remove(llvm::Twine(full_path), false);
  }

  /// \brief Creates a temporary directory that will be removed on exit.
  bool CreateDirectory(const std::string& relative_path) {
    llvm::SmallString<512> full_path(root_);
    llvm::sys::path::append(full_path, relative_path);
    if (!llvm::sys::fs::create_directory(llvm::Twine(full_path), true)) {
      directories_to_remove_.insert(full_path.str());
      return true;
    }
    return false;
  }

  /// \brief Read all data from `stream` into `content`.
  /// \return true on success; false on failure
  static bool ReadFromStream(google::protobuf::io::ZeroCopyInputStream* stream,
                             std::string* content) {
    content->clear();
    const void* data;
    int size;
    while (stream->Next(&data, &size)) {
      if (size <= 0) {
        continue;
      }
      content->append(static_cast<const char*>(data), size);
    }
    return true;
  }

  /// \brief Write all data from `content` to `stream`.
  /// \return true on success; false on failure.
  static bool WriteToStream(
      const std::string& content,
      google::protobuf::io::ZeroCopyOutputStream* stream) {
    size_t write_pos = 0;
    while (write_pos < content.size()) {
      void* buffer;
      int buffer_size;
      if (!stream->Next(&buffer, &buffer_size)) {
        return false;
      }
      if (buffer_size <= 0) {
        continue;
      }
      size_t to_write =
          std::min<size_t>(content.size() - write_pos, buffer_size);
      ::memcpy(buffer, content.data() + write_pos, to_write);
      write_pos += buffer_size;
    }
    if (!content.empty() && write_pos > content.size()) {
      stream->BackUp(write_pos - content.size());
    }
    return true;
  }

  /// \brief Writes a gzipped file that will be removed on exit.
  bool CreateFile(const std::string& file_dir, const std::string& file_name,
                  const std::string& content) {
    llvm::SmallString<512> full_path(root_);
    llvm::sys::path::append(full_path, file_dir, file_name);
    int write_fd;
    if (llvm::sys::fs::openFileForWrite(llvm::Twine(full_path), write_fd,
                                        llvm::sys::fs::CD_CreateAlways,
                                        llvm::sys::fs::OF_Text))
      return false;
    files_to_remove_.insert(full_path.str());
    google::protobuf::io::FileOutputStream file_stream(write_fd);
    google::protobuf::io::GzipOutputStream::Options options;
    options.format = google::protobuf::io::GzipOutputStream::GZIP;
    google::protobuf::io::GzipOutputStream stream(&file_stream, options);
    return WriteToStream(content, &stream) && stream.Close() &&
           file_stream.Close();
  }

  /// \brief Returns the root of the temporary filesystem.
  std::string root() const { return root_.str(); }

  /// \brief Create a simple index pack.
  /// \return true on success; false on failure
  bool MakeDefault() {
    std::string data_dir = "files";
    std::string unit_dir = "units";
    return CreateDirectory(data_dir) && CreateDirectory(unit_dir) &&
           CreateFile(data_dir, std::string(kData1Sha) + ".data", "data1") &&
           CreateFile(data_dir, std::string(kData2Sha) + ".data", "data2") &&
           CreateFile(data_dir, "sometemp.new", "temp") &&
           CreateFile(unit_dir, std::string(kUnit1Sha) + ".unit", "unit1") &&
           CreateFile(unit_dir, std::string(kUnit2Sha) + ".unit", "unit2");
  }

 private:
  /// Path to a directory for test files.
  llvm::SmallString<256> root_;
  /// Files to clean up after the test ends.
  std::set<std::string> files_to_remove_;
  /// Directories to clean up after the test ends.
  std::set<std::string> directories_to_remove_;
};

TEST(IndexPack, PosixOpenNew) {
  TemporaryFilesystem files;
  std::string error_text;
  auto posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadWrite, &error_text);
  EXPECT_NE(nullptr, posix);
  EXPECT_TRUE(error_text.empty());
  EXPECT_TRUE(files.RemoveDirectoryIfExists("units"));
  EXPECT_TRUE(files.RemoveDirectoryIfExists("files"));
  EXPECT_TRUE(files.Cleanup());
}

TEST(IndexPack, PosixOpenBad) {
  TemporaryFilesystem files;
  std::string error_text;
  auto posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadOnly, &error_text);
  EXPECT_EQ(nullptr, posix);
  EXPECT_FALSE(error_text.empty());
  EXPECT_TRUE(files.Cleanup());
}

TEST(IndexPack, PosixOpenExisting) {
  TemporaryFilesystem files;
  ASSERT_TRUE(files.MakeDefault());
  std::string error_text;
  auto posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadOnly, &error_text);
  EXPECT_NE(nullptr, posix);
  EXPECT_TRUE(error_text.empty());
  EXPECT_TRUE(files.Cleanup());
}

TEST(IndexPack, PosixScanFiles) {
  TemporaryFilesystem files;
  ASSERT_TRUE(files.MakeDefault());
  std::string error_text;
  auto posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadOnly, &error_text);
  ASSERT_NE(nullptr, posix);
  ASSERT_TRUE(error_text.empty());
  std::vector<std::string> scanned_files;
  EXPECT_TRUE(posix->ScanFiles(
      IndexPackFilesystem::DataKind::kFileData,
      [&scanned_files](const std::string& file_name) {
        scanned_files.push_back(file_name);
        return true;
      },
      &error_text));
  std::set<std::string> expect_scan_files = {kData1Sha, kData2Sha};
  EXPECT_EQ(scanned_files.size(), expect_scan_files.size());
  EXPECT_TRUE(error_text.empty());
  for (const auto& file : expect_scan_files) {
    EXPECT_EQ(1, std::count(scanned_files.begin(), scanned_files.end(), file))
        << "When looking for " << file;
  }
  scanned_files.clear();
  EXPECT_TRUE(posix->ScanFiles(
      IndexPackFilesystem::DataKind::kCompilationUnit,
      [&scanned_files](const std::string& file_name) {
        scanned_files.push_back(file_name);
        return true;
      },
      &error_text));
  std::set<std::string> expect_scan_units = {kUnit1Sha, kUnit2Sha};
  EXPECT_EQ(scanned_files.size(), expect_scan_units.size());
  EXPECT_TRUE(error_text.empty());
  for (const auto& file : expect_scan_units) {
    EXPECT_EQ(1, std::count(scanned_files.begin(), scanned_files.end(), file))
        << "When looking for " << file;
  }
  EXPECT_TRUE(files.Cleanup());
}

TEST(IndexPack, PosixReadContent) {
  TemporaryFilesystem files;
  ASSERT_TRUE(files.MakeDefault());
  std::string error_text;
  auto posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadOnly, &error_text);
  ASSERT_NE(nullptr, posix);
  ASSERT_TRUE(error_text.empty());
  struct ExpectedContent {
    IndexPackFilesystem::DataKind kind;
    std::string filename;
    std::string content;
  };
  std::vector<ExpectedContent> test_data = {
      {IndexPackFilesystem::DataKind::kFileData, kData1Sha, "data1"},
      {IndexPackFilesystem::DataKind::kCompilationUnit, kUnit1Sha, "unit1"}};
  for (const auto& test : test_data) {
    std::string content;
    error_text.clear();
    EXPECT_TRUE(posix->ReadFileContent(
        test.kind, test.filename,
        [&content](google::protobuf::io::ZeroCopyInputStream* stream,
                   std::string* error_text) {
          return TemporaryFilesystem::ReadFromStream(stream, &content);
        },
        &error_text))
        << "Bad read of " << test.filename << ": " << error_text;
    EXPECT_TRUE(error_text.empty());
    EXPECT_TRUE(test.content == content);
  }
  EXPECT_TRUE(files.Cleanup());
}

TEST(IndexPack, PosixAddContent) {
  TemporaryFilesystem files;
  std::string error_text;
  auto posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadWrite, &error_text);
  ASSERT_NE(nullptr, posix);
  ASSERT_TRUE(error_text.empty());

  auto Insert = [&posix, &error_text](IndexPackFilesystem::DataKind kind,
                                      const std::string& hash,
                                      const std::string& content) {
    return posix->AddFileContent(
        kind,
        [&hash, &content](google::protobuf::io::ZeroCopyOutputStream* stream,
                          std::string* file_name, std::string* error_text) {
          *file_name = hash;
          return TemporaryFilesystem::WriteToStream(content, stream);
        },
        &error_text);
  };

  EXPECT_TRUE(
      Insert(IndexPackFilesystem::DataKind::kFileData, kData1Sha, "data1"));
  // NB: We check that data2 is overwriting data1 in kData1Sha.
  EXPECT_TRUE(
      Insert(IndexPackFilesystem::DataKind::kFileData, kData1Sha, "data2"));
  EXPECT_TRUE(Insert(IndexPackFilesystem::DataKind::kCompilationUnit, kData1Sha,
                     "unit1"));

  auto second_posix = IndexPackPosixFilesystem::Open(
      files.root(), IndexPackFilesystem::OpenMode::kReadOnly, &error_text);
  ASSERT_NE(nullptr, posix);
  ASSERT_TRUE(error_text.empty());
  std::string content;

  EXPECT_TRUE(second_posix->ReadFileContent(
      IndexPackFilesystem::DataKind::kFileData, kData1Sha,
      [&content](google::protobuf::io::ZeroCopyInputStream* stream,
                 std::string* error_text) {
        return TemporaryFilesystem::ReadFromStream(stream, &content);
      },
      &error_text));
  EXPECT_EQ("data2", content);

  EXPECT_TRUE(second_posix->ReadFileContent(
      IndexPackFilesystem::DataKind::kCompilationUnit, kData1Sha,
      [&content](google::protobuf::io::ZeroCopyInputStream* stream,
                 std::string* error_text) {
        return TemporaryFilesystem::ReadFromStream(stream, &content);
      },
      &error_text));
  EXPECT_EQ("unit1", content);

  // Check that we've made files with the right names.
  EXPECT_TRUE(
      files.RemoveFileIfExists("files", std::string(kData1Sha) + ".data"));
  EXPECT_TRUE(
      files.RemoveFileIfExists("units", std::string(kData1Sha) + ".unit"));
  // Check that we've removed temporary files. (RemoveDirectoryIfExists fails
  // if the directories are non-empty.)
  EXPECT_TRUE(files.RemoveDirectoryIfExists("units"));
  EXPECT_TRUE(files.RemoveDirectoryIfExists("files"));
  EXPECT_TRUE(files.Cleanup());
}

}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
