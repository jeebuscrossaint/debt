{
  description = "Command prefix handler that allows pasting commands with $ or > prompts";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
  flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = nixpkgs.legacyPackages.${system};

      # Define Windows flag outside of buildPhase for clarity
      windowsFlag = if pkgs.stdenv.hostPlatform.isWindows then "-D_WIN32" else "";

      # Check if we're building for Windows
      isWindows = pkgs.stdenv.hostPlatform.isWindows;

      # Get executable extension (empty on Unix, .exe on Windows)
      exeSuffix = pkgs.stdenv.hostPlatform.extensions.executable or "";

      debt = pkgs.stdenv.mkDerivation {
        name = "debt";
        version = "0.1.0";

        src = ./.;

        # Include both C and H files
        unpackPhase = ''
          cp ${./debt.c} debt.c
          cp ${./debt.h} debt.h
        '';

        buildPhase = ''
          $CC -o debt debt.c -O3 -march=native -flto -fomit-frame-pointer -pipe -DNDEBUG ${windowsFlag}
          strip debt
        '';

        installPhase = ''
          mkdir -p $out/bin
          cp debt${exeSuffix} $out/bin/debt${exeSuffix}
          ${if isWindows then ''
            cp $out/bin/debt${exeSuffix} "$out/bin/\$${exeSuffix}"
            cp $out/bin/debt${exeSuffix} "$out/bin/>${exeSuffix}"
          '' else ''
            ln -s debt $out/bin/$
            ln -s debt $out/bin/\>
          ''}
        '';

        meta = with pkgs.lib; {
          description = "Strips leading $ or > from commands for easy copy-pasting";
          platforms = platforms.all;
        };
      };
    in
    {
      packages = {
        default = debt;
        debt = debt;
      };

      apps = {
        default = {
          type = "app";
          program = "${debt}/bin/debt";
        };
      };
    }
  );
}
