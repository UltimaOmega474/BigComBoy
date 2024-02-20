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
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <optional>
#include <span>
#include <string_view>

namespace GL
{
    GLuint CreateVAO();
    void SetVAOElement(GLuint vao, GLuint index, GLint num_components, GLenum component_type,
                       GLboolean normalize, GLsizei stride, GLvoid *offset);
    void DestroyVAO(GLuint vao);

    GLuint CreateTexture(GLsizei width, GLsizei height);
    void UpdateTextureData(GLuint texture, GLsizei width, GLsizei height,
                           std::span<uint8_t> pixels);
    void DestroyTexture(GLuint texture);

    template <class T>
    GLuint CreateBuffer(GLenum type, GLsizeiptr buffer_size, std::span<T> init_data)
    {
        GLuint buffer = 0;
        glGenBuffers(1, &buffer);
        glBindBuffer(type, buffer);
        glBufferData(type, buffer_size, init_data.empty() ? nullptr : init_data.data(),
                     GL_DYNAMIC_DRAW);
        return buffer;
    }

    template <class T> void UpdateBufferData(GLuint buffer, GLenum buffer_type, std::span<T> data)
    {
        glBindBuffer(buffer_type, buffer);
        glBufferSubData(buffer_type, 0, data.size() * sizeof(T), data.data());
    }

    void DestroyBuffer(GLuint buffer);

    std::optional<GLuint> CreateShaderProgram(GLenum shader_stage, std::string_view source);
    void BindSamplerToTextureSlot(GLuint shader, std::string_view name, GLint slot);
    std::optional<GLuint> CreatePipeline(GLuint vertex_shader, GLuint fragment_shader);
    void DestroyPipeline(GLuint pipeline);
}