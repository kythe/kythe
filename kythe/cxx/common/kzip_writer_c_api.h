/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CPP_COMMON_KZIP_WRITER_C_API_H_
#define KYTHE_CPP_COMMON_KZIP_WRITER_C_API_H_

/// /brief This is a C API wrapper for kythe::KZipWriter.
///
/// Used for linking to non-C languages such as Rust.  This is a partial API,
/// which is enough to write out basic files and compilation units.

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define KZIP_WRITER_ENCODING_JSON (1)
#define KZIP_WRITER_ENCODING_PROTO (2)
#define KZIP_WRITER_ENCODING_ALL \
  (KZIP_WRITER_ENCODING_JSON | KZIP_WRITER_ENCODING_PROTO)

#define KZIP_WRITER_BUFFER_TOO_SMALL_ERROR (-1)
#define KZIP_WRITER_PROTO_PARSING_ERROR (-2)

/// \brief The opaque kzip writer object.
struct KzipWriter;

/// \brief Creates a new kzip writer.
/// \param path The path to the file to create.
/// \param path_len the string length of path, required due to the C API.
/// \param encoding The compilation unit encoding, either
///        KZIP_WRITER_ENCODING_JSON, or KZIP_WRITER_ENCODING_PROTO
/// \param create_status zero if created fine, nonzero in case of an error.
///        Positive error values are the same as in absl::Status.  Negative
///        error values are either KZIP_WRITER_BUFFER_TOO_SMALL_ERROR, or
///        KZIP_WRITER_PROTO_PARSING_ERROR.
/// Caller takes the ownership of the returned pointer.  KzipWriter_Close must
/// be called to release the memory.
struct KzipWriter* KzipWriter_Create(const char* path, const size_t path_len,
                                     int32_t encoding, int32_t* create_status);

/// \brief Deallocates KzipWriter.
///
/// Must be called once KzipWriter is no longer needed.
void KzipWriter_Delete(struct KzipWriter* writer);

/// \brief Closes the writer.
/// Must be called before destroying the object.
int32_t KzipWriter_Close(struct KzipWriter* writer);

/// \brief Writes a piece of content into the kzip, returning a digest.
///
/// The content is
/// specified using both the pointer to the beginning and the size.  The caller
/// must provide a buffer to put the digest into, and a buffer size.  Nonzero
/// status is returned in case of an error writing.  Specifically, if there is
/// insufficient space in the buffer, KZIP_WRITER_BUFFER_TOO_SMALL_ERROR is
/// returned.
/// \param writer The writer to write into.
/// \param content The file content buffer to write.
/// \param content_length The length of the buffer to write.
/// \param digest_buffer The output buffer to store the digest into.  Must be
///        large enough to hold a digest.  The buffer is NOT NUL-terminated.
/// \param buffer_length The length of digest_buffer in bytes.
int32_t KzipWriter_WriteFile(struct KzipWriter* writer, const char* content,
                             const size_t content_length, char* digest_buffer,
                             size_t buffer_length,
                             size_t* resulting_digest_size);

/// \brief Writes a compilation unit into the kzip, returning a digest.
//
/// The caller must provide a buffer to put the digest into, and a buffer size.
/// Nonzero status is returned in case of an error while writing.  The result
/// in the buffer is NOT NUL-terminated.
int32_t KzipWriter_WriteUnit(struct KzipWriter* writer, const char* proto,
                             size_t proto_length, char* digest_buffer,
                             size_t buffer_length,
                             size_t* resulting_digest_size);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // KYTHE_CPP_COMMON_KZIP_WRITER_C_API_H_
