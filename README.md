\# TRS80-Tools



A collection of TRS-80 related tools, utilities, and supporting documentation.



This repository is structured as a multi-tool repo. Each tool lives in its own subfolder and can have its own source, solution/project files, documentation, and tool-specific README.



\## Current tools



\### \[XModem](./XModem/README.md)

Checksum-based XMODEM tooling and related documentation.



\## Repository layout



TRS80-Tools/

├── .clang-format

├── .gitignore

├── CONTRIBUTING.md

├── LICENSE

└── XModem/

&#x20;   ├── README.md

&#x20;   ├── XModem.sln

&#x20;   ├── Docs/

&#x20;   └── XModem/



Additional tools may be added later as peer folders, for example:



&#x20;   TRS80-Tools/

&#x20;   ├── XModem/

&#x20;   ├── ND80BasicDetokenizer/

&#x20;   └── ...



\## How this repo is organized



\- The \*\*repo root\*\* describes the overall tool collection.

\- Each \*\*tool folder\*\* contains the files specific to that tool.

\- Shared repo-level files such as the license, contributing notes, formatting rules, and ignore rules remain at the top level.



\## Releases



GitHub Releases are used for packaged binaries and release assets.



Auto-generated GitHub source archives reflect the entire repository at a tagged commit. Tool-specific packaged binaries, when provided, are attached separately as release assets.



\## Building



Build instructions, usage notes, and tool-specific details are documented in each tool’s own README.



For the current tool, see:



\- \[XModem README](./XModem/README.md)



\## Contributing



See \[CONTRIBUTING.md](./CONTRIBUTING.md).



\## License



This repository is licensed under the terms of the \[LICENSE](./LICENSE) file.



