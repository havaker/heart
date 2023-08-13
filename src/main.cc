#include <cstddef>
#include <iostream>
#include <numbers>
#include <algorithm>
#include <vector>

#include "linalg.hh"

using std::numbers::pi;

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

    void clear() {
        std::fill(_pixels.begin(), _pixels.end(), vec<3>{0.0, 0.0, 0.0});
    }

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

class surface {
public:
    struct point {
        vec<4> position = {0.0, 0.0, 0.0, 1.0};
        vec<4> normal = {0.0, 0.0, 0.0, 0.0};
    };

    // Returns a sampled point on the surface in world coordinates.
    point sample(vec<2> uv) const {
        uv = to_sample_space * uv;
        point result;

        ffi::surface(uv[0], uv[1], result.position.data());
        ffi::normal(uv[0], uv[1], result.normal.data());

        result.position = _world_transform * _model_transform * result.position;
        result.normal = _world_normal * _model_normal * result.normal;
        return result;
    }

    void set_transform(mat<4, 4> transform, mat<4, 4> normal) {
        _world_transform = transform;
        _world_normal = normal;
    }

private:
    mat<4, 4> _world_transform = translate({0.0, 0.0, 0.0});
    mat<4, 4> _world_normal = translate({0.0, 0.0, 0.0});

    const mat<4, 4> _model_normal = rotate_along_x(-pi / 2);
    const mat<4, 4> _model_transform = scale(1.0 / 20) * _model_normal;

    const mat<2,2> to_sample_space = {{
        {2.0 * std::numbers::pi, 0.0},
        {0.0, std::numbers::pi}
    }};
};

class renderer {
public:
    renderer(size_t width, size_t height) 
    : _image(width, height), _z_buffer(width, height) {
        const double fov = pi / 4;

        // Given a field of view and an image size, compute the focal lengths.
        auto fx = static_cast<double>(width) / 2 / std::tan(fov / 2);
        auto fy = static_cast<double>(height) / 2 / std::tan(fov / 2);

        auto ox = static_cast<double>(width) / 2;
        auto oy = static_cast<double>(height) / 2;

        _camera_matrix = mat<3, 3>{{
            {fx,  0.0, ox},
            {0.0, -fy,  oy},
            {0.0, 0.0, 1.0}
        }};

    }

    void render(size_t fps, size_t length, size_t quality) {
        for (size_t frame = 0; frame < fps * length; ++frame) {
            auto t = static_cast<double>(frame) / fps;
            render_single_frame(t, quality);
            _image.print_ppm();
        }
    }

private:
    void render_single_frame(double t, size_t quality) {
        auto angle = t * pi / 2;

        auto normal = rotate_along_y(angle);
        auto transform = translate(_surface_position) * normal;
        _surface.set_transform(transform, normal);

        _image.clear();
        _z_buffer.clear();

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

    void render_single_sample(vec<2> uv) {
        surface::point p = _surface.sample(uv);

        const double ambient_strength = 0.1;
        vec<3> ambient = ambient_strength * _ambient_color;

        double diff = std::max(dot(p.normal, _light_direction), 0.0);
        vec<3> diffuse = diff * _light_color;
        
        const double specular_strength = 0.9;
        vec<4> view_dir = {0, 0, 1, 0};
        auto reflected = reflect(_light_direction * -1.0, p.normal);
        double spec = std::pow(std::max(dot(view_dir, reflected), 0.0), 64);
        vec<3> specular = specular_strength * spec * _light_color;

        vec<3> color = (ambient + diffuse + specular) * _surface_color;

        vec<2> image_pos = from_homogeneus(_camera_matrix * from_homogeneus(p.position));

        long x = static_cast<long>(image_pos[0]);
        long y = static_cast<long>(image_pos[1]);
        auto z = from_homogeneus(p.position)[2];

        if (x < 0 || x >= (long) _image.width() || y < 0 || y >= (long) _image.height()) {
            return;
        }
        if (_z_buffer(x, y) < z) {
            return;
        }

        _z_buffer(x, y) = z;
        _image(x, y) = color;
    }

    const vec<3> _ambient_color = {0.1, 0, 0};
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
    const char* help_message = "Render a rotating surface.\n"
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
