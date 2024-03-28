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

#include "GLFunctions.hpp"
#include <array>
#include <fmt/format.h>

namespace QtFrontend
{
    GLuint GLFunctions::create_vao()
    {
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        return vao;
    }

    void GLFunctions::set_vao_element(GLuint vao, GLuint index, GLint num_components,
                                      GLenum component_type, GLboolean normalize, GLsizei stride,
                                      GLvoid *offset)
    {
        glVertexAttribPointer(index, num_components, component_type, normalize, stride, offset);
        glEnableVertexAttribArray(index);
    }

    void GLFunctions::destroy_vao(GLuint vao) { glDeleteVertexArrays(1, &vao); }

    GLuint GLFunctions::create_texture(GLsizei width, GLsizei height)
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        return texture;
    }

    void GLFunctions::update_texture_data(GLuint texture, GLsizei width, GLsizei height,
                                          std::span<uint8_t> pixels)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                        pixels.data());
    }

    void GLFunctions::set_texture_filter(GLuint texture, GLint min_mag_filter)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, min_mag_filter);
    }

    void GLFunctions::destroy_texture(GLuint texture) { glDeleteTextures(1, &texture); }

    void GLFunctions::destroy_buffer(GLuint buffer) { glDeleteBuffers(1, &buffer); }

    std::optional<GLuint> GLFunctions::create_shader_program(GLenum shader_stage,
                                                             std::string_view source)
    {
        const char *data = source.data();

        GLsizei err_msg_length = 0;
        std::string err_msg_buffer{};
        err_msg_buffer.resize(512);

        GLuint program = glCreateShaderProgramv(shader_stage, 1, &data);
        glGetProgramInfoLog(program, static_cast<GLsizei>(err_msg_buffer.size()), &err_msg_length,
                            err_msg_buffer.data());

        if (err_msg_length)
        {
            fmt::print("#Shader Compiler Error:\n {} \n", err_msg_buffer);
            fflush(stdout);
            glDeleteProgram(program);
            return {};
        }

        return program;
    }

    std::optional<GLuint> GLFunctions::create_pipeline(GLuint vertex_shader, GLuint fragment_shader)
    {
        if (!vertex_shader || !fragment_shader)
            return {};

        GLuint pipeline = 0;
        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertex_shader);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragment_shader);

        return pipeline;
    }

    void GLFunctions::bind_sampler(GLuint shader, std::string_view name, GLint slot)
    {
        auto location = glGetUniformLocation(shader, name.data());
        glUniform1i(location, slot);
    }

    void GLFunctions::destroy_pipeline(GLuint pipeline)
    {
        std::array<GLuint, 2> programs{};
        GLsizei count = 0;
        glGetAttachedShaders(pipeline, programs.size(), &count, programs.data());

        glDeleteProgramPipelines(1, &pipeline);
        for (int i = 0; i < count; i++)
        {
            glDeleteProgram(programs[i]);
        }
    }
}
