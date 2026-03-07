# Repository Guidelines

## Project Structure & Module Organization
Source lives under `src/`. The entry point is `src/main.cpp`. Engine code is organized under `src/Sidekick`, with core runtime pieces in `src/Sidekick/Core` such as `Application` and `Window`. Build configuration lives in `CMakeLists.txt`, reusable CMake helpers are in `cmake/`, and local build artifacts are generated under `build/` and `.cache/`. Do not commit generated files from those directories.

## Build, Test, and Development Commands
Use `just` for routine workflows:

- `just configure` configures the `dev` CMake preset.
- `just build` builds the project with the `dev` preset.
- `just run` builds and launches `./build/dev/sidekick`.

If you need lower-level control, use the preset directly: `cmake --preset dev` and `cmake --build --preset dev`.

## Coding Style & Naming Conventions
This project uses C++23. Formatting is enforced by `.clang-format`: 2-space indentation, no tabs, Allman braces, left-aligned pointers/references, and a 120-column limit. Run `clang-format -i src/main.cpp src/Sidekick/Core/*.cpp src/Sidekick/Core/*.hpp src/Sidekick/Renderer/*.cpp src/Sidekick/Renderer/*.hpp` before submitting changes.

For core systems such as `Window` and `GraphicsContext`, prefer fail-fast initialization over `IsValid`-style runtime checks. If two-phase setup is needed, let `Init()` return `bool` and promote failure at the owning boundary.

Use `PascalCase` for types (`Application`, `WindowDescriptor`) and public fields, `m_` prefixes for private members (`m_Window`), `g_snake_case` for globals (`g_glfw_window_owners`), and `snake_case` for local variables where it improves readability. For template parameter names, do not use a `T` prefix; prefer names like `EventType` or `Callback`. Prefer brace initialization with `{}` by default, and use `=` only when necessary. Keep headers focused on declarations and prefer moving non-trivial definitions into `.cpp` files.

## Testing Guidelines
There is no dedicated test suite yet. At minimum, verify that `just build` succeeds and `just run` launches the application window without immediate shutdown or runtime errors. When tests are added, place them in a dedicated test target and keep names aligned with the subsystem under test, for example `WindowTests` or `ApplicationTests`.

## Commit & Pull Request Guidelines
Git history is not available in this workspace, so no repository-specific commit pattern could be derived. Use short, imperative commit messages such as `Extract window ownership from Application` or `Add movable Window abstraction`.

Pull requests should include:

- a brief summary of the change
- build/run verification steps performed
- linked issue or task context when applicable
- screenshots or short notes for visible runtime changes
