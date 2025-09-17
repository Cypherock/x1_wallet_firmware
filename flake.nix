{
  description = "Flake for x1 firmware";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";

  # reference to olds nixpkgs with gcc-arm-embedded-10
  inputs.old-nixpkgs.url = "github:NixOS/nixpkgs/dd613136ee91f67e5dba3f3f41ac99ae89c5406b";

  outputs =
    {
      nixpkgs,
      old-nixpkgs,
      flake-utils,
      ...
    }:
    let
      gcc-arm-embedded-10-overlay = final: prev: {
        gcc-arm-embedded-10 =
          (import old-nixpkgs {
            inherit (prev) system;
          }).gcc-arm-embedded-10;
      };
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ gcc-arm-embedded-10-overlay ];
        };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # can be used to generate compile_commands.json using: bear -- $build_command
            bear

            clang-tools
            cppcheck
            cmake
            ninja

            # can be used to flash using: dfu-util -a 0 -s 0x8023000:leave -D ./binary.bin
            dfu-util

            gcc-arm-embedded-10
            protobuf
            (python3.withPackages (
              ps: with ps; [
                wheel
                protobuf
                grpcio-tools
                ecdsa
              ]
            ))
          ];
        };

        formatter = pkgs.nixfmt-tree;
      }
    );
}
