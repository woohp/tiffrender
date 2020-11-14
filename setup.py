#!/usr/bin/env python
from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import sys


__version__ = '0.3.2'


libraries = ['tiff', 'tiffxx']


ext_modules = [
    Pybind11Extension(
        'tiffrender',
        [
            'src/tiffrender.cpp',
        ],
        libraries=libraries,
        library_dirs=['/usr/local/lib'],
        language='c++',
        define_macros=[('VERSION_INFO', __version__)],
    ),
]


setup(
    name='tiffrender',
    description='Tiff Renderer',
    version='0.3.2',
    setup_requires=['Pillow>=8.0.1'],
    install_requires=['Pillow>=8.0.1'],
    ext_modules=ext_modules,
    cmdclass={'build_ext': build_ext},
    test_suite='tests',
    zip_safe=False,
)
