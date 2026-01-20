{
  description = "Flake for x1 firmware using fhs environment";

  # reference to olds nixpkgs with gcc-arm-embedded-10
  inputs.old-nixpkgs.url = "github:NixOS/nixpkgs/dd613136ee91f67e5dba3f3f41ac99ae89c5406b";
  inputs.flake-utils.url = "github:numtide/flake-utils";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.11";
  outputs =
    {
      nixpkgs,
      old-nixpkgs,
      flake-utils,
      ...
    }:
    let
      shell = "zsh"; # Change this to your preferred shell
      fhs-name = "x1-fhs-env";
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
          config = {
            permittedInsecurePackages = [
              "python3.13-ecdsa-0.19.1"
            ];
          };

        };
        fhs = (
          pkgs.buildFHSEnv {
            name = fhs-name;
            targetPkgs =
              pkgs:
              (with pkgs; [
                # can be used to generate compile_commands.json using: bear -- $build_command
                bear

                clang-tools
                cppcheck
                cmake
                ninja

                # can be used to flash using: dfu-util -a 0 -s 0x8023000 -D ./binary.bin
                dfu-util
                libusb1
                libusb1.dev

                # for additional header files
                glibc_multi.dev

                gdb

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

              ]);
            multiPkgs = pkgs: [ ];
            runScript = shell;
          }
        );
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            fhs
          ];

          shellHook = ''
            echo "Activating fhs env using ${shell} as shell."
            echo "Shell can be changed from inside flake.nix"
            ${fhs-name}
            exit
          '';
        };

        formatter = pkgs.nixfmt-tree;
      }
    );
}
