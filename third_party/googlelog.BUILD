package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD

filegroup(
    name = "license",
    srcs = ["COPYING"],
)

cc_library(
    name = "glog",
    srcs = [
        "src/base/commandlineflags.h",
        "src/base/googleinit.h",
        "src/base/mutex.h",
        "src/demangle.cc",
        "src/demangle.h",
        "src/googletest.h",
        "src/logging.cc",
        "src/raw_logging.cc",
        "src/signalhandler.cc",
        "src/stacktrace.h",
        "src/stacktrace_generic-inl.h",
        "src/stacktrace_libunwind-inl.h",
        "src/stacktrace_powerpc-inl.h",
        "src/stacktrace_x86-inl.h",
        "src/stacktrace_x86_64-inl.h",
        "src/symbolize.cc",
        "src/symbolize.h",
        "src/utilities.cc",
        "src/utilities.h",
        "src/vlog_is_on.cc",
    ],
    hdrs = [
        "src/glog/log_severity.h",
        ":headers",
    ],
    copts = [
        "-Ithird_party/googlelog/src",
    ],
    includes = [
        "include",
    ],
    deps = [
        "@//third_party/googlelog:config_h",
        "@com_github_gflags_gflags//:gflags",
    ],
)

genrule(
    name = "headers",
    srcs = [
        "src/glog/vlog_is_on.h.in",
        "src/glog/raw_logging.h.in",
        "src/glog/stl_logging.h.in",
        "src/glog/logging.h.in",
        "src/glog/log_severity.h",
    ],
    outs = [
        "include/glog/vlog_is_on.h",
        "include/glog/raw_logging.h",
        "include/glog/stl_logging.h",
        "include/glog/logging.h",
        "include/glog/log_severity.h",
    ],
    cmd = """
    declare -A ac_subst
    ac_subst=(
      [ac_cv___attribute___noinline]="__attribute__ ((noinline))"
      [ac_cv___attribute___noreturn]="__attribute__ ((noreturn))"
      [ac_cv___attribute___printf_4_5]="__attribute__((__format__ (__printf__, 4, 5)))"
      [ac_cv_cxx_using_operator]=1
      [ac_cv_have___builtin_expect]=1
      [ac_cv_have_inttypes_h]=1
      [ac_cv_have_libgflags]=1
      [ac_cv_have_stdint_h]=1
      [ac_cv_have_systypes_h]=1
      [ac_cv_have___uint16]=0
      [ac_cv_have_u_int16_t]=0
      [ac_cv_have_uint16_t]=1
      [ac_cv_have_unistd_h]=1
      [ac_google_end_namespace]="}"
      [ac_google_namespace]="google"
      [ac_google_start_namespace]="namespace google {"
    )
    replace() {
      local script="$$(
        for key in "$${!ac_subst[@]}"; do
          echo -n "s|@$$key@|$${ac_subst[$$key]}|g;"
        done
      )"
      sed -e "$$script" "$$1" > "$$2"
    }
    replace $(location src/glog/vlog_is_on.h.in) $(location include/glog/vlog_is_on.h)
    replace $(location src/glog/raw_logging.h.in) $(location include/glog/raw_logging.h)
    replace $(location src/glog/stl_logging.h.in) $(location include/glog/stl_logging.h)
    replace $(location src/glog/logging.h.in) $(location include/glog/logging.h)
    mv $(location src/glog/log_severity.h) $(location include/glog/log_severity.h)
    """,
    visibility = ["//visibility:private"],
)
