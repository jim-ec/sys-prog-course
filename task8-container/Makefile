# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2


# cat /tmp/nix-build-wttr.drv-0/env-vars | grep '^declare -x SHELL=' | sed 's/^.\+="\(.\+\)"/\1/'


all:
	$(CXX) $(CXXFLAGS) -o nix-build-shell nix-build-shell.cpp -std=c++17 -Wall -Wextra
#	$(CXX) $(CXXFLAGS) -o nix-build-shell nix-build-shell.cpp -std=c++17 -Wall -Wextra -lcap


check: all
	$(MAKE) -C tests check


clean:
	rm -rf tests/failed-build-cache
