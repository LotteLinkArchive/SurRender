project('SurRender', 'c')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
thread_dep = dependency('threads')
sdl2_dep = dependency('sdl2')
openmp_dep = dependency('openmp')

subdir('src')
subdir('tools')
