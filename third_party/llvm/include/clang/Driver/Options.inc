/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Option Parsing Definitions                                                  *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

/////////
// Prefixes

#ifdef PREFIX
#define COMMA ,
PREFIX(prefix_0, {0})
PREFIX(prefix_1, {"-" COMMA 0})
PREFIX(prefix_4, {"-" COMMA "--" COMMA 0})
PREFIX(prefix_3, {"--" COMMA 0})
PREFIX(prefix_2, {"/" COMMA "-" COMMA 0})
#undef COMMA
#endif

/////////
// Groups

#ifdef OPTION
OPTION(0, "<action group>", Action_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<CompileOnly group>", CompileOnly_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<I group>", I_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<M group>", M_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<O group>", O_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<R group>", R_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<R (with value) group>", R_value_Group, Group, R_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<T group>", T_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<W group>", W_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<W (with value) group>", W_value_Group, Group, W_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "</M group>", _SLASH_M_Group, Group, cl_compile_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang-cl options>", cl_Group, Group, INVALID, INVALID, 0, 0, 0,
       "CL.EXE COMPATIBILITY OPTIONS", 0)
OPTION(0, "<clang-cl compile-only options>", cl_compile_Group, Group, cl_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang-cl ignored options>", cl_ignored_Group, Group, cl_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang i group>", clang_i_Group, Group, i_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang ignored f group>", clang_ignored_f_Group, Group, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang_ignored_gcc_optimization_f_Group>", clang_ignored_gcc_optimization_f_Group, Group, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang ignored m group>", clang_ignored_m_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<d group>", d_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<f group>", f_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<f (clang-only) group>", f_clang_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<g group>", g_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<g flags group>", g_flags_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "gfortran Group", gfortran_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<i group>", i_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang internal options>", internal_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang debug/development internal options>", internal_debug_Group, Group, internal_Group, INVALID, 0, 0, 0,
       "DEBUG/DEVELOPMENT OPTIONS", 0)
OPTION(0, "<clang driver internal options>", internal_driver_Group, Group, internal_Group, INVALID, 0, 0, 0,
       "DRIVER OPTIONS", 0)
OPTION(0, "<m group>", m_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<m aarch64 features group>", m_aarch64_Features_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<m arm features group>", m_arm_Features_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<m hexagon features group>", m_hexagon_Features_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<m libc group>", m_libc_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<m ppc features group>", m_ppc_Features_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<m x86 features group>", m_x86_Features_Group, Group, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<pedantic group>", pedantic_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<reserved libs group>", reserved_lib_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<u group>", u_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)

//////////
// Options

OPTION(prefix_0, "<input>", INPUT, Input, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_0, "<unknown>", UNKNOWN, Unknown, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "###", _HASH_HASH_HASH, Flag, INVALID, INVALID, 0, DriverOption | CoreOption, 0,
       "Print (but do not run) the commands to run for this compilation", 0)
OPTION(prefix_2, "?", _SLASH_QUESTION, Flag, cl_Group, help, 0, CLOption | DriverOption, 0,
       "Display available options", 0)
OPTION(prefix_1, "A-", A_DASH, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "add-plugin", add_plugin, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use the named plugin action in addition to the default action", "<name>")
OPTION(prefix_2, "AI", _SLASH_AI, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "all-warnings", _all_warnings, Flag, INVALID, Wall, 0, 0, 0, 0, 0)
OPTION(prefix_1, "all_load", all__load, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "allowable_client", allowable__client, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "analyze-auto", _analyze_auto, Flag, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "analyze-function=", analyze_function_EQ, Joined, INVALID, analyze_function, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyze-function", analyze_function, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Run analysis on specific function", 0)
OPTION(prefix_2, "analyze-", _SLASH_analyze_, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "analyzer-checker-help", analyzer_checker_help, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Display the list of analyzer checkers that are available", 0)
OPTION(prefix_1, "analyzer-checker=", analyzer_checker_EQ, Joined, INVALID, analyzer_checker, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-checker", analyzer_checker, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Choose analyzer checkers to enable", 0)
OPTION(prefix_1, "analyzer-config", analyzer_config, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Choose analyzer options to enable", 0)
OPTION(prefix_1, "analyzer-constraints=", analyzer_constraints_EQ, Joined, INVALID, analyzer_constraints, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-constraints", analyzer_constraints, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Source Code Analysis - Symbolic Constraint Engines", 0)
OPTION(prefix_1, "analyzer-disable-all-checks", analyzer_disable_all_checks, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable all static analyzer checks", 0)
OPTION(prefix_1, "analyzer-disable-checker=", analyzer_disable_checker_EQ, Joined, INVALID, analyzer_disable_checker, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-disable-checker", analyzer_disable_checker, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Choose analyzer checkers to disable", 0)
OPTION(prefix_1, "analyzer-disable-retry-exhausted", analyzer_disable_retry_exhausted, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not re-analyze paths leading to exhausted nodes with a different strategy (may decrease code coverage)", 0)
OPTION(prefix_1, "analyzer-display-progress", analyzer_display_progress, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit verbose output about the analyzer's progress", 0)
OPTION(prefix_1, "analyzer-eagerly-assume", analyzer_eagerly_assume, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Eagerly assume the truth/falseness of some symbolic constraints", 0)
OPTION(prefix_1, "analyzer-inline-max-stack-depth=", analyzer_inline_max_stack_depth_EQ, Joined, INVALID, analyzer_inline_max_stack_depth, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-inline-max-stack-depth", analyzer_inline_max_stack_depth, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Bound on stack depth while inlining (4 by default)", 0)
OPTION(prefix_1, "analyzer-inlining-mode=", analyzer_inlining_mode_EQ, Joined, INVALID, analyzer_inlining_mode, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-inlining-mode", analyzer_inlining_mode, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Specify the function selection heuristic used during inlining", 0)
OPTION(prefix_1, "analyzer-max-loop", analyzer_max_loop, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "The maximum number of times the analyzer will go through a loop", 0)
OPTION(prefix_3, "analyzer-no-default-checks", _analyzer_no_default_checks, Flag, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-opt-analyze-headers", analyzer_opt_analyze_headers, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Force the static analyzer to analyze functions defined in header files", 0)
OPTION(prefix_1, "analyzer-opt-analyze-nested-blocks", analyzer_opt_analyze_nested_blocks, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Analyze the definitions of blocks in addition to functions", 0)
OPTION(prefix_1, "analyzer-output=", analyzer_output_EQ, Joined, INVALID, analyzer_output, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_3, "analyzer-output", _analyzer_output, JoinedOrSeparate, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-output", analyzer_output, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Source Code Analysis - Output Options", 0)
OPTION(prefix_1, "analyzer-purge=", analyzer_purge_EQ, Joined, INVALID, analyzer_purge, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-purge", analyzer_purge, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Source Code Analysis - Dead Symbol Removal Frequency", 0)
OPTION(prefix_1, "analyzer-stats", analyzer_stats, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Print internal analyzer statistics.", 0)
OPTION(prefix_1, "analyzer-store=", analyzer_store_EQ, Joined, INVALID, analyzer_store, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "analyzer-store", analyzer_store, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Source Code Analysis - Abstract Memory Store Models", 0)
OPTION(prefix_1, "analyzer-viz-egraph-graphviz", analyzer_viz_egraph_graphviz, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Display exploded graph using GraphViz", 0)
OPTION(prefix_1, "analyzer-viz-egraph-ubigraph", analyzer_viz_egraph_ubigraph, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Display exploded graph using Ubigraph", 0)
OPTION(prefix_3, "analyze", _analyze, Flag, INVALID, INVALID, 0, DriverOption | CoreOption, 0,
       "Run the static analyzer", 0)
OPTION(prefix_1, "analyze", analyze, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Run static analysis engine", 0)
OPTION(prefix_4, "ansi", ansi, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "arange_sections", arange_sections, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit DWARF .debug_arange sections", 0)
OPTION(prefix_2, "arch:", _SLASH_arch, Joined, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set architecture for code generation", 0)
OPTION(prefix_1, "arch_errors_fatal", arch__errors__fatal, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "arch_only", arch__only, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "arch", arch, Separate, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "arcmt-check", arcmt_check, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Check for ARC migration issues that need manual handling", 0)
OPTION(prefix_1, "arcmt-migrate-emit-errors", arcmt_migrate_emit_arc_errors, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Emit ARC errors even if the migrator can fix them", 0)
OPTION(prefix_1, "arcmt-migrate-report-output", arcmt_migrate_report_output, Separate, INVALID, INVALID, 0, CC1Option, 0,
       "Output path for the plist report", 0)
OPTION(prefix_1, "arcmt-migrate", arcmt_migrate, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply modifications and produces temporary files that conform to ARC", 0)
OPTION(prefix_1, "arcmt-modify", arcmt_modify, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply modifications to files to conform to ARC", 0)
OPTION(prefix_3, "assemble", _assemble, Flag, INVALID, S, 0, 0, 0, 0, 0)
OPTION(prefix_3, "assert=", _assert_EQ, Joined, INVALID, A, 0, 0, 0, 0, 0)
OPTION(prefix_3, "assert", _assert, Separate, INVALID, A, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ast-dump-filter", ast_dump_filter, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use with -ast-dump or -ast-print to dump/print only AST declaration nodes having a certain substring in a qualified name. Use -ast-list to list all filterable declaration node names.", "<dump_filter>")
OPTION(prefix_1, "ast-dump-lookups", ast_dump_lookups, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs and then debug dump their name lookup tables", 0)
OPTION(prefix_1, "ast-dump", ast_dump, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs and then debug dump them", 0)
OPTION(prefix_1, "ast-list", ast_list, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs and print the list of declaration node qualified names", 0)
OPTION(prefix_1, "ast-merge", ast_merge, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Merge the given AST file into the translation unit being compiled.", "<ast file>")
OPTION(prefix_1, "ast-print", ast_print, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs and then pretty-print them", 0)
OPTION(prefix_1, "ast-view", ast_view, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs and view them with GraphViz", 0)
OPTION(prefix_1, "A", A, JoinedOrSeparate, INVALID, INVALID, 0, RenderJoined, 0, 0, 0)
OPTION(prefix_1, "a", a, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "backend-option", backend_option, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Additional arguments to forward to LLVM backend (during code gen)", 0)
OPTION(prefix_2, "bigobj", _SLASH_bigobj, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "bind_at_load", bind__at__load, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "bootclasspath=", _bootclasspath_EQ, Joined, INVALID, fbootclasspath_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "bootclasspath", _bootclasspath, Separate, INVALID, fbootclasspath_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "bundle_loader", bundle__loader, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "bundle", bundle, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "B", B, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "b", b, JoinedOrSeparate, INVALID, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "c-isystem", c_isystem, JoinedOrSeparate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add directory to the C SYSTEM include search path", "<directory>")
OPTION(prefix_1, "cc1as", cc1as, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "cc1", cc1, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "ccc-arcmt-check", ccc_arcmt_check, Flag, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Check for ARC migration issues that need manual handling", 0)
OPTION(prefix_1, "ccc-arcmt-migrate", ccc_arcmt_migrate, Separate, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Apply modifications and produces temporary files that conform to ARC", 0)
OPTION(prefix_1, "ccc-arcmt-modify", ccc_arcmt_modify, Flag, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Apply modifications to files to conform to ARC", 0)
OPTION(prefix_1, "ccc-gcc-name", ccc_gcc_name, Separate, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Name for native GCC compiler", "<gcc-path>")
OPTION(prefix_1, "ccc-install-dir", ccc_install_dir, Separate, internal_debug_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Simulate installation in the given directory", 0)
OPTION(prefix_1, "ccc-objcmt-migrate", ccc_objcmt_migrate, Separate, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Apply modifications and produces temporary files to migrate to modern ObjC syntax", 0)
OPTION(prefix_1, "ccc-pch-is-pch", ccc_pch_is_pch, Flag, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Use lazy PCH for precompiled headers", 0)
OPTION(prefix_1, "ccc-pch-is-pth", ccc_pch_is_pth, Flag, internal_driver_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Use pretokenized headers for precompiled headers", 0)
OPTION(prefix_1, "ccc-print-bindings", ccc_print_bindings, Flag, internal_debug_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Show bindings of tools to actions", 0)
OPTION(prefix_1, "ccc-print-phases", ccc_print_phases, Flag, internal_debug_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Dump list of actions to perform", 0)
OPTION(prefix_1, "ccc-", ccc_, Joined, internal_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "CC", CC, Flag, INVALID, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "cfg-add-implicit-dtors", analysis_CFGAddImplicitDtors, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add C++ implicit destructors to CFGs for all analyses", 0)
OPTION(prefix_1, "chain-include", chain_include, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include and chain a header file after turning it into PCH", "<file>")
OPTION(prefix_1, "cl-denorms-are-zero", cl_denorms_are_zero, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Allow denormals to be flushed to zero", 0)
OPTION(prefix_1, "cl-fast-relaxed-math", cl_fast_relaxed_math, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Sets -cl-finite-math-only and -cl-unsafe-math-optimizations, and defines __FAST_RELAXED_MATH__", 0)
OPTION(prefix_1, "cl-finite-math-only", cl_finite_math_only, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Allow floating-point optimizations that assume arguments and results are not NaNs or +-Inf.", 0)
OPTION(prefix_1, "cl-kernel-arg-info", cl_kernel_arg_info, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Generate kernel argument metadata.", 0)
OPTION(prefix_1, "cl-mad-enable", cl_mad_enable, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Enable less precise MAD instructions to be generated.", 0)
OPTION(prefix_1, "cl-opt-disable", cl_opt_disable, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. This option disables all optimizations. The default is optimizations are enabled.", 0)
OPTION(prefix_1, "cl-single-precision-constant", cl_single_precision_constant, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Treat double precision floating-point constant as single precision constant.", 0)
OPTION(prefix_1, "cl-std=", cl_std_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL language standard to compile for", 0)
OPTION(prefix_1, "cl-unsafe-math-optimizations", cl_unsafe_math_optimizations, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "OpenCL only. Allow unsafe floating-point optimizations.  Also implies -cl-no-signed-zeros and -cl-mad-enable", 0)
OPTION(prefix_3, "CLASSPATH=", _CLASSPATH_EQ, Joined, INVALID, fclasspath_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "classpath=", _classpath_EQ, Joined, INVALID, fclasspath_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "CLASSPATH", _CLASSPATH, Separate, INVALID, fclasspath_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "classpath", _classpath, Separate, INVALID, fclasspath_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "client_name", client__name, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "clr", _SLASH_clr, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "code-completion-at=", code_completion_at_EQ, Joined, INVALID, code_completion_at, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "code-completion-at", code_completion_at, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Dump code-completion information at a location", "<file>:<line>:<column>")
OPTION(prefix_1, "code-completion-brief-comments", code_completion_brief_comments, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include brief documentation comments in code-completion results.", 0)
OPTION(prefix_1, "code-completion-macros", code_completion_macros, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include macros in code-completion results", 0)
OPTION(prefix_1, "code-completion-patterns", code_completion_patterns, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include code patterns in code-completion results", 0)
OPTION(prefix_4, "combine", combine, Flag, INVALID, INVALID, 0, DriverOption | Unsupported, 0, 0, 0)
OPTION(prefix_3, "comments-in-macros", _comments_in_macros, Flag, INVALID, CC, 0, 0, 0, 0, 0)
OPTION(prefix_3, "comments", _comments, Flag, INVALID, C, 0, 0, 0, 0, 0)
OPTION(prefix_1, "compatibility_version", compatibility__version, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "compile", _compile, Flag, INVALID, c, 0, 0, 0, 0, 0)
OPTION(prefix_1, "compress-debug-sections", compress_debug_sections, Flag, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Compress DWARF debug sections using zlib", 0)
OPTION(prefix_3, "constant-cfstrings", _constant_cfstrings, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "coverage-cfg-checksum", coverage_cfg_checksum, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit CFG checksum for functions in .gcno files.", 0)
OPTION(prefix_1, "coverage-file=", coverage_file_EQ, Joined, INVALID, coverage_file, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "coverage-file", coverage_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit coverage data to this filename. The extension will be replaced.", 0)
OPTION(prefix_1, "coverage-no-function-names-in-data", coverage_no_function_names_in_data, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit function names in .gcda files.", 0)
OPTION(prefix_1, "coverage-version=", coverage_version_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Four-byte version string for gcov files.", 0)
OPTION(prefix_4, "coverage", coverage, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "cpp-precomp", cpp_precomp, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "cpp", cpp, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "current_version", current__version, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "cxx-isystem", cxx_isystem, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Add directory to the C++ SYSTEM include search path", "<directory>")
OPTION(prefix_1, "C", C, Flag, INVALID, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "C", _SLASH_C, Flag, cl_Group, C, 0, CLOption | DriverOption, 0,
       "Don't discard comments when preprocessing", 0)
OPTION(prefix_1, "c", c, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Only run preprocess, compile, and assemble steps", 0)
OPTION(prefix_2, "c", _SLASH_c, Flag, cl_Group, c, 0, CLOption | DriverOption, 0,
       "Compile only", 0)
OPTION(prefix_2, "d2Zi+", _SLASH_d2Zi_PLUS, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "dA", dA, Flag, d_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dD", dD, Flag, d_Group, INVALID, 0, CC1Option, 0,
       "Print macro definitions in -E mode in addition to normal output", 0)
OPTION(prefix_1, "dead_strip", dead__strip, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "debug=", _debug_EQ, Joined, INVALID, g_Flag, 0, 0, 0, 0, 0)
OPTION(prefix_3, "debug", _debug, Flag, INVALID, g_Flag, 0, 0, 0, 0, 0)
OPTION(prefix_3, "define-macro=", _define_macro_EQ, Joined, INVALID, D, 0, 0, 0, 0, 0)
OPTION(prefix_3, "define-macro", _define_macro, Separate, INVALID, D, 0, 0, 0, 0, 0)
OPTION(prefix_3, "dependencies", _dependencies, Flag, INVALID, M, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dependency-dot", dependency_dot, Separate, INVALID, INVALID, 0, CC1Option, 0,
       "Filename to write DOT-formatted header dependencies to", 0)
OPTION(prefix_1, "dependency-file", dependency_file, Separate, INVALID, INVALID, 0, CC1Option, 0,
       "Filename (or -) to write dependency output to", 0)
OPTION(prefix_3, "dependent-lib=", dependent_lib, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add dependent library", 0)
OPTION(prefix_1, "detailed-preprocessing-record", detailed_preprocessing_record, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "include a detailed record of preprocessing actions", 0)
OPTION(prefix_1, "diagnostic-log-file", diagnostic_log_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Filename (or -) to log diagnostics to", 0)
OPTION(prefix_1, "disable-free", disable_free, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable freeing of memory on exit", 0)
OPTION(prefix_1, "disable-llvm-optzns", disable_llvm_optzns, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Don't run LLVM optimization passes", 0)
OPTION(prefix_1, "disable-llvm-verifier", disable_llvm_verifier, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Don't run the LLVM IR verifier pass", 0)
OPTION(prefix_1, "disable-objc-default-synthesize-properties", disable_objc_default_synthesize_properties, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "disable the default synthesis of Objective-C properties", 0)
OPTION(prefix_1, "disable-red-zone", disable_red_zone, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not emit code that uses the red zone.", 0)
OPTION(prefix_1, "dM", dM, Flag, d_Group, INVALID, 0, CC1Option, 0,
       "Print macro definitions in -E mode instead of normal output", 0)
OPTION(prefix_2, "doc", _SLASH_doc, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "driver-mode=", driver_mode, Joined, internal_driver_Group, INVALID, 0, CoreOption | DriverOption | HelpHidden, 0,
       "Set the driver mode to either 'gcc', 'g++', 'cpp', or 'cl'", 0)
OPTION(prefix_1, "dump-coverage-mapping", dump_coverage_mapping, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Dump the coverage mapping records, for testing", 0)
OPTION(prefix_1, "dump-deserialized-decls", dump_deserialized_pch_decls, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Dump declarations that are deserialized from PCH, for testing", 0)
OPTION(prefix_1, "dump-raw-tokens", dump_raw_tokens, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Lex file in raw mode and dump raw tokens", 0)
OPTION(prefix_1, "dump-tokens", dump_tokens, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Run preprocessor, dump internal rep of tokens", 0)
OPTION(prefix_1, "dumpmachine", dumpmachine, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dumpspecs", dumpspecs, Flag, INVALID, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "dumpversion", dumpversion, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dwarf-column-info", dwarf_column_info, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Turn on column location information.", 0)
OPTION(prefix_1, "dwarf-debug-flags", dwarf_debug_flags, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "The string to embed in the Dwarf debug flags record.", 0)
OPTION(prefix_1, "dwarf-debug-producer", dwarf_debug_producer, Separate, INVALID, INVALID, 0, CC1AsOption | NoDriverOption, 0,
       "The string to embed in the Dwarf debug AT_producer record.", 0)
OPTION(prefix_3, "dyld-prefix=", _dyld_prefix_EQ, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "dyld-prefix", _dyld_prefix, Separate, INVALID, _dyld_prefix_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dylib_file", dylib__file, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dylinker_install_name", dylinker__install__name, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dylinker", dylinker, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dynamiclib", dynamiclib, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "dynamic", dynamic, Flag, INVALID, INVALID, 0, NoArgumentUnused, 0, 0, 0)
OPTION(prefix_1, "D", D, JoinedOrSeparate, CompileOnly_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "D", _SLASH_D, JoinedOrSeparate, cl_Group, D, 0, CLOption | DriverOption, 0,
       "Define macro", "<macro[=value]>")
OPTION(prefix_1, "d", d_Flag, Flag, d_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "d", d_Joined, Joined, d_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "EB", EB, Flag, INVALID, mbig_endian, 0, 0, 0, 0, 0)
OPTION(prefix_2, "EH", _SLASH_EH, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Exception handling model", 0)
OPTION(prefix_1, "EL", EL, Flag, INVALID, mlittle_endian, 0, 0, 0, 0, 0)
OPTION(prefix_1, "emit-ast", emit_ast, Flag, INVALID, INVALID, 0, 0, 0,
       "Emit Clang AST files for source inputs", 0)
OPTION(prefix_1, "emit-codegen-only", emit_codegen_only, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate machine code, but discard output", 0)
OPTION(prefix_1, "emit-html", emit_html, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Output input source as HTML", 0)
OPTION(prefix_1, "emit-llvm-bc", emit_llvm_bc, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs then convert to LLVM, emit .bc file", 0)
OPTION(prefix_1, "emit-llvm-only", emit_llvm_only, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Build ASTs and convert to LLVM, discarding output", 0)
OPTION(prefix_1, "emit-llvm", emit_llvm, Flag, Action_Group, INVALID, 0, CC1Option, 0,
       "Use the LLVM representation for assembler and object files", 0)
OPTION(prefix_1, "emit-module", emit_module, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate pre-compiled module file from a module map", 0)
OPTION(prefix_1, "emit-obj", emit_obj, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit native object files", 0)
OPTION(prefix_1, "emit-pch", emit_pch, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate pre-compiled header file", 0)
OPTION(prefix_1, "emit-pth", emit_pth, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate pre-tokenized header file", 0)
OPTION(prefix_3, "encoding=", _encoding_EQ, Joined, INVALID, fencoding_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "encoding", _encoding, Separate, INVALID, fencoding_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "entry", _entry, Flag, INVALID, e, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Eonly", Eonly, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Just run preprocessor, no output (for timings)", 0)
OPTION(prefix_2, "EP", _SLASH_EP, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Disable linemarker output and preprocess to stdout", 0)
OPTION(prefix_1, "error-on-deserialized-decl=", error_on_deserialized_pch_decl_EQ, Joined, INVALID, error_on_deserialized_pch_decl, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "error-on-deserialized-decl", error_on_deserialized_pch_decl, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit error if a specific declaration is deserialized from PCH, for testing", 0)
OPTION(prefix_2, "errorReport", _SLASH_errorReport, Joined, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "exported_symbols_list", exported__symbols__list, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "extdirs=", _extdirs_EQ, Joined, INVALID, fextdirs_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "extdirs", _extdirs, Separate, INVALID, fextdirs_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "extra-warnings", _extra_warnings, Flag, INVALID, W_Joined, 0, 0, 0, 0, 0)
OPTION(prefix_1, "E", E, Flag, Action_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Only run the preprocessor", 0)
OPTION(prefix_2, "E", _SLASH_E, Flag, cl_Group, E, 0, CLOption | DriverOption, 0,
       "Preprocess to stdout", 0)
OPTION(prefix_1, "e", e, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "faccess-control", faccess_control, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "faddress-space-map-mangling=", faddress_space_map_mangling_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Set the mode for address space map based mangling; OpenCL testing purposes only", "<yes|no|target>")
OPTION(prefix_1, "faggressive-function-elimination", aggressive_function_elimination_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-commons", align_commons_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-functions=", falign_functions_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-functions", align_functions_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-jumps=", falign_jumps_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-jumps", align_jumps_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-labels=", falign_labels_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-labels", align_labels_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-loops=", falign_loops_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "falign-loops", align_loops_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fall-intrinsics", all_intrinsics_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "fallback", _SLASH_fallback, Flag, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Fall back to cl.exe if clang-cl fails to compile", 0)
OPTION(prefix_1, "fallow-half-arguments-and-returns", fallow_half_arguments_and_returns, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Allow function arguments and returns of type half", 0)
OPTION(prefix_1, "fallow-unsupported", fallow_unsupported, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "faltivec", faltivec, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable AltiVec vector initializer syntax", 0)
OPTION(prefix_1, "fansi-escape-codes", fansi_escape_codes, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use ANSI escape codes for diagnostics", 0)
OPTION(prefix_1, "fapple-kext", fapple_kext, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use Apple's kernel extensions ABI", 0)
OPTION(prefix_1, "fapple-pragma-pack", fapple_pragma_pack, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable Apple gcc-compatible #pragma pack handling", 0)
OPTION(prefix_1, "fasm-blocks", fasm_blocks, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fasm", fasm, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fassociative-math", fassociative_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fassume-sane-operator-new", fassume_sane_operator_new, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fastcp", fastcp, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fastf", fastf, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fast", fast, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fasynchronous-unwind-tables", fasynchronous_unwind_tables, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fauto-profile=", fauto_profile_EQ, Joined, INVALID, fprofile_sample_use_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fautolink", fautolink, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fautomatic", automatic_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "favor", _SLASH_favor, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "FA", _SLASH_FA, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Output assembly code file during compilation", 0)
OPTION(prefix_2, "FA", _SLASH_FA_joined, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Fa", _SLASH_Fa, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Output assembly code to this file during compilation", "<file or directory>")
OPTION(prefix_1, "fbackslash", backslash_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fbacktrace", backtrace_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fblas-matmul-limit=", fblas_matmul_limit_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fblocks-runtime-optional", fblocks_runtime_optional, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Weakly link in the blocks runtime", 0)
OPTION(prefix_1, "fblocks", fblocks, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable the 'blocks' language feature", 0)
OPTION(prefix_1, "fbootclasspath=", fbootclasspath_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fborland-extensions", fborland_extensions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Accept non-standard constructs supported by the Borland compiler", 0)
OPTION(prefix_1, "fbounds-check", bounds_check_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fbracket-depth=", fbracket_depth_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fbracket-depth", fbracket_depth, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Maximum nesting level for parentheses, brackets, and braces", 0)
OPTION(prefix_1, "fbranch-count-reg", branch_count_reg_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fbuild-session-file=", fbuild_session_file, Joined, i_Group, INVALID, 0, 0, 0,
       "Use the last modification time of <file> as the build session timestamp", "<file>")
OPTION(prefix_1, "fbuild-session-timestamp=", fbuild_session_timestamp, Joined, i_Group, INVALID, 0, CC1Option, 0,
       "Time when the current build session started", "<time since Epoch in seconds>")
OPTION(prefix_1, "fbuiltin", fbuiltin, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcaller-saves", caller_saves_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcaret-diagnostics", fcaret_diagnostics, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcheck-array-temporaries", check_array_temporaries_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcheck-new", fcheck_new_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcheck=", fcheck_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fclasspath=", fclasspath_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcoarray=", fcoarray_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcolor-diagnostics", fcolor_diagnostics, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use colors in diagnostics", 0)
OPTION(prefix_1, "fcomment-block-commands=", fcomment_block_commands, CommaJoined, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Treat each comma separated argument in <arg> as a documentation comment block command", "<arg>")
OPTION(prefix_1, "fcommon", fcommon, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcompile-resource=", fcompile_resource_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fconst-strings", fconst_strings, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use a const qualified type for string literals in C and ObjC", 0)
OPTION(prefix_1, "fconstant-cfstrings", fconstant_cfstrings, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fconstant-string-class=", fconstant_string_class_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fconstant-string-class", fconstant_string_class, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Specify the class to use for constant Objective-C string objects.", "<class name>")
OPTION(prefix_1, "fconstexpr-backtrace-limit=", fconstexpr_backtrace_limit_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fconstexpr-backtrace-limit", fconstexpr_backtrace_limit, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Set the maximum number of entries to print in a constexpr evaluation backtrace (0 = no limit).", "<N>")
OPTION(prefix_1, "fconstexpr-depth=", fconstexpr_depth_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fconstexpr-depth", fconstexpr_depth, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Maximum depth of recursive constexpr function calls", 0)
OPTION(prefix_1, "fconstexpr-steps=", fconstexpr_steps_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fconstexpr-steps", fconstexpr_steps, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Maximum number of steps in constexpr function evaluation", 0)
OPTION(prefix_1, "fconvert=", fconvert_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcoverage-mapping", fcoverage_mapping, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Generate coverage mapping to enable code coverage analysis", 0)
OPTION(prefix_1, "fcray-pointer", cray_pointer_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcreate-profile", fcreate_profile, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcuda-is-device", fcuda_is_device, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate code for CUDA device", 0)
OPTION(prefix_1, "fcxx-exceptions", fcxx_exceptions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable C++ exceptions", 0)
OPTION(prefix_1, "fcxx-modules", fcxx_modules, Flag, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_2, "FC", _SLASH_FC, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "fd-lines-as-code", d_lines_as_code_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fd-lines-as-comments", d_lines_as_comments_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdata-sections", fdata_sections, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Place each data in its own section (ELF Only)", 0)
OPTION(prefix_1, "fdebug-compilation-dir", fdebug_compilation_dir, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "The compilation directory to embed in the debug info.", 0)
OPTION(prefix_1, "fdebug-pass-arguments", fdebug_pass_arguments, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdebug-pass-structure", fdebug_pass_structure, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdebug-types-section", fdebug_types_section, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Place debug types in their own section (ELF Only)", 0)
OPTION(prefix_1, "fdebugger-cast-result-to-id", fdebugger_cast_result_to_id, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable casting unknown expression results to id", 0)
OPTION(prefix_1, "fdebugger-objc-literal", fdebugger_objc_literal, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable special debugger support for Objective-C subscripting and literals", 0)
OPTION(prefix_1, "fdebugger-support", fdebugger_support, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable special debugger support behavior", 0)
OPTION(prefix_1, "fdefault-double-8", default_double_8_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdefault-inline", default_inline_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdefault-integer-8", default_integer_8_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdefault-real-8", default_real_8_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdefer-pop", anonymous_3, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdelayed-template-parsing", fdelayed_template_parsing, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Parse templated function definitions at the end of the translation unit", 0)
OPTION(prefix_1, "fdelete-null-pointer-checks", delete_null_pointer_checks_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdeprecated-macro", fdeprecated_macro, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Defines the __DEPRECATED macro", 0)
OPTION(prefix_1, "fdevirtualize-speculatively", devirtualize_speculatively_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdevirtualize", devirtualize_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-color=", fdiagnostics_color_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-color", fdiagnostics_color, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-fixit-info", fdiagnostics_fixit_info, Flag, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-format=", fdiagnostics_format_EQ, Joined, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-format", fdiagnostics_format, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Change diagnostic formatting to match IDE and command line tools", 0)
OPTION(prefix_1, "fdiagnostics-parseable-fixits", fdiagnostics_parseable_fixits, Flag, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Print fix-its in machine parseable form", 0)
OPTION(prefix_1, "fdiagnostics-print-source-range-info", fdiagnostics_print_source_range_info, Flag, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Print source range spans in numeric form", 0)
OPTION(prefix_1, "fdiagnostics-show-category=", fdiagnostics_show_category_EQ, Joined, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-show-category", fdiagnostics_show_category, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Print diagnostic category", 0)
OPTION(prefix_1, "fdiagnostics-show-location=", fdiagnostics_show_location_EQ, Joined, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdiagnostics-show-note-include-stack", fdiagnostics_show_note_include_stack, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Display include stacks for diagnostic notes", 0)
OPTION(prefix_1, "fdiagnostics-show-option", fdiagnostics_show_option, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Print option name with mappable diagnostics", 0)
OPTION(prefix_1, "fdiagnostics-show-template-tree", fdiagnostics_show_template_tree, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Print a template comparison tree for differing templates", 0)
OPTION(prefix_1, "fdisable-module-hash", fdisable_module_hash, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable the module hash", 0)
OPTION(prefix_1, "fdollar-ok", dollar_ok_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdollars-in-identifiers", fdollars_in_identifiers, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Allow '$' in identifiers", 0)
OPTION(prefix_1, "fdump-fortran-optimized", dump_fortran_optimized_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdump-fortran-original", dump_fortran_original_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdump-parse-tree", dump_parse_tree_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdump-record-layouts-simple", fdump_record_layouts_simple, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Dump record layout information in a simple form used for testing", 0)
OPTION(prefix_1, "fdump-record-layouts", fdump_record_layouts, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Dump record layout information", 0)
OPTION(prefix_1, "fdump-vtable-layouts", fdump_vtable_layouts, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Dump the layouts of all vtables that will be emitted in a translation unit", 0)
OPTION(prefix_1, "fdwarf-directory-asm", fdwarf_directory_asm, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fdwarf2-cfi-asm", fdwarf2_cfi_asm, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Fd", _SLASH_Fd, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "felide-constructors", felide_constructors, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "feliminate-unused-debug-symbols", feliminate_unused_debug_symbols, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "feliminate-unused-debug-types", eliminate_unused_debug_types_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "femit-all-decls", femit_all_decls, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Emit all declarations, even if unused", 0)
OPTION(prefix_1, "femit-coverage-data", femit_coverage_data, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Instrument the program to emit gcov coverage data when run.", 0)
OPTION(prefix_1, "femit-coverage-notes", femit_coverage_notes, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit a gcov coverage notes file when compiling.", 0)
OPTION(prefix_1, "fencode-extended-block-signature", fencode_extended_block_signature, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "enable extended encoding of block type signature", 0)
OPTION(prefix_1, "fencoding=", fencoding_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ferror-limit=", ferror_limit_EQ, Joined, f_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "ferror-limit", ferror_limit, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Set the maximum number of errors to emit before stopping (0 = no limit).", "<N>")
OPTION(prefix_1, "fexceptions", fexceptions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable support for exception handling", 0)
OPTION(prefix_1, "fexcess-precision=", fexcess_precision_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fexec-charset=", fexec_charset_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fexpensive-optimizations", anonymous_1, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fextdirs=", fextdirs_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fextended-identifiers", anonymous_5, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fexternal-blas", external_blas_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Fe", _SLASH_Fe, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set output executable file or directory (ends in / or \\)", "<file or directory>")
OPTION(prefix_1, "ff2c", f2c_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffake-address-space-map", ffake_address_space_map, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use a fake address space map; OpenCL testing purposes only", 0)
OPTION(prefix_1, "ffast-math", ffast_math, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable the *frontend*'s 'fast-math' mode. This has no effect on optimizations, but provides a preprocessor macro __FAST_MATH__ the same as GCC's -ffast-math flag", 0)
OPTION(prefix_1, "ffat-lto-objects", fat_lto_objects_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffinite-math-only", ffinite_math_only, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "ffixed-form", fixed_form_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffixed-line-length-", ffixed_line_length_VALUE, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffixed-r9", ffixed_r9, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Reserve the r9 register (ARM only)", 0)
OPTION(prefix_1, "ffloat-store", float_store_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffor-scope", ffor_scope, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fforbid-guard-variables", fforbid_guard_variables, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit an error if a C++ static local initializer would need a guard variable", 0)
OPTION(prefix_1, "fforce-addr", force_addr, Joined, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffp-contract=", ffp_contract, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Form fused FP ops (e.g. FMAs): fast (everywhere) | on (according to FP_CONTRACT pragma, default) | off (never fuse)", 0)
OPTION(prefix_1, "ffpe-trap=", ffpe_trap_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffree-form", free_form_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffree-line-length-", ffree_line_length_VALUE, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffreestanding", ffreestanding, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Assert that the compilation takes place in a freestanding environment", 0)
OPTION(prefix_1, "ffriend-injection", friend_injection_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffrontend-optimize", frontend_optimize_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffunction-attribute-list", function_attribute_list_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ffunction-sections", ffunction_sections, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Place each function in its own section (ELF Only)", 0)
OPTION(prefix_1, "fgcse-after-reload", gcse_after_reload_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fgcse-las", gcse_las_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fgcse-sm", gcse_sm_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fgcse", gcse_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fgnu-keywords", fgnu_keywords, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Allow GNU-extension keywords regardless of language standard", 0)
OPTION(prefix_1, "fgnu-runtime", fgnu_runtime, Flag, f_Group, INVALID, 0, 0, 0,
       "Generate output compatible with the standard GNU Objective-C runtime", 0)
OPTION(prefix_1, "fgnu89-inline", fgnu89_inline, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use the gnu89 inline semantics", 0)
OPTION(prefix_1, "fgnu", gnu_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fheinous-gnu-extensions", fheinous_gnu_extensions, Flag, INVALID, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fhonor-infinites", anonymous_7, Flag, INVALID, fhonor_infinities, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fhonor-infinities", fhonor_infinities, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fhonor-nans", fhonor_nans, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fhosted", fhosted, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fident", ident_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "filelist", filelist, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_1, "filetype", filetype, Separate, INVALID, INVALID, 0, CC1AsOption | NoDriverOption, 0,
       "Specify the output file type ('asm', 'null', or 'obj')", 0)
OPTION(prefix_1, "fimplement-inlines", implement_inlines_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fimplicit-none", implicit_none_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fimplicit-templates", implicit_templates_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "findirect-virtual-calls", anonymous_9, Flag, INVALID, fapple_kext, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finit-character=", finit_character_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finit-integer=", finit_integer_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finit-local-zero", init_local_zero_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finit-logical=", finit_logical_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finit-real=", finit_real_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finline-functions-called-once", inline_functions_called_once_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finline-functions", finline_functions, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finline-limit=", finline_limit_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finline-limit", finline_limit_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finline-small-functions", inline_small_functions_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finline", finline, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finput-charset=", finput_charset_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "finstrument-functions", finstrument_functions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Generate calls to instrument function entry and exit", 0)
OPTION(prefix_1, "finteger-4-integer-8", integer_4_integer_8_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fintegrated-as", fintegrated_as, Flag, f_Group, INVALID, 0, DriverOption, 0,
       "Enable the integrated assembler", 0)
OPTION(prefix_1, "fintrinsic-modules-path", intrinsic_modules_path_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fipa-cp", ipa_cp_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fivopts", ivopts_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fix-only-warnings", fix_only_warnings, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply fix-it advice only for warnings, not errors", 0)
OPTION(prefix_1, "fix-what-you-can", fix_what_you_can, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply fix-it advice even in the presence of unfixable errors", 0)
OPTION(prefix_1, "fixit-recompile", fixit_recompile, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply fix-it changes and recompile", 0)
OPTION(prefix_1, "fixit-to-temporary", fixit_to_temp, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply fix-it changes to temporary files", 0)
OPTION(prefix_1, "fixit=", fixit_EQ, Joined, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply fix-it advice creating a file with the given suffix", 0)
OPTION(prefix_1, "fixit", fixit, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Apply fix-it advice to the input source", 0)
OPTION(prefix_2, "FI", _SLASH_FI, JoinedOrSeparate, cl_Group, include, 0, CLOption | DriverOption, 0,
       "Include file before parsing", 0)
OPTION(prefix_2, "Fi", _SLASH_Fi, Joined, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set preprocess output file name", "<file>")
OPTION(prefix_1, "fkeep-inline-functions", anonymous_20_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "flat_namespace", flat__namespace, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "flax-vector-conversions", flax_vector_conversions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "flimit-debug-info", flimit_debug_info, Flag, INVALID, fno_standalone_debug, 0, 0, 0, 0, 0)
OPTION(prefix_1, "flimited-precision=", flimited_precision_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "flto=", flto_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "flto", flto, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmacro-backtrace-limit=", fmacro_backtrace_limit_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmacro-backtrace-limit", fmacro_backtrace_limit, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Set the maximum number of entries to print in a macro expansion backtrace (0 = no limit).", "<N>")
OPTION(prefix_1, "fmath-errno", fmath_errno, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Require math functions to indicate errors by setting errno", 0)
OPTION(prefix_1, "fmax-array-constructor=", fmax_array_constructor_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmax-errors=", fmax_errors_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmax-identifier-length", max_identifier_length_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmax-stack-var-size=", fmax_stack_var_size_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmax-subrecord-length=", fmax_subrecord_length_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmax-type-align=", fmax_type_align_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Specify the maximum alignment to enforce on pointers lacking an explicit alignment", 0)
OPTION(prefix_1, "fmerge-all-constants", fmerge_all_constants, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmerge-constants", merge_constants_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmerge-functions", fmerge_functions, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Permit merging of identical functions when optimizing.", 0)
OPTION(prefix_1, "fmessage-length=", fmessage_length_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmessage-length", fmessage_length, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Format message diagnostics so that they fit within N columns or fewer, when possible.", "<N>")
OPTION(prefix_1, "fmodule-file=", fmodule_file, Joined, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Load this precompiled module file", "<file>")
OPTION(prefix_1, "fmodule-implementation-of", fmodule_implementation_of, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Specify the name of the module whose implementation file this is", "<name>")
OPTION(prefix_1, "fmodule-map-file=", fmodule_map_file, Joined, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Load this module map file", "<file>")
OPTION(prefix_1, "fmodule-maps", fmodule_maps, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Read module maps to understand the structure of library headers", 0)
OPTION(prefix_1, "fmodule-name=", fmodule_name, JoinedOrSeparate, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Specify the name of the module to build", "<name>")
OPTION(prefix_1, "fmodule-private", module_private_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmodules-cache-path=", fmodules_cache_path, Joined, i_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Specify the module cache path", "<directory>")
OPTION(prefix_1, "fmodules-decluse", fmodules_decluse, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Require declaration of modules used within a module", 0)
OPTION(prefix_1, "fmodules-ignore-macro=", fmodules_ignore_macro, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Ignore the definition of the given macro when building and loading modules", 0)
OPTION(prefix_1, "fmodules-implicit-maps", fmodules_implicit_maps, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0, 0, 0)
OPTION(prefix_1, "fmodules-prune-after=", fmodules_prune_after, Joined, i_Group, INVALID, 0, CC1Option, 0,
       "Specify the interval (in seconds) after which a module file will be considered unused", "<seconds>")
OPTION(prefix_1, "fmodules-prune-interval=", fmodules_prune_interval, Joined, i_Group, INVALID, 0, CC1Option, 0,
       "Specify the interval (in seconds) between attempts to prune the module cache", "<seconds>")
OPTION(prefix_1, "fmodules-search-all", fmodules_search_all, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Search even non-imported modules to resolve references", 0)
OPTION(prefix_1, "fmodules-strict-decluse", fmodules_strict_decluse, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Like -fmodules-decluse but requires all headers to be in modules", 0)
OPTION(prefix_1, "fmodules-user-build-path", fmodules_user_build_path, Separate, i_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Specify the module user build path", "<directory>")
OPTION(prefix_1, "fmodules-validate-once-per-build-session", fmodules_validate_once_per_build_session, Flag, i_Group, INVALID, 0, CC1Option, 0,
       "Don't verify input files for the modules if the module has been successfully validated or loaded during this build session", 0)
OPTION(prefix_1, "fmodules-validate-system-headers", fmodules_validate_system_headers, Flag, i_Group, INVALID, 0, CC1Option, 0,
       "Validate the system headers that a module depends on when loading the module", 0)
OPTION(prefix_1, "fmodules", fmodules, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Enable the 'modules' language feature", 0)
OPTION(prefix_1, "fmodulo-sched-allow-regmoves", modulo_sched_allow_regmoves_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmodulo-sched", modulo_sched_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fms-compatibility-version=", fms_compatibility_version, Joined, f_Group, INVALID, 0, CC1Option | CoreOption, 0,
       "Dot-separated value representing the Microsoft compiler version number to report in _MSC_VER (0 = don't define it (default))", 0)
OPTION(prefix_1, "fms-compatibility", fms_compatibility, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable full Microsoft Visual C++ compatibility", 0)
OPTION(prefix_1, "fms-extensions", fms_extensions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Accept some non-standard constructs supported by the Microsoft compiler", 0)
OPTION(prefix_1, "fms-memptr-rep=", fms_memptr_rep_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fmsc-version=", fmsc_version, Joined, f_Group, INVALID, 0, DriverOption | CoreOption, 0,
       "Microsoft compiler version number to report in _MSC_VER (0 = don't define it (default))", 0)
OPTION(prefix_1, "fmudflapth", fmudflapth, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fmudflap", fmudflap, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Fm", _SLASH_Fm, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "fnested-functions", fnested_functions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fnext-runtime", fnext_runtime, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-access-control", fno_access_control, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable C++ access control", 0)
OPTION(prefix_1, "fno-aggressive-function-elimination", aggressive_function_elimination_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-align-commons", align_commons_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-align-functions", align_functions_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-align-jumps", align_jumps_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-align-labels", align_labels_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-align-loops", align_loops_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-all-intrinsics", all_intrinsics_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-altivec", fno_altivec, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-apple-pragma-pack", fno_apple_pragma_pack, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-asm-blocks", fno_asm_blocks, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-asm", fno_asm, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-associative-math", fno_associative_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-assume-sane-operator-new", fno_assume_sane_operator_new, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Don't assume that C++'s global operator new can't alias any pointer", 0)
OPTION(prefix_1, "fno-asynchronous-unwind-tables", fno_asynchronous_unwind_tables, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-autolink", fno_autolink, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Disable generation of linker directives for automatic library linking", 0)
OPTION(prefix_1, "fno-automatic", automatic_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-backslash", backslash_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-backtrace", backtrace_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-bitfield-type-align", fno_bitfield_type_align, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Ignore bit-field types when aligning structures", 0)
OPTION(prefix_1, "fno-blocks", fno_blocks, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-borland-extensions", fno_borland_extensions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-bounds-check", bounds_check_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-branch-count-reg", branch_count_reg_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-builtin-", fno_builtin_, Joined, clang_ignored_f_Group, INVALID, 0, 0, 0,
       "Disable implicit builtin knowledge of a specific function", 0)
OPTION(prefix_1, "fno-builtin", fno_builtin, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable implicit builtin knowledge of functions", 0)
OPTION(prefix_1, "fno-caller-saves", caller_saves_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-caret-diagnostics", fno_caret_diagnostics, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-check-array-temporaries", check_array_temporaries_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-check-new", fcheck_new_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-color-diagnostics", fno_color_diagnostics, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-common", fno_common, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Compile common globals like normal definitions", 0)
OPTION(prefix_1, "fno-const-strings", fno_const_strings, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Don't use a const qualified type for string literals in C and ObjC", 0)
OPTION(prefix_1, "fno-constant-cfstrings", fno_constant_cfstrings, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable creation of CodeFoundation-type constant strings", 0)
OPTION(prefix_1, "fno-crash-diagnostics", fno_crash_diagnostics, Flag, f_clang_Group, INVALID, 0, NoArgumentUnused, 0, 0, 0)
OPTION(prefix_1, "fno-cray-pointer", cray_pointer_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-cxx-exceptions", fno_cxx_exceptions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-cxx-modules", fno_cxx_modules, Flag, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "fno-d-lines-as-code", d_lines_as_code_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-d-lines-as-comments", d_lines_as_comments_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-data-sections", fno_data_sections, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-debug-types-section", fno_debug_types_section, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-default-double-8", default_double_8_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-default-inline", default_inline_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-default-integer-8", default_integer_8_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-default-real-8", default_real_8_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-defer-pop", anonymous_4, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-delayed-template-parsing", fno_delayed_template_parsing, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-delete-null-pointer-checks", delete_null_pointer_checks_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-deprecated-macro", fno_deprecated_macro, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Undefines the __DEPRECATED macro", 0)
OPTION(prefix_1, "fno-devirtualize-speculatively", devirtualize_speculatively_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-devirtualize", devirtualize_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-diagnostics-color", fno_diagnostics_color, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-diagnostics-fixit-info", fno_diagnostics_fixit_info, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Do not include fixit information in diagnostics", 0)
OPTION(prefix_1, "fno-diagnostics-show-note-include-stack", fno_diagnostics_show_note_include_stack, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-diagnostics-show-option", fno_diagnostics_show_option, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-diagnostics-use-presumed-location", fno_diagnostics_use_presumed_location, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Ignore #line directives when displaying diagnostic locations", 0)
OPTION(prefix_1, "fno-dollar-ok", dollar_ok_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-dollars-in-identifiers", fno_dollars_in_identifiers, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disallow '$' in identifiers", 0)
OPTION(prefix_1, "fno-dump-fortran-optimized", dump_fortran_optimized_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-dump-fortran-original", dump_fortran_original_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-dump-parse-tree", dump_parse_tree_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-dwarf-directory-asm", fno_dwarf_directory_asm, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-dwarf2-cfi-asm", fno_dwarf2_cfi_asm, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-elide-constructors", fno_elide_constructors, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable C++ copy constructor elision", 0)
OPTION(prefix_1, "fno-elide-type", fno_elide_type, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Do not elide types when printing diagnostics", 0)
OPTION(prefix_1, "fno-eliminate-unused-debug-symbols", fno_eliminate_unused_debug_symbols, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-eliminate-unused-debug-types", eliminate_unused_debug_types_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-exceptions", fno_exceptions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-expensive-optimizations", anonymous_2, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-extended-identifiers", anonymous_6, Flag, f_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "fno-external-blas", external_blas_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-f2c", f2c_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-fast-math", fno_fast_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-fat-lto-objects", fat_lto_objects_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-finite-math-only", fno_finite_math_only, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-fixed-form", fixed_form_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-float-store", float_store_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-for-scope", fno_for_scope, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-free-form", free_form_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-friend-injection", friend_injection_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-frontend-optimize", frontend_optimize_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-function-attribute-list", function_attribute_list_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-function-sections", fno_function_sections, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-gcse-after-reload", gcse_after_reload_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-gcse-las", gcse_las_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-gcse-sm", gcse_sm_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-gcse", gcse_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-gnu-keywords", fno_gnu_keywords, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-gnu89-inline", fno_gnu89_inline, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-gnu", gnu_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-honor-infinites", anonymous_8, Flag, INVALID, fno_honor_infinities, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-honor-infinities", fno_honor_infinities, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-honor-nans", fno_honor_nans, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-ident", ident_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-implement-inlines", implement_inlines_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-implicit-none", implicit_none_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-implicit-templates", implicit_templates_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-init-local-zero", init_local_zero_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-inline-functions-called-once", inline_functions_called_once_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-inline-functions", fno_inline_functions, Flag, f_clang_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-inline-limit", finline_limit_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-inline-small-functions", inline_small_functions_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-inline", fno_inline, Flag, f_clang_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-integer-4-integer-8", integer_4_integer_8_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-integrated-as", fno_integrated_as, Flag, f_Group, INVALID, 0, CC1Option | DriverOption, 0,
       "Disable the integrated assembler", 0)
OPTION(prefix_1, "fno-intrinsic-modules-path", intrinsic_modules_path_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-ipa-cp", ipa_cp_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-ivopts", ivopts_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-keep-inline-functions", anonymous_20_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-lax-vector-conversions", fno_lax_vector_conversions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disallow implicit conversions between vectors with a different number of elements or different element types", 0)
OPTION(prefix_1, "fno-limit-debug-info", fno_limit_debug_info, Flag, INVALID, fstandalone_debug, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-lto", fno_lto, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-math-builtin", fno_math_builtin, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable implicit builtin knowledge of math functions", 0)
OPTION(prefix_1, "fno-math-errno", fno_math_errno, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-max-identifier-length", max_identifier_length_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-max-type-align", fno_max_type_align, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-merge-all-constants", fno_merge_all_constants, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disallow merging of constants", 0)
OPTION(prefix_1, "fno-merge-constants", merge_constants_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-module-maps", fno_module_maps, Flag, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "fno-module-private", module_private_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-modules-decluse", fno_modules_decluse, Flag, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "fno-modules-error-recovery", fno_modules_error_recovery, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not automatically import modules for error recovery", 0)
OPTION(prefix_1, "fno-modules-global-index", fno_modules_global_index, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not automatically generate or update the global module index", 0)
OPTION(prefix_1, "fno-modules-implicit-maps", fno_modules_implicit_maps, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-modules-search-all", fno_modules_search_all, Flag, f_Group, INVALID, 0, DriverOption | CC1Option, 0, 0, 0)
OPTION(prefix_1, "fno-modules", fno_modules, Flag, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "fno-modulo-sched-allow-regmoves", modulo_sched_allow_regmoves_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-modulo-sched", modulo_sched_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-ms-compatibility", fno_ms_compatibility, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-ms-extensions", fno_ms_extensions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-non-call-exceptions", non_call_exceptions_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-objc-arc-exceptions", fno_objc_arc_exceptions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-objc-arc", fno_objc_arc, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-objc-exceptions", fno_objc_exceptions, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-objc-infer-related-result-type", fno_objc_infer_related_result_type, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "do not infer Objective-C related result type based on method family", 0)
OPTION(prefix_1, "fno-objc-legacy-dispatch", fno_objc_legacy_dispatch, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-objc-nonfragile-abi", fno_objc_nonfragile_abi, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-omit-frame-pointer", fno_omit_frame_pointer, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-operator-names", fno_operator_names, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Do not treat C++ operator name keywords as synonyms for operators", 0)
OPTION(prefix_1, "fno-optimize-sibling-calls", fno_optimize_sibling_calls, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-pack-derived", pack_derived_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-pack-struct", fno_pack_struct, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-pascal-strings", fno_pascal_strings, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-peel-loops", peel_loops_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-permissive", permissive_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-PIC", fno_PIC, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-pic", fno_pic, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-PIE", fno_PIE, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-pie", fno_pie, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-prefetch-loop-arrays", prefetch_loop_arrays_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-printf", printf_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile-arcs", fno_profile_arcs, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile-correction", profile_correction_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile-generate-sampling", profile_generate_sampling_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile-reusedist", profile_reusedist_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile-use", profile_use_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile-values", profile_values_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-profile", profile_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-protect-parens", protect_parens_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-range-check", range_check_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-real-4-real-10", real_4_real_10_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-real-4-real-16", real_4_real_16_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-real-4-real-8", real_4_real_8_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-real-8-real-10", real_8_real_10_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-real-8-real-16", real_8_real_16_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-real-8-real-4", real_8_real_4_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-realloc-lhs", realloc_lhs_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-reciprocal-math", fno_reciprocal_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-recursive", recursive_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-regs-graph", regs_graph_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-rename-registers", rename_registers_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-reorder-blocks", reorder_blocks_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-repack-arrays", repack_arrays_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-reroll-loops", fno_reroll_loops, Flag, f_Group, INVALID, 0, 0, 0,
       "Turn off loop reroller", 0)
OPTION(prefix_1, "fno-rewrite-includes", fno_rewrite_includes, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-ripa", ripa_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-rounding-math", rounding_math_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-rtti-data", fno_rtti_data, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Control emission of RTTI data", 0)
OPTION(prefix_1, "fno-rtti", fno_rtti, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable generation of rtti information", 0)
OPTION(prefix_1, "fno-sanitize-blacklist", fno_sanitize_blacklist, Flag, f_clang_Group, INVALID, 0, 0, 0,
       "Don't use blacklist file for sanitizers", 0)
OPTION(prefix_1, "fno-sanitize-memory-track-origins", fno_sanitize_memory_track_origins, Flag, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Disable origins tracking in MemorySanitizer", 0)
OPTION(prefix_1, "fno-sanitize-recover", fno_sanitize_recover, Flag, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Disable sanitizer check recovery", 0)
OPTION(prefix_1, "fno-sanitize-undefined-trap-on-error", fno_sanitize_undefined_trap_on_error, Flag, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-sanitize=", fno_sanitize_EQ, CommaJoined, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-schedule-insns2", schedule_insns2_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-schedule-insns", schedule_insns_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-second-underscore", second_underscore_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-see", see_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-short-enums", fno_short_enums, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-short-wchar", fno_short_wchar, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Force wchar_t to be an unsigned int", 0)
OPTION(prefix_1, "fno-show-column", fno_show_column, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Do not include column number on diagnostics", 0)
OPTION(prefix_1, "fno-show-source-location", fno_show_source_location, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Do not include source location information with diagnostics", 0)
OPTION(prefix_1, "fno-sign-zero", sign_zero_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-signaling-math", fno_signaling_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-signaling-nans", signaling_nans_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-signed-char", fno_signed_char, Flag, clang_ignored_f_Group, INVALID, 0, CC1Option, 0,
       "Char is unsigned", 0)
OPTION(prefix_1, "fno-signed-zeros", fno_signed_zeros, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-single-precision-constant", single_precision_constant_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-slp-vectorize-aggressive", fno_slp_vectorize_aggressive, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-slp-vectorize", fno_slp_vectorize, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-spec-constr-count", spec_constr_count_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-spell-checking", fno_spell_checking, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Disable spell-checking", 0)
OPTION(prefix_1, "fno-stack-arrays", stack_arrays_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-stack-check", stack_check_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-stack-protector", fno_stack_protector, Flag, f_Group, INVALID, 0, 0, 0,
       "Disable the use of stack protectors", 0)
OPTION(prefix_1, "fno-standalone-debug", fno_standalone_debug, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Limit debug information produced to reduce size of debug binary", 0)
OPTION(prefix_1, "fno-strength-reduce", strength_reduce_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-strict-aliasing", fno_strict_aliasing, Flag, f_Group, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_1, "fno-strict-enums", fno_strict_enums, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-strict-modules-decluse", fno_modules_strict_decluse, Flag, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "fno-strict-overflow", fno_strict_overflow, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-struct-path-tbaa", fno_struct_path_tbaa, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-threadsafe-statics", fno_threadsafe_statics, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Do not emit code to make initialization of local statics thread safe", 0)
OPTION(prefix_1, "fno-tls-model", tls_model_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tracer", tracer_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-trapping-math", fno_trapping_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-dce", tree_dce_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-salias", tree_salias_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-slp-vectorize", anonymous_14, Flag, INVALID, fno_slp_vectorize, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-ter", tree_ter_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-vectorizer-verbose", tree_vectorizer_verbose_fno, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-vectorize", anonymous_12, Flag, INVALID, fno_vectorize, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree-vrp", tree_vrp_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree_loop_im", tree_loop_im_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree_loop_ivcanon", tree_loop_ivcanon_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-tree_loop_linear", tree_loop_linear_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-underscoring", underscoring_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unit-at-a-time", fno_unit_at_a_time, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unroll-all-loops", unroll_all_loops_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unroll-loops", fno_unroll_loops, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Turn off loop unroller", 0)
OPTION(prefix_1, "fno-unsafe-loop-optimizations", unsafe_loop_optimizations_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unsafe-math-optimizations", fno_unsafe_math_optimizations, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unsigned-char", fno_unsigned_char, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unswitch-loops", unswitch_loops_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-unwind-tables", fno_unwind_tables, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-use-cxa-atexit", fno_use_cxa_atexit, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Don't use __cxa_atexit for calling destructors", 0)
OPTION(prefix_1, "fno-use-init-array", fno_use_init_array, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Don't use .init_array instead of .ctors", 0)
OPTION(prefix_1, "fno-use-linker-plugin", use_linker_plugin_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-validate-pch", fno_validate_pch, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable validation of precompiled headers", 0)
OPTION(prefix_1, "fno-var-tracking", fno_var_tracking, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-variable-expansion-in-unroller", variable_expansion_in_unroller_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-vect-cost-model", vect_cost_model_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-vectorize", fno_vectorize, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-verbose-asm", fno_verbose_asm, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-wchar", fno_wchar, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable C++ builtin type wchar_t", 0)
OPTION(prefix_1, "fno-web", web_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-whole-file", whole_file_fno, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-whole-program", whole_program_fno, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-working-directory", fno_working_directory, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-wrapv", fno_wrapv, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fno-zero-initialized-in-bss", fno_zero_initialized_in_bss, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fnon-call-exceptions", non_call_exceptions_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-abi-version=", fobjc_abi_version_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-arc-cxxlib=", fobjc_arc_cxxlib_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Objective-C++ Automatic Reference Counting standard library kind", 0)
OPTION(prefix_1, "fobjc-arc-exceptions", fobjc_arc_exceptions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use EH-safe code when synthesizing retains and releases in -fobjc-arc", 0)
OPTION(prefix_1, "fobjc-arc", fobjc_arc, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Synthesize retain and release calls for Objective-C pointers", 0)
OPTION(prefix_1, "fobjc-atdefs", fobjc_atdefs, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-call-cxx-cdtors", fobjc_call_cxx_cdtors, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-dispatch-method=", fobjc_dispatch_method_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Objective-C dispatch method to use", 0)
OPTION(prefix_1, "fobjc-exceptions", fobjc_exceptions, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable Objective-C exceptions", 0)
OPTION(prefix_1, "fobjc-gc-only", fobjc_gc_only, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use GC exclusively for Objective-C related memory management", 0)
OPTION(prefix_1, "fobjc-gc", fobjc_gc, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable Objective-C garbage collection", 0)
OPTION(prefix_1, "fobjc-infer-related-result-type", fobjc_infer_related_result_type, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-legacy-dispatch", fobjc_legacy_dispatch, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-link-runtime", fobjc_link_runtime, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-new-property", fobjc_new_property, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-nonfragile-abi-version=", fobjc_nonfragile_abi_version_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-nonfragile-abi", fobjc_nonfragile_abi, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-runtime-has-weak", fobjc_runtime_has_weak, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "The target Objective-C runtime supports ARC weak operations", 0)
OPTION(prefix_1, "fobjc-runtime=", fobjc_runtime_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Specify the target Objective-C runtime kind and version", 0)
OPTION(prefix_1, "fobjc-sender-dependent-dispatch", fobjc_sender_dependent_dispatch, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fobjc-subscripting-legacy-runtime", fobjc_subscripting_legacy_runtime, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Allow Objective-C array and dictionary subscripting in legacy runtime", 0)
OPTION(prefix_1, "fomit-frame-pointer", fomit_frame_pointer, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fopenmp=", fopenmp_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fopenmp", fopenmp, Flag, f_Group, INVALID, 0, CC1Option | NoArgumentUnused, 0, 0, 0)
OPTION(prefix_1, "foperator-arrow-depth=", foperator_arrow_depth_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "foperator-arrow-depth", foperator_arrow_depth, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Maximum number of 'operator->'s to call for a member access", 0)
OPTION(prefix_1, "foptimize-sibling-calls", foptimize_sibling_calls, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "for-linker=", _for_linker_EQ, Joined, INVALID, Xlinker, 0, 0, 0, 0, 0)
OPTION(prefix_3, "for-linker", _for_linker, Separate, INVALID, Xlinker, 0, 0, 0, 0, 0)
OPTION(prefix_3, "force-link=", _force_link_EQ, Joined, INVALID, u, 0, 0, 0, 0, 0)
OPTION(prefix_3, "force-link", _force_link, Separate, INVALID, u, 0, 0, 0, 0, 0)
OPTION(prefix_1, "force_cpusubtype_ALL", force__cpusubtype__ALL, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "force_flat_namespace", force__flat__namespace, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "force_load", force__load, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "foutput-class-dir=", foutput_class_dir_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "foverride-record-layout=", foverride_record_layout_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Override record layouts with those in the given file", 0)
OPTION(prefix_2, "Fo", _SLASH_Fo, Joined, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set output object file, or directory (ends in / or \\)", "<file or directory>")
OPTION(prefix_1, "fpack-derived", pack_derived_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fpack-struct=", fpack_struct_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Specify the default maximum struct packing alignment", 0)
OPTION(prefix_1, "fpack-struct", fpack_struct, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fparse-all-comments", fparse_all_comments, Flag, f_clang_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fpascal-strings", fpascal_strings, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Recognize and construct Pascal-style string literals", 0)
OPTION(prefix_1, "fpcc-struct-return", fpcc_struct_return, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Override the default ABI to return all structs on the stack", 0)
OPTION(prefix_1, "fpch-preprocess", fpch_preprocess, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fpeel-loops", peel_loops_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fpermissive", permissive_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fPIC", fPIC, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fpic", fpic, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fPIE", fPIE, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fpie", fpie, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprefetch-loop-arrays", prefetch_loop_arrays_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprintf", printf_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-arcs", fprofile_arcs, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-correction", profile_correction_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-dir=", fprofile_dir, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-generate-sampling", profile_generate_sampling_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-generate", fprofile_generate, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-instr-generate", fprofile_instr_generate, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Generate instrumented code to collect execution counts", 0)
OPTION(prefix_1, "fprofile-instr-use=", fprofile_instr_use_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Use instrumentation data for profile-guided optimization", 0)
OPTION(prefix_1, "fprofile-instr-use", fprofile_instr_use, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-reusedist", profile_reusedist_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-sample-use=", fprofile_sample_use_EQ, Joined, f_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Enable sample-based profile guided optimizations", 0)
OPTION(prefix_1, "fprofile-use=", fprofile_use_EQ, Joined, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-use", profile_use_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile-values", profile_values_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprofile", profile_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fprotect-parens", protect_parens_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Fp", _SLASH_Fp, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "fp", _SLASH_fp, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "framework", framework, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_1, "frandom-seed=", frandom_seed_EQ, Joined, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frange-check", range_check_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freal-4-real-10", real_4_real_10_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freal-4-real-16", real_4_real_16_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freal-4-real-8", real_4_real_8_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freal-8-real-10", real_8_real_10_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freal-8-real-16", real_8_real_16_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freal-8-real-4", real_8_real_4_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frealloc-lhs", realloc_lhs_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freciprocal-math", freciprocal_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frecord-marker=", frecord_marker_EQ, Joined, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frecursive", recursive_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freg-struct-return", freg_struct_return, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Override the default ABI to return small structs in registers", 0)
OPTION(prefix_1, "fregs-graph", regs_graph_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frename-registers", rename_registers_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freorder-blocks", reorder_blocks_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frepack-arrays", repack_arrays_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "freroll-loops", freroll_loops, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Turn on loop reroller", 0)
OPTION(prefix_1, "fretain-comments-from-system-headers", fretain_comments_from_system_headers, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "frewrite-includes", frewrite_includes, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fripa", ripa_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frounding-math", rounding_math_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "frtti", frtti, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "FR", _SLASH_FR, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Fr", _SLASH_Fr, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "fsanitize-address-field-padding=", fsanitize_address_field_padding, Joined, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Level of field padding for AddressSanitizer", 0)
OPTION(prefix_1, "fsanitize-blacklist=", fsanitize_blacklist, Joined, f_clang_Group, INVALID, 0, CC1Option | CoreOption, 0,
       "Path to blacklist file for sanitizers", 0)
OPTION(prefix_1, "fsanitize-coverage=", fsanitize_coverage, Joined, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Enable coverage instrumentation for Sanitizers", 0)
OPTION(prefix_1, "fsanitize-link-c++-runtime", fsanitize_link_cxx_runtime, Flag, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsanitize-memory-track-origins=", fsanitize_memory_track_origins_EQ, Joined, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Enable origins tracking in MemorySanitizer", 0)
OPTION(prefix_1, "fsanitize-memory-track-origins", fsanitize_memory_track_origins, Flag, f_clang_Group, INVALID, 0, CC1Option, 0,
       "Enable origins tracking in MemorySanitizer", 0)
OPTION(prefix_1, "fsanitize-recover", fsanitize_recover, Flag, f_clang_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsanitize-undefined-trap-on-error", fsanitize_undefined_trap_on_error, Flag, f_clang_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fsanitize=", fsanitize_EQ, CommaJoined, f_clang_Group, INVALID, 0, CC1Option | CoreOption, 0,
       "Enable runtime instrumentation for bug detection: address (memory errors) | thread (race detection) | undefined (miscellaneous undefined behavior)", "<check>")
OPTION(prefix_1, "fsched-interblock", anonymous_10, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fschedule-insns2", schedule_insns2_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fschedule-insns", schedule_insns_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsecond-underscore", second_underscore_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsee", see_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fshort-enums", fshort_enums, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Allocate to an enum type only as many bytes as it needs for the declared range of possible values", 0)
OPTION(prefix_1, "fshort-wchar", fshort_wchar, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Force wchar_t to be a short unsigned int", 0)
OPTION(prefix_1, "fshow-column", fshow_column, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "fshow-overloads=", fshow_overloads_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Which overload candidates to show when overload resolution fails: best|all; defaults to all", 0)
OPTION(prefix_1, "fshow-source-location", fshow_source_location, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsign-zero", sign_zero_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsignaling-math", fsignaling_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsignaling-nans", signaling_nans_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsigned-bitfields", fsigned_bitfields, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsigned-char", fsigned_char, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsigned-zeros", fsigned_zeros, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsingle-precision-constant", single_precision_constant_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsized-deallocation", fsized_deallocation, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable C++14 sized global deallocation functions", 0)
OPTION(prefix_1, "fsjlj-exceptions", fsjlj_exceptions, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use SjLj style exceptions", 0)
OPTION(prefix_1, "fslp-vectorize-aggressive", fslp_vectorize_aggressive, Flag, f_Group, INVALID, 0, 0, 0,
       "Enable the BB vectorization passes", 0)
OPTION(prefix_1, "fslp-vectorize", fslp_vectorize, Flag, f_Group, INVALID, 0, 0, 0,
       "Enable the superword-level parallelism vectorization passes", 0)
OPTION(prefix_1, "fspec-constr-count", spec_constr_count_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fspell-checking", fspell_checking, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsplit-stack", fsplit_stack, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fstack-arrays", stack_arrays_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fstack-check", stack_check_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fstack-protector-all", fstack_protector_all, Flag, f_Group, INVALID, 0, 0, 0,
       "Force the usage of stack protectors for all functions", 0)
OPTION(prefix_1, "fstack-protector-strong", fstack_protector_strong, Flag, f_Group, INVALID, 0, 0, 0,
       "Use a strong heuristic to apply stack protectors to functions", 0)
OPTION(prefix_1, "fstack-protector", fstack_protector, Flag, f_Group, INVALID, 0, 0, 0,
       "Enable stack protectors for functions potentially vulnerable to stack smashing", 0)
OPTION(prefix_1, "fstandalone-debug", fstandalone_debug, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Emit full debug info for all types used by the program", 0)
OPTION(prefix_1, "fstrength-reduce", strength_reduce_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fstrict-aliasing", fstrict_aliasing, Flag, f_Group, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_1, "fstrict-enums", fstrict_enums, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Enable optimizations based on the strict definition of an enum's value range", 0)
OPTION(prefix_1, "fstrict-overflow", fstrict_overflow, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fstruct-path-tbaa", fstruct_path_tbaa, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fsyntax-only", fsyntax_only, Flag, Action_Group, INVALID, 0, DriverOption | CC1Option, 0, 0, 0)
OPTION(prefix_2, "FS", _SLASH_FS, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0,
       "Force synchronous PDB writes", 0)
OPTION(prefix_1, "ftabstop=", ftabstop_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftabstop", ftabstop, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Set the tab stop distance.", "<N>")
OPTION(prefix_1, "ftemplate-backtrace-limit=", ftemplate_backtrace_limit_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftemplate-backtrace-limit", ftemplate_backtrace_limit, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Set the maximum number of entries to print in a template instantiation backtrace (0 = no limit).", "<N>")
OPTION(prefix_1, "ftemplate-depth-", ftemplate_depth_, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftemplate-depth=", ftemplate_depth_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftemplate-depth", ftemplate_depth, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Maximum depth of recursive template instantiation", 0)
OPTION(prefix_1, "fterminated-vtables", anonymous_15, Flag, INVALID, fapple_kext, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftest-coverage", ftest_coverage, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fthreadsafe-statics", fthreadsafe_statics, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftime-report", ftime_report, Flag, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "ftls-model=", ftlsmodel_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "ftls-model", tls_model_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftracer", tracer_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftrap-function=", ftrap_function_EQ, Joined, f_Group, INVALID, 0, CC1Option, 0,
       "Issue call to specified function rather than a trap instruction", 0)
OPTION(prefix_1, "ftrapping-math", ftrapping_math, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftrapv-handler=", ftrapv_handler_EQ, Joined, f_Group, INVALID, 0, 0, 0,
       "Specify the function to be called on overflow", "<function name>")
OPTION(prefix_1, "ftrapv-handler", ftrapv_handler, Separate, f_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "ftrapv", ftrapv, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Trap on integer overflow", 0)
OPTION(prefix_1, "ftree-dce", tree_dce_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree-salias", tree_salias_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree-slp-vectorize", anonymous_13, Flag, INVALID, fslp_vectorize, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree-ter", tree_ter_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree-vectorizer-verbose", tree_vectorizer_verbose_f, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree-vectorize", anonymous_11, Flag, INVALID, fvectorize, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree-vrp", tree_vrp_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree_loop_im", tree_loop_im_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree_loop_ivcanon", tree_loop_ivcanon_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftree_loop_linear", tree_loop_linear_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ftype-visibility", ftype_visibility, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Default type visibility", 0)
OPTION(prefix_1, "funderscoring", underscoring_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funit-at-a-time", funit_at_a_time, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funknown-anytype", funknown_anytype, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable parser support for the __unknown_anytype type; for testing purposes only", 0)
OPTION(prefix_1, "funroll-all-loops", unroll_all_loops_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funroll-loops", funroll_loops, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Turn on loop unroller", 0)
OPTION(prefix_1, "funsafe-loop-optimizations", unsafe_loop_optimizations_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funsafe-math-optimizations", funsafe_math_optimizations, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funsigned-bitfields", funsigned_bitfields, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funsigned-char", funsigned_char, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funswitch-loops", unswitch_loops_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "funwind-tables", funwind_tables, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fuse-cxa-atexit", fuse_cxa_atexit, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fuse-init-array", fuse_init_array, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Use .init_array instead of .ctors", 0)
OPTION(prefix_1, "fuse-ld=", fuse_ld_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fuse-linker-plugin", use_linker_plugin_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fuse-register-sized-bitfield-access", fuse_register_sized_bitfield_access, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use register sized accesses to bit-fields, when possible.", 0)
OPTION(prefix_2, "FU", _SLASH_FU, JoinedOrSeparate, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "fvariable-expansion-in-unroller", variable_expansion_in_unroller_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fvect-cost-model", vect_cost_model_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fvectorize", fvectorize, Flag, f_Group, INVALID, 0, 0, 0,
       "Enable the loop vectorization passes", 0)
OPTION(prefix_1, "fverbose-asm", fverbose_asm, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fvisibility-inlines-hidden", fvisibility_inlines_hidden, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Give inline C++ member functions default visibility by default", 0)
OPTION(prefix_1, "fvisibility-ms-compat", fvisibility_ms_compat, Flag, f_Group, INVALID, 0, 0, 0,
       "Give global types 'default' visibility and global functions and variables 'hidden' visibility by default", 0)
OPTION(prefix_1, "fvisibility=", fvisibility_EQ, Joined, f_Group, INVALID, 0, 0, 0,
       "Set the default symbol visibility for all global declarations", 0)
OPTION(prefix_1, "fvisibility", fvisibility, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Default type and symbol visibility", 0)
OPTION(prefix_1, "fweb", web_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fwhole-file", whole_file_f, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fwhole-program", whole_program_f, Flag, clang_ignored_gcc_optimization_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fwrapv", fwrapv, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Treat signed integer overflow as two's complement", 0)
OPTION(prefix_1, "fwritable-strings", fwritable_strings, Flag, f_Group, INVALID, 0, CC1Option, 0,
       "Store string literals as writable data", 0)
OPTION(prefix_2, "Fx", _SLASH_Fx, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "fzero-initialized-in-bss", fzero_initialized_in_bss, Flag, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "F", F, JoinedOrSeparate, INVALID, INVALID, 0, RenderJoined | CC1Option, 0,
       "Add directory to framework include search path", 0)
OPTION(prefix_2, "F", _SLASH_F, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "g0", g0, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "G1", _SLASH_G1, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "g1", g1, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "G2", _SLASH_G2, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "g2", g2, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "g3", g3, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "G=", G_EQ, Joined, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_2, "GA", _SLASH_GA, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "gcc-toolchain=", gcc_toolchain, Joined, INVALID, INVALID, 0, DriverOption, 0,
       "Use the gcc toolchain at the given directory", 0)
OPTION(prefix_1, "gcc-toolchain", gcc_toolchain_legacy_spelling, Separate, INVALID, gcc_toolchain, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gcoff", gcoff, Joined, g_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "gcolumn-info", gcolumn_info, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gdwarf-2", gdwarf_2, Flag, g_Group, INVALID, 0, CC1Option | CC1AsOption, 0,
       "Generate source-level debug information with dwarf version 2", 0)
OPTION(prefix_1, "gdwarf-3", gdwarf_3, Flag, g_Group, INVALID, 0, CC1Option | CC1AsOption, 0,
       "Generate source-level debug information with dwarf version 3", 0)
OPTION(prefix_1, "gdwarf-4", gdwarf_4, Flag, g_Group, INVALID, 0, CC1Option | CC1AsOption, 0,
       "Generate source-level debug information with dwarf version 4", 0)
OPTION(prefix_1, "gdwarf-aranges", gdwarf_aranges, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Gd", _SLASH_Gd, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Ge", _SLASH_Ge, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "GF-", _SLASH_GF_, Flag, cl_Group, fwritable_strings, 0, CLOption | DriverOption, 0,
       "Disable string pooling", 0)
OPTION(prefix_1, "gfull", gfull, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "GF", _SLASH_GF, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "ggdb0", ggdb0, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ggdb1", ggdb1, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ggdb2", ggdb2, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ggdb3", ggdb3, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ggdb", ggdb, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "ggnu-pubnames", ggnu_pubnames, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "GH", _SLASH_GH, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Gh", _SLASH_Gh, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "GL-", _SLASH_GL_, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "gline-tables-only", gline_tables_only, Flag, g_Group, INVALID, 0, CC1Option, 0,
       "Emit debug line number tables only", 0)
OPTION(prefix_2, "GL", _SLASH_GL, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Gm-", _SLASH_Gm_, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "gmlt", gmlt, Flag, INVALID, gline_tables_only, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Gm", _SLASH_Gm, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "gno-column-info", gno_column_info, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gno-record-gcc-switches", gno_record_gcc_switches, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gno-strict-dwarf", gno_strict_dwarf, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gnu-pubnames", gnu_pubnames, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit newer GNU style pubnames", 0)
OPTION(prefix_2, "GR-", _SLASH_GR_, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Disable emission of RTTI data", 0)
OPTION(prefix_1, "grecord-gcc-switches", grecord_gcc_switches, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "GR", _SLASH_GR, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Enable emission of RTTI data", 0)
OPTION(prefix_2, "Gr", _SLASH_Gr, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "GS-", _SLASH_GS_, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "gsplit-dwarf", gsplit_dwarf, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gstabs", gstabs, Joined, g_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "gstrict-dwarf", gstrict_dwarf, Flag, g_flags_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "GS", _SLASH_GS, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Gs", _SLASH_Gs, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "gtoggle", gtoggle, Flag, g_flags_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_2, "GT", _SLASH_GT, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "gused", gused, Flag, g_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "gvms", gvms, Joined, g_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_2, "Gw-", _SLASH_Gw_, Flag, cl_Group, fno_data_sections, 0, CLOption | DriverOption, 0,
       "Don't put each data item in its own section", 0)
OPTION(prefix_2, "Gw", _SLASH_Gw, Flag, cl_Group, fdata_sections, 0, CLOption | DriverOption, 0,
       "Put each data item in its own section", 0)
OPTION(prefix_1, "gxcoff", gxcoff, Joined, g_Group, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_2, "GX", _SLASH_GX, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Gy-", _SLASH_Gy_, Flag, cl_Group, fno_function_sections, 0, CLOption | DriverOption, 0,
       "Don't put each function in its own section", 0)
OPTION(prefix_2, "Gy", _SLASH_Gy, Flag, cl_Group, ffunction_sections, 0, CLOption | DriverOption, 0,
       "Put each function in its own section", 0)
OPTION(prefix_2, "GZ", _SLASH_GZ, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Gz", _SLASH_Gz, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "G", G, JoinedOrSeparate, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "g", g_Flag, Flag, g_Group, INVALID, 0, CC1Option | CC1AsOption, 0,
       "Generate source-level debug information", 0)
OPTION(prefix_1, "header-include-file", header_include_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Filename (or -) to write header include output to", 0)
OPTION(prefix_1, "headerpad_max_install_names", headerpad__max__install__names, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "help-hidden", _help_hidden, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "HELP", _SLASH_HELP, Flag, cl_Group, help, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_4, "help", help, Flag, INVALID, INVALID, 0, CC1Option | CC1AsOption, 0,
       "Display available options", 0)
OPTION(prefix_2, "help", _SLASH_help, Flag, cl_Group, help, 0, CLOption | DriverOption, 0,
       "Display available options", 0)
OPTION(prefix_2, "homeparams", _SLASH_homeparams, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "hotpatch", _SLASH_hotpatch, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "H", H, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Show header includes and nesting depth", 0)
OPTION(prefix_2, "H", _SLASH_H, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "I-", I_, Flag, I_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "idirafter", idirafter, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Add directory to AFTER include search path", 0)
OPTION(prefix_1, "iframework", iframework, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Add directory to SYSTEM framework search path", 0)
OPTION(prefix_3, "imacros=", _imacros_EQ, Joined, INVALID, imacros, 0, 0, 0, 0, 0)
OPTION(prefix_4, "imacros", imacros, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Include macros from file before parsing", "<file>")
OPTION(prefix_1, "image_base", image__base, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-barrier", _include_barrier, Flag, INVALID, I_, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-directory-after=", _include_directory_after_EQ, Joined, INVALID, idirafter, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-directory-after", _include_directory_after, Separate, INVALID, idirafter, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-directory=", _include_directory_EQ, Joined, INVALID, I, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-directory", _include_directory, Separate, INVALID, I, 0, 0, 0, 0, 0)
OPTION(prefix_1, "include-pch", include_pch, Separate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Include precompiled header file", "<file>")
OPTION(prefix_3, "include-prefix=", _include_prefix_EQ, Joined, INVALID, iprefix, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-prefix", _include_prefix, Separate, INVALID, iprefix, 0, 0, 0, 0, 0)
OPTION(prefix_1, "include-pth", include_pth, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include file before parsing", "<file>")
OPTION(prefix_3, "include-with-prefix-after=", _include_with_prefix_after_EQ, Joined, INVALID, iwithprefix, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-with-prefix-after", _include_with_prefix_after, Separate, INVALID, iwithprefix, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-with-prefix-before=", _include_with_prefix_before_EQ, Joined, INVALID, iwithprefixbefore, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-with-prefix-before", _include_with_prefix_before, Separate, INVALID, iwithprefixbefore, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-with-prefix=", _include_with_prefix_EQ, Joined, INVALID, iwithprefix, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include-with-prefix", _include_with_prefix, Separate, INVALID, iwithprefix, 0, 0, 0, 0, 0)
OPTION(prefix_3, "include=", _include_EQ, Joined, INVALID, include, 0, 0, 0, 0, 0)
OPTION(prefix_4, "include", include, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Include file before parsing", "<file>")
OPTION(prefix_1, "index-header-map", index_header_map, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Make the next included directory (-I or -F) an indexer header map", 0)
OPTION(prefix_1, "init-only", init_only, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Only execute frontend initialization", 0)
OPTION(prefix_1, "init", init, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "install_name", install__name, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "integrated-as", anonymous_18, Flag, INVALID, fintegrated_as, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "internal-externc-isystem", internal_externc_isystem, JoinedOrSeparate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add directory to the internal system include search path with implicit extern \"C\" semantics; these are assumed to not be user-provided and are used to model system and standard headers' paths.", "<directory>")
OPTION(prefix_1, "internal-isystem", internal_isystem, JoinedOrSeparate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add directory to the internal system include search path; these are assumed to not be user-provided and are used to model system and standard headers' paths.", "<directory>")
OPTION(prefix_1, "iprefix", iprefix, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Set the -iwithprefix/-iwithprefixbefore prefix", "<dir>")
OPTION(prefix_1, "iquote", iquote, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Add directory to QUOTE include search path", "<directory>")
OPTION(prefix_1, "isysroot", isysroot, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Set the system root directory (usually /)", "<dir>")
OPTION(prefix_1, "isystem", isystem, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Add directory to SYSTEM include search path", "<directory>")
OPTION(prefix_1, "ivfsoverlay", ivfsoverlay, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Overlay the virtual filesystem described by file over the real file system", 0)
OPTION(prefix_1, "iwithprefixbefore", iwithprefixbefore, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Set directory to include search path with prefix", "<dir>")
OPTION(prefix_1, "iwithprefix", iwithprefix, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Set directory to SYSTEM include search path with prefix", "<dir>")
OPTION(prefix_1, "iwithsysroot", iwithsysroot, JoinedOrSeparate, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Add directory to SYSTEM include search path, absolute paths are relative to -isysroot", "<directory>")
OPTION(prefix_1, "I", I, JoinedOrSeparate, I_Group, INVALID, 0, CC1Option | CC1AsOption, 0,
       "Add directory to include search path", 0)
OPTION(prefix_2, "I", _SLASH_I, JoinedOrSeparate, cl_Group, I, 0, CLOption | DriverOption, 0,
       "Add directory to include search path", "<dir>")
OPTION(prefix_1, "i", i, Joined, i_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "J", J, JoinedOrSeparate, gfortran_Group, INVALID, 0, RenderJoined, 0, 0, 0)
OPTION(prefix_2, "J", _SLASH_J, Flag, cl_Group, funsigned_char, 0, CLOption | DriverOption, 0,
       "Make char type unsigned", 0)
OPTION(prefix_1, "keep_private_externs", keep__private__externs, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "kernel-", _SLASH_kernel_, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "kernel", _SLASH_kernel, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "language=", _language_EQ, Joined, INVALID, x, 0, 0, 0, 0, 0)
OPTION(prefix_3, "language", _language, Separate, INVALID, x, 0, 0, 0, 0, 0)
OPTION(prefix_1, "lazy_framework", lazy__framework, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_1, "lazy_library", lazy__library, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_2, "LDd", _SLASH_LDd, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Create debug DLL", 0)
OPTION(prefix_2, "LD", _SLASH_LD, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Create DLL", 0)
OPTION(prefix_3, "library-directory=", _library_directory_EQ, Joined, INVALID, L, 0, 0, 0, 0, 0)
OPTION(prefix_3, "library-directory", _library_directory, Separate, INVALID, L, 0, 0, 0, 0, 0)
OPTION(prefix_2, "link", _SLASH_link, RemainingArgs, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Forward options to the linker", "<options>")
OPTION(prefix_2, "LN", _SLASH_LN, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "load", load, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Load the named plugin (dynamic shared object)", "<dsopath>")
OPTION(prefix_1, "L", L, JoinedOrSeparate, INVALID, INVALID, 0, RenderJoined, 0, 0, 0)
OPTION(prefix_1, "l", l, JoinedOrSeparate, INVALID, INVALID, 0, LinkerInput | RenderJoined, 0, 0, 0)
OPTION(prefix_1, "m16", m16, Flag, m_Group, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_1, "m32", m32, Flag, m_Group, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_1, "m3dnowa", m3dnowa, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "m3dnow", m3dnow, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "m64", m64, Flag, m_Group, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_1, "mabi=", mabi_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mabicalls", mabicalls, Flag, m_Group, INVALID, 0, 0, 0,
       "Enable SVR4-style position-independent code (Mips only)", 0)
OPTION(prefix_1, "Mach", Mach, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "madx", madx, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "maes", maes, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "main-file-name", main_file_name, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Main file name to use for debug info", 0)
OPTION(prefix_1, "malign-functions=", malign_functions_EQ, Joined, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "malign-jumps=", malign_jumps_EQ, Joined, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "malign-loops=", malign_loops_EQ, Joined, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "maltivec", maltivec, Flag, INVALID, faltivec, 0, 0, 0, 0, 0)
OPTION(prefix_1, "march=", march_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "marm", marm, Flag, INVALID, mno_thumb, 0, 0, 0, 0, 0)
OPTION(prefix_1, "masm-verbose", masm_verbose, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate verbose assembly output", 0)
OPTION(prefix_1, "masm=", masm_EQ, Joined, m_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "massembler-fatal-warnings", massembler_fatal_warnings, Flag, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Make assembler warnings fatal", 0)
OPTION(prefix_1, "mavx2", mavx2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512bw", mavx512bw, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512cd", mavx512cd, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512dq", mavx512dq, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512er", mavx512er, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512f", mavx512f, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512pf", mavx512pf, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx512vl", mavx512vl, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mavx", mavx, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mbig-endian", mbig_endian, Flag, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "mbmi2", mbmi2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mbmi", mbmi, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mcheck-zero-division", mcheck_zero_division, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mcmodel=", mcmodel_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mcode-model", mcode_model, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "The code model to use", 0)
OPTION(prefix_1, "mconstant-cfstrings", mconstant_cfstrings, Flag, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mconstructor-aliases", mconstructor_aliases, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Emit complete constructors and destructors as aliases when possible", 0)
OPTION(prefix_1, "mcpu=", mcpu_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mcrbits", mcrbits, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mcrc", mcrc, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Allow use of CRC instructions (ARM only)", 0)
OPTION(prefix_1, "mcx16", mcx16, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_2, "MDd", _SLASH_MDd, Flag, _SLASH_M_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Use DLL debug run-time", 0)
OPTION(prefix_1, "mdebug-pass", mdebug_pass, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable additional debug output", 0)
OPTION(prefix_1, "mdisable-fp-elim", mdisable_fp_elim, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable frame pointer elimination optimization", 0)
OPTION(prefix_1, "mdisable-tail-calls", mdisable_tail_calls, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable tail call optimization, keeping the call stack accurate", 0)
OPTION(prefix_1, "mdouble-float", mdouble_float, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mdspr2", mdspr2, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mdsp", mdsp, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mdynamic-no-pic", mdynamic_no_pic, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "MD", MD, Flag, M_Group, INVALID, 0, 0, 0,
       "Write a depfile containing user and system headers", 0)
OPTION(prefix_2, "MD", _SLASH_MD, Flag, _SLASH_M_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Use DLL run-time", 0)
OPTION(prefix_1, "menable-no-infs", menable_no_infinities, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Allow optimization to assume there are no infinities.", 0)
OPTION(prefix_1, "menable-no-nans", menable_no_nans, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Allow optimization to assume there are no NaNs.", 0)
OPTION(prefix_1, "menable-unsafe-fp-math", menable_unsafe_fp_math, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Allow unsafe floating-point math optimizations which may decrease precision", 0)
OPTION(prefix_1, "mf16c", mf16c, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mfancy-math-387", mfancy_math_387, Flag, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mfix-and-continue", mfix_and_continue, Flag, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mfix-cortex-a53-835769", mfix_cortex_a53_835769, Flag, m_aarch64_Features_Group, INVALID, 0, 0, 0,
       "Workaround Cortex-A53 erratum 835769 (AArch64 only)", 0)
OPTION(prefix_1, "mfloat-abi=", mfloat_abi_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mfloat-abi", mfloat_abi, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "The float ABI to use", 0)
OPTION(prefix_1, "mfma4", mfma4, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mfma", mfma, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mfp32", mfp32, Flag, m_Group, INVALID, 0, 0, 0,
       "Use 32-bit floating point registers (MIPS only)", 0)
OPTION(prefix_1, "mfp64", mfp64, Flag, m_Group, INVALID, 0, 0, 0,
       "Use 64-bit floating point registers (MIPS only)", 0)
OPTION(prefix_1, "mfpmath=", mfpmath_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mfpmath", mfpmath, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Which unit to use for fp math", 0)
OPTION(prefix_1, "mfprnd", mfprnd, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mfpu=", mfpu_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mfpxx", mfpxx, Flag, m_Group, INVALID, 0, HelpHidden, 0,
       "Avoid FPU mode dependent operations when used with the O32 ABI", 0)
OPTION(prefix_1, "mfsgsbase", mfsgsbase, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "MF", MF, JoinedOrSeparate, M_Group, INVALID, 0, 0, 0,
       "Write depfile output from -MMD, -MD, -MM, or -M to <file>", "<file>")
OPTION(prefix_1, "mgeneral-regs-only", mgeneral_regs_only, Flag, m_aarch64_Features_Group, INVALID, 0, 0, 0,
       "Generate code which only uses the general purpose registers (AArch64 only)", 0)
OPTION(prefix_1, "mglibc", mglibc, Flag, m_libc_Group, INVALID, 0, HelpHidden, 0, 0, 0)
OPTION(prefix_1, "mglobal-merge", mglobal_merge, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "MG", MG, Flag, M_Group, INVALID, 0, CC1Option, 0,
       "Add missing headers to depfile", 0)
OPTION(prefix_1, "mhard-float", mhard_float, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "mhwdiv=", _mhwdiv_EQ, Joined, INVALID, mhwdiv_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mhwdiv=", mhwdiv_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "mhwdiv", _mhwdiv, Separate, INVALID, mhwdiv_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mieee-fp", mieee_fp, Flag, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mieee-rnd-near", mieee_rnd_near, Flag, m_hexagon_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "migrate", _migrate, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Run the migrator", 0)
OPTION(prefix_1, "migrate", migrate, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Migrate source code", 0)
OPTION(prefix_1, "mimplicit-float", mimplicit_float, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "minline-all-stringops", minline_all_stringops, Flag, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mios-simulator-version-min=", mios_simulator_version_min_EQ, Joined, INVALID, miphoneos_version_min_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mios-version-min=", mios_version_min_EQ, Joined, INVALID, miphoneos_version_min_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "miphoneos-version-min=", miphoneos_version_min_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mips16", mips16, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mips1", mips1, Flag, INVALID, march_EQ, "mips1\0", HelpHidden, 0,
       "Equivalent to -march=mips1", 0)
OPTION(prefix_1, "mips2", mips2, Flag, INVALID, march_EQ, "mips2\0", HelpHidden, 0,
       "Equivalent to -march=mips2", 0)
OPTION(prefix_1, "mips32r2", mips32r2, Flag, INVALID, march_EQ, "mips32r2\0", HelpHidden, 0,
       "Equivalent to -march=mips32r2", 0)
OPTION(prefix_1, "mips32r6", mips32r6, Flag, INVALID, march_EQ, "mips32r6\0", HelpHidden, 0,
       "Equivalent to -march=mips32r6", 0)
OPTION(prefix_1, "mips32", mips32, Flag, INVALID, march_EQ, "mips32\0", HelpHidden, 0,
       "Equivalent to -march=mips32", 0)
OPTION(prefix_1, "mips3", mips3, Flag, INVALID, march_EQ, "mips3\0", HelpHidden, 0,
       "Equivalent to -march=mips3", 0)
OPTION(prefix_1, "mips4", mips4, Flag, INVALID, march_EQ, "mips4\0", HelpHidden, 0,
       "Equivalent to -march=mips4", 0)
OPTION(prefix_1, "mips5", mips5, Flag, INVALID, march_EQ, "mips5\0", HelpHidden, 0,
       "Equivalent to -march=mips5", 0)
OPTION(prefix_1, "mips64r2", mips64r2, Flag, INVALID, march_EQ, "mips64r2\0", HelpHidden, 0,
       "Equivalent to -march=mips64r2", 0)
OPTION(prefix_1, "mips64r6", mips64r6, Flag, INVALID, march_EQ, "mips64r6\0", HelpHidden, 0,
       "Equivalent to -march=mips64r6", 0)
OPTION(prefix_1, "mips64", mips64, Flag, INVALID, march_EQ, "mips64\0", HelpHidden, 0,
       "Equivalent to -march=mips64", 0)
OPTION(prefix_1, "mkernel", mkernel, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mldc1-sdc1", mldc1_sdc1, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mlimit-float-precision", mlimit_float_precision, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Limit float precision to the given value", 0)
OPTION(prefix_1, "mlink-bitcode-file", mlink_bitcode_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Link the given bitcode file before performing optimizations.", 0)
OPTION(prefix_1, "mlinker-version=", mlinker_version_EQ, Joined, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "mlittle-endian", mlittle_endian, Flag, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "mllvm", mllvm, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | CoreOption, 0,
       "Additional arguments to forward to LLVM's option processing", 0)
OPTION(prefix_1, "mlong-calls", mlong_calls, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Generate an indirect jump to enable jumps further than 64M", 0)
OPTION(prefix_1, "mlzcnt", mlzcnt, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mmacosx-version-min=", mmacosx_version_min_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "MMD", MMD, Flag, M_Group, INVALID, 0, 0, 0,
       "Write a depfile containing user headers", 0)
OPTION(prefix_1, "mmfcrf", mmfcrf, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mmicromips", mmicromips, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mmmx", mmmx, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mms-bitfields", mms_bitfields, Flag, m_Group, INVALID, 0, CC1Option, 0,
       "Set the default structure layout to be compatible with the Microsoft compiler standard", 0)
OPTION(prefix_1, "mmsa", mmsa, Flag, m_Group, INVALID, 0, 0, 0,
       "Enable MSA ASE (MIPS only)", 0)
OPTION(prefix_1, "MM", MM, Flag, M_Group, INVALID, 0, 0, 0,
       "Like -MMD, but also implies -E and writes to stdout by default", 0)
OPTION(prefix_1, "mnan=", mnan_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-3dnowa", mno_3dnowa, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-3dnow", mno_3dnow, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-abicalls", mno_abicalls, Flag, m_Group, INVALID, 0, 0, 0,
       "Disable SVR4-style position-independent code (Mips only)", 0)
OPTION(prefix_1, "mno-adx", mno_adx, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-aes", mno_aes, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-altivec", mno_altivec, Flag, INVALID, fno_altivec, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-avx2", mno_avx2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512bw", mno_avx512bw, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512cd", mno_avx512cd, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512dq", mno_avx512dq, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512er", mno_avx512er, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512f", mno_avx512f, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512pf", mno_avx512pf, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx512vl", mno_avx512vl, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-avx", mno_avx, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-bmi2", mno_bmi2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-bmi", mno_bmi, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-check-zero-division", mno_check_zero_division, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-constant-cfstrings", mno_constant_cfstrings, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-crbits", mno_crbits, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-dspr2", mno_dspr2, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-dsp", mno_dsp, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-f16c", mno_f16c, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-fix-cortex-a53-835769", mno_fix_cortex_a53_835769, Flag, m_aarch64_Features_Group, INVALID, 0, 0, 0,
       "Don't workaround Cortex-A53 erratum 835769 (AArch64 only)", 0)
OPTION(prefix_1, "mno-fma4", mno_fma4, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-fma", mno_fma, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-fprnd", mno_fprnd, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-fsgsbase", mno_fsgsbase, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-global-merge", mno_global_merge, Flag, m_Group, INVALID, 0, CC1Option, 0,
       "Disable merging of globals", 0)
OPTION(prefix_1, "mno-implicit-float", mno_implicit_float, Flag, m_Group, INVALID, 0, 0, 0,
       "Don't generate implicit floating point instructions", 0)
OPTION(prefix_1, "mno-inline-all-stringops", mno_inline_all_stringops, Flag, clang_ignored_m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-ldc1-sdc1", mno_ldc1_sdc1, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-long-calls", mno_long_calls, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Restore the default behaviour of not generating long calls", 0)
OPTION(prefix_1, "mno-lzcnt", mno_lzcnt, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-mfcrf", mno_mfcrf, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-micromips", mno_micromips, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-mips16", mno_mips16, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-mmx", mno_mmx, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-msa", mno_msa, Flag, m_Group, INVALID, 0, 0, 0,
       "Disable MSA ASE (MIPS only)", 0)
OPTION(prefix_1, "mno-odd-spreg", mno_odd_spreg, Flag, m_Group, INVALID, 0, HelpHidden, 0,
       "Disable odd single-precision floating point registers", 0)
OPTION(prefix_1, "mno-omit-leaf-frame-pointer", mno_omit_leaf_frame_pointer, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-pascal-strings", mno_pascal_strings, Flag, INVALID, fno_pascal_strings, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-pclmul", mno_pclmul, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-popcntd", mno_popcntd, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-popcnt", mno_popcnt, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-power8-vector", mno_power8_vector, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-prfchw", mno_prfchw, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-qpx", mno_qpx, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-rdrnd", mno_rdrnd, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-rdseed", mno_rdseed, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-red-zone", mno_red_zone, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-relax-all", mno_relax_all, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-restrict-it", mno_restrict_it, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Allow generation of deprecated IT blocks for ARMv8. It is off by default for ARMv8 Thumb mode", 0)
OPTION(prefix_1, "mno-rtd", mno_rtd, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-rtm", mno_rtm, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sha", mno_sha, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-soft-float", mno_soft_float, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-sse2", mno_sse2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sse3", mno_sse3, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sse4.1", mno_sse4_1, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sse4.2", mno_sse4_2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sse4a", mno_sse4a, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sse4", mno_sse4, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-sse", mno_sse, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-ssse3", mno_ssse3, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-stackrealign", mno_stackrealign, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-tbm", mno_tbm, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-thumb", mno_thumb, Flag, m_arm_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-unaligned-access", mno_unaligned_access, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Force all memory accesses to be aligned (AArch32/AArch64 only)", 0)
OPTION(prefix_1, "mno-vsx", mno_vsx, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-warn-nonportable-cfstrings", mno_warn_nonportable_cfstrings, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-xgot", mno_xgot, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mno-xop", mno_xop, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mno-zero-initialized-in-bss", mno_zero_initialized_in_bss, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not put zero initialized data in the BSS", 0)
OPTION(prefix_1, "mnocrc", mnocrc, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Disallow use of CRC instructions (ARM only)", 0)
OPTION(prefix_1, "mnoexecstack", mno_exec_stack, Flag, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Mark the file as not needing an executable stack", 0)
OPTION(prefix_1, "modd-spreg", modd_spreg, Flag, m_Group, INVALID, 0, HelpHidden, 0,
       "Enable odd single-precision floating point registers", 0)
OPTION(prefix_1, "module-dependency-dir", module_dependency_dir, Separate, INVALID, INVALID, 0, CC1Option, 0,
       "Directory to dump module dependencies to", 0)
OPTION(prefix_1, "module-file-deps", module_file_deps, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include module files in dependency output", 0)
OPTION(prefix_1, "module-file-info", module_file_info, Flag, Action_Group, INVALID, 0, DriverOption | CC1Option, 0, 0, 0)
OPTION(prefix_1, "momit-leaf-frame-pointer", momit_leaf_frame_pointer, Flag, m_Group, INVALID, 0, CC1Option, 0,
       "Omit frame pointer setup for leaf functions", 0)
OPTION(prefix_1, "moslib=", moslib_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mpascal-strings", mpascal_strings, Flag, INVALID, fpascal_strings, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mpclmul", mpclmul, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mpopcntd", mpopcntd, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mpopcnt", mpopcnt, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mpower8-vector", mpower8_vector, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mprfchw", mprfchw, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "MP", MP, Flag, M_Group, INVALID, 0, CC1Option, 0,
       "Create phony target for each dependency (other than main file)", 0)
OPTION(prefix_2, "MP", _SLASH_MP, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "mqdsp6-compat", mqdsp6_compat, Flag, m_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Enable hexagon-qdsp6 backward compatibility", 0)
OPTION(prefix_1, "mqpx", mqpx, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "MQ", MQ, JoinedOrSeparate, M_Group, INVALID, 0, CC1Option, 0,
       "Specify name of main file output to quote in depfile", 0)
OPTION(prefix_1, "mrdrnd", mrdrnd, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mrdseed", mrdseed, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mred-zone", mred_zone, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mregparm=", mregparm_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mregparm", mregparm, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Limit the number of registers available for integer arguments", 0)
OPTION(prefix_1, "mrelax-all", mrelax_all, Flag, m_Group, INVALID, 0, CC1Option | CC1AsOption, 0,
       "(integrated-as) Relax all machine instructions", 0)
OPTION(prefix_1, "mrelocation-model", mrelocation_model, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "The relocation model to use", 0)
OPTION(prefix_1, "mrestrict-it", mrestrict_it, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Disallow generation of deprecated IT blocks for ARMv8. It is on by default for ARMv8 Thumb mode.", 0)
OPTION(prefix_1, "mrtd", mrtd, Flag, m_Group, INVALID, 0, CC1Option, 0,
       "Make StdCall calling convention the default", 0)
OPTION(prefix_1, "mrtm", mrtm, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msave-temp-labels", msave_temp_labels, Flag, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Save temporary labels in the symbol table. Note this may change .s semantics and shouldn't generally be used on compiler-generated code.", 0)
OPTION(prefix_1, "msha", msha, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msingle-float", msingle_float, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "msmall-data-threshold=", msmall_data_threshold_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "msoft-float", msoft_float, Flag, m_Group, INVALID, 0, CC1Option, 0,
       "Use software floating point", 0)
OPTION(prefix_1, "msse2", msse2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msse3", msse3, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msse4.1", msse4_1, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msse4.2", msse4_2, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msse4a", msse4a, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msse4", msse4, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "msse", msse, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mssse3", mssse3, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "mstack-alignment=", mstack_alignment, Joined, m_Group, INVALID, 0, CC1Option, 0,
       "Set the stack alignment", 0)
OPTION(prefix_1, "mstackrealign", mstackrealign, Flag, m_Group, INVALID, 0, CC1Option, 0,
       "Force realign the stack at entry to every function", 0)
OPTION(prefix_1, "mstrict-align", mstrict_align, Flag, INVALID, mno_unaligned_access, 0, CC1Option | HelpHidden, 0,
       "Force all memory accesses to be aligned (same as mno-unaligned-access)", 0)
OPTION(prefix_1, "mt-migrate-directory", mt_migrate_directory, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Directory for temporary files produced during ARC or ObjC migration", 0)
OPTION(prefix_1, "mtbm", mtbm, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_2, "MTd", _SLASH_MTd, Flag, _SLASH_M_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Use static debug run-time", 0)
OPTION(prefix_1, "mthread-model", mthread_model, Separate, m_Group, INVALID, 0, CC1Option, 0,
       "The thread model to use, e.g. posix, single (posix by default)", 0)
OPTION(prefix_1, "mthumb", mthumb, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mtune=", mtune_EQ, Joined, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "MT", MT, JoinedOrSeparate, M_Group, INVALID, 0, CC1Option, 0,
       "Specify name of main file output in depfile", 0)
OPTION(prefix_2, "MT", _SLASH_MT, Flag, _SLASH_M_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Use static run-time", 0)
OPTION(prefix_1, "muclibc", muclibc, Flag, m_libc_Group, INVALID, 0, HelpHidden, 0, 0, 0)
OPTION(prefix_1, "multi_module", multi__module, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "multiply_defined_unused", multiply__defined__unused, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "multiply_defined", multiply__defined, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "munaligned-access", munaligned_access, Flag, m_arm_Features_Group, INVALID, 0, 0, 0,
       "Allow memory accesses to be unaligned (AArch32/AArch64 only)", 0)
OPTION(prefix_1, "munwind-tables", munwind_tables, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate unwinding tables for all functions", 0)
OPTION(prefix_1, "mv1", mv1, Flag, m_hexagon_Features_Group, march_EQ, "v1\0", 0, 0, 0, 0)
OPTION(prefix_1, "mv2", mv2, Flag, m_hexagon_Features_Group, march_EQ, "v2\0", 0, 0, 0, 0)
OPTION(prefix_1, "mv3", mv3, Flag, m_hexagon_Features_Group, march_EQ, "v3\0", 0, 0, 0, 0)
OPTION(prefix_1, "mv4", mv4, Flag, m_hexagon_Features_Group, march_EQ, "v4\0", 0, 0, 0, 0)
OPTION(prefix_1, "mv5", mv5, Flag, m_hexagon_Features_Group, march_EQ, "v5\0", 0, 0, 0, 0)
OPTION(prefix_1, "mvsx", mvsx, Flag, m_ppc_Features_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mwarn-nonportable-cfstrings", mwarn_nonportable_cfstrings, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mx32", mx32, Flag, m_Group, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_1, "mxgot", mxgot, Flag, m_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "mxop", mxop, Flag, m_x86_Features_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "M", M, Flag, M_Group, INVALID, 0, 0, 0,
       "Like -MD, but also implies -E and writes to stdout by default", 0)
OPTION(prefix_1, "no-canonical-prefixes", no_canonical_prefixes, Flag, INVALID, INVALID, 0, HelpHidden, 0,
       "Use relative instead of canonical paths", 0)
OPTION(prefix_1, "no-code-completion-globals", no_code_completion_globals, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not include global declarations in code-completion results.", 0)
OPTION(prefix_1, "no-cpp-precomp", no_cpp_precomp, Flag, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "no-finalize-removal", migrator_no_finalize_removal, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not remove finalize method in gc mode", 0)
OPTION(prefix_1, "no-implicit-float", no_implicit_float, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Don't generate implicit floating point instructions", 0)
OPTION(prefix_1, "no-integrated-as", anonymous_19, Flag, INVALID, fno_integrated_as, 0, CC1Option | DriverOption, 0, 0, 0)
OPTION(prefix_4, "no-integrated-cpp", no_integrated_cpp, Flag, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_3, "no-line-commands", _no_line_commands, Flag, INVALID, P, 0, 0, 0, 0, 0)
OPTION(prefix_1, "no-ns-alloc-error", migrator_no_nsalloc_error, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not error on use of NSAllocateCollectable/NSReallocateCollectable", 0)
OPTION(prefix_4, "no-pedantic", no_pedantic, Flag, pedantic_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "no-standard-includes", _no_standard_includes, Flag, INVALID, nostdinc, 0, 0, 0, 0, 0)
OPTION(prefix_3, "no-standard-libraries", _no_standard_libraries, Flag, INVALID, nostdlib, 0, 0, 0, 0, 0)
OPTION(prefix_1, "no-struct-path-tbaa", no_struct_path_tbaa, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Turn off struct-path aware Type Based Alias Analysis", 0)
OPTION(prefix_3, "no-system-header-prefix=", no_system_header_prefix, Joined, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Treat all #include paths starting with <prefix> as not including a system header.", "<prefix>")
OPTION(prefix_3, "no-system-header-prefix", anonymous_17, Separate, INVALID, no_system_header_prefix, 0, 0, 0, 0, 0)
OPTION(prefix_3, "no-undefined", _no_undefined, Flag, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_3, "no-warnings", _no_warnings, Flag, INVALID, w, 0, 0, 0, 0, 0)
OPTION(prefix_1, "no_dead_strip_inits_and_terms", no__dead__strip__inits__and__terms, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nobuiltininc", nobuiltininc, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Disable builtin #include directories", 0)
OPTION(prefix_1, "nocpp", nocpp, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nodefaultlibs", nodefaultlibs, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nofixprebinding", nofixprebinding, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nolibc", nolibc, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "nologo", _SLASH_nologo, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "nomultidefs", nomultidefs, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nopie", nopie, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "noprebind", noprebind, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "noseglinkedit", noseglinkedit, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nostartfiles", nostartfiles, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nostdinc++", nostdincxx, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Disable standard #include directories for the C++ standard library", 0)
OPTION(prefix_1, "nostdinc", nostdinc, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nostdlibinc", nostdlibinc, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nostdlib", nostdlib, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "nostdsysteminc", nostdsysteminc, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Disable standard system #include directories", 0)
OPTION(prefix_1, "n", n, Flag, INVALID, INVALID, 0, CC1AsOption | NoDriverOption, 0,
       "Don't automatically start assembly file with a text section", 0)
OPTION(prefix_1, "O0", O0, Flag, O_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "O4", O4, Flag, O_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "Ob0", _SLASH_Ob0, Flag, cl_Group, fno_inline, 0, CLOption | DriverOption, 0,
       "Disable inlining", 0)
OPTION(prefix_2, "Ob1", _SLASH_Ob1, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Ob2", _SLASH_Ob2, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "ObjC++", ObjCXX, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Treat source input files as Objective-C++ inputs", 0)
OPTION(prefix_1, "objc-isystem", objc_isystem, JoinedOrSeparate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add directory to the ObjC SYSTEM include search path", "<directory>")
OPTION(prefix_1, "objcmt-atomic-property", objcmt_atomic_property, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Make migration to 'atomic' properties", 0)
OPTION(prefix_1, "objcmt-migrate-all", objcmt_migrate_all, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to modern ObjC", 0)
OPTION(prefix_1, "objcmt-migrate-annotation", objcmt_migrate_annotation, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to property and method annotations", 0)
OPTION(prefix_1, "objcmt-migrate-designated-init", objcmt_migrate_designated_init, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to infer NS_DESIGNATED_INITIALIZER for initializer methods", 0)
OPTION(prefix_1, "objcmt-migrate-instancetype", objcmt_migrate_instancetype, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to infer instancetype for method result type", 0)
OPTION(prefix_1, "objcmt-migrate-literals", objcmt_migrate_literals, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to modern ObjC literals", 0)
OPTION(prefix_1, "objcmt-migrate-ns-macros", objcmt_migrate_nsmacros, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to NS_ENUM/NS_OPTIONS macros", 0)
OPTION(prefix_1, "objcmt-migrate-property-dot-syntax", objcmt_migrate_property_dot_syntax, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration of setter/getter messages to property-dot syntax", 0)
OPTION(prefix_1, "objcmt-migrate-property", objcmt_migrate_property, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to modern ObjC property", 0)
OPTION(prefix_1, "objcmt-migrate-protocol-conformance", objcmt_migrate_protocol_conformance, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to add protocol conformance on classes", 0)
OPTION(prefix_1, "objcmt-migrate-readonly-property", objcmt_migrate_readonly_property, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to modern ObjC readonly property", 0)
OPTION(prefix_1, "objcmt-migrate-readwrite-property", objcmt_migrate_readwrite_property, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to modern ObjC readwrite property", 0)
OPTION(prefix_1, "objcmt-migrate-subscripting", objcmt_migrate_subscripting, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to modern ObjC subscripting", 0)
OPTION(prefix_1, "objcmt-ns-nonatomic-iosonly", objcmt_ns_nonatomic_iosonly, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to use NS_NONATOMIC_IOSONLY macro for setting property's 'atomic' attribute", 0)
OPTION(prefix_1, "objcmt-returns-innerpointer-property", objcmt_returns_innerpointer_property, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable migration to annotate property with NS_RETURNS_INNER_POINTER", 0)
OPTION(prefix_1, "objcmt-white-list-dir-path=", anonymous_0, Joined, INVALID, objcmt_whitelist_dir_path, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "objcmt-whitelist-dir-path=", objcmt_whitelist_dir_path, Joined, INVALID, INVALID, 0, CC1Option, 0,
       "Only modify files with a filename contained in the provided directory path", 0)
OPTION(prefix_1, "objcxx-isystem", objcxx_isystem, JoinedOrSeparate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Add directory to the ObjC++ SYSTEM include search path", "<directory>")
OPTION(prefix_1, "ObjC", ObjC, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Treat source input files as Objective-C inputs", 0)
OPTION(prefix_1, "object", object, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Od", _SLASH_Od, Flag, cl_Group, O0, 0, CLOption | DriverOption, 0,
       "Disable optimization", 0)
OPTION(prefix_1, "Ofast", Ofast, Joined, O_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "Oi-", _SLASH_Oi_, Flag, cl_Group, fno_builtin, 0, CLOption | DriverOption, 0,
       "Disable use of builtin functions", 0)
OPTION(prefix_2, "Oi", _SLASH_Oi, Flag, cl_Group, fbuiltin, 0, CLOption | DriverOption, 0,
       "Enable use of builtin functions", 0)
OPTION(prefix_2, "openmp", _SLASH_openmp, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "optimize=", _optimize_EQ, Joined, INVALID, O, 0, 0, 0, 0, 0)
OPTION(prefix_3, "optimize", _optimize, Flag, INVALID, O, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Os", _SLASH_Os, Flag, cl_Group, O, "s\0", CLOption | DriverOption, 0,
       "Optimize for size", 0)
OPTION(prefix_2, "Ot", _SLASH_Ot, Flag, cl_Group, O, "2\0", CLOption | DriverOption, 0,
       "Optimize for speed", 0)
OPTION(prefix_1, "output-asm-variant", output_asm_variant, Separate, INVALID, INVALID, 0, CC1AsOption | NoDriverOption, 0,
       "Select the asm variant index to use for output", 0)
OPTION(prefix_3, "output-class-directory=", _output_class_directory_EQ, Joined, INVALID, foutput_class_dir_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "output-class-directory", _output_class_directory, Separate, INVALID, foutput_class_dir_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "output=", _output_EQ, Joined, INVALID, o, 0, 0, 0, 0, 0)
OPTION(prefix_3, "output", _output, Separate, INVALID, o, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Ox", _SLASH_Ox, Flag, cl_Group, O, "3\0", CLOption | DriverOption, 0,
       "Maximum optimization", 0)
OPTION(prefix_2, "Oy-", _SLASH_Oy_, Flag, cl_Group, fno_omit_frame_pointer, 0, CLOption | DriverOption, 0,
       "Disable frame pointer omission", 0)
OPTION(prefix_2, "Oy", _SLASH_Oy, Flag, cl_Group, fomit_frame_pointer, 0, CLOption | DriverOption, 0,
       "Enable frame pointer omission", 0)
OPTION(prefix_1, "O", O_flag, Flag, INVALID, O, "2\0", CC1Option, 0, 0, 0)
OPTION(prefix_1, "O", O, Joined, O_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "O", _SLASH_O, Joined, cl_Group, O, 0, CLOption | DriverOption, 0,
       "Optimization level", "<n>")
OPTION(prefix_1, "o", o, JoinedOrSeparate, INVALID, INVALID, 0, DriverOption | RenderAsInput | CC1Option | CC1AsOption, 0,
       "Write output to <file>", "<file>")
OPTION(prefix_2, "o", _SLASH_o, JoinedOrSeparate, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set output file or directory (ends in / or \\)", "<file or directory>")
OPTION(prefix_1, "pagezero_size", pagezero__size, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "param=", _param_EQ, Joined, INVALID, _param, 0, 0, 0, 0, 0)
OPTION(prefix_3, "param", _param, Separate, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "pass-exit-codes", pass_exit_codes, Flag, INVALID, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_4, "pedantic-errors", pedantic_errors, Flag, pedantic_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_4, "pedantic", pedantic, Flag, pedantic_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "pg", pg, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable mcount instrumentation", 0)
OPTION(prefix_1, "pic-level", pic_level, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Value for __PIC__", 0)
OPTION(prefix_1, "pie-level", pie_level, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Value for __PIE__", 0)
OPTION(prefix_1, "pie", pie, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "pipe", pipe, Flag, INVALID, INVALID, 0, 0, 0,
       "Use pipes between commands, when possible", 0)
OPTION(prefix_1, "plugin-arg-", plugin_arg, JoinedAndSeparate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Pass <arg> to plugin <name>", "<name> <arg>")
OPTION(prefix_1, "plugin", plugin, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use the named plugin action instead of the default action (use \"help\" to list available options)", "<name>")
OPTION(prefix_1, "preamble-bytes=", preamble_bytes_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Assume that the precompiled header is a precompiled preamble covering the first N bytes of the main file", 0)
OPTION(prefix_1, "prebind_all_twolevel_modules", prebind__all__twolevel__modules, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "prebind", prebind, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "prefix=", _prefix_EQ, Joined, INVALID, B, 0, 0, 0, 0, 0)
OPTION(prefix_3, "prefix", _prefix, Separate, INVALID, B, 0, 0, 0, 0, 0)
OPTION(prefix_1, "preload", preload, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "preprocess", _preprocess, Flag, INVALID, E, 0, 0, 0, 0, 0)
OPTION(prefix_1, "print-decl-contexts", print_decl_contexts, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Print DeclContexts and their Decls", 0)
OPTION(prefix_3, "print-diagnostic-categories", _print_diagnostic_categories, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "print-file-name=", print_file_name_EQ, Joined, INVALID, INVALID, 0, 0, 0,
       "Print the full library path of <file>", "<file>")
OPTION(prefix_3, "print-file-name", _print_file_name, Separate, INVALID, print_file_name_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "print-ivar-layout", print_ivar_layout, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable Objective-C Ivar layout bitmap print trace", 0)
OPTION(prefix_4, "print-libgcc-file-name", print_libgcc_file_name, Flag, INVALID, INVALID, 0, 0, 0,
       "Print the library path for \"libgcc.a\"", 0)
OPTION(prefix_3, "print-missing-file-dependencies", _print_missing_file_dependencies, Flag, INVALID, MG, 0, 0, 0, 0, 0)
OPTION(prefix_4, "print-multi-directory", print_multi_directory, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "print-multi-lib", print_multi_lib, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "print-multi-os-directory", print_multi_os_directory, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "print-preamble", print_preamble, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Print the \"preamble\" of a file, which is a candidate for implicit precompiled headers.", 0)
OPTION(prefix_4, "print-prog-name=", print_prog_name_EQ, Joined, INVALID, INVALID, 0, 0, 0,
       "Print the full program path of <name>", "<name>")
OPTION(prefix_3, "print-prog-name", _print_prog_name, Separate, INVALID, print_prog_name_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_4, "print-search-dirs", print_search_dirs, Flag, INVALID, INVALID, 0, 0, 0,
       "Print the paths used for finding libraries and programs", 0)
OPTION(prefix_1, "print-stats", print_stats, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Print performance metrics and statistics", 0)
OPTION(prefix_1, "private_bundle", private__bundle, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "profile-blocks", _profile_blocks, Flag, INVALID, a, 0, 0, 0, 0, 0)
OPTION(prefix_3, "profile", _profile, Flag, INVALID, p, 0, 0, 0, 0, 0)
OPTION(prefix_1, "pthreads", pthreads, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "pthread", pthread, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Support POSIX threads in generated code", 0)
OPTION(prefix_1, "P", P, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Disable linemarker output in -E mode", 0)
OPTION(prefix_2, "P", _SLASH_P, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Preprocess to file", 0)
OPTION(prefix_1, "p", p, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Qfast_transcendentals", _SLASH_Qfast_transcendentals, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "QIfist", _SLASH_QIfist, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Qimprecise_fwaits", _SLASH_Qimprecise_fwaits, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "Qn", Qn, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "Qpar", _SLASH_Qpar, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "Qunused-arguments", Qunused_arguments, Flag, INVALID, INVALID, 0, DriverOption | CoreOption, 0,
       "Don't emit warning for unused driver arguments", 0)
OPTION(prefix_2, "Qvec-report", _SLASH_Qvec_report, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "Q", Q, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "rdynamic", rdynamic, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "read_only_relocs", read__only__relocs, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "relaxed-aliasing", relaxed_aliasing, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Turn off Type Based Alias Analysis", 0)
OPTION(prefix_4, "relocatable-pch", relocatable_pch, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Whether to build a relocatable precompiled header", 0)
OPTION(prefix_1, "remap-file", remap_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Replace the contents of the <from> file with the contents of the <to> file", "<from>;<to>")
OPTION(prefix_1, "remap", remap, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "resource-dir=", resource_dir_EQ, Joined, INVALID, resource_dir, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "resource-dir", resource_dir, Separate, INVALID, INVALID, 0, DriverOption | CC1Option | HelpHidden, 0,
       "The directory which holds the compiler resource files", 0)
OPTION(prefix_3, "resource=", _resource_EQ, Joined, INVALID, fcompile_resource_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "resource", _resource, Separate, INVALID, fcompile_resource_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "rewrite-legacy-objc", rewrite_legacy_objc, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Rewrite Legacy Objective-C source to C++", 0)
OPTION(prefix_1, "rewrite-macros", rewrite_macros, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Expand macros without full preprocessing", 0)
OPTION(prefix_1, "rewrite-objc", rewrite_objc, Flag, Action_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Rewrite Objective-C source to C++", 0)
OPTION(prefix_1, "rewrite-test", rewrite_test, Flag, Action_Group, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Rewriter playground", 0)
OPTION(prefix_1, "Rpass-analysis=", Rpass_analysis_EQ, Joined, R_value_Group, INVALID, 0, CC1Option, 0,
       "Report transformation analysis from optimization passes whose name matches the given POSIX regular expression", 0)
OPTION(prefix_1, "Rpass-missed=", Rpass_missed_EQ, Joined, R_value_Group, INVALID, 0, CC1Option, 0,
       "Report missed transformations by optimization passes whose name matches the given POSIX regular expression", 0)
OPTION(prefix_1, "Rpass=", Rpass_EQ, Joined, R_value_Group, INVALID, 0, CC1Option, 0,
       "Report transformations performed by optimization passes whose name matches the given POSIX regular expression", 0)
OPTION(prefix_1, "rpath", rpath, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_2, "RTC", _SLASH_RTC, Joined, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_4, "rtlib=", rtlib_EQ, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "rtlib", _rtlib, Separate, INVALID, rtlib_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "R", R_Joined, Joined, R_Group, INVALID, 0, CC1Option | CoreOption, 0,
       "Enable the specified remark", "<remark>")
OPTION(prefix_1, "r", r, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "save-temps", save_temps, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Save intermediate compilation results", 0)
OPTION(prefix_2, "sdl-", _SLASH_sdl_, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "sdl", _SLASH_sdl, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "sectalign", sectalign, MultiArg, INVALID, INVALID, 0, 0, 3, 0, 0)
OPTION(prefix_1, "sectcreate", sectcreate, MultiArg, INVALID, INVALID, 0, 0, 3, 0, 0)
OPTION(prefix_1, "sectobjectsymbols", sectobjectsymbols, MultiArg, INVALID, INVALID, 0, 0, 2, 0, 0)
OPTION(prefix_1, "sectorder", sectorder, MultiArg, INVALID, INVALID, 0, 0, 3, 0, 0)
OPTION(prefix_1, "seg1addr", seg1addr, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "seg_addr_table_filename", seg__addr__table__filename, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "seg_addr_table", seg__addr__table, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "segaddr", segaddr, MultiArg, INVALID, INVALID, 0, 0, 2, 0, 0)
OPTION(prefix_1, "segcreate", segcreate, MultiArg, INVALID, INVALID, 0, 0, 3, 0, 0)
OPTION(prefix_1, "seglinkedit", seglinkedit, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "segprot", segprot, MultiArg, INVALID, INVALID, 0, 0, 3, 0, 0)
OPTION(prefix_1, "segs_read_only_addr", segs__read__only__addr, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "segs_read_write_addr", segs__read__write__addr, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "segs_read_", segs__read__, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "serialize-diagnostic-file", diagnostic_serialized_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "File for serializing diagnostics in a binary format", "<filename>")
OPTION(prefix_4, "serialize-diagnostics", _serialize_diags, Separate, INVALID, INVALID, 0, DriverOption, 0,
       "Serialize compiler diagnostics to a file", 0)
OPTION(prefix_1, "shared-libasan", shared_libasan, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "shared-libgcc", shared_libgcc, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "shared", shared, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "show-encoding", show_encoding, Flag, INVALID, INVALID, 0, CC1AsOption | NoDriverOption, 0,
       "Show instruction encoding information in transliterate mode", 0)
OPTION(prefix_3, "show-includes", show_includes, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Print cl.exe style /showIncludes to stdout", 0)
OPTION(prefix_1, "show-inst", show_inst, Flag, INVALID, INVALID, 0, CC1AsOption | NoDriverOption, 0,
       "Show internal instruction representation in transliterate mode", 0)
OPTION(prefix_2, "showIncludes", _SLASH_showIncludes, Flag, cl_Group, show_includes, 0, CLOption | DriverOption, 0,
       "Print info about included files to stderr", 0)
OPTION(prefix_3, "signed-char", _signed_char, Flag, INVALID, fsigned_char, 0, 0, 0, 0, 0)
OPTION(prefix_1, "single_module", single__module, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "specs=", specs_EQ, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "specs", specs, Separate, INVALID, INVALID, 0, Unsupported, 0, 0, 0)
OPTION(prefix_1, "split-dwarf-file", split_dwarf_file, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "File name to use for split dwarf debug info output", 0)
OPTION(prefix_1, "split-dwarf", split_dwarf, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Split out the dwarf .dwo sections", 0)
OPTION(prefix_1, "split-stacks", split_stacks, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Try to use a split stack if possible.", 0)
OPTION(prefix_1, "stack-protector-buffer-size", stack_protector_buffer_size, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Lower bound for a buffer to be considered for stack protection", 0)
OPTION(prefix_1, "stack-protector", stack_protector, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Enable stack protectors", 0)
OPTION(prefix_1, "static-define", static_define, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Should __STATIC__ be defined", 0)
OPTION(prefix_1, "static-libgcc", static_libgcc, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "static-libgfortran", static_libgfortran, Flag, gfortran_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "static-libstdc++", static_libstdcxx, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "static", static, Flag, INVALID, INVALID, 0, NoArgumentUnused, 0, 0, 0)
OPTION(prefix_1, "std-default=", std_default_EQ, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "std=", std_EQ, Joined, CompileOnly_Group, INVALID, 0, CC1Option, 0,
       "Language standard to compile for", 0)
OPTION(prefix_4, "stdlib=", stdlib_EQ, Joined, INVALID, INVALID, 0, CC1Option, 0,
       "C++ standard library to use", 0)
OPTION(prefix_3, "stdlib", _stdlib, Separate, INVALID, stdlib_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "std", _std, Separate, INVALID, std_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "sub_library", sub__library, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "sub_umbrella", sub__umbrella, JoinedOrSeparate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "sys-header-deps", sys_header_deps, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Include system headers in dependency output", 0)
OPTION(prefix_3, "sysroot=", _sysroot_EQ, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "sysroot", _sysroot, Separate, INVALID, _sysroot_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_3, "system-header-prefix=", system_header_prefix, Joined, clang_i_Group, INVALID, 0, CC1Option, 0,
       "Treat all #include paths starting with <prefix> as including a system header.", "<prefix>")
OPTION(prefix_3, "system-header-prefix", anonymous_16, Separate, INVALID, system_header_prefix, 0, 0, 0, 0, 0)
OPTION(prefix_1, "S", S, Flag, Action_Group, INVALID, 0, DriverOption | CC1Option, 0,
       "Only run preprocess and compilation steps", 0)
OPTION(prefix_1, "s", s, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "target-abi", target_abi, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Target a particular ABI type", 0)
OPTION(prefix_1, "target-cpu", target_cpu, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Target a specific cpu type", 0)
OPTION(prefix_1, "target-feature", target_feature, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Target specific attributes", 0)
OPTION(prefix_3, "target-help", _target_help, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "target-linker-version", target_linker_version, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Target linker version", 0)
OPTION(prefix_3, "target=", target, Joined, INVALID, INVALID, 0, DriverOption | CoreOption, 0,
       "Generate code for the given target", 0)
OPTION(prefix_1, "target", target_legacy_spelling, Separate, INVALID, target, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Tbss", Tbss, JoinedOrSeparate, T_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "TC", _SLASH_TC, Flag, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Treat all source files as C", 0)
OPTION(prefix_2, "Tc", _SLASH_Tc, JoinedOrSeparate, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Specify a C source file", "<filename>")
OPTION(prefix_1, "Tdata", Tdata, JoinedOrSeparate, T_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "test-coverage", test_coverage, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Do not generate coverage files or remove coverage changes from IR", 0)
OPTION(prefix_1, "time", time, Flag, INVALID, INVALID, 0, 0, 0,
       "Time individual commands", 0)
OPTION(prefix_1, "token-cache", token_cache, Separate, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Use specified token cache file", "<path>")
OPTION(prefix_2, "TP", _SLASH_TP, Flag, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Treat all source files as C++", 0)
OPTION(prefix_2, "Tp", _SLASH_Tp, JoinedOrSeparate, cl_compile_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Specify a C++ source file", "<filename>")
OPTION(prefix_3, "trace-includes", _trace_includes, Flag, INVALID, H, 0, 0, 0, 0, 0)
OPTION(prefix_4, "traditional-cpp", traditional_cpp, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Enable some traditional CPP emulation", 0)
OPTION(prefix_4, "traditional", traditional, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_4, "trigraphs", trigraphs, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Process trigraph sequences", 0)
OPTION(prefix_1, "trim-egraph", trim_egraph, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Only show error-related paths in the analysis graph", 0)
OPTION(prefix_1, "triple=", triple_EQ, Joined, INVALID, triple, 0, CC1Option | NoDriverOption, 0, 0, 0)
OPTION(prefix_1, "triple", triple, Separate, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Specify target triple (e.g. i686-apple-darwin9)", 0)
OPTION(prefix_1, "Ttext", Ttext, JoinedOrSeparate, T_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "twolevel_namespace_hints", twolevel__namespace__hints, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "twolevel_namespace", twolevel__namespace, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "T", T, JoinedOrSeparate, T_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "t", t, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "umbrella", umbrella, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "undefine-macro=", _undefine_macro_EQ, Joined, INVALID, U, 0, 0, 0, 0, 0)
OPTION(prefix_3, "undefine-macro", _undefine_macro, Separate, INVALID, U, 0, 0, 0, 0, 0)
OPTION(prefix_1, "undefined", undefined, JoinedOrSeparate, u_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "undef", undef, Flag, u_Group, INVALID, 0, CC1Option, 0,
       "undef all system defines", 0)
OPTION(prefix_1, "unexported_symbols_list", unexported__symbols__list, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "unoptimized-cfg", analysis_UnoptimizedCFG, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Generate unoptimized CFGs for all analyses", 0)
OPTION(prefix_3, "unsigned-char", _unsigned_char, Flag, INVALID, funsigned_char, 0, 0, 0, 0, 0)
OPTION(prefix_3, "user-dependencies", _user_dependencies, Flag, INVALID, MM, 0, 0, 0, 0, 0)
OPTION(prefix_1, "U", U, JoinedOrSeparate, CompileOnly_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "U", _SLASH_U, JoinedOrSeparate, cl_Group, U, 0, CLOption | DriverOption, 0,
       "Undefine macro", "<macro>")
OPTION(prefix_1, "u", u, JoinedOrSeparate, u_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "u", _SLASH_u, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "vd", _SLASH_vd, Joined, cl_Group, vtordisp_mode_EQ, 0, CLOption | DriverOption, 0,
       "Control vtordisp placement", 0)
OPTION(prefix_1, "vectorize-loops", vectorize_loops, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Run the Loop vectorization passes", 0)
OPTION(prefix_1, "vectorize-slp-aggressive", vectorize_slp_aggressive, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Run the BB vectorization passes", 0)
OPTION(prefix_1, "vectorize-slp", vectorize_slp, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Run the SLP vectorization passes", 0)
OPTION(prefix_3, "verbose", _verbose, Flag, INVALID, v, 0, 0, 0, 0, 0)
OPTION(prefix_3, "verify-debug-info", verify_debug_info, Flag, INVALID, INVALID, 0, DriverOption, 0,
       "Verify the binary representation of debug output", 0)
OPTION(prefix_1, "verify-pch", verify_pch, Flag, Action_Group, INVALID, 0, CC1Option, 0,
       "Load and verify that a pre-compiled header file is not stale", 0)
OPTION(prefix_1, "verify", verify, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Verify diagnostic output using comment directives", 0)
OPTION(prefix_3, "version", _version, Flag, INVALID, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "version", version, Flag, INVALID, INVALID, 0, CC1Option | CC1AsOption | NoDriverOption, 0,
       "Print the compiler version", 0)
OPTION(prefix_4, "via-file-asm", via_file_asm, Flag, internal_debug_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "Write assembly to file for input to assemble jobs", 0)
OPTION(prefix_2, "vmb", _SLASH_vmb, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Use a best-case representation method for member pointers", 0)
OPTION(prefix_2, "vmg", _SLASH_vmg, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Use a most-general representation for member pointers", 0)
OPTION(prefix_2, "vmm", _SLASH_vmm, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set the default most-general representation to multiple inheritance", 0)
OPTION(prefix_2, "vms", _SLASH_vms, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set the default most-general representation to single inheritance", 0)
OPTION(prefix_2, "vmv", _SLASH_vmv, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0,
       "Set the default most-general representation to virtual inheritance", 0)
OPTION(prefix_2, "volatile", _SLASH_volatile, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "vtordisp-mode=", vtordisp_mode_EQ, Joined, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Control vtordisp placement on win32 targets", 0)
OPTION(prefix_1, "V", V, JoinedOrSeparate, INVALID, INVALID, 0, DriverOption | Unsupported, 0, 0, 0)
OPTION(prefix_2, "V", _SLASH_V, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "v", v, Flag, INVALID, INVALID, 0, CC1Option | CoreOption, 0,
       "Show commands to run and use verbose output", 0)
OPTION(prefix_2, "W0", _SLASH_W0, Flag, cl_Group, w, 0, CLOption | DriverOption, 0,
       "Disable all warnings", 0)
OPTION(prefix_2, "W1", _SLASH_W1, Flag, cl_Group, Wall, 0, CLOption | DriverOption, 0,
       "Enable -Wall", 0)
OPTION(prefix_2, "W2", _SLASH_W2, Flag, cl_Group, Wall, 0, CLOption | DriverOption, 0,
       "Enable -Wall", 0)
OPTION(prefix_2, "W3", _SLASH_W3, Flag, cl_Group, Wall, 0, CLOption | DriverOption, 0,
       "Enable -Wall", 0)
OPTION(prefix_2, "W4", _SLASH_W4, Flag, cl_Group, Wall, 0, CLOption | DriverOption, 0,
       "Enable -Wall", 0)
OPTION(prefix_1, "Wa,", Wa_COMMA, CommaJoined, INVALID, INVALID, 0, 0, 0,
       "Pass the comma separated arguments in <arg> to the assembler", "<arg>")
OPTION(prefix_1, "Wall", Wall, Flag, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "Wall", _SLASH_Wall, Flag, cl_Group, Wall, 0, CLOption | DriverOption, 0,
       "Enable -Wall", 0)
OPTION(prefix_3, "warn-=", _warn__EQ, Joined, INVALID, W_Joined, 0, 0, 0, 0, 0)
OPTION(prefix_3, "warn-", _warn_, Joined, INVALID, W_Joined, 0, 0, 0, 0, 0)
OPTION(prefix_2, "wd4005", _SLASH_wd4005, Flag, cl_Group, W_Joined, "no-macro-redefined\0", CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "Wdeprecated", Wdeprecated, Flag, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "weak-l", weak_l, Joined, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_1, "weak_framework", weak__framework, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_1, "weak_library", weak__library, Separate, INVALID, INVALID, 0, LinkerInput, 0, 0, 0)
OPTION(prefix_1, "weak_reference_mismatches", weak__reference__mismatches, Separate, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Wextra", Wextra, Flag, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "Wframe-larger-than=", Wframe_larger_than_EQ, Joined, f_Group, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "whatsloaded", whatsloaded, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "whyload", whyload, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Wl,", Wl_COMMA, CommaJoined, INVALID, INVALID, 0, LinkerInput | RenderAsInput, 0,
       "Pass the comma separated arguments in <arg> to the linker", "<arg>")
OPTION(prefix_1, "Wlarge-by-value-copy=", Wlarge_by_value_copy_EQ, Joined, INVALID, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "Wlarge-by-value-copy", Wlarge_by_value_copy_def, Flag, INVALID, INVALID, 0, HelpHidden, 0,
       "Warn if a function definition returns or accepts an object larger in bytes than a given value", 0)
OPTION(prefix_1, "Wlarger-than-", Wlarger_than_, Joined, INVALID, Wlarger_than_EQ, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Wlarger-than=", Wlarger_than_EQ, Joined, clang_ignored_f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "WL", _SLASH_WL, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "Wno-deprecated", Wno_deprecated, Flag, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "Wno-nonportable-cfstrings", Wno_nonportable_cfstrings, Joined, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "Wno-rewrite-macros", Wno_rewrite_macros, Flag, INVALID, INVALID, 0, CC1Option | NoDriverOption, 0,
       "Silence ObjC rewriting warnings", 0)
OPTION(prefix_1, "Wno-write-strings", Wno_write_strings, Flag, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "Wnonportable-cfstrings", Wnonportable_cfstrings, Joined, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "working-directory=", working_directory_EQ, Joined, INVALID, working_directory, 0, CC1Option, 0, 0, 0)
OPTION(prefix_1, "working-directory", working_directory, JoinedOrSeparate, INVALID, INVALID, 0, CC1Option, 0,
       "Resolve file paths relative to the specified directory", 0)
OPTION(prefix_1, "Wp,", Wp_COMMA, CommaJoined, INVALID, INVALID, 0, 0, 0,
       "Pass the comma separated arguments in <arg> to the preprocessor", "<arg>")
OPTION(prefix_2, "Wp64", _SLASH_Wp64, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "write-dependencies", _write_dependencies, Flag, INVALID, MD, 0, 0, 0, 0, 0)
OPTION(prefix_3, "write-user-dependencies", _write_user_dependencies, Flag, INVALID, MMD, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Wwrite-strings", Wwrite_strings, Flag, W_Group, INVALID, 0, CC1Option, 0, 0, 0)
OPTION(prefix_2, "WX-", _SLASH_WX_, Flag, cl_Group, W_Joined, "no-error\0", CLOption | DriverOption, 0,
       "Do not treat warnings as errors", 0)
OPTION(prefix_2, "WX", _SLASH_WX, Flag, cl_Group, W_Joined, "error\0", CLOption | DriverOption, 0,
       "Treat warnings as errors", 0)
OPTION(prefix_1, "W", W_Joined, Joined, W_Group, INVALID, 0, CC1Option | CoreOption, 0,
       "Enable the specified warning", "<warning>")
OPTION(prefix_1, "w", w, Flag, INVALID, INVALID, 0, CC1Option, 0,
       "Suppress all warnings", 0)
OPTION(prefix_2, "w", _SLASH_w_flag, Flag, cl_Group, w, 0, CLOption | DriverOption, 0,
       "Disable all warnings", 0)
OPTION(prefix_2, "w", _SLASH_w, Joined, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_1, "Xanalyzer", Xanalyzer, Separate, INVALID, INVALID, 0, 0, 0,
       "Pass <arg> to the static analyzer", "<arg>")
OPTION(prefix_1, "Xarch_", Xarch__, JoinedAndSeparate, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_1, "Xassembler", Xassembler, Separate, INVALID, INVALID, 0, 0, 0,
       "Pass <arg> to the assembler", "<arg>")
OPTION(prefix_1, "Xclang", Xclang, Separate, INVALID, INVALID, 0, DriverOption | CoreOption, 0,
       "Pass <arg> to the clang compiler", "<arg>")
OPTION(prefix_1, "Xlinker", Xlinker, Separate, INVALID, INVALID, 0, LinkerInput | RenderAsInput, 0,
       "Pass <arg> to the linker", "<arg>")
OPTION(prefix_1, "Xpreprocessor", Xpreprocessor, Separate, INVALID, INVALID, 0, 0, 0,
       "Pass <arg> to the preprocessor", "<arg>")
OPTION(prefix_1, "X", X_Flag, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "X", X_Joined, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_2, "X", _SLASH_X, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "x", x, JoinedOrSeparate, INVALID, INVALID, 0, DriverOption | CC1Option, 0,
       "Treat subsequent input files as having type <language>", "<language>")
OPTION(prefix_2, "Y-", _SLASH_Y_, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Yc", _SLASH_Yc, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Yd", _SLASH_Yd, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Yl", _SLASH_Yl, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Yu", _SLASH_Yu, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "y", y, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Z-reserved-lib-cckext", Z_reserved_lib_cckext, Flag, reserved_lib_Group, INVALID, 0, LinkerInput | NoArgumentUnused | Unsupported, 0, 0, 0)
OPTION(prefix_1, "Z-reserved-lib-stdc++", Z_reserved_lib_stdcxx, Flag, reserved_lib_Group, INVALID, 0, LinkerInput | NoArgumentUnused | Unsupported, 0, 0, 0)
OPTION(prefix_1, "Z-Xlinker-no-demangle", Z_Xlinker__no_demangle, Flag, INVALID, INVALID, 0, Unsupported | NoArgumentUnused, 0, 0, 0)
OPTION(prefix_2, "Z7", _SLASH_Z7, Flag, cl_Group, gline_tables_only, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Za", _SLASH_Za, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Zc:forScope", _SLASH_Zc_forScope, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zc:inline", _SLASH_Zc_inline, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zc:rvalueCast", _SLASH_Zc_rvalueCast, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zc:wchar_t", _SLASH_Zc_wchar_t, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zc:", _SLASH_Zc, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Ze", _SLASH_Ze, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Zg", _SLASH_Zg, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "ZI", _SLASH_ZI, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Zi", _SLASH_Zi, Flag, cl_Group, gline_tables_only, 0, CLOption | DriverOption, 0,
       "Enable debug information", 0)
OPTION(prefix_1, "Zlinker-input", Zlinker_input, Separate, INVALID, INVALID, 0, Unsupported | NoArgumentUnused, 0, 0, 0)
OPTION(prefix_2, "Zl", _SLASH_Zl, Flag, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_2, "Zm", _SLASH_Zm, Joined, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zo-", _SLASH_Zo_, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zo", _SLASH_Zo, Flag, cl_ignored_Group, INVALID, 0, CLOption | DriverOption | HelpHidden, 0, 0, 0)
OPTION(prefix_2, "Zp", _SLASH_Zp_flag, Flag, cl_Group, fpack_struct_EQ, "1\0", CLOption | DriverOption, 0,
       "Set the default maximum struct packing alignment to 1", 0)
OPTION(prefix_2, "Zp", _SLASH_Zp, Joined, cl_Group, fpack_struct_EQ, 0, CLOption | DriverOption, 0,
       "Specify the default maximum struct packing alignment", 0)
OPTION(prefix_2, "Zs", _SLASH_Zs, Flag, cl_Group, fsyntax_only, 0, CLOption | DriverOption, 0,
       "Syntax-check only", 0)
OPTION(prefix_2, "ZW", _SLASH_ZW, Joined, cl_Group, INVALID, 0, CLOption | DriverOption, 0, 0, 0)
OPTION(prefix_1, "Z", Z_Flag, Flag, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Z", Z_Joined, Joined, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "z", z, Separate, INVALID, INVALID, 0, LinkerInput | RenderAsInput, 0,
       "Pass -z <arg> to the linker", "<arg>")
OPTION(prefix_3, "", _DASH_DASH, RemainingArgs, INVALID, INVALID, 0, DriverOption | CoreOption, 0, 0, 0)
OPTION(prefix_3, "", _, Joined, INVALID, INVALID, 0, Unsupported, 0, 0, 0)
#endif
