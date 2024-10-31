#include <glad/glad.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

GLuint create_shader(const char *vertex_source, const char *fragment_source) {
  GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
  GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

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

// You should probably have a struct to hold all of this information
// but im using global variables here cuz im lazy :)
const float screen_verts[] = {1.0f, -1.0f, 1.0f,  0.0f, -1.0f, -1.0f,
                              0.0f, 0.0f,  -1.0f, 1.0f, 0.0f,  1.0f,

                              1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  -1.0f,
                              1.0f, 0.0f,  -1.0f, 1.0f, 0.0f,  1.0f};
#ifdef __APPLE__
const int screen_width = 640 * 2, screen_height = 480 * 2;
#else
const int screen_width = 640, screen_height = 480;
#endif
GLuint screen_rect_vao, screen_rect_vbo;
GLuint post_processing_fbo;
GLuint post_processing_texture;
GLuint post_processing_shader, program;

GLuint uniform_screen_pos_location;

const char *post_processing_vertex =
    "#version 410 core\n"
    "in vec2 pos;\n"
    "in vec2 tex_coords;\n"
    "out vec2 out_tex_coords;\n"

    "void main() {\n"
    "    gl_Position = vec4(pos, 1.0, 1.0);\n"
    "    out_tex_coords = tex_coords;\n"
    "}\n";

const char *post_processing_fragment =
    "#version 410 core\n"
    "out vec4 color;\n"
    "in vec2 out_tex_coords;\n"

    "uniform sampler2D screen_texture;\n"
    "uniform vec2 screen_resolution;"

    "void main() {\n"
    "    vec2 block_size = screen_resolution / 5.0;\n"
    "    vec2 uv = floor((out_tex_coords + 0.5) * block_size) / block_size - 0.5;\n"
    "    float color_tmp = texture(screen_texture, uv).x;"
    "    color = vec4(color_tmp);\n"
    "}\n";
;

void init_post_processing() {
  unsigned int rect_vbo;
  glGenVertexArrays(1, &screen_rect_vao);
  glGenBuffers(1, &rect_vbo);
  glBindVertexArray(screen_rect_vao);
  glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(screen_verts),
      &screen_verts,
      GL_STATIC_DRAW
  );
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      0,
      2,
      GL_FLOAT,
      GL_FALSE,
      4 * sizeof(float),
      (void *) 0
  );
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1,
      2,
      GL_FLOAT,
      GL_FALSE,
      4 * sizeof(float),
      (void *) (2 * sizeof(float))
  );

  // Create Frame Buffer Object
  glGenFramebuffers(1, &post_processing_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, post_processing_fbo);

  // Create Framebuffer Texture
  glGenTextures(1, &post_processing_texture);
  glBindTexture(GL_TEXTURE_2D, post_processing_texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      screen_width,
      screen_height,
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      NULL
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_WRAP_S,
      GL_CLAMP_TO_EDGE
  ); // Prevents edge bleeding
  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_WRAP_T,
      GL_CLAMP_TO_EDGE
  ); // Prevents edge bleeding
  glFramebufferTexture2D(
      GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D,
      post_processing_texture,
      0
  );

  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(
      GL_RENDERBUFFER,
      GL_DEPTH24_STENCIL8,
      screen_width,
      screen_height
  );
  glFramebufferRenderbuffer(
      GL_FRAMEBUFFER,
      GL_DEPTH_STENCIL_ATTACHMENT,
      GL_RENDERBUFFER,
      rbo
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("uh oh cannot create the buffer :(\n");
    exit(1);
  }

  post_processing_shader =
      create_shader(post_processing_vertex, post_processing_fragment);

  uniform_screen_pos_location =
      glGetUniformLocation(post_processing_shader, "screen_resolution");
}

void post_processing_begin() {
  glBindFramebuffer(GL_FRAMEBUFFER, post_processing_fbo);
  //glEnable(GL_DEPTH_TEST);
  glUseProgram(program);
}

void post_processing_end() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(post_processing_shader);

  glUniform2f(uniform_screen_pos_location, screen_width, screen_height);

  glBindVertexArray(screen_rect_vao);
  //glDisable(GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D, post_processing_texture);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void post_processing_cleanup() {
  glDeleteBuffers(1, &screen_rect_vbo);
  glDeleteVertexArrays(1, &screen_rect_vao);
  glDeleteProgram(post_processing_shader);
  glDeleteTextures(1, &post_processing_texture);
}
