import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

from conan import ConanFile

# example: https://github.com/conan-io/examples2/blob/main/tutorial/consuming_packages/conanfile_py/conanfile.py
class NetRecipe(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {
    "asio": [True, False]
  }
  default_options = {
    "asio": False
  }
  generators = "CMakeToolchain", "CMakeDeps"

  #def configure(self):
  #  if not hasattr(self.options, 'asio'):
  #    self.options.asio = False

  def requirements(self):
    #self.requires("zlib/1.2.11")
    if hasattr(self.options, 'asio') and self.options.asio == True:
        self.requires("asio/1.30.2")

#  def layout(self):
#    cmake_layout(self)

