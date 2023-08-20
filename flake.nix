{
  description = "Parametric surface renderer";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let pkgs = import nixpkgs {
      system = "x86_64-linux";
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

    packages.x86_64-linux.video = pkgs.writeScriptBin "video" ''
      #!${pkgs.python311}/bin/python
      import argparse
      import subprocess

      parser = argparse.ArgumentParser(description='Render a parametric surface to a video file.')
      parser.add_argument('output_file', type=str, help='output video file')
      parser.add_argument('--fps', type=int, default=60, help='frames per second of output video')
      parser.add_argument('args', nargs=argparse.REMAINDER, help='arguments to pass to renderer')
      args = parser.parse_args()

      renderer = subprocess.Popen(
        [
          "${self.packages.x86_64-linux.default}/bin/renderer",
          "--fps", str(args.fps),
          *args.args
        ],
        stdout=subprocess.PIPE
      )

      ffmpeg = subprocess.Popen(
        [
          "${pkgs.ffmpeg_6-headless}/bin/ffmpeg",
          "-y",
          "-f", "image2pipe",
          "-framerate", str(args.fps),
          "-i", "-",
          "-c:v", "libx264",
          "-preset", "veryslow",
          "-crf", "18",
          "-pix_fmt", "yuv420p",
          "-f", "mp4",
          args.output_file
        ],
        stdin=renderer.stdout
      )

      ffmpeg.communicate()
    '';

    apps.x86_64-linux.renderer = {
      type = "app";
      program = "${self.packages.x86_64-linux.default}/bin/renderer";
    };

    apps.x86_64-linux.video = {
      type = "app";
      program = "${self.packages.x86_64-linux.video}/bin/video";
    };
  };
}
