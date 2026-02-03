alias r := run
alias b := build

build:
  odin build src -out:sidekick -debug -show-timings 

run:
  odin run src -out:sidekick -debug -show-timings 
