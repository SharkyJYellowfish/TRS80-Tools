# TRS80-Tools

A collection of TRS-80 related tools, utilities, and supporting documentation.

This repository is structured as a multi-tool repo. Each tool lives in its own
subfolder and can have its own source, solution/project files, documentation,
tests, and tool-specific README.

## Current tools

### [XModem](./XModem/README.md)

Checksum-based XMODEM tooling and related documentation.

Current status:
- development complete
- project-local versioning via `XModem/VERSION`
- current milestone: `1.0.0`
- major transmit/receive logic in place
- tested against the TRS-80 Modem80 terminal application

This tool currently handles:
- send/receive of binary level files
- checksum based block integrity
- retry and timeout logic
- support to specify COMx port and baud rate
- fixed settings set to 8-bit word size, no stop bits and no parity

### [NewDos80BasicDetokenizer](./NewDos80BasicDetokenizer/README.md)

A C# utility for converting tokenized **TRS-80 Model III/4 NewDOS/80 Disk BASIC**
program files into readable ASCII BASIC source and related documentation.

Current status:
- active development
- project-local versioning via `NewDos80BasicDetokenizer/VERSION`
- current milestone: `0.8.0`
- major parsing and token-expansion work is in place
- regression test data is being added under `NewDos80BasicDetokenizer/Tests/Data/`

This tool currently handles:
- linked BASIC line parsing
- quoted string preservation
- raw comment preservation for `REM` and apostrophe comments
- compound relational operator folding (`<>`, `<=`, `>=`)
- token decoding including `USING`, `VARPTR`, and `USR`

Output is already useful for source recovery and archival work, but formatting
fidelity is still being refined.

## Repository layout


```text
TRS80-Tools/
├── .clang-format
├── .editorconfig
├── .gitignore
├── CONTRIBUTING.md
├── LICENSE
├── README.md
├── XModem/
│   ├── README.md
│   ├── XModem.sln
│   ├── Docs/
│   └── XModem/
└── NewDos80BasicDetokenizer/
    ├── README.md
    ├── VERSION
    ├── Docs/
    ├── Tests/
    │   └── Data/
    └── ...

## How this repo is organized

- The **repo root** describes the overall tool collection.
- Each **tool folder** contains the files specific to that tool.
- Shared repo-level files such as the license, contributing notes, formatting rules, and ignore rules remain at the top level.

## Releases

GitHub Releases are used for packaged binaries and release assets.

Auto-generated GitHub source archives reflect the entire repository at a tagged commit. Tool-specific packaged binaries, when provided, are attached separately as release assets.

## Building

Build instructions, usage notes, and tool-specific details are documented in each tool’s own README.

For each tool, see:

- [XModem README](./XModem/README.md)
- [NewDos80BasicDetokenizer README](./NewDos80BasicDetokenizer/README.md)

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md).

## License

This repository is licensed under the terms of the [LICENSE](./LICENSE) file.
