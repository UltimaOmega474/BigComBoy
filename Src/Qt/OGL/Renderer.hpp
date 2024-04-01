/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <QOpenGLFunctions_4_1_Core>
#include <array>
#include <cinttypes>

namespace QtFrontend {
    class GLFunctions;
    constexpr size_t MAX_IMAGES_PER_FRAME = 32;
    constexpr size_t VERTICES_PER_IMAGE = 4;
    constexpr size_t INDICES_PER_IMAGE = 6;

    struct Color {
        float r = 0;
        float g = 0;
        float b = 0;
        float a = 0;
    };

    struct Vertex {
        float x = 0;
        float y = 0;
        float u = 0;
        float v = 0;
        Color color;
    };

    class Renderer {
    public:
        Renderer(GLFunctions *functions);
        Renderer(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer &operator=(Renderer &&) = delete;
        ~Renderer();

        void reset_state(float screen_width, float screen_height);
        void draw_image(GLuint texture, float x, float y, float width, float height,
                        const Color &color = Color{.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f});

    private:
        GLuint pipeline = 0;
        GLuint vao = 0, vertex_buffer = 0, index_buffer = 0, uniform_buffer = 0;

        int32_t vertex_offset = 0, index_offset = 0;
        std::array<Vertex, VERTICES_PER_IMAGE * MAX_IMAGES_PER_FRAME> vertices{};
        std::array<uint32_t, INDICES_PER_IMAGE * MAX_IMAGES_PER_FRAME> indices{};
        std::array<float, 16> matrix{};
        GLFunctions *glfn = nullptr;
    };
}