from setuptools import setup, find_namespace_packages
from pathlib import Path

setup(
    name="mecatro_telemetry",
    version="1.0.0",
    description="Telemetry client for Mecatro projects: converts Arduino message to csv file",
    author="Matthieu Vigne",
    license="MIT",
    packages=find_namespace_packages("src"),
    package_dir={"": "src"},
    install_requires=[
        "pyserial",
        "tqdm"
    ],
    entry_points={"console_scripts": ["mecatro_telemetry = mecatro_telemetry.main:main"]},
    include_package_data=True,
    zip_safe=False,
)
