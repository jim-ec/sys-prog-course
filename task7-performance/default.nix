with import <nixpkgs> {};
gcc10Stdenv.mkDerivation {
  name = "env";
  buildInputs = [
    bashInteractive
    cmake
    gflags
  ];
}
