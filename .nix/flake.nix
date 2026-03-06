{
  description = "C++ development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
      config = {
        allowUnfree = true;
      };
    };
  in {
    formatter.${system} = pkgs.alejandra;

    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        gcc
        gdb
        cmake
        binutils
        pkg-config

        ffmpeg
        sdl3
        sdl3-image
        sdl3-ttf

        valgrind
        kdePackages.kcachegrind 
        perf
      ];
    };
  };
}

