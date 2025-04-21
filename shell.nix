with import <nixpkgs> {};
  stdenv.mkDerivation {
    name = "firmware";
    buildInputs = [
      glibc_multi
      gcc-arm-embedded-10
      ninja
      cmake
      protobuf
      python312
      python312Packages.grpcio-tools
      python312Packages.wheel
      python312Packages.protobuf
      python312Packages.ecdsa
      python312Packages.gcovr
      SDL2
    ];
  }
