# This file exists so that visibility rules can be relaxed for open source
# clients of kythe, but more restrictive for use within google. Targets that are
# allowed to be used by kythe clients should be marked as "//visibility:public",
# but separate aliases are used so they can map to different package groups
# within google.

PUBLIC_PROTO_VISIBILITY = "//visibility:public"

PUBLIC_VISIBILITY = "//visibility:public"
