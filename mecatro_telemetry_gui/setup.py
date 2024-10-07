from setuptools import setup, find_namespace_packages
from pathlib import Path

setup(
    name="mecatro_telemetry_gui",
    version="1.0.0",
    description="GUI Telemetry client for Mecatro projects: converts Arduino message to csv file",
    author="Matthieu Vigne",
    license="MIT",
    packages=find_namespace_packages("src"),
    package_dir={"": "src"},
    install_requires=[
        "matplotlib",
        "pyserial"
    ],
    entry_points={"console_scripts": ["mecatro_telemetry_gui = mecatro_telemetry.gui:main"]},
    include_package_data=True,
    zip_safe=False,
)
