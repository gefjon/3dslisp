{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    devkitNix.url = "github:bandithedoge/devkitNix";
  };

  outputs = {
    self, nixpkgs, flake-utils, devkitNix, ...
  }: flake-utils.lib.eachDefaultSystem (system:
    let pkgs = import nixpkgs {
          inherit system;
          overlays = [devkitNix.overlays.default];
        };
    in {
      devShells.default = pkgs.mkShell {
        buildInputs = [pkgs.devkitNix.devkitARM];
        shellHook = ''
          ${pkgs.devkitNix.devkitARM.shellHook}
        '';
      };
      packages.default = pkgs.stdenv.mkDerivation {
        name = "3dslisp";
        src = ./.;
        preBuild = pkgs.devkitNix.devkitARM.shellHook;
        installPhase = ''
          cp 3dslisp.3dsx $out
        '';
      };
    }
  );
}
