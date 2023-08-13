{
  description = "Parametric surface renderer";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let pkgs = import nixpkgs {
      system = "x86_64-linux";
      config = {allowUnfree = true;};
    }; 
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        clang
        clang-tools

        cmake
        gnumake

        python311
        python311Packages.sympy
        python311Packages.matplotlib

        ffmpeg_6-full
      ];
    };

    packages.x86_64-linux.default = pkgs.stdenv.mkDerivation rec {
      name = "renderer";
      src = ./.;
      buildInputs = with pkgs; [
        clang
        cmake
        gnumake
      ];
      buildPhase = ''
        cmake .. -DCMAKE_BUILD_TYPE=Release
        make
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp renderer $out/bin
      '';
    };

    packages.x86_64-linux.render_video = pkgs.writeScriptBin "render_video" ''
      #!${pkgs.stdenv.shell}

      if [ $# -eq 0 ]; then
        echo "output file required"
        exit 1
      fi

      output_file=$1
      shift

      ${self.packages.x86_64-linux.default}/bin/renderer $@ | \
      ${pkgs.ffmpeg_6-full}/bin/ffmpeg -y -f image2pipe -framerate 60 -i - -c:v libx264 -crf 0 -f matroska -y $output_file
    '';


    apps.x86_64-linux.renderer = {
      type = "app";
      program = "${self.packages.x86_64-linux.default}/bin/renderer";
    };

    apps.x86_64-linux.video = {
      type = "app";
      program = "${self.packages.x86_64-linux.render_video}/bin/render_video";
    };
  };
}
