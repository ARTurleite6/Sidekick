#pragma once

#include <cstdint>
#include <format>
#include <string_view>

namespace Sidekick::Core
{
enum class KeyCode : std::uint16_t
{
    Unknown = 0,
    Space = 32,
    A = 65,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z = 90,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    // ... add more as needed
};

inline KeyCode GLFWKeyToKeyCode(int glfwKey)
{
    if (glfwKey >= 32 && glfwKey <= 90)
        return static_cast<KeyCode>(glfwKey);
    switch (glfwKey)
    {
    case 256:
        return KeyCode::Escape;
    case 257:
        return KeyCode::Enter;
    case 258:
        return KeyCode::Tab;
    case 259:
        return KeyCode::Backspace;
    case 260:
        return KeyCode::Insert;
    case 261:
        return KeyCode::Delete;
    case 262:
        return KeyCode::Right;
    case 263:
        return KeyCode::Left;
    case 264:
        return KeyCode::Down;
    case 265:
        return KeyCode::Up;
    default:
        return KeyCode::Unknown;
    }
}

constexpr std::string_view ToString(KeyCode code) noexcept
{
    using enum KeyCode;

    switch (code)
    {
    case KeyCode::Unknown:
        return "unknown";
    case KeyCode::Space:
        return "space";
    case KeyCode::A:
        return "a";
    case KeyCode::B:
        return "b";
    case KeyCode::C:
        return "c";
    case KeyCode::D:
        return "d";
    case KeyCode::E:
        return "e";
    case KeyCode::F:
        return "f";
    case KeyCode::G:
        return "g";
    case KeyCode::H:
        return "h";
    case KeyCode::I:
        return "i";
    case KeyCode::J:
        return "j";
    case KeyCode::K:
        return "k";
    case KeyCode::L:
        return "l";
    case KeyCode::M:
        return "m";
    case KeyCode::N:
        return "n";
    case KeyCode::O:
        return "o";
    case KeyCode::P:
        return "p";
    case KeyCode::Q:
        return "q";
    case KeyCode::R:
        return "r";
    case KeyCode::S:
        return "s";
    case KeyCode::T:
        return "t";
    case KeyCode::U:
        return "u";
    case KeyCode::V:
        return "v";
    case KeyCode::W:
        return "w";
    case KeyCode::X:
        return "x";
    case KeyCode::Y:
        return "y";
    case KeyCode::Z:
        return "z";
    case KeyCode::Escape:
        return "escape";
    case KeyCode::Enter:
        return "enter";
    case KeyCode::Tab:
        return "tab";
    case KeyCode::Backspace:
        return "backspace";
    case KeyCode::Insert:
        return "insert";
    case KeyCode::Delete:
        return "delete";
    case KeyCode::Right:
        return "right";
    case KeyCode::Left:
        return "left";
    case KeyCode::Down:
        return "down";
    case KeyCode::Up:
        return "up";
        break;
    }
}
} // namespace Sidekick::Core

template <> struct std::formatter<Sidekick::Core::KeyCode> : std::formatter<std::string_view>
{
    auto format(Sidekick::Core::KeyCode code, auto& ctx) const
    {
        return std::formatter<std::string_view>::format(Sidekick::Core::ToString(code), ctx);
    }
};
