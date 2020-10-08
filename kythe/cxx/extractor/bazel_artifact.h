#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_H_

#include <string>
#include <vector>

namespace kythe {

/// \brief A pair of local path and canonical URI for a given Bazel output file.
/// Bazel typically provides both a workspace-local path to a file as well as a
/// canonical URI and both can be useful to clients.
struct BazelArtifactFile {
  std::string local_path;  ///< The workspace-relative path to the file.
  std::string uri;  ///< The canonical URI for the file. Clients should be able
                    ///< to handle at least `file:` and `data:` schemes.
};

/// \brief A list of extracted compilation units and the target which owns them.
struct BazelArtifact {
  std::string label;  ///< The target from which these artifacts originate.
  std::vector<BazelArtifactFile>
      files;  ///< A list of paths and URIs at which the artifacts can be found.
};

}  // namespace kythe
#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_H_
