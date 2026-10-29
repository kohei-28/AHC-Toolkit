# AHC Short Toolkit

A small personal toolkit for short and long AtCoder Heuristic Contests.

```text
ahc-short-toolkit/
|-- README.md
|-- runner/
|   +-- runner.py
+-- template/
    |-- short/
    |   |-- beam_search/
    |   |   +-- basic.cpp
    |   +-- hill_climb/
    |       +-- basic.cpp
    +-- long/
        +-- state_action/
            |-- state_action.cpp
            +-- examples/
                |-- ahc069_example.cpp
                +-- ahc072_example.cpp
```

`runner/` contains a simple local test runner. `template/` contains reusable C++ templates and complete examples.

## Using the Runner

### 1. Download the Official Tools

Download and extract the precompiled Windows tools for the target AHC problem.

The archive commonly contains programs such as `gen.exe` and `vis.exe`. Their names and usage may differ between contests, so check the README included with the official tools.

### 2. Copy Two Files

Place the following files in the root of the extracted official tools directory:

- Your solution, named `main.cpp`
- `runner/runner.py` from this repository

The directory should look similar to this:

```text
official-tools/
|-- gen.exe
|-- vis.exe
|-- main.cpp
|-- runner.py
`-- in/
```

`runner.py` only requires `main.cpp` and at least one `.txt` input file inside `in/`.

### 3. Generate Inputs

Use the official generator according to its README. For tools that accept a seed file, a typical command is:

```powershell
.\gen.exe seeds.txt
```

Confirm that the generated inputs are stored in `in/`.

### 4. Run the Tests

Open PowerShell in the official tools directory and run:

```powershell
py -3 .\runner.py
```

The runner:

1. Compiles `main.cpp` with GCC and C++23.
2. Runs the solution sequentially for every `in/*.txt` file.
3. Writes each output to `out/<input-file-name>`.
4. Writes standard error to `build/logs/`.
5. Prints the status and elapsed time of each case.

The compiled executable is stored at `build/main.exe`. Existing outputs and logs with the same names are overwritten.

### 5. Check the Results

The runner does not calculate scores or launch the visualizer. Use the official visualizer manually according to its README. A typical command is:

```powershell
.\vis.exe .\in\0000.txt .\out\0000.txt
```

## Timeout

The default hard timeout is five seconds per case. This local timeout does not reproduce the AtCoder judge time limit exactly.

```powershell
# Change the timeout to three seconds.
py -3 .\runner.py --timeout 3

# Disable the timeout.
py -3 .\runner.py --timeout 0
```

## Options

```powershell
py -3 .\runner.py --help
```

Useful options include `--source`, `--input-dir`, `--output-dir`, `--compiler`, and `--timeout`.

## Requirements

- Windows
- Python 3.11 or later
- GCC with C++23 support available as `g++`
- Official Windows tools for input generation and visualization
