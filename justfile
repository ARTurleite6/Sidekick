default:
    @just --list

alias c := configure
alias b := build
alias r := run

configure:
    cmake --preset dev

build:
    cmake --build --preset dev

run: build
    ./build/dev/sidekick
