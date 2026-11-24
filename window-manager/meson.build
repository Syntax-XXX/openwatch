# Der Fenstermanager ist unabhängig baubar: nur Xlib + XShape, Stapellogik als eigene Bibliothek (testbar).
owm_stack = static_library('owm_stack', 'owm_stack.c')
owm = executable('owm', 'owm.c', link_with: owm_stack, dependencies: [x11, xext], install: true)
owm_stack_dep = declare_dependency(link_with: owm_stack, include_directories: include_directories('.'))
