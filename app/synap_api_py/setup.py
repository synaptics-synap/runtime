import os
from pathlib import Path
from setuptools import setup, find_packages, Extension

module_name = "synap"
shared_lib = next(Path(module_name).glob("*.so"), None)

if not shared_lib:
    raise FileNotFoundError(f"Missing SyNAP shared library")

setup(
    name=module_name,
    version="0.0.1",
    description="Python bindings for SyNAP framework",
    packages=[module_name],
    package_data={module_name: [shared_lib.name]}, 
    include_package_data=True,
    zip_safe=False,
)
