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
#include <optional>
#include <span>
#include <string_view>

namespace QtFrontend {
    class GLFunctions : public QOpenGLFunctions_4_1_Core {
    public:
        GLuint create_vao();
        void set_vao_element(GLuint vao, GLuint index, GLint num_components, GLenum component_type,
                             GLboolean normalize, GLsizei stride, GLvoid *offset);
        void destroy_vao(GLuint vao);

        GLuint create_texture(GLsizei width, GLsizei height);
        void update_texture_data(GLuint texture, GLsizei width, GLsizei height,
                                 std::span<uint8_t> pixels);
        void set_texture_filter(GLuint texture, GLint min_mag_filter);
        void destroy_texture(GLuint texture);

        template <class T>
        GLuint create_buffer(GLenum type, GLsizeiptr buffer_size, std::span<T> init_data);
        template <class T>
        void update_buffer_data(GLuint buffer, GLenum buffer_type, std::span<T> data);
        void destroy_buffer(GLuint buffer);

        std::optional<GLuint> create_shader_program(GLenum shader_stage, std::string_view source);
        void bind_sampler(GLuint shader, std::string_view name, GLint slot);
        std::optional<GLuint> create_pipeline(GLuint vertex_shader, GLuint fragment_shader);
        void destroy_pipeline(GLuint pipeline);
    };

    template <class T>
    inline GLuint GLFunctions::create_buffer(GLenum type, GLsizeiptr buffer_size,
                                             std::span<T> init_data) {
        GLuint buffer = 0;
        glGenBuffers(1, &buffer);
        glBindBuffer(type, buffer);
        glBufferData(type, buffer_size, init_data.empty() ? nullptr : init_data.data(),
                     GL_DYNAMIC_DRAW);
        return buffer;
    }

    template <class T>
    inline void GLFunctions::update_buffer_data(GLuint buffer, GLenum buffer_type,
                                                std::span<T> data) {
        glBindBuffer(buffer_type, buffer);
        glBufferSubData(buffer_type, 0, data.size() * sizeof(T), data.data());
    }
}