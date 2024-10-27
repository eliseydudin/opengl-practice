project('opengl-practice', 'c')

dependencies = [
    dependency('sdl2'),
    dependency('assimp'),
    dependency('appleframeworks', modules: ['OpenGL'])
]

if get_option('sandwich')
    subdir('sandwich')
endif
if get_option('minimal')
    subdir('minimal')
endif
if get_option('triangle')
    subdir('triangle')
endif
