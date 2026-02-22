# Repository Guidelines

## Project Structure & Module Organization
- `CMakeLists.txt` defines the CMake build and the `Sidekick` executable target.
- `src/` holds application source code.
- `src/main.cpp` is the current entry point.
- There are no dedicated `tests/` or `assets/` directories yet.

## Build, Test, and Development Commands
- `cmake -S . -B build -G Ninja` configures the project into `build/`.
- `cmake --build build` builds the `Sidekick` executable.
- `./build/Sidekick` runs the application after a successful build.
- `clang-tidy -p build <file.cpp>` runs clang-tidy using the build compile database.
- No automated tests are configured yet.

## Coding Style & Naming Conventions
- Indentation: 2 spaces, no tabs (match existing `src/main.cpp`).
- Naming:
  - Types, functions, files and methods: `PascalCase` (e.g., `GameState`, `Initialize()`)
  - Variables: `snake_case` (e.g., `max_retries`, `load_config`)
  - Private member fields: `m_` prefix (e.g., `m_is_ready`)
- Keep headers minimal; prefer `#include` in `.cpp` unless an interface requires it.
- Keep the ordering of headers the following: Local, third-party and then standard-library. 
    Group them by project.
- use clang-format formatter, with the .clang-format config
- Use `.clang-tidy` for static analysis and keep warnings under control in touched files.
- Always try to reduce the number of memory allocations when possible
- NEVER add an I prefix to Interfaces (e.g. IGraphicsBackend vs GraphicsBackend)

## Testing Guidelines
- There is no test framework configured.
- If you add tests, place them under `tests/` and document how to run them.
- Prefer descriptive test names in `snake_case` (e.g., `loads_default_config`).
- Always after each change, you should try to at least compile the project to see if it is working
- Always format code at the end of each task using .clang-format
- Always run clang-tidy at the end of each task for the files you changed.

## Commit & Pull Request Guidelines
- Git history is not available in this repository, so there is no established commit message convention.
- Suggested default: short, imperative messages (e.g., `Add initial input parser`).
- PRs should include:
  - A brief summary of changes and rationale.
  - Build/run steps used (e.g., `cmake -S . -B build -G Ninja`).
  - Any new tests added or a note explaining why none were added.

## Notes for Contributors
- Keep changes focused and small; update `CMakeLists.txt` when adding new source files.
- If you introduce new dependencies or tools, document the required setup steps here.
