#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numbers>
#include <vector>

#include "linalg.hh"

using std::numbers::pi;

// Represents an image with pixels of type `vec<3>`.
// Colors are represented as RGB vectors with values in the range [0, 1].
class image {
public:
    image(size_t width, size_t height) : _width(width), _height(height) {
        _pixels.resize(width * height);
    }

    vec<3>& operator()(size_t x, size_t y) {
        return _pixels[x + y * _width];
    }

    size_t width() const {
        return _width;
    }

    size_t height() const {
        return _height;
    }

    // Clears the image by setting all pixels to black.
    void clear() {
        std::fill(_pixels.begin(), _pixels.end(), vec<3>{0.0, 0.0, 0.0});
    }

    // Prints the image in PPM format to stdout.
    // https://en.wikipedia.org/wiki/Netpbm#File_formats
    void print_ppm() {
        std::cout << "P3\n" << _width << " " << _height << "\n255\n";
        for (size_t y = 0; y < _height; ++y) {
            for (size_t x = 0; x < _width; ++x) {
                auto& pixel = (*this)(x, y);
                auto r = static_cast<int>(pixel[0] * 256);
                auto g = static_cast<int>(pixel[1] * 256);
                auto b = static_cast<int>(pixel[2] * 256);

                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);

                std::cout << r << " " << g << " " << b << "\n";
            }
        }
    }

private:
    std::vector<vec<3>> _pixels;
    size_t _width;
    size_t _height;
};

// Represents a depth buffer.
//
// The depth buffer is used to keep track of the closest point to the camera at
// each pixel. The depth buffer is initialized with the maximum possible depth at
// each pixel. When a point is drawn at a pixel, its depth is compared to the
// depth stored in the buffer. If the point is closer to the camera, it is
// drawn, and its depth is stored in the buffer.
class z_buffer {
public:
    z_buffer(size_t width, size_t height) : _width(width) {
        _pixels.resize(width * height);
        clear();
    }

    double& operator()(size_t x, size_t y) {
        return _pixels[x + y * _width];
    }

    void clear() {
        std::fill(_pixels.begin(), _pixels.end(), std::numeric_limits<double>::max());
    }
private:
    std::vector<double> _pixels;
    size_t _width;
};

namespace ffi {
    extern "C" {
        #include "codegen/surface.h"
    }
}

// `surface` class provides a way to sample points on a pre-defined 3D surface.
//
// The surface is defined by a parametric equation, in python module
// `src/codegen/surface.py`. Thanks to `sympy`'s code generation abillities, the
// equation can be translated to C code and used here (see the `ffi` namespace
// and the `codegen` directory for more details) .
//
// The surface is defined in a coordinate system where the surface is centered
// at the origin. The surface is then transformed to world coordinates by
// applying a model transform and a world transform. The model transform is
// applied first, and is used to scale and rotate the surface. The world
// transform is applied second, and is used to position the surface in the
// world.
class surface {
public:
    // The convention kept throughout the code is that points and vectors are
    // using homogeneous coordinates. A point is represented as a 4-dimensional
    // vector, where the last coordinate is 1.0, and a vector is represented as
    // a 4-dimensional vector, where the last coordinate is 0.0. This allows
    // using the same transformation matrices for both points and vectors.
    struct point {
        vec<4> position = {0.0, 0.0, 0.0, 1.0};
        vec<4> normal = {0.0, 0.0, 0.0, 0.0};
    };

    // Samples point on the surface in world coordinates.
    // Input vector `uv`
    // (which contains parameters for the surface equation) should be in the
    // range [0, 1] x [0, 1].
    // Returns a point on the surface in world coordinates together with the
    // normal vector at that point.
    point sample(vec<2> uv) const {
        uv = to_sample_space * uv;
        point result;

        ffi::surface(uv[0], uv[1], result.position.data());
        ffi::normal(uv[0], uv[1], result.normal.data());

        result.position = _world_transform * _model_transform * result.position;
        result.normal = _world_normal * _model_normal * result.normal;
        return result;
    }

    // Sets the world transform for the surface.
    //
    // The world transform is used to position the surface in the world - it
    // maps the surface's local coordinate system to the world coordinate
    // system.
    //
    // The transform is split into two parts. The first one contains all
    // world-space transformations, such as translation, rotation, and scaling.
    // The second one should contain only transformations that can be
    // applied to normals (such as rotation).
    void set_transform(mat<4, 4> transform, mat<4, 4> normal) {
        _world_transform = transform;
        _world_normal = normal;
    }

private:
    mat<4, 4> _world_transform = translate({0.0, 0.0, 0.0});
    mat<4, 4> _world_normal = translate({0.0, 0.0, 0.0});

    // Model transform is used to scale the surface to a reasonable size and
    // rotate it so that its larger dimensions are along x y axes.
    //
    // Similarly to the world transform, the model transform is split into two
    // parts.
    const mat<4, 4> _model_normal = rotate_along_x(-pi / 2);
    const mat<4, 4> _model_transform = scale(1.0 / 20) * _model_normal;

    // The equation for the surface should be sampled in range [0, 2pi] x [0,
    // pi]. This matrix is used to transform the input vector to that range.
    const mat<2,2> to_sample_space = {{
        {2.0 * std::numbers::pi, 0.0},
        {0.0, std::numbers::pi}
    }};
};

// `renderer` class is used to generate series of images of a rotating surface.
//
// The renderer uses a `surface` object to sample points on the surface. It
// applies lighting to the points using a simple Phong model. It then projects
// the points onto a 2D plane using a perspective projection. Finally, it uses
// a z-buffer to determine which points are visible and which are not.
//
// Output images are written to stdout in PPM format.
class renderer {
public:
    // Creates a renderer with a given image size.
    renderer(size_t width, size_t height) 
    : _image(width, height), _z_buffer(width, height) {
        // Camera is positioned at (0, 0, 0) and looks along the positive z
        // axis. It follows the pinhole camera model (see
        // https://en.wikipedia.org/wiki/Pinhole_camera_model for details).

        // Field of view is 45 degrees.
        const double fov = pi / 4;

        // Given a field of view and an image size, compute the focal lengths.
        auto fx = static_cast<double>(width) / 2 / std::tan(fov / 2);
        auto fy = static_cast<double>(height) / 2 / std::tan(fov / 2);

        auto ox = static_cast<double>(width) / 2;
        auto oy = static_cast<double>(height) / 2;

        // Camera matrix (also known as projection matrix) maps points in the
        // view space to points in image. In this renderer, the view space is
        // the same as the world space (Relations between world, view, and
        // projection spaces are described here:
        // https://learnopengl.com/Getting-started/Coordinate-Systems and there:
        // https://gamedev.stackexchange.com/a/56203).
        //
        // See https://en.wikipedia.org/wiki/Camera_matrix and
        // https://www.baeldung.com/cs/focal-length-intrinsic-camera-parameters#camera-intrinsic-matrix
        // for details on how the camera matrix is constructed.
        _camera_matrix = mat<3, 3>{{
            {fx,  0.0, ox},
            {0.0, -fy,  oy},
            {0.0, 0.0, 1.0}
        }};

    }

    // Given frame per second, length of the animation in seconds, and quality
    // of the output image, renders the animation and writes it to stdout.
    void render(size_t fps, size_t length, size_t quality) {
        for (size_t frame = 0; frame < fps * length; ++frame) {
            auto t = static_cast<double>(frame) / fps;
            render_single_frame(t, quality);
            _image.print_ppm();
        }
    }

private:
    // Renders a single frame of the animation to the `_image` buffer. The frame
    // is determined by the time `t` in seconds. The `quality` parameter
    // determines how many samples are taken per pixel (`quality^2` samples per
    // pixel).
    void render_single_frame(double t, size_t quality) {
        // Derive surface rotation angle from frame's time.
        auto angle = t * pi / 2;

        // Rotate the surface around y axis and move it to `_surface_position`.
        auto normal = rotate_along_y(angle);
        auto transform = translate(_surface_position) * normal;
        _surface.set_transform(transform, normal);

        _image.clear();
        _z_buffer.clear();

        // Sample the [0, 1] x [0, 1] square `quality^2` times per pixel.
        // Render the surface at each sample point.
        for (size_t y = 0; y < _image.height() * quality; ++y) {
            for (size_t x = 0; x < _image.width() * quality; ++x) {
                vec<2> uv = {
                    (x + 0.5) / _image.width() / quality,
                    (y + 0.5) / _image.height() / quality
                };

                render_single_sample(uv);
            }
        }
    }

    // Renders a sampled 3D point to the `_image` buffer.
    //
    // Note that we are rendering each `uv` sample as a single pixel. `uv`
    // samples do not correspond to pixels on the image. By rendering a
    // sufficiently large number of samples, we can hope to get a good coverage
    // of the image. This isn't ideal, but it is simple and works well enough
    // for this example.
    void render_single_sample(vec<2> uv) {
        // Feed the surface parameter eqation with the sampled parameters `uv` to
        // get a 3D point on the surface.
        surface::point p = _surface.sample(uv);

        // Use Phong lighting model to compute the color of the point.
        // Phong model is a simple model that approximates the way light
        // interacts with a surface. It is composed of three components:
        // ambient, diffuse, and specular.
        //
        // See https://en.wikipedia.org/wiki/Phong_reflection_model for details.

        // Ambient component is a constant color that is added to the surface
        // color. It represents the light that is reflected from other surfaces
        // in the scene.
        const double ambient_strength = 0.1;
        vec<3> ambient = ambient_strength * _ambient_color;

        // Diffuse component is computed using the Lambert's cosine law. It
        // represents the light that is reflected from the surface in all
        // directions equally.
        double diff = std::max(dot(p.normal, _light_direction), 0.0);
        vec<3> diffuse = diff * _light_color;
        
        // Specular component is computed using the Phong's reflection model.
        // It represents the light that is reflected from the surface in a
        // mirror-like fashion.
        const double specular_strength = 0.9;
        vec<4> view_dir = {0, 0, 1, 0};
        auto reflected = reflect(_light_direction * -1.0, p.normal);
        double spec = std::pow(std::max(dot(view_dir, reflected), 0.0), 64);
        vec<3> specular = specular_strength * spec * _light_color;

        // Combine all components to get the final color of the point.
        vec<3> color = (ambient + diffuse + specular) * _surface_color;

        // Project the point to the image plane.
        vec<2> image_pos = from_homogeneus(_camera_matrix * from_homogeneus(p.position));

        long x = static_cast<long>(image_pos[0]);
        long y = static_cast<long>(image_pos[1]);
        auto z = from_homogeneus(p.position)[2];

        // Clip the point if it is outside of the image plane.
        if (x < 0 || x >= (long) _image.width() || y < 0 || y >= (long) _image.height()) {
            return;
        }

        // Discard the point unless it is closer than the previously rendered
        // point at the same position.
        if (_z_buffer(x, y) < z) {
            return;
        }

        _z_buffer(x, y) = z;
        _image(x, y) = color;
    }

    // Slightly red ambient light.
    const vec<3> _ambient_color = {0.1, 0, 0};

    // White-ish light coming from the top-left.
    const vec<3> _light_color = {1, 0.9, 0.8};
    const vec<4> _light_direction = normalize<4>({-0.5, -0.5, 1.0, 0});

    surface _surface;
    const vec<3> _surface_color = {0.9, 0.3, 0.5};
    const vec<3> _surface_position = {0, 0, 4};

    mat<3, 3> _camera_matrix;

    image _image;
    z_buffer _z_buffer;
};

int main(int argc, char** argv) {
    size_t width = 256,
           height = 256,
           fps = 60,
           length = 4,
           quality = 3;

    const char* usage = " [--help] [--width <width>] [--height <height>] [--fps <fps>] [--length <length>] [--quality <quality>]";
    const char* help_message = "Render a rotating surface by printing a sequence of PPM images to the\n"
                               "standard output.\n"
                               "\n"
                               "Arguments:\n"
                               "  --width <width>     Width of the output image in pixels. Default: 256.\n"
                               "  --height <height>   Height of the output image in pixels. Default: 256.\n"
                               "  --fps <fps>         Number of frames per second. Default: 60.\n"
                               "  --length <length>   Length of the animation in seconds. Default: 4.\n"
                               "  --quality <quality> Controls the quality of the output image. Higher values\n"
                               "                      result in better quality but longer rendering times. Default: 3.\n";


    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--help") {
            std::cout << argv[0] << usage << std::endl;
            std::cout << help_message << std::endl;
            return 0;
        } else if (std::string(argv[i]) == "--width") {
            width = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "--height") {
            height = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "--fps") {
            fps = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "--length") {
            length = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "--quality") {
            quality = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            std::cerr << "Usage " << argv[0] << usage << std::endl;
            return 1;
        }
    }

    renderer r(width, height);
    r.render(fps, length, quality);

    return 0;
}
