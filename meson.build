project('opengl-practice', 'c')

dependencies = [
    dependency('sdl2'),
    dependency('assimp'),
    dependency('GL', method: 'auto')
]

subdir('glad')

if get_option('sandwich')
    subdir('sandwich')
endif

if get_option('minimal')
    subdir('minimal')
endif

if get_option('triangle')
    subdir('triangle')
endif

if get_option('picture')
    subdir('picture')
endif

if get_option('post_processing')
    subdir('post_processing')
endif
