# NewDOS/80 Disk Basic Detokenizer

A C# utility for converting tokenized **TRS-80 Model III/4 NewDOS/80 Disk BASIC** program files into readable ASCII BASIC source.

This tool is intended to reconstruct source programs saved in tokenized Disk BASIC format and emit a text form suitable for inspection, diffing, archival, and further cleanup.

## Status

Current project version is tracked in:

`NewDos80BasicDetokenizer/VERSION`

Current development milestone:

`0.8.0`

The detokenizer is working and already handles the major structural and token-expansion cases, but output is not yet fully canonical to original TRS-80 ASCII `SAVE` formatting.

## What it currently does

- Parses linked BASIC line records
- Skips the leading `0xFF` file marker
- Stops correctly at end-of-program when:
  - `nextLinePtr == 0`
  - `lineNumber == 0`
- Expands BASIC keywords and functions from tokenized form
- Preserves quoted strings
- Preserves raw comment text for:
  - `REM`
  - apostrophe comments
- Folds compound relational operators:
  - `<>`
  - `<=`
  - `>=`
- Resolves tokenized forms including:
  - `USING`
  - `VARPTR`
  - `USR`
- Suppresses embedded control bytes such as stray `0x0A` / `0x0D` from corrupting output line structure
- Emits unknown tokens visibly as:
  - `<$XX>`

## File format handled

The tokenized BASIC file format is treated as:

```text
0xFF
repeat
    uint16 nextLinePtr   (little endian)
    uint16 lineNumber    (little endian)
    byte[] tokenized line body
    0x00                 end-of-line
until nextLinePtr == 0 && lineNumber == 0
