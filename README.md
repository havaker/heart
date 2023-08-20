# Julia's Parametric Heart Surface Renderer

C++ stdlib-only renderer that generates a sequence of images to create an
animation of a rotating heart-shaped surface. B-day gift for @ainnnia :)))

https://github.com/havaker/heart/assets/41525193/32af7cde-45d6-4f82-8fc8-b66cdc2d927c

## How it Works

[Julia's parametric heart surface](https://math.stackexchange.com/a/3935220) is defined by the following equations:
```math
\begin{align*}
   x &= \left(15 \sin{\left(u \right)} - 4 \sin{\left(3 u \right)}\right) \sin{\left(v \right)} \\
   y &= 8 \cos{\left(v \right)} \\
   z &= \left(15 \cos{\left(u \right)} - 5 \cos{\left(2 u \right)} - 2 \cos{\left(3 u \right)} - \cos{\left(4 u \right)}\right) \sin{\left(v \right)}
\end{align*}
```

Given a function $`f(u, v) = (x, y, z)`$ we can sample a point belonging to the heart surface by sampling $`(u, v) \in [0, 2 \pi] \times [0, \pi]`$.
Drawing such point is done by a (somewhat) standard series of projections, that map a 3D point to 2D pixel coordinates.
By sampling enough points, a high quality picture can be produced.

### Code structure

The heart surface is defined in `src/codegen/generate.py`.
Functions that return sampled surface points & normal vectors are expressed in Python using SymPy.

SymPy provides C code generation capabilites, they are used to create `src/codegen/surface.{c,h}` files:
https://github.com/havaker/heart/blob/c9f098772d69b0f0c4fb2aff43e4086950157be5/src/codegen/surface.h#L13-L14

`src/main.cc` contains the renderer code. It uses `src/codegen/surface.h` and `src/linalg.hh` (linear algebra module).
The renderer:
- samples the heart surface in the parameter domain
- applies a simplified Phong lighting model
- projects the points onto a 2D plane, using z-buffer to determine visibility
- outputs the rendered frames in PPM format

For more details, see the comments in the source code. They should be quite
verbose.

## Running

The simplest way to run the renderer is to use [Nix](https://nixos.org/):
```fish
nix run github:havaker/heart#video heart.mp4
```

That command will fetch the source code, build the renderer, and render the
animation video to `heart.mp4`.

If you don't have Nix, you can still build the renderer manually using the
provided `CmakeLists.txt` file. To render the animation, install `ffmpeg` and
follow the steps described in the `flake.nix` file.

## Development

To develop the renderer, you can use the provided `flake.nix` file to create a
development environment with all the necessary dependencies (Nix required). To
enter the development environment, run `nix develop` in the cloned repository.

In that environment, you can use the `./run.sh` script to build and run the
preview animation.

## Compatibility

**x86-64 Linux** only. It should be possible to run the renderer on other platforms
by simply modyfing the `system = "..."` line in `flake.nix`, but I haven't
tested it.
