#include <SDL2/SDL.h>
#if defined(__APPLE__)
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl3.h>
#elif defined(linux)
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
  #include <GL/glext.h>
#else
  #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../stbi.h" // Include stb_image.h for texture loading

// Vertex Shader Source Code
const GLchar *vertex_shader_source =
    "#version 410 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec3 color;\n"
    "layout (location = 2) in vec2 tex_coord;\n"
    "out vec3 our_color;\n"
    "out vec2 our_tex_coord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform int width;\n"
    "uniform int height;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * view * model * vec4(position, 1.0);\n"
    "    our_color = color;\n"
    "    our_tex_coord = tex_coord;\n"
    "}\n";

// Fragment Shader Source Code
const GLchar *fragment_shader_source =
    "#version 410 core\n"
    "in vec3 our_color;\n"
    "in vec2 our_tex_coord;\n"
    "out vec4 color;\n"
    "uniform sampler2D texture1;\n"
    "uniform int width;\n"
    "uniform int height;\n"
    "void main()\n"
    "{\n"
    "    vec2 resolution = vec2(width, height);\n"
    "    vec2 block_size = resolution / 3.4;\n"
    "    vec2 uv = floor((our_tex_coord + 0.5) * block_size) / block_size - 0.5;\n"
    "    color = texture(texture1, uv) * vec4(our_color, 1.0);\n"
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
    if (event->type == SDL_QUIT) {
      *running = 0;
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

GLuint load_texture(const char *path) {
  //stbi_set_flip_vertically_on_load(1);

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
        GL_RGBA,
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

const struct aiScene *scene;
struct aiMesh *mesh;

float *load_vertices(int *size) {
  scene = aiImportFile("model.obj", aiProcess_Triangulate | aiProcess_FlipUVs);
  mesh = scene->mMeshes[0];

  *size = mesh->mNumVertices;

  float *vertices = malloc(sizeof(float) * mesh->mNumVertices * 8);
  for (int i = 0; i < mesh->mNumVertices; i++) {
    struct aiVector3D vert = mesh->mVertices[i],
                      tex = mesh->mTextureCoords[0][i];

    vertices[i * 8 + 0] = vert.x;
    vertices[i * 8 + 1] = vert.y;
    vertices[i * 8 + 2] = vert.z;

    vertices[i * 8 + 3] = vertices[i * 8 + 4] = vertices[i * 8 + 5] = 1.0;

    vertices[i * 8 + 6] = tex.x;
    vertices[i * 8 + 7] = tex.y;
  }

  return vertices;
}

uint32_t *load_indices(int *size) {
  *size = mesh->mNumFaces * 3;
  uint32_t *indices = malloc(sizeof(int) * (*size));

  for (int i = 0; i < *size; i += 3) {
    indices[i] = mesh->mFaces[i / 3].mIndices[0];
    indices[i + 1] = mesh->mFaces[i / 3].mIndices[1];
    indices[i + 2] = mesh->mFaces[i / 3].mIndices[2];
  }

  aiReleaseImport(scene);

  return indices;
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    sdl_die("Couldn't initialize SDL");
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Window *window = SDL_CreateWindow(
      "Sandwich example",
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

  // Set up vertex data (Cube vertices with color and texture coordinates)
  int size;
  GLfloat *vertices = load_vertices(&size);
  printf("%d\n", size);

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(float) * size * 8,
      vertices,
      GL_STATIC_DRAW
  );

  GLuint *indices = load_indices(&size);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      size * sizeof(uint32_t),
      indices,
      GL_STATIC_DRAW
  );

  // Position attribute
  glVertexAttribPointer(
      0,
      3,
      GL_FLOAT,
      GL_FALSE,
      8 * sizeof(GLfloat),
      (GLvoid *) 0
  );
  glEnableVertexAttribArray(0);

  // Color attribute
  glVertexAttribPointer(
      1,
      3,
      GL_FLOAT,
      GL_FALSE,
      8 * sizeof(GLfloat),
      (GLvoid *) (3 * sizeof(GLfloat))
  );
  glEnableVertexAttribArray(1);

  // Texture Coord attribute
  glVertexAttribPointer(
      2,
      2,
      GL_FLOAT,
      GL_FALSE,
      8 * sizeof(GLfloat),
      (GLvoid *) (6 * sizeof(GLfloat))
  );
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  free(indices);
  free(vertices);

  // Load and create a texture
  GLuint texture = load_texture("texture.png");
  glUseProgram(shader_program);
  glUniform1i(
      glGetUniformLocation(shader_program, "texture1"),
      0
  ); // set the texture as sampler2D 0

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

    int x, y;
    SDL_GetWindowSize(window, &x, &y);

    setup_int(shader_program, "width", x);
    setup_int(shader_program, "height", y);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Draw the object
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(window);

    SDL_Delay(1);
  }

  // Cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shader_program);
  glDeleteTextures(1, &texture);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
