#include "Sidekick/Core/Application.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <print>
#include <span>
#include <vector>

#include "Sidekick/Core/Event.hpp"
#include "Sidekick/Core/Input.hpp"
#include "Sidekick/Graphics/Program.hpp"

namespace Sidekick::Core
{

constexpr std::string_view simple_vert = R"(#version 330 core

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec4 inColor;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 proj;
    uniform mat4 mvp;

    out vec4 vertexColor;

    void main()
    {
        gl_Position = mvp * vec4(inPosition, 1.0);

        vertexColor = inColor;
    }
    )";

constexpr std::string_view simple_frag = R"(#version 330 core

    in vec4 vertexColor;
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(vertexColor);
    }
    )";

constexpr std::string_view static_vert = R"(#version 330 core

    layout(location = 0) in vec2 aPos;

    void main()
    {
        // Hardcoded triangle vertices
        vec2 positions[3] = vec2[](
                vec2(-0.5, -0.5),
                vec2(0.5, -0.5),
                vec2(0.0, 0.5)
            );
        gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    }
    )";

constexpr std::string_view static_frag = R"(#version 330 core

    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    )";

struct Camera
{
    glm::vec3 Position{0, 2, 5}, Target{0, 0, 0}, Up{0, 1, 0};
    float Fov{glm::radians(60.0)}, Aspect{16.0 / 9.0}, Near{0.1}, Far{100.0};
};

class Mesh
{
  public:
    Mesh(std::span<glm::vec3> vertices, std::span<glm::vec4> colors, std::span<std::uint32_t> indices = {})
        : m_Vertices(vertices.begin(), vertices.end()), m_Colors(colors.begin(), colors.end()),
          m_Indices(indices.begin(), indices.end())
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_PositionsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_PositionsVBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), m_Vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &m_ColorsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_ColorsVBO);
        glBufferData(GL_ARRAY_BUFFER, m_Colors.size() * sizeof(glm::vec4), m_Colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(std::uint32_t), m_Indices.data(),
                     GL_STATIC_DRAW);
    }

    ~Mesh()
    {
        glDeleteBuffers(1, &m_PositionsVBO);
        glDeleteBuffers(1, &m_ColorsVBO);
        glDeleteBuffers(1, &m_EBO);
        glDeleteVertexArrays(1, &m_VAO);
    }

    void Render() const
    {
        glBindVertexArray(m_VAO);

        if (m_Indices.empty())
        {
            glDrawArrays(GL_TRIANGLES, 0, m_Vertices.size());
        }
        else
        {
            glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);
    }

  private:
    std::vector<glm::vec3> m_Vertices;
    std::vector<glm::vec4> m_Colors;
    std::vector<std::uint32_t> m_Indices;
    GLuint m_VAO;
    GLuint m_PositionsVBO, m_ColorsVBO;
    GLuint m_EBO{0};
};

Application::Application()
    : m_Window{WindowSpecification{
          .Width = 800, .Height = 600, .Title = "Sidekick Engine", .OnEvent = [this](Event& event) { OnEvent(event); }}}
{
    glfwInit();
    m_Window.Create();
    Input::Init();
}

Application::~Application()
{
    m_Window.Destroy();
    glfwTerminate();
}

void Application::Run()
{
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    Graphics::Program program{simple_vert, simple_frag};

    // Define cube vertex positions
    std::vector<glm::vec3> positions = {
        {-0.7f, -0.7f, -0.7f}, // 0
        {0.7f, -0.7f, -0.7f},  // 1
        {0.7f, 0.7f, -0.7f},   // 2
        {-0.7f, 0.7f, -0.7f},  // 3
        {-0.7f, -0.7f, 0.7f},  // 4
        {0.7f, -0.7f, 0.7f},   // 5
        {0.7f, 0.7f, 0.7f},    // 6
        {-0.7f, 0.7f, 0.7f}    // 7
    };

    // Define cube vertex colors
    std::vector<glm::vec4> colors = {
        {1.f, 0.f, 0.f, 1.f}, // Red
        {0.f, 1.f, 0.f, 1.f}, // Green
        {0.f, 0.f, 1.f, 1.f}, // Blue
        {1.f, 1.f, 0.f, 1.f}, // Yellow
        {1.f, 0.f, 1.f, 1.f}, // Magenta
        {0.f, 1.f, 1.f, 1.f}, // Cyan
        {1.f, 1.f, 1.f, 1.f}, // White
        {0.f, 0.f, 0.f, 1.f}  // Black
    };

    // Define cube indices
    std::vector<std::uint32_t> indices = {// Front face
                                          4, 5, 6, 4, 6, 7,
                                          // Back face
                                          0, 1, 2, 0, 2, 3,
                                          // Left face
                                          0, 3, 7, 0, 7, 4,
                                          // Right face
                                          1, 5, 6, 1, 6, 2,
                                          // Bottom face
                                          0, 1, 5, 0, 5, 4,
                                          // Top face
                                          3, 2, 6, 3, 6, 7};

    Mesh mesh{positions, colors, indices};

    Camera camera{};

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    while (m_Running)
    {
        Input::Update();
        glfwPollEvents();

        if (Input::IsKeyDown(KeyCode::Escape))
        {
            m_Running = false;
            break;
        }

        if (Input::IsKeyDown(KeyCode::W))
        {
            glm::vec3 forward = glm::normalize(camera.Target - camera.Position);
            float speed = 0.005f;
            camera.Position += forward * speed;
            camera.Target += forward * speed;
        }
        if (Input::IsKeyDown(KeyCode::S))
        {
            glm::vec3 forward = glm::normalize(camera.Target - camera.Position);
            float speed = 0.005f;
            camera.Position -= forward * speed;
            camera.Target -= forward * speed;
        }

        auto [width, height] = m_Window.GetSize();

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Aspect = static_cast<float>(width) / static_cast<float>(height);

        glm::mat4 model{1.0f};
        glm::mat4 view = glm::lookAt(camera.Position, camera.Target, camera.Up);
        glm::mat4 proj = glm::perspective(camera.Fov, camera.Aspect, camera.Near, camera.Far);
        glm::mat4 mvp = proj * view * glm::mat4{1.0f};

        program.Bind();

        auto program_id = program.GetId();
        glUniformMatrix4fv(glGetUniformLocation(program_id, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(program_id, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program_id, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(program_id, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

        mesh.Render();

        m_Window.SwapBuffers();
    }
}

void Application::OnEvent(Event& event)
{
    EventDispatcher dispatcher{&event};
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) { return OnWindowClose(event); });

    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& event) {
        Input::OnKeyPressedEvent(event);
        return false;
    });

    dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& event) {
        Input::OnKeyReleasedEvent(event);
        return false;
    });
}

bool Application::OnWindowClose(WindowCloseEvent& event)
{
    m_Running = false;
    return true;
}

} // namespace Sidekick::Core
