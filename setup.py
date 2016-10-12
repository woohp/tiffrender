from setuptools import setup

setup(
    name='tiffrender',
    description='Tiff Renderer',
    version='0.1.0',
    setup_requires=['cffi>=1.7.0'],
    cffi_modules=['tiffrender_build.py:ffi'],
    install_requires=['cffi>=1.7.0'],
    packages=['tiffrender'],
)
