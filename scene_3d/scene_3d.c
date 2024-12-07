#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <math.h>
#include <stdio.h>

#include "SDL_events.h"

static int is_wireframe = 0;

const GLfloat cube_vertices[] = { // Positions
    -0.5f, -0.5f, -0.5f,

    0.5f,  -0.5f, -0.5f,

    0.5f,  0.5f,  -0.5f,

    0.5f,  0.5f,  -0.5f,

    -0.5f, 0.5f,  -0.5f,

    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,

    0.5f,  -0.5f, 0.5f,

    0.5f,  0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,

    -0.5f, 0.5f,  0.5f,

    -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f,  0.5f,

    -0.5f, 0.5f,  -0.5f,

    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,

    0.5f,  0.5f,  -0.5f,

    0.5f,  -0.5f, -0.5f,

    0.5f,  -0.5f, -0.5f,

    0.5f,  -0.5f, 0.5f,

    0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,

    0.5f,  -0.5f, -0.5f,

    0.5f,  -0.5f, 0.5f,

    0.5f,  -0.5f, 0.5f,

    -0.5f, -0.5f, 0.5f,

    -0.5f, -0.5f, -0.5f,

    -0.5f, 0.5f,  -0.5f,

    0.5f,  0.5f,  -0.5f,

    0.5f,  0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,

    -0.5f, 0.5f,  0.5f,

    -0.5f, 0.5f,  -0.5f
};

const GLfloat colors[] = {
    0.583f, 0.771f, 0.014f, 0.609f, 0.115f, 0.436f, 0.327f, 0.483f, 0.844f,
    0.822f, 0.569f, 0.201f, 0.435f, 0.602f, 0.223f, 0.310f, 0.747f, 0.185f,
    0.597f, 0.770f, 0.761f, 0.559f, 0.436f, 0.730f, 0.359f, 0.583f, 0.152f,
    0.483f, 0.596f, 0.789f, 0.559f, 0.861f, 0.639f, 0.195f, 0.548f, 0.859f,
    0.014f, 0.184f, 0.576f, 0.771f, 0.328f, 0.970f, 0.406f, 0.615f, 0.116f,
    0.676f, 0.977f, 0.133f, 0.971f, 0.572f, 0.833f, 0.140f, 0.616f, 0.489f,
    0.997f, 0.513f, 0.064f, 0.945f, 0.719f, 0.592f, 0.543f, 0.021f, 0.978f,
    0.279f, 0.317f, 0.505f, 0.167f, 0.620f, 0.077f, 0.347f, 0.857f, 0.137f,
    0.055f, 0.953f, 0.042f, 0.714f, 0.505f, 0.345f, 0.783f, 0.290f, 0.734f,
    0.722f, 0.645f, 0.174f, 0.302f, 0.455f, 0.848f, 0.225f, 0.587f, 0.040f,
    0.517f, 0.713f, 0.338f, 0.053f, 0.959f, 0.120f, 0.393f, 0.621f, 0.362f,
    0.673f, 0.211f, 0.457f, 0.820f, 0.883f, 0.371f, 0.982f, 0.099f, 0.879f
};

// Vertex Shader Source Code
const GLchar *vertex_shader_source =
    "#version 410 core\n"

    "layout(location = 0) in vec3 position;\n"
    "layout(location = 1) in vec3 color;\n"
    "out vec4 our_color;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "void main()\n"
    "{\n"
    "    gl_Position = projection * view * model * vec4(position, 1.0);\n"
    "    our_color = vec4(color, 1.0);\n"
    "}\n";

// Fragment Shader Source Code
const GLchar *fragment_shader_source =
    "#version 410 core\n"
    "in vec4 our_color;\n"
    "out vec4 color;\n"

    "void main()\n"
    "{\n"
    "    color = our_color;\n"
    "}\n";

void sdl_die(const char *message) {
  printf("%s: %s\n", message, SDL_GetError());
  SDL_Quit();
  exit(1);
}

GLuint compile_shader(GLenum shader_type, const GLchar *source) {
  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[512];
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", info_log);
  }
  return shader;
}

void process_input(SDL_Event *event, int *running) {
  while (SDL_PollEvent(event)) {
    switch (event->type) {
      case SDL_QUIT: {
        *running = 0;
      } break;

      case SDL_KEYDOWN: {
        if (event->key.keysym.sym == SDLK_w) {
          is_wireframe = !is_wireframe;

          if (is_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            // this enables the wireframe rendering thingy
          } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          }
        }
      } break;
    }
  }
}

void setup_matrix(GLuint shader_program, const char *name, float *matrix) {
  GLuint location = glGetUniformLocation(shader_program, name);
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
}

void setup_int(GLuint shader, const char *name, int num) {
  GLuint location = glGetUniformLocation(shader, name);
  glUniform1i(location, num);
}

void multiply_matrices(float *result, const float *mat1, const float *mat2) {
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      result[row * 4 + col] = 0;
      for (int k = 0; k < 4; ++k) {
        result[row * 4 + col] += mat1[row * 4 + k] * mat2[k * 4 + col];
      }
    }
  }
}

void rotate_matrix(float *matrix, float angle, float x, float y, float z) {
  float c = cos(angle);
  float s = sin(angle);
  float axis_length = sqrt(x * x + y * y + z * z);
  x /= axis_length;
  y /= axis_length;
  z /= axis_length;

  float rotation[16] = {
      c + (1 - c) * x * x,
      (1 - c) * x * y - s * z,
      (1 - c) * x * z + s * y,
      0.0f,
      (1 - c) * y * x + s * z,
      c + (1 - c) * y * y,
      (1 - c) * y * z - s * x,
      0.0f,
      (1 - c) * z * x - s * y,
      (1 - c) * z * y + s * x,
      c + (1 - c) * z * z,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f
  };

  float result[16];
  multiply_matrices(result, matrix, rotation);
  for (int i = 0; i < 16; i++)
    matrix[i] = result[i];
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    sdl_die("Couldn't initialize SDL");
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Window *window = SDL_CreateWindow(
      "3D Scene",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      800,
      600,
      SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
  );

  if (!window) {
    sdl_die("Couldn't create SDL window");
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) {
    sdl_die("Couldn't create OpenGL context");
  }

  if (gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress) == 0) {
    perror("Cannot initialize GLAD\n");
    return 0;
  }

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Compile shaders
  GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
  GLuint fragment_shader =
      compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

  // Link shaders into a program
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  GLint success;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar info_log[512];
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    printf("Shader linking failed\n%s\n", info_log);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(cube_vertices),
      cube_vertices,
      GL_STATIC_DRAW
  );

  GLuint colorbuffer;
  glGenBuffers(1, &colorbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

  // Position attribute
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);
  glEnableVertexAttribArray(0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

  glBindVertexArray(0);

  // Load and create a texture
  glUseProgram(shader_program);

  // Transformation matrices
  float model[16] = {
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f,
  };

  float view[16] = {
      0.25f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.25f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.25f,
      0.0f,
      0.0f,
      0.0f,
      -1.0f,
      0.25f
  };

  float projection[16] = {
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.3333f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      -1.02f,
      -1.0f,
      0.0f,
      0.0f,
      -2.02f,
      0.0f
  };

  /**/

  // Main loop
  SDL_Event event;
  int running = 1;
  float angle = 0.0f;

  uint32_t start = SDL_GetTicks();
  glClearColor(0.2f, 0.5f, 0.7f, 1.0f);

  while (running) {
    process_input(&event, &running);

    // Update model matrix to rotate
    uint32_t curr = SDL_GetTicks();
    uint32_t diff = curr - start;
    float delta = (float) (diff) / 1000.0;
    start = curr;

    angle += 1.0f * delta;
    if (angle > 360.0f)
      angle -= 360.0f;

    // Identity matrix
    model[0] = 0.1f;
    model[4] = 0.0f;
    model[8] = 0.0f;
    model[12] = 0.0f;
    model[1] = 0.0f;
    model[5] = 0.1f;
    model[9] = 0.0f;
    model[13] = 0.0f;
    model[2] = 0.0f;
    model[6] = 0.0f;
    model[10] = 0.1f;
    model[14] = 0.0f;
    model[3] = 0.0f;
    model[7] = 0.0f;
    model[11] = 0.0f;
    model[15] = 0.1f;

    // Apply rotation around Y axis
    rotate_matrix(model, angle, 0.0f, 1.0f, 0.0f);

    // Render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    // Set transformation matrices
    setup_matrix(shader_program, "model", model);
    setup_matrix(shader_program, "view", view);
    setup_matrix(shader_program, "projection", projection);

    // Draw the object
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(window);

    SDL_Delay(1);
  }

  // Cleanup
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(shader_program);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
