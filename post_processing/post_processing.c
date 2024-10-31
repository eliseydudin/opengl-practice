#include <SDL2/SDL.h>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../stbi.h"
#include "post_processing.h"

const char *vertex_shader_source =
    "#version 410 core\n"
    "in vec2 position;\n"
    "in vec3 color;\n"
    "out vec2 frag_pos;\n"
    "out vec3 fragment_color;\n"

    "uniform float pos_x;\n"
    "uniform float pos_y;\n"

    "void main() {\n"
    "    gl_Position = vec4(position + vec2(pos_x, pos_y), 1.0, 1.0);\n"
    "    fragment_color = color;\n"
    "    frag_pos = position;\n"
    "}\n";

const char *fragment_shader_source =
    "#version 410 core\n"
    "in vec3 fragment_color;\n"
    "out vec4 color;\n"
    "in vec2 frag_pos;\n"
    "uniform sampler2D sampler;\n"

    "void main() {\n"
    "    color = vec4(fragment_color, 1.0) * texture(sampler, frag_pos + 0.5);\n"
    "}\n";

GLuint load_texture(const char *path) {
  stbi_set_flip_vertically_on_load(1);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Load image
  int width, height, nr_channels;
  unsigned char *data = stbi_load(path, &width, &height, &nr_channels, 0);
  if (data) {
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data
    );
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("Failed to load texture\n");
  }
  stbi_image_free(data);

  return texture;
}

int main(int argc, const char *argv[]) {
  SDL_Init(SDL_INIT_EVERYTHING);

  // Use the latest (for macOS) version of OpenGL
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Window *window = SDL_CreateWindow(
      "Post Processing",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      640,
      480,
      SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
  );
  SDL_GLContext context = SDL_GL_CreateContext(window);

  if (gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress) == 0) {
    perror("Cannot initialize GLAD\n");
    return 0;
  }

  // GL stuff
  glClearColor(
      0x49 / 255.0,
      0x36 / 255.0,
      0x57 / 255.0,
      1.0f
  ); // Set the clear color to #493657

  // data is stored as position & color
  float triangle_data[] = {
      0.5f,
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

      0.5f,
      0.5f,

      // #FFD1BA
      1.0f,
      0.819f,
      0.729f,

      -0.5f,
      0.5f,

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
  program = create_shader(vertex_shader_source, fragment_shader_source);
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

  GLuint texture = load_texture("picture.png");
  glUniform1i(glGetUniformLocation(program, "sampler"), 0);

  init_post_processing();

  // The funnies
  const uint8_t *keyboard = SDL_GetKeyboardState(NULL);
  float pos_x = 0.0f, pos_y = 0.0f;
  uint64_t prev = SDL_GetTicks64();

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

    uint64_t curr = SDL_GetTicks64();
    float delta = (curr - prev) / 1000.0f;
    prev = curr;

    if (keyboard[SDL_SCANCODE_W]) {
      pos_y += delta;
    } else if (keyboard[SDL_SCANCODE_S]) {
      pos_y -= delta;
    }

    if (keyboard[SDL_SCANCODE_D]) {
      pos_x += delta;
    } else if (keyboard[SDL_SCANCODE_A]) {
      pos_x -= delta;
    }

    post_processing_begin();
    glClear(GL_COLOR_BUFFER_BIT); // Clear the background with color

    glUniform1f(glGetUniformLocation(program, "pos_x"), pos_x);
    glUniform1f(glGetUniformLocation(program, "pos_y"), pos_y);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Rendering
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    post_processing_end();

    SDL_GL_SwapWindow(window); // Swap window buffers
    // Delay so that there's at least some time between frames
    SDL_Delay(1);

    GLuint err = glGetError();
    if (err != 0) {
      printf("%u\n", glGetError());
      running = SDL_FALSE;
    }
  }

  printf("post_processing_shader: %u\n", post_processing_shader);
  printf("other program: %u\n", program);

  post_processing_cleanup();

  // Quit from OpenGL
  glDeleteTextures(1, &texture);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(program);

  // Quit from SDL
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
