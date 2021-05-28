# Starter SConstruct for enscons
# (filled by enscons.setup2toml)

import enscons, enscons.cpyext
import pytoml as toml
import sys

metadata = dict(toml.load(open("pyproject.toml")))["tool"]["enscons"]

# set to True if package is not pure Python
HAS_NATIVE_CODE = True

if HAS_NATIVE_CODE:
    full_tag = enscons.get_binary_tag()
else:
    # pure Python packages compatible with 2+3
    full_tag = enscons.get_universal_tag()

# From: https://github.com/dholth/cryptacular
MSVC_VERSION = None
SHLIBSUFFIX = None
TARGET_ARCH = None  # only set for win32
if sys.platform == "win32":
    import distutils.msvccompiler
    MSVC_VERSION = str(distutils.msvccompiler.get_build_version())  # it is a float
    SHLIBSUFFIX = ".pyd"
    TARGET_ARCH = "x86_64" if sys.maxsize.bit_length() == 63 else "x86"

env = Environment(
    tools=["default", "packaging", enscons.generate, enscons.cpyext.generate],
    PACKAGE_METADATA=metadata,
    WHEEL_TAG=full_tag,
    MSVC_VERSION=MSVC_VERSION,
    TARGET_ARCH=TARGET_ARCH,
)

# usually adds ".so"
ext_filename = enscons.cpyext.extension_filename("escode")

extension = env.SharedLibrary(
    target=ext_filename,
    source=["escode.c"],
    LIBPREFIX="",
    SHLIBSUFFIX=SHLIBSUFFIX,
    CPPPATH=env["CPPPATH"],
    CPPFLAGS=["-std=c99"],
)

# Only *.py is included automatically by setup2toml.
# Add extra 'purelib' files or package_data here.
py_source = []

lib = env.Whl("platlib" if HAS_NATIVE_CODE else "purelib", py_source+extension, root='')
whl = env.WhlFile(lib)

# Add automatic source files, plus any other needed files.
sdist_source = FindSourceFiles() + ["PKG-INFO", "setup.py", "pyproject.toml"]

sdist = env.SDist(source=sdist_source)

env.NoClean(sdist)
env.Alias("sdist", sdist)

develop = env.Command("#DEVELOP", enscons.egg_info_targets(env), enscons.develop)
env.Alias("develop", develop)

# needed for pep517 / enscons.api to work
env.Default(whl, sdist)
