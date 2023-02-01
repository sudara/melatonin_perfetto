#!/usr/bin/env python3
"""
This script runs CMake configure, build, and test twice in sequence -- once with no options specified, and once with PERFETTO=ON.
This is to ensure that the default has Perfetto off, and that toggling it on actually has an effect.
An environment variable is used to communicate what the correct value of PERFETTO should be to the child CMake process.
"""

import os.path as path
from os import chdir, environ
from shutil import rmtree
import subprocess

#

def run_command(command, workingDir):

    print (f"Running command {command}")

    try:
        cp = subprocess.run (command, 
                             cwd=workingDir, 
                             stderr=subprocess.PIPE, 
                             stdout=subprocess.PIPE, 
                             text=True, 
                             shell=True,
                             check=True)

        print(cp.stdout)
        print(cp.stderr)

    except subprocess.CalledProcessError as error:
        print (error.output)
        print (f"Command {error.cmd} failed with exit code {error.returncode}")
        exit (1)

#

REPO_ROOT = path.dirname(path.dirname(path.realpath(__file__)))

BUILD_DIR = path.join(REPO_ROOT, "Builds")

if path.isdir(BUILD_DIR):
    rmtree (BUILD_DIR)

environ["MP_PERFETTO_SHOULD_BE_ON"] = "FALSE"

run_command (command=f"cmake -B {BUILD_DIR}",
             workingDir=REPO_ROOT)

run_command (command=f"cmake --build {BUILD_DIR}",
             workingDir=REPO_ROOT)

run_command (command="ctest -C Debug",
             workingDir=BUILD_DIR)

environ["MP_PERFETTO_SHOULD_BE_ON"] = "TRUE"

run_command (command=f"cmake -B {BUILD_DIR} -D PERFETTO=ON",
             workingDir=REPO_ROOT)

run_command (command=f"cmake --build {BUILD_DIR}",
             workingDir=REPO_ROOT)

run_command (command="ctest -C Debug",
             workingDir=BUILD_DIR)
