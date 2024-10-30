#include <SDL2/SDL.h>

#include "../opengl_include.h"

const char *vertex_shader_source =
    "#version 410 core\n"
    "in vec2 position;\n"
    "in vec3 color;\n"
    "out vec3 fragment_color;\n"
    "void main() {\n"
    "    gl_Position = vec4(position, 1.0, 1.0);\n"
    "    fragment_color = color;\n"
    "}\n";

const char *fragment_shader_source =
    "#version 410 core\n"
    "in vec3 fragment_color;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "    color = vec4(fragment_color, 1.0);\n"
    "}\n";

GLuint compile_shader(GLenum shader_type, const char *source) {
  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[512];
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    printf("Shader compilation failed\n%s\n", info_log);
    exit(1);
  }

  return shader;
}

GLuint create_shader() {
  GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
  GLuint fragment_shader =
      compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar info_log[512];
    glGetProgramInfoLog(program, 512, NULL, info_log);
    printf("Shader linking failed\n%s\n", info_log);
    exit(1);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}

int main(int argc, const char *argv[]) {
  SDL_Init(SDL_INIT_EVERYTHING);

  // Use the latest (for macOS) version of OpenGL
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Window *window = SDL_CreateWindow(
      "Triangle",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      640,
      480,
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
  );
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GL stuff
  glClearColor(
      0x49 / 255.0,
      0x36 / 255.0,
      0x57 / 255.0,
      1
  ); // Set the clear color to #493657

  // data is stored as position & color
  float triangle_data[] = {
      0.0f,
      0.5f,

      // #FFD1BA
      1.0f,
      0.819f,
      0.729f,

      0.5f,
      -0.5f,

      // #CE7DA5
      0.807f,
      0.490f,
      0.647f,

      -0.5f,
      -0.5f,

      // #BEE5BF
      0.745f,
      0.8980f,
      0.749f,
  };

  // Create the vertex array object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create the vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(triangle_data),
      triangle_data,
      GL_STATIC_DRAW
  );

  // Start using the shaders defined at the start of the file
  GLuint program = create_shader();
  glUseProgram(program);

  // Enable position & color attributes
  GLint position_attribute = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(position_attribute);
  glVertexAttribPointer(
      position_attribute,
      2,
      GL_FLOAT,
      GL_FALSE,
      5 * sizeof(float),
      0
  );

  GLint color_attribute = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(color_attribute);
  glVertexAttribPointer(
      color_attribute,
      3,
      GL_FLOAT,
      GL_FALSE,
      5 * sizeof(float),
      (void *) (2 * sizeof(float))
  );

  // Main loop
  SDL_bool running = SDL_TRUE;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = SDL_FALSE;
          break;
        default:
          continue;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT); // Clear the background with color

    // Rendering
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(window); // Swap window buffers
    // Delay so that there's at least some time between frames
    SDL_Delay(1);
  }

  // Quit from OpenGL
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(program);

  // Quit from SDL
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
