def generated_llvm_build_targets(ctx):
    ctx.group(ctx, name = "Miscellaneous", parent = "$ROOT")
    ctx.group(ctx, name = "Bindings", parent = "$ROOT")
    ctx.group(ctx, name = "Docs", parent = "$ROOT")
    ctx.group(ctx, parent = "$ROOT", name = "Examples")
    ctx.group(ctx, parent = "$ROOT", name = "Libraries")
    ctx.library(ctx, parent = "Libraries", required_libraries = ["BinaryFormat", "Core", "Object", "ProfileData", "Support"], name = "Analysis")
    ctx.library(ctx, name = "AsmParser", parent = "Libraries", required_libraries = ["BinaryFormat", "Core", "Support"])
    ctx.group(ctx, name = "Bitcode", parent = "Libraries")
    ctx.library(ctx, name = "BitReader", parent = "Bitcode", required_libraries = ["BitstreamReader", "Core", "Support"])
    ctx.library(ctx, parent = "Bitcode", required_libraries = ["Analysis", "Core", "MC", "Object", "Support"], name = "BitWriter")
    ctx.group(ctx, name = "Bitstream", parent = "Libraries")
    ctx.library(ctx, name = "BitstreamReader", parent = "Bitstream", required_libraries = ["Support"])
    ctx.library(ctx, name = "CodeGen", parent = "Libraries", required_libraries = ["Analysis", "BitReader", "BitWriter", "Core", "MC", "ProfileData", "Scalar", "Support", "Target", "TransformUtils"])
    ctx.library(ctx, required_libraries = ["Analysis", "BinaryFormat", "CodeGen", "Core", "DebugInfoCodeView", "DebugInfoDWARF", "DebugInfoMSF", "MC", "MCParser", "Remarks", "Support", "Target"], name = "AsmPrinter", parent = "Libraries")
    ctx.library(ctx, parent = "CodeGen", required_libraries = ["Analysis", "CodeGen", "Core", "MC", "Support", "Target", "TransformUtils"], name = "SelectionDAG")
    ctx.library(ctx, required_libraries = ["AsmParser", "BinaryFormat", "CodeGen", "Core", "MC", "Support", "Target"], name = "MIRParser", parent = "CodeGen")
    ctx.library(ctx, name = "GlobalISel", parent = "CodeGen", required_libraries = ["Analysis", "CodeGen", "Core", "MC", "SelectionDAG", "Support", "Target", "TransformUtils"])
    ctx.group(ctx, parent = "$ROOT", name = "DebugInfo")
    ctx.library(ctx, required_libraries = ["BinaryFormat", "Object", "MC", "Support"], name = "DebugInfoDWARF", parent = "DebugInfo")
    ctx.library(ctx, name = "DebugInfoGSYM", parent = "DebugInfo", required_libraries = ["MC", "Object", "Support", "DebugInfoDWARF"])
    ctx.library(ctx, name = "DebugInfoMSF", parent = "DebugInfo", required_libraries = ["Support"])
    ctx.library(ctx, name = "DebugInfoCodeView", parent = "DebugInfo", required_libraries = ["Support", "DebugInfoMSF"])
    ctx.library(ctx, required_libraries = ["Object", "Support", "DebugInfoCodeView", "DebugInfoMSF"], name = "DebugInfoPDB", parent = "DebugInfo")
    ctx.library(ctx, required_libraries = ["DebugInfoDWARF", "DebugInfoPDB", "Object", "Support", "Demangle"], name = "Symbolize", parent = "DebugInfo")
    ctx.library(ctx, name = "Demangle", parent = "Libraries")
    ctx.library(ctx, required_libraries = ["DebugInfoDWARF", "AsmPrinter", "CodeGen", "MC", "Object", "Support"], name = "DWARFLinker", parent = "Libraries")
    ctx.library(ctx, required_libraries = ["Core", "MC", "Object", "RuntimeDyld", "Support", "Target"], name = "ExecutionEngine", parent = "Libraries")
    ctx.library(ctx, name = "Interpreter", parent = "ExecutionEngine", required_libraries = ["CodeGen", "Core", "ExecutionEngine", "Support"])
    ctx.library(ctx, parent = "ExecutionEngine", required_libraries = ["Core", "ExecutionEngine", "Object", "RuntimeDyld", "Support", "Target"], name = "MCJIT")
    ctx.library(ctx, parent = "ExecutionEngine", required_libraries = ["BinaryFormat", "Object", "Support"], name = "JITLink")
    ctx.library(ctx, name = "RuntimeDyld", parent = "ExecutionEngine", required_libraries = ["MC", "Object", "Support"])
    ctx.optional_library(ctx, required_libraries = ["CodeGen", "Core", "DebugInfoDWARF", "Support", "Object", "ExecutionEngine"], name = "IntelJITEvents", parent = "ExecutionEngine")
    ctx.optional_library(ctx, parent = "ExecutionEngine", required_libraries = ["DebugInfoDWARF", "Support", "Object", "ExecutionEngine"], name = "OProfileJIT")
    ctx.library(ctx, required_libraries = ["Core", "ExecutionEngine", "JITLink", "Object", "OrcError", "MC", "Passes", "RuntimeDyld", "Support", "Target", "TransformUtils"], name = "OrcJIT", parent = "ExecutionEngine")
    ctx.library(ctx, name = "OrcError", parent = "ExecutionEngine", required_libraries = ["Support"])
    ctx.optional_library(ctx, required_libraries = ["CodeGen", "Core", "DebugInfoDWARF", "ExecutionEngine", "Object", "Support"], name = "PerfJITEvents", parent = "ExecutionEngine")
    ctx.group(ctx, name = "Frontend", parent = "Libraries")
    ctx.library(ctx, name = "FrontendOpenMP", parent = "Frontend", required_libraries = ["Core", "Support", "TransformUtils"])
    ctx.library(ctx, name = "FuzzMutate", parent = "Libraries", required_libraries = ["Analysis", "BitReader", "BitWriter", "Core", "Scalar", "Support", "Target"])
    ctx.library(ctx, required_libraries = ["Support"], name = "LineEditor", parent = "Libraries")
    ctx.library(ctx, required_libraries = ["Core", "Support", "TransformUtils"], name = "Linker", parent = "Libraries")
    ctx.library(ctx, required_libraries = ["BinaryFormat", "Remarks", "Support"], name = "Core", parent = "Libraries")
    ctx.library(ctx, name = "IRReader", parent = "Libraries", required_libraries = ["AsmParser", "BitReader", "Core", "Support"])
    ctx.library(ctx, name = "LTO", parent = "Libraries", required_libraries = ["AggressiveInstCombine", "Analysis", "BinaryFormat", "BitReader", "BitWriter", "CodeGen", "Core", "IPO", "InstCombine", "Linker", "MC", "ObjCARC", "Object", "Passes", "Remarks", "Scalar", "Support", "Target", "TransformUtils"])
    ctx.library(ctx, parent = "Libraries", required_libraries = ["Support", "BinaryFormat", "DebugInfoCodeView"], name = "MC")
    ctx.library(ctx, parent = "MC", required_libraries = ["MC", "Support"], name = "MCDisassembler")
    ctx.library(ctx, name = "MCParser", parent = "MC", required_libraries = ["MC", "Support"])
    ctx.library(ctx, name = "MCA", parent = "Libraries", required_libraries = ["MC", "Support"])
    ctx.library(ctx, name = "Object", parent = "Libraries", required_libraries = ["BitReader", "Core", "MC", "BinaryFormat", "MCParser", "Support", "TextAPI"])
    ctx.library(ctx, required_libraries = ["Support"], name = "BinaryFormat", parent = "Libraries")
    ctx.library(ctx, parent = "Libraries", required_libraries = ["Object", "Support", "DebugInfoCodeView", "MC"], name = "ObjectYAML")
    ctx.library(ctx, name = "Option", parent = "Libraries", required_libraries = ["Support"])
    ctx.library(ctx, name = "Remarks", parent = "Libraries", required_libraries = ["BitstreamReader", "Support"])
    ctx.library(ctx, name = "Passes", parent = "Libraries", required_libraries = ["AggressiveInstCombine", "Analysis", "CodeGen", "Core", "Coroutines", "IPO", "InstCombine", "Scalar", "Support", "Target", "TransformUtils", "Vectorize", "Instrumentation"])
    ctx.library(ctx, name = "ProfileData", parent = "Libraries", required_libraries = ["Core", "Support"])
    ctx.library(ctx, parent = "ProfileData", required_libraries = ["Core", "Object", "ProfileData", "Support"], name = "Coverage")
    ctx.library(ctx, name = "Support", parent = "Libraries", required_libraries = ["Demangle"])
    ctx.library(ctx, required_libraries = ["Support"], name = "TableGen", parent = "Libraries")
    ctx.library(ctx, required_libraries = ["Support", "BinaryFormat"], name = "TextAPI", parent = "Libraries")
    ctx.library_group(ctx, name = "Engine", parent = "Libraries")
    ctx.library_group(ctx, parent = "Libraries", name = "Native")
    ctx.library_group(ctx, name = "NativeCodeGen", parent = "Libraries")
    ctx.library(ctx, name = "Target", parent = "Libraries", required_libraries = ["Analysis", "Core", "MC", "Support"])
    ctx.library_group(ctx, name = "all-targets", parent = "Libraries")
    ctx.library(ctx, name = "AArch64CodeGen", parent = "AArch64", required_libraries = ["AArch64Desc", "AArch64Info", "AArch64Utils", "Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "Scalar", "SelectionDAG", "Support", "Target", "TransformUtils", "GlobalISel", "CFGuard"], add_to_library_groups = ["AArch64"])
    ctx.target_group(ctx, name = "AArch64", parent = "Target")
    ctx.library(ctx, name = "AArch64AsmParser", parent = "AArch64", required_libraries = ["AArch64Desc", "AArch64Info", "AArch64Utils", "MC", "MCParser", "Support"], add_to_library_groups = ["AArch64"])
    ctx.library(ctx, add_to_library_groups = ["AArch64"], name = "AArch64Disassembler", parent = "AArch64", required_libraries = ["AArch64Desc", "AArch64Info", "AArch64Utils", "MC", "MCDisassembler", "Support"])
    ctx.library(ctx, name = "AArch64Desc", parent = "AArch64", required_libraries = ["AArch64Info", "AArch64Utils", "MC", "BinaryFormat", "Support"], add_to_library_groups = ["AArch64"])
    ctx.library(ctx, add_to_library_groups = ["AArch64"], name = "AArch64Info", parent = "AArch64", required_libraries = ["Support"])
    ctx.library(ctx, name = "AArch64Utils", parent = "AArch64", required_libraries = ["Support"], add_to_library_groups = ["AArch64"])
    ctx.target_group(ctx, name = "AMDGPU", parent = "Target")
    ctx.library(ctx, add_to_library_groups = ["AMDGPU"], name = "AMDGPUCodeGen", parent = "AMDGPU", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "IPO", "MC", "AMDGPUDesc", "AMDGPUInfo", "AMDGPUUtils", "Scalar", "SelectionDAG", "Support", "Target", "TransformUtils", "Vectorize", "GlobalISel", "BinaryFormat", "MIRParser"])
    ctx.library(ctx, parent = "AMDGPU", required_libraries = ["MC", "MCParser", "AMDGPUDesc", "AMDGPUInfo", "AMDGPUUtils", "Support"], add_to_library_groups = ["AMDGPU"], name = "AMDGPUAsmParser")
    ctx.library(ctx, add_to_library_groups = ["AMDGPU"], name = "AMDGPUDisassembler", parent = "AMDGPU", required_libraries = ["AMDGPUDesc", "AMDGPUInfo", "AMDGPUUtils", "MC", "MCDisassembler", "Support"])
    ctx.library(ctx, add_to_library_groups = ["AMDGPU"], name = "AMDGPUDesc", parent = "AMDGPU", required_libraries = ["Core", "MC", "AMDGPUInfo", "AMDGPUUtils", "Support", "BinaryFormat"])
    ctx.library(ctx, parent = "AMDGPU", required_libraries = ["Support"], add_to_library_groups = ["AMDGPU"], name = "AMDGPUInfo")
    ctx.library(ctx, name = "AMDGPUUtils", parent = "AMDGPU", required_libraries = ["Core", "MC", "BinaryFormat", "Support"], add_to_library_groups = ["AMDGPU"])
    ctx.target_group(ctx, name = "ARC", parent = "Target")
    ctx.library(ctx, add_to_library_groups = ["ARC"], name = "ARCCodeGen", parent = "ARC", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "SelectionDAG", "Support", "Target", "TransformUtils", "ARCDesc", "ARCInfo"])
    ctx.library(ctx, parent = "ARC", required_libraries = ["MCDisassembler", "Support", "ARCInfo"], add_to_library_groups = ["ARC"], name = "ARCDisassembler")
    ctx.library(ctx, add_to_library_groups = ["ARC"], name = "ARCDesc", parent = "ARC", required_libraries = ["MC", "Support", "ARCInfo"])
    ctx.library(ctx, add_to_library_groups = ["ARC"], name = "ARCInfo", parent = "ARC", required_libraries = ["Support"])
    ctx.target_group(ctx, name = "ARM", parent = "Target")
    ctx.library(ctx, required_libraries = ["ARMDesc", "ARMInfo", "Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "Scalar", "SelectionDAG", "Support", "Target", "GlobalISel", "ARMUtils", "TransformUtils", "CFGuard"], add_to_library_groups = ["ARM"], name = "ARMCodeGen", parent = "ARM")
    ctx.library(ctx, name = "ARMAsmParser", parent = "ARM", required_libraries = ["ARMDesc", "ARMInfo", "MC", "MCParser", "Support", "ARMUtils"], add_to_library_groups = ["ARM"])
    ctx.library(ctx, required_libraries = ["ARMDesc", "ARMInfo", "MCDisassembler", "Support", "ARMUtils"], add_to_library_groups = ["ARM"], name = "ARMDisassembler", parent = "ARM")
    ctx.library(ctx, required_libraries = ["ARMInfo", "ARMUtils", "MC", "MCDisassembler", "Support", "BinaryFormat"], add_to_library_groups = ["ARM"], name = "ARMDesc", parent = "ARM")
    ctx.library(ctx, name = "ARMInfo", parent = "ARM", required_libraries = ["Support"], add_to_library_groups = ["ARM"])
    ctx.library(ctx, required_libraries = ["Support"], add_to_library_groups = ["ARM"], name = "ARMUtils", parent = "ARM")
    ctx.target_group(ctx, name = "AVR", parent = "Target")
    ctx.library(ctx, name = "AVRCodeGen", parent = "AVR", required_libraries = ["AsmPrinter", "CodeGen", "Core", "MC", "AVRDesc", "AVRInfo", "SelectionDAG", "Support", "Target"], add_to_library_groups = ["AVR"])
    ctx.library(ctx, parent = "AVR", required_libraries = ["MC", "MCParser", "AVRDesc", "AVRInfo", "Support"], add_to_library_groups = ["AVR"], name = "AVRAsmParser")
    ctx.library(ctx, name = "AVRDisassembler", parent = "AVR", required_libraries = ["MCDisassembler", "AVRInfo", "Support"], add_to_library_groups = ["AVR"])
    ctx.library(ctx, add_to_library_groups = ["AVR"], name = "AVRDesc", parent = "AVR", required_libraries = ["MC", "AVRInfo", "Support"])
    ctx.library(ctx, add_to_library_groups = ["AVR"], name = "AVRInfo", parent = "AVR", required_libraries = ["Support"])
    ctx.library(ctx, name = "BPFCodeGen", parent = "BPF", required_libraries = ["AsmPrinter", "CodeGen", "Core", "MC", "BPFDesc", "BPFInfo", "SelectionDAG", "Support", "Target"], add_to_library_groups = ["BPF"])
    ctx.target_group(ctx, name = "BPF", parent = "Target")
    ctx.library(ctx, name = "BPFAsmParser", parent = "BPF", required_libraries = ["MC", "MCParser", "BPFDesc", "BPFInfo", "Support"], add_to_library_groups = ["BPF"])
    ctx.library(ctx, parent = "BPF", required_libraries = ["MCDisassembler", "BPFInfo", "Support"], add_to_library_groups = ["BPF"], name = "BPFDisassembler")
    ctx.library(ctx, add_to_library_groups = ["BPF"], name = "BPFDesc", parent = "BPF", required_libraries = ["MC", "BPFInfo", "Support"])
    ctx.library(ctx, required_libraries = ["Support"], add_to_library_groups = ["BPF"], name = "BPFInfo", parent = "BPF")
    ctx.target_group(ctx, name = "Hexagon", parent = "Target")
    ctx.library(ctx, name = "HexagonCodeGen", parent = "Hexagon", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "HexagonAsmParser", "HexagonDesc", "HexagonInfo", "IPO", "MC", "Scalar", "SelectionDAG", "Support", "Target", "TransformUtils"], add_to_library_groups = ["Hexagon"])
    ctx.library(ctx, name = "HexagonAsmParser", parent = "Hexagon", required_libraries = ["MC", "MCParser", "Support", "HexagonDesc", "HexagonInfo"], add_to_library_groups = ["Hexagon"])
    ctx.library(ctx, required_libraries = ["HexagonDesc", "HexagonInfo", "MC", "MCDisassembler", "Support"], add_to_library_groups = ["Hexagon"], name = "HexagonDisassembler", parent = "Hexagon")
    ctx.library(ctx, required_libraries = ["HexagonInfo", "MC", "Support"], add_to_library_groups = ["Hexagon"], name = "HexagonDesc", parent = "Hexagon")
    ctx.library(ctx, parent = "Hexagon", required_libraries = ["Support"], add_to_library_groups = ["Hexagon"], name = "HexagonInfo")
    ctx.target_group(ctx, parent = "Target", name = "Lanai")
    ctx.library(ctx, name = "LanaiCodeGen", parent = "Lanai", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "LanaiAsmParser", "LanaiDesc", "LanaiInfo", "MC", "SelectionDAG", "Support", "Target", "TransformUtils"], add_to_library_groups = ["Lanai"])
    ctx.library(ctx, required_libraries = ["MC", "MCParser", "Support", "LanaiDesc", "LanaiInfo"], add_to_library_groups = ["Lanai"], name = "LanaiAsmParser", parent = "Lanai")
    ctx.library(ctx, required_libraries = ["LanaiDesc", "LanaiInfo", "MC", "MCDisassembler", "Support"], add_to_library_groups = ["Lanai"], name = "LanaiDisassembler", parent = "Lanai")
    ctx.library(ctx, name = "LanaiDesc", parent = "Lanai", required_libraries = ["LanaiInfo", "MC", "MCDisassembler", "Support"], add_to_library_groups = ["Lanai"])
    ctx.library(ctx, name = "LanaiInfo", parent = "Lanai", required_libraries = ["Support"], add_to_library_groups = ["Lanai"])
    ctx.library(ctx, name = "MSP430CodeGen", parent = "MSP430", required_libraries = ["AsmPrinter", "CodeGen", "Core", "MC", "MSP430Desc", "MSP430Info", "SelectionDAG", "Support", "Target"], add_to_library_groups = ["MSP430"])
    ctx.target_group(ctx, name = "MSP430", parent = "Target")
    ctx.library(ctx, parent = "MSP430", required_libraries = ["MC", "MCParser", "MSP430Desc", "MSP430Info", "Support"], add_to_library_groups = ["MSP430"], name = "MSP430AsmParser")
    ctx.library(ctx, name = "MSP430Disassembler", parent = "MSP430", required_libraries = ["MCDisassembler", "MSP430Info", "Support"], add_to_library_groups = ["MSP430"])
    ctx.library(ctx, name = "MSP430Desc", parent = "MSP430", required_libraries = ["MC", "MSP430Info", "Support"], add_to_library_groups = ["MSP430"])
    ctx.library(ctx, required_libraries = ["Support"], add_to_library_groups = ["MSP430"], name = "MSP430Info", parent = "MSP430")
    ctx.target_group(ctx, name = "Mips", parent = "Target")
    ctx.library(ctx, add_to_library_groups = ["Mips"], name = "MipsCodeGen", parent = "Mips", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "MipsDesc", "MipsInfo", "SelectionDAG", "Support", "Target", "GlobalISel"])
    ctx.library(ctx, required_libraries = ["MC", "MCParser", "MipsDesc", "MipsInfo", "Support"], add_to_library_groups = ["Mips"], name = "MipsAsmParser", parent = "Mips")
    ctx.library(ctx, name = "MipsDisassembler", parent = "Mips", required_libraries = ["MCDisassembler", "MipsInfo", "Support"], add_to_library_groups = ["Mips"])
    ctx.library(ctx, required_libraries = ["MC", "MipsInfo", "Support"], add_to_library_groups = ["Mips"], name = "MipsDesc", parent = "Mips")
    ctx.library(ctx, name = "MipsInfo", parent = "Mips", required_libraries = ["Support"], add_to_library_groups = ["Mips"])
    ctx.library(ctx, name = "NVPTXCodeGen", parent = "NVPTX", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "IPO", "MC", "NVPTXDesc", "NVPTXInfo", "Scalar", "SelectionDAG", "Support", "Target", "TransformUtils", "Vectorize"], add_to_library_groups = ["NVPTX"])
    ctx.target_group(ctx, parent = "Target", name = "NVPTX")
    ctx.library(ctx, add_to_library_groups = ["NVPTX"], name = "NVPTXDesc", parent = "NVPTX", required_libraries = ["MC", "NVPTXInfo", "Support"])
    ctx.library(ctx, add_to_library_groups = ["NVPTX"], name = "NVPTXInfo", parent = "NVPTX", required_libraries = ["Support"])
    ctx.target_group(ctx, parent = "Target", name = "PowerPC")
    ctx.library(ctx, name = "PowerPCCodeGen", parent = "PowerPC", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "PowerPCDesc", "PowerPCInfo", "Scalar", "SelectionDAG", "Support", "Target", "TransformUtils"], add_to_library_groups = ["PowerPC"])
    ctx.library(ctx, parent = "PowerPC", required_libraries = ["MC", "MCParser", "PowerPCDesc", "PowerPCInfo", "Support"], add_to_library_groups = ["PowerPC"], name = "PowerPCAsmParser")
    ctx.library(ctx, required_libraries = ["MCDisassembler", "PowerPCInfo", "Support"], add_to_library_groups = ["PowerPC"], name = "PowerPCDisassembler", parent = "PowerPC")
    ctx.library(ctx, required_libraries = ["MC", "PowerPCInfo", "Support", "BinaryFormat"], add_to_library_groups = ["PowerPC"], name = "PowerPCDesc", parent = "PowerPC")
    ctx.library(ctx, add_to_library_groups = ["PowerPC"], name = "PowerPCInfo", parent = "PowerPC", required_libraries = ["Support"])
    ctx.target_group(ctx, name = "RISCV", parent = "Target")
    ctx.library(ctx, add_to_library_groups = ["RISCV"], name = "RISCVCodeGen", parent = "RISCV", required_libraries = ["Analysis", "AsmPrinter", "Core", "CodeGen", "MC", "RISCVDesc", "RISCVInfo", "RISCVUtils", "SelectionDAG", "Support", "Target", "GlobalISel"])
    ctx.library(ctx, name = "RISCVAsmParser", parent = "RISCV", required_libraries = ["MC", "MCParser", "RISCVDesc", "RISCVInfo", "RISCVUtils", "Support"], add_to_library_groups = ["RISCV"])
    ctx.library(ctx, name = "RISCVDisassembler", parent = "RISCV", required_libraries = ["MCDisassembler", "RISCVInfo", "Support"], add_to_library_groups = ["RISCV"])
    ctx.library(ctx, required_libraries = ["Support"], add_to_library_groups = ["RISCV"], name = "RISCVInfo", parent = "RISCV")
    ctx.library(ctx, required_libraries = ["MC", "RISCVInfo", "RISCVUtils", "Support"], add_to_library_groups = ["RISCV"], name = "RISCVDesc", parent = "RISCV")
    ctx.library(ctx, add_to_library_groups = ["RISCV"], name = "RISCVUtils", parent = "RISCV", required_libraries = ["Support"])
    ctx.target_group(ctx, name = "Sparc", parent = "Target")
    ctx.library(ctx, name = "SparcCodeGen", parent = "Sparc", required_libraries = ["AsmPrinter", "CodeGen", "Core", "MC", "SelectionDAG", "SparcDesc", "SparcInfo", "Support", "Target"], add_to_library_groups = ["Sparc"])
    ctx.library(ctx, required_libraries = ["MC", "MCParser", "SparcDesc", "SparcInfo", "Support"], add_to_library_groups = ["Sparc"], name = "SparcAsmParser", parent = "Sparc")
    ctx.library(ctx, add_to_library_groups = ["Sparc"], name = "SparcDisassembler", parent = "Sparc", required_libraries = ["MCDisassembler", "SparcInfo", "Support"])
    ctx.library(ctx, required_libraries = ["MC", "SparcInfo", "Support"], add_to_library_groups = ["Sparc"], name = "SparcDesc", parent = "Sparc")
    ctx.library(ctx, add_to_library_groups = ["Sparc"], name = "SparcInfo", parent = "Sparc", required_libraries = ["Support"])
    ctx.library(ctx, name = "SystemZCodeGen", parent = "SystemZ", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "Scalar", "SelectionDAG", "Support", "SystemZDesc", "SystemZInfo", "Target"], add_to_library_groups = ["SystemZ"])
    ctx.target_group(ctx, name = "SystemZ", parent = "Target")
    ctx.library(ctx, add_to_library_groups = ["SystemZ"], name = "SystemZAsmParser", parent = "SystemZ", required_libraries = ["MC", "MCParser", "Support", "SystemZDesc", "SystemZInfo"])
    ctx.library(ctx, parent = "SystemZ", required_libraries = ["MC", "MCDisassembler", "Support", "SystemZDesc", "SystemZInfo"], add_to_library_groups = ["SystemZ"], name = "SystemZDisassembler")
    ctx.library(ctx, required_libraries = ["MC", "Support", "SystemZInfo"], add_to_library_groups = ["SystemZ"], name = "SystemZDesc", parent = "SystemZ")
    ctx.library(ctx, required_libraries = ["Support"], add_to_library_groups = ["SystemZ"], name = "SystemZInfo", parent = "SystemZ")
    ctx.target_group(ctx, parent = "Target", name = "VE")
    ctx.library(ctx, parent = "VE", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "SelectionDAG", "VEDesc", "VEInfo", "Support", "Target"], add_to_library_groups = ["VE"], name = "VECodeGen")
    ctx.library(ctx, name = "VEDesc", parent = "VE", required_libraries = ["MC", "VEInfo", "Support"], add_to_library_groups = ["VE"])
    ctx.library(ctx, name = "VEInfo", parent = "VE", required_libraries = ["Support"], add_to_library_groups = ["VE"])
    ctx.target_group(ctx, parent = "Target", name = "WebAssembly")
    ctx.library(ctx, required_libraries = ["Analysis", "AsmPrinter", "BinaryFormat", "CodeGen", "Core", "MC", "Scalar", "SelectionDAG", "Support", "Target", "TransformUtils", "WebAssemblyDesc", "WebAssemblyInfo"], add_to_library_groups = ["WebAssembly"], name = "WebAssemblyCodeGen", parent = "WebAssembly")
    ctx.library(ctx, add_to_library_groups = ["WebAssembly"], name = "WebAssemblyAsmParser", parent = "WebAssembly", required_libraries = ["MC", "MCParser", "WebAssemblyInfo", "Support"])
    ctx.library(ctx, add_to_library_groups = ["WebAssembly"], name = "WebAssemblyDisassembler", parent = "WebAssembly", required_libraries = ["WebAssemblyDesc", "MCDisassembler", "WebAssemblyInfo", "Support", "MC"])
    ctx.library(ctx, add_to_library_groups = ["WebAssembly"], name = "WebAssemblyDesc", parent = "WebAssembly", required_libraries = ["MC", "Support", "WebAssemblyInfo"])
    ctx.library(ctx, required_libraries = ["Support"], add_to_library_groups = ["WebAssembly"], name = "WebAssemblyInfo", parent = "WebAssembly")
    ctx.target_group(ctx, parent = "Target", name = "X86")
    ctx.library(ctx, name = "X86CodeGen", parent = "X86", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "SelectionDAG", "Support", "Target", "X86Desc", "X86Info", "GlobalISel", "ProfileData", "CFGuard"], add_to_library_groups = ["X86"])
    ctx.library(ctx, name = "X86AsmParser", parent = "X86", required_libraries = ["MC", "MCParser", "Support", "X86Desc", "X86Info"], add_to_library_groups = ["X86"])
    ctx.library(ctx, add_to_library_groups = ["X86"], name = "X86Disassembler", parent = "X86", required_libraries = ["MCDisassembler", "Support", "X86Info"])
    ctx.library(ctx, parent = "X86", required_libraries = ["MC", "MCDisassembler", "Support", "X86Info", "BinaryFormat"], add_to_library_groups = ["X86"], name = "X86Desc")
    ctx.library(ctx, parent = "X86", required_libraries = ["Support"], add_to_library_groups = ["X86"], name = "X86Info")
    ctx.library(ctx, name = "XCoreCodeGen", parent = "XCore", required_libraries = ["Analysis", "AsmPrinter", "CodeGen", "Core", "MC", "SelectionDAG", "Support", "Target", "TransformUtils", "XCoreDesc", "XCoreInfo"], add_to_library_groups = ["XCore"])
    ctx.target_group(ctx, name = "XCore", parent = "Target")
    ctx.library(ctx, add_to_library_groups = ["XCore"], name = "XCoreDisassembler", parent = "XCore", required_libraries = ["MCDisassembler", "Support", "XCoreInfo"])
    ctx.library(ctx, name = "XCoreDesc", parent = "XCore", required_libraries = ["MC", "Support", "XCoreInfo"], add_to_library_groups = ["XCore"])
    ctx.library(ctx, name = "XCoreInfo", parent = "XCore", required_libraries = ["Support"], add_to_library_groups = ["XCore"])
    ctx.library(ctx, name = "TestingSupport", parent = "Libraries", required_libraries = ["Support"])
    ctx.group(ctx, name = "ToolDrivers", parent = "Libraries")
    ctx.library(ctx, name = "DlltoolDriver", parent = "Libraries", required_libraries = ["Object", "Option", "Support"])
    ctx.library(ctx, name = "LibDriver", parent = "Libraries", required_libraries = ["BinaryFormat", "BitReader", "Object", "Option", "Support"])
    ctx.group(ctx, name = "Transforms", parent = "Libraries")
    ctx.library(ctx, name = "AggressiveInstCombine", parent = "Transforms", required_libraries = ["Analysis", "Core", "Support", "TransformUtils"])
    ctx.library(ctx, parent = "Transforms", required_libraries = ["Analysis", "Core", "IPO", "Scalar", "Support", "TransformUtils"], name = "Coroutines")
    ctx.library(ctx, name = "IPO", parent = "Transforms", library_name = "ipo", required_libraries = ["AggressiveInstCombine", "Analysis", "BitReader", "BitWriter", "Core", "FrontendOpenMP", "InstCombine", "IRReader", "Linker", "Object", "ProfileData", "Scalar", "Support", "TransformUtils", "Vectorize", "Instrumentation"])
    ctx.library(ctx, name = "InstCombine", parent = "Transforms", required_libraries = ["Analysis", "Core", "Support", "TransformUtils"])
    ctx.library(ctx, name = "Instrumentation", parent = "Transforms", required_libraries = ["Analysis", "Core", "MC", "Support", "TransformUtils", "ProfileData"])
    ctx.library(ctx, library_name = "ScalarOpts", required_libraries = ["AggressiveInstCombine", "Analysis", "Core", "InstCombine", "Support", "TransformUtils"], name = "Scalar", parent = "Transforms")
    ctx.library(ctx, name = "TransformUtils", parent = "Transforms", required_libraries = ["Analysis", "Core", "Support"])
    ctx.library(ctx, name = "Vectorize", parent = "Transforms", library_name = "Vectorize", required_libraries = ["Analysis", "Core", "Support", "TransformUtils"])
    ctx.library(ctx, parent = "Transforms", library_name = "ObjCARCOpts", required_libraries = ["Analysis", "Core", "Support", "TransformUtils"], name = "ObjCARC")
    ctx.library(ctx, name = "CFGuard", parent = "Transforms", required_libraries = ["Core", "Support"])
    ctx.library(ctx, name = "WindowsManifest", parent = "Libraries", required_libraries = ["Support"])
    ctx.library(ctx, name = "XRay", parent = "Libraries", required_libraries = ["Support", "Object"])
    ctx.group(ctx, name = "Projects", parent = "$ROOT")
    ctx.group(ctx, parent = "$ROOT", name = "Tools")
    ctx.tool(ctx, name = "bugpoint", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "BitWriter", "CodeGen", "IRReader", "IPO", "Instrumentation", "Linker", "ObjCARC", "Scalar", "all-targets"])
    ctx.tool(ctx, parent = "Tools", required_libraries = ["AsmPrinter", "DebugInfoDWARF", "DWARFLinker", "MC", "Object", "CodeGen", "Support", "all-targets"], name = "dsymutil")
    ctx.tool(ctx, parent = "Tools", required_libraries = ["AsmParser", "BitReader", "IRReader", "MIRParser", "TransformUtils", "Scalar", "Vectorize", "all-targets"], name = "llc")
    ctx.tool(ctx, name = "lli", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "IRReader", "Instrumentation", "Interpreter", "MCJIT", "Native", "NativeCodeGen", "SelectionDAG", "TransformUtils"])
    ctx.tool(ctx, parent = "lli", name = "lli-child-target")
    ctx.tool(ctx, parent = "Tools", name = "llvm-ar")
    ctx.tool(ctx, required_libraries = ["AsmParser", "BitWriter"], name = "llvm-as", parent = "Tools")
    ctx.tool(ctx, parent = "Tools", required_libraries = ["BitReader", "BitstreamReader", "Support"], name = "llvm-bcanalyzer")
    ctx.tool(ctx, name = "llvm-cat", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "BitWriter"])
    ctx.tool(ctx, parent = "Tools", required_libraries = ["all-targets", "MC", "MCDisassembler", "MCParser", "Support", "Symbolize"], name = "llvm-cfi-verify")
    ctx.tool(ctx, parent = "Tools", required_libraries = ["Coverage", "Support", "Instrumentation"], name = "llvm-cov")
    ctx.tool(ctx, name = "llvm-cvtres", parent = "Tools", required_libraries = ["Object", "Option", "Support"])
    ctx.tool(ctx, name = "llvm-diff", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "IRReader"])
    ctx.tool(ctx, name = "llvm-dis", parent = "Tools", required_libraries = ["Analysis", "BitReader"])
    ctx.tool(ctx, name = "llvm-dwarfdump", parent = "Tools", required_libraries = ["DebugInfoDWARF", "Object"])
    ctx.tool(ctx, required_libraries = ["AsmPrinter", "DebugInfoDWARF", "MC", "Object", "Support", "all-targets"], name = "llvm-dwp", parent = "Tools")
    ctx.tool(ctx, required_libraries = ["Object", "Support", "TextAPI"], name = "llvm-elfabi", parent = "Tools")
    ctx.tool(ctx, name = "llvm-ifs", parent = "Tools", required_libraries = ["Object", "Support", "TextAPI"])
    ctx.tool(ctx, parent = "Tools", required_libraries = ["CodeGen", "ExecutionEngine", "MC", "MCJIT", "Native", "NativeCodeGen", "Object", "Support"], name = "llvm-exegesis")
    ctx.tool(ctx, required_libraries = ["AsmParser", "BitReader", "BitWriter", "IRReader", "IPO"], name = "llvm-extract", parent = "Tools")
    ctx.tool(ctx, name = "llvm-jitlistener", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "IRReader", "Interpreter", "MCJIT", "NativeCodeGen", "Object", "SelectionDAG", "Native"])
    ctx.tool(ctx, required_libraries = ["JITLink", "BinaryFormat", "MC", "Object", "RuntimeDyld", "Support", "all-targets"], name = "llvm-jitlink", parent = "Tools")
    ctx.tool(ctx, name = "llvm-link", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "BitWriter", "IRReader", "Linker", "Object", "TransformUtils", "IPO"])
    ctx.tool(ctx, name = "llvm-lto", parent = "Tools", required_libraries = ["BitWriter", "Core", "IRReader", "LTO", "Object", "Support", "all-targets"])
    ctx.tool(ctx, name = "llvm-mc", parent = "Tools", required_libraries = ["MC", "MCDisassembler", "MCParser", "Support", "all-targets"])
    ctx.tool(ctx, required_libraries = ["MC", "MCA", "MCParser", "Support", "all-targets"], name = "llvm-mca", parent = "Tools")
    ctx.tool(ctx, name = "llvm-modextract", parent = "Tools", required_libraries = ["BitReader", "BitWriter"])
    ctx.tool(ctx, name = "llvm-mt", parent = "Tools", required_libraries = ["Option", "Support", "WindowsManifest"])
    ctx.tool(ctx, name = "llvm-nm", parent = "Tools", required_libraries = ["BitReader", "Object"])
    ctx.tool(ctx, name = "llvm-objcopy", parent = "Tools", required_libraries = ["Object", "Option", "Support", "MC"])
    ctx.tool(ctx, name = "llvm-objdump", parent = "Tools", required_libraries = ["DebugInfoDWARF", "MC", "MCDisassembler", "MCParser", "Object", "all-targets", "Demangle"])
    ctx.tool(ctx, name = "llvm-pdbutil", parent = "Tools", required_libraries = ["DebugInfoMSF", "DebugInfoPDB"])
    ctx.tool(ctx, name = "llvm-profdata", parent = "Tools", required_libraries = ["ProfileData", "Support"])
    ctx.tool(ctx, parent = "Tools", required_libraries = ["Option"], name = "llvm-rc")
    ctx.tool(ctx, name = "llvm-reduce", parent = "Tools", required_libraries = ["BitReader", "IRReader", "all-targets"])
    ctx.tool(ctx, parent = "Tools", required_libraries = ["MC", "Object", "RuntimeDyld", "Support", "all-targets"], name = "llvm-rtdyld")
    ctx.tool(ctx, parent = "Tools", required_libraries = ["Object"], name = "llvm-size")
    ctx.tool(ctx, name = "llvm-split", parent = "Tools", required_libraries = ["TransformUtils", "BitWriter", "Core", "IRReader", "Support"])
    ctx.tool(ctx, name = "llvm-undname", parent = "Tools", required_libraries = ["Demangle", "Support"])
    ctx.tool(ctx, name = "opt", parent = "Tools", required_libraries = ["AsmParser", "BitReader", "BitWriter", "CodeGen", "IRReader", "IPO", "Instrumentation", "Scalar", "ObjCARC", "Passes", "all-targets"])
    ctx.tool(ctx, name = "verify-uselistorder", parent = "Tools", required_libraries = ["IRReader", "BitWriter", "Support"])
    ctx.group(ctx, name = "BuildTools", parent = "$ROOT")
    ctx.group(ctx, name = "UtilityTools", parent = "$ROOT")
    ctx.build_tool(ctx, required_libraries = ["Support", "TableGen", "MC"], name = "tblgen", parent = "BuildTools")
    ctx.library(ctx, name = "gtest", parent = "Libraries", required_libraries = ["Support"])
    ctx.library(ctx, name = "gtest_main", parent = "Libraries", required_libraries = ["gtest"])
    return ctx
