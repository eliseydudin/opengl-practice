project('opengl-practice', 'c')

dependencies = [
    dependency('sdl2'),
    dependency('assimp'),
    dependency('appleframeworks', modules: ['OpenGL'])
]

if get_option('sandwich')
    subdir('sandwich')
endif
