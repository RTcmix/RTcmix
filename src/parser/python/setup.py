from setuptools import setup, Extension

include = ["../../../include", "../../rtcmix"]

rtcmix_ext = Extension("rtcmix", ["rtcmixmodule.cpp"], include_dirs = include)

setup(name = "rtcmix",
      version = "5.2",
      ext_modules = [rtcmix_ext]
)

