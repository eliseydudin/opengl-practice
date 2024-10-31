project('opengl-practice', 'c')

dependencies = [
    dependency('sdl2'),
]

if host_machine.system() == 'linux'
    add_global_link_arguments('-lm', language: 'c')
endif

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
