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

#include "Renderer.hpp"
#include "Common/Math.hpp"
#include <string_view>

namespace GL
{
    using namespace std::string_view_literals;

    constexpr std::string_view DEFAULT_VERTEX_SHADER =
        "#version 410 core\n"
        "\n"
        "uniform MVP \n"
        "{\n"
        "\tmat4 Projection;\n"
        "};\n"
        "\n"
        "layout (location = 0) in vec2 VertexPosition;\n"
        "layout (location = 1) in vec2 TextureCoords;\n"
        "layout (location = 2) in vec4 VertexColor;\n"
        "\n"
        "out vec2 TextureUV;\n"
        "out vec4 VColor;\n"
        "out gl_PerVertex \n"
        "{\n"
        "\tvec4 gl_Position;\n"
        "};\n"
        "\n"
        "void main()\n"
        "{\n"
        "\tTextureUV = TextureCoords;\n"
        "\tVColor = VertexColor;\n"
        "\tgl_Position = vec4(VertexPosition, 0.0, 1.0) * Projection;\n"
        "}"sv;

    constexpr std::string_view DEFAULT_FRAGMENT_SHADER =
        "#version 410 core\n"
        "\n"
        "in vec2 TextureUV;\n"
        "in vec4 VColor;\n"
        "uniform sampler2D DiffuseTexture;\n"
        "\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "\tFragColor = texture(DiffuseTexture, TextureUV) * VColor;\n"
        "}"sv;

    Renderer::Renderer()
    {
        for (int i = 0; i < MAX_IMAGES_PER_FRAME; ++i)
        {
            indices[(i * 6)] = (i * 4);
            indices[(i * 6) + 1] = ((i * 4) + 1);
            indices[(i * 6) + 2] = ((i * 4) + 2);

            indices[(i * 6) + 3] = (i * 4);
            indices[(i * 6) + 4] = ((i * 4) + 3);
            indices[(i * 6) + 5] = ((i * 4) + 1);
        }

        std::optional<GLuint> vtx =
            GL::CreateShaderProgram(GL_VERTEX_SHADER, DEFAULT_VERTEX_SHADER);
        std::optional<GLuint> frag =
            GL::CreateShaderProgram(GL_FRAGMENT_SHADER, DEFAULT_FRAGMENT_SHADER);

        pipeline = GL::CreatePipeline(vtx.value_or(0), frag.value_or(0)).value_or(0);

        auto vtx_size = static_cast<GLsizei>(sizeof(Vertex) * vertices.size());
        vertex_buffer = GL::CreateBuffer<Vertex>(GL_ARRAY_BUFFER, vtx_size, vertices);

        auto idx_size = static_cast<GLsizei>(sizeof(uint32_t) * indices.size());
        index_buffer = GL::CreateBuffer<uint32_t>(GL_ELEMENT_ARRAY_BUFFER, idx_size, indices);

        vao = GL::CreateVAO();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        GL::SetVAOElement(vao, 0, 2, GL_FLOAT, false, sizeof(Vertex), static_cast<GLvoid *>(0));
        GL::SetVAOElement(vao, 1, 2, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<GLvoid *>(offsetof(Vertex, u)));

        GL::SetVAOElement(vao, 2, 4, GL_FLOAT, true, sizeof(Vertex),
                          reinterpret_cast<GLvoid *>(offsetof(Vertex, color)));

        auto mtx_size = static_cast<GLsizei>(sizeof(float) * matrix.size());
        uniform_buffer = GL::CreateBuffer<float>(GL_UNIFORM_BUFFER, mtx_size, matrix);
    }

    Renderer::~Renderer()
    {
        GL::DestroyBuffer(uniform_buffer);
        GL::DestroyVAO(vao);
        GL::DestroyBuffer(vertex_buffer);
        GL::DestroyBuffer(index_buffer);
        GL::DestroyPipeline(pipeline);
    }

    void Renderer::reset_state(float screen_width, float screen_height)
    {
        vertex_offset = 0;
        index_offset = 0;

        Common::Math::OrthroProject(matrix, 0, screen_width, 0, screen_height, 0, 1);
        GL::UpdateBufferData<float>(uniform_buffer, GL_UNIFORM_BUFFER, matrix);
    }

    void Renderer::draw_image(GLuint texture, float x, float y, float width, float height,
                              const Color &color)
    {
        if (vertex_offset >= vertices.size())
            return;

        constexpr std::array<float, 4> u_list{0.0f, 1.0f, 0.0f, 1.0f};
        constexpr std::array<float, 4> v_list{0.0f, 1.0f, 1.0f, 0.0f};
        for (int i = 0; i < 4; ++i)
        {
            vertices[vertex_offset + i] = {
                .x = x + (width * u_list[i]),
                .y = y + (height * v_list[i]),
                .u = u_list[i],
                .v = v_list[i],
                .color = color,
            };
        }
        vertex_offset += 4;

        GL::UpdateBufferData<Vertex>(vertex_buffer, GL_ARRAY_BUFFER, vertices);

        glBindProgramPipeline(pipeline);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
        GL::BindTextureToSlot(0, texture);
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

        glDrawElements(GL_TRIANGLES, INDICES_PER_IMAGE, GL_UNSIGNED_INT,
                       reinterpret_cast<void *>(index_offset * sizeof(uint32_t)));

        index_offset += 6;
    }
}