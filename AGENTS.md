# Repository Guidelines

## Project Structure & Module Organization
Source lives under `src/`. The entry point is `src/main.cpp`. Engine code is organized under `src/sidekick`, with core runtime pieces in `src/sidekick/core` such as `application` and `window`. Build configuration lives in `CMakeLists.txt`, reusable CMake helpers are in `cmake/`, and local build artifacts are generated under `build/` and `.cache/`. Do not commit generated files from those directories.

## Build, Test, and Development Commands
Use `just` for routine workflows:

- `just configure` configures the `dev` CMake preset.
- `just build` builds the project with the `dev` preset.
- `just run` builds and launches `./build/dev/sidekick`.

If you need lower-level control, use the preset directly: `cmake --preset dev` and `cmake --build --preset dev`.

## Coding Style & Naming Conventions
This project uses C++23. Formatting is enforced by `.clang-format`: 2-space indentation, no tabs, Allman braces, left-aligned pointers/references, and a 120-column limit. Run `clang-format -i src/main.cpp src/sidekick/core/*.cpp src/sidekick/core/*.hpp src/sidekick/renderer/*.cpp src/sidekick/renderer/*.hpp src/sidekick/platform/wgpu/*.cpp src/sidekick/platform/wgpu/*.hpp` before submitting changes.

For core systems such as `window` and `graphics_context`, prefer fail-fast initialization over `is_valid`-style runtime checks. If two-phase setup is needed, let `init()` return `bool` and promote failure at the owning boundary.

Use `snake_case` for project-defined names, including types, functions, namespaces, files, and fields. Keep `m_` prefixes for private members (`m_window`) and use `UPPER_CASE` for global constants. For template parameter names, do not use a `T` prefix; prefer names like `EventType` or `Callback`. Prefer brace initialization with `{}` by default, and use `=` only when necessary. When C++20 designated initializers are valid for the type being constructed, prefer them over positional aggregate initialization. Keep headers focused on declarations and prefer moving non-trivial definitions into `.cpp` files.

## Testing Guidelines
There is no dedicated test suite yet. At minimum, verify that `just build` succeeds and `just run` launches the application window without immediate shutdown or runtime errors. When tests are added, place them in a dedicated test target and keep names aligned with the subsystem under test, for example `window_tests` or `application_tests`.

## Commit & Pull Request Guidelines
Git history is not available in this workspace, so no repository-specific commit pattern could be derived. Use short, imperative commit messages such as `Extract window ownership from Application` or `Add movable Window abstraction`.

Pull requests should include:

- a brief summary of the change
- build/run verification steps performed
- linked issue or task context when applicable
- screenshots or short notes for visible runtime changes
