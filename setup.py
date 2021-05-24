#!/usr/bin/python

import setuptools
from distutils.core import setup, Extension

__version__ = "1.0.0"


with open("README.md", "r") as fh:
        long_description = fh.read()

macros = [('MODULE_VERSION', '"%s"' % __version__)]
setup(name         = "escode",
      version      = __version__,
      author       = "Akhil Wable",
      author_email = "awable@gmail.com",
      description  = "ESCODE binary serialization",
      long_description=long_description,
      long_description_content_type="text/markdown",
      url="https://github.com/awable/escode",
      project_urls={"Bug Tracker": "https://github.com/awable/escode/issues"},
      ext_modules  = [
        Extension(
            name='escode',
            sources=['escode.c'],
            define_macros=macros,
            extra_compile_args=['-std=c99'])
      ]
)
