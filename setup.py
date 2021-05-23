#!/usr/bin/python

from distutils.core import setup, Extension

__version__ = "1.0.0"

macros = [('MODULE_VERSION', '"%s"' % __version__)]

setup(name         = "python-escode",
      version      = __version__,
      author       = "Akhil Wable",
      author_email = "awable@gmail.com",
      description  = "ESCODE binary encoding encoder/decoder module",
      ext_modules  = [
        Extension(
            name='escode',
            sources=['escode.c'],
            define_macros=macros,
            extra_compile_args=['-std=c99'])
      ]
)
