from distutils.core import setup, Extension

setup(name = "pyzwave",
      version = "1.0",
      ext_modules = [Extension("pyzwave", ["pyzwave.c", "pyzwave-testrtt.c"])])
