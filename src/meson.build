surlibsrc = ['colours.c', 'canvmerge.c', 'textrender.c', 'umman.c', '2dscene.c', 'canvas.c', 'srtexldr.c']
surlib = library('surrender', surlibsrc, version : '0.0.0', soversion : '0', dependencies : [m_dep, openmp_dep], c_args : '-DSIMDE_ENABLE_OPENMP', install : true)
surlibhdrs = ['umman.h', 'errors.h', 'remote_holy.h', 'colours.h', 'canvmerge.h', 'textrender.h', '2dscene.h', 'srtexldr.h', 'canvas.h', 'surrender.h', 'srtex.h', 'env.h', 'glbl.h']
install_headers(surlibhdrs, subdir : 'surrender')