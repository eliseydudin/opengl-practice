sources = ['glad.c']

glad = shared_library('glad', sources: sources)
glad_dep = declare_dependency(
    link_with: glad,
    include_directories: include_directories('.'),
)

dependencies += glad_dep
