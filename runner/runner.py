#!/usr/bin/env python3
"""Compile a C++ solution and run it against every input file in a directory."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compile main.cpp and run it sequentially against in/*.txt."
    )
    parser.add_argument(
        "--project",
        type=Path,
        default=Path.cwd(),
        help="Problem directory. Defaults to the current directory.",
    )
    parser.add_argument("--source", type=Path, default=Path("main.cpp"))
    parser.add_argument("--input-dir", type=Path, default=Path("in"))
    parser.add_argument("--output-dir", type=Path, default=Path("out"))
    parser.add_argument("--build-dir", type=Path, default=Path("build"))
    parser.add_argument(
        "--compiler",
        default="g++",
        help="C++ compiler command. Defaults to g++.",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="Hard timeout in seconds per case. Use 0 to disable it.",
    )
    return parser.parse_args()


def path_inside(project: Path, path: Path) -> Path:
    resolved = (project / path).resolve() if not path.is_absolute() else path.resolve()
    try:
        resolved.relative_to(project)
    except ValueError as error:
        raise ValueError(f"Path must be inside the project directory: {resolved}") from error
    return resolved


def display_command(command: list[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(command)
    import shlex

    return shlex.join(command)


def compile_solution(
    project: Path,
    source: Path,
    solver: Path,
    compiler: str,
) -> bool:
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        print(f"Compiler not found: {compiler}", file=sys.stderr)
        return False

    source_relative = source.relative_to(project)
    solver_relative = solver.relative_to(project)
    command = [
        compiler_path,
        str(source_relative),
        "-std=gnu++23",
        "-O2",
        "-Wall",
        "-Wextra",
        "-DLOCAL",
        "-o",
        str(solver_relative),
    ]

    print(f"Build: {display_command(command)}")
    completed = subprocess.run(command, cwd=project, check=False)
    if completed.returncode != 0:
        print(f"Build failed with exit code {completed.returncode}.", file=sys.stderr)
        return False
    return True


def run_case(
    project: Path,
    solver: Path,
    input_file: Path,
    output_file: Path,
    log_file: Path,
    timeout: float | None,
) -> tuple[str, float]:
    solver_relative = solver.relative_to(project)
    started_at = time.perf_counter()

    try:
        with input_file.open("rb") as standard_input, output_file.open(
            "wb"
        ) as standard_output, log_file.open("wb") as standard_error:
            completed = subprocess.run(
                [str(solver_relative)],
                cwd=project,
                stdin=standard_input,
                stdout=standard_output,
                stderr=standard_error,
                timeout=timeout,
                check=False,
            )
    except subprocess.TimeoutExpired:
        elapsed = time.perf_counter() - started_at
        output_file.unlink(missing_ok=True)
        with log_file.open("ab") as log:
            log.write(f"\nRunner timeout after {elapsed:.3f} seconds.\n".encode())
        return "TIMEOUT", elapsed
    except OSError as error:
        elapsed = time.perf_counter() - started_at
        output_file.unlink(missing_ok=True)
        with log_file.open("ab") as log:
            log.write(f"\nRunner error: {error}\n".encode())
        return "ERROR", elapsed

    elapsed = time.perf_counter() - started_at
    if completed.returncode != 0:
        return f"ERROR({completed.returncode})", elapsed
    return "OK", elapsed


def main() -> int:
    args = parse_args()
    project = args.project.resolve()

    if not project.is_dir():
        print(f"Project directory not found: {project}", file=sys.stderr)
        return 2
    if args.timeout < 0:
        print("--timeout must be non-negative.", file=sys.stderr)
        return 2

    try:
        source = path_inside(project, args.source)
        input_dir = path_inside(project, args.input_dir)
        output_dir = path_inside(project, args.output_dir)
        build_dir = path_inside(project, args.build_dir)
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2

    if not source.is_file():
        print(f"Source file not found: {source}", file=sys.stderr)
        return 2
    if not input_dir.is_dir():
        print(f"Input directory not found: {input_dir}", file=sys.stderr)
        return 2

    input_files = sorted(path for path in input_dir.glob("*.txt") if path.is_file())
    if not input_files:
        print(f"No .txt input files found in: {input_dir}", file=sys.stderr)
        return 2

    output_dir.mkdir(parents=True, exist_ok=True)
    build_dir.mkdir(parents=True, exist_ok=True)
    log_dir = build_dir / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    solver_name = "main.exe" if os.name == "nt" else "main"
    solver = build_dir / solver_name
    if not compile_solution(project, source, solver, args.compiler):
        return 1

    timeout = None if args.timeout == 0 else args.timeout
    failures = 0
    print(f"Run: {len(input_files)} case(s), timeout={args.timeout:g}s")

    for input_file in input_files:
        output_file = output_dir / input_file.name
        log_file = log_dir / f"{input_file.stem}.log"
        status, elapsed = run_case(
            project,
            solver,
            input_file,
            output_file,
            log_file,
            timeout,
        )
        if status != "OK":
            failures += 1
        print(f"{input_file.name:<24} {status:<12} {elapsed:8.3f} s")

    print(f"Finished: {len(input_files) - failures} succeeded, {failures} failed")
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())

