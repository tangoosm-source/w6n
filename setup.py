from setuptools import setup, Distribution
from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
import os, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC  = os.path.join(HERE, "w6n", "src")
BUILD = os.path.join(SRC, "build")
BIN_OUT = os.path.join(HERE, "w6n", "bin", "asmanalyzer")

def detect_cxx():
    for c in ("g++", "clang++"):
        if shutil.which(c):
            return c
    sys.exit("HATA: g++ veya clang++ bulunamadi")

class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True

class CMakeWheel(_bdist_wheel):
    def run(self):
        # 1) cmake + make
        os.makedirs(BUILD, exist_ok=True)
        cxx = detect_cxx()
        print(f"[w6n] cmake derleniyor: {cxx}", flush=True)
        subprocess.check_call(
            ["cmake", f"-DCMAKE_CXX_COMPILER={cxx}", ".."], cwd=BUILD)
        subprocess.check_call(["make", "-j4"], cwd=BUILD)
        # 2) binary'yi w6n/bin/ altına kopyala (package-data)
        os.makedirs(os.path.dirname(BIN_OUT), exist_ok=True)
        shutil.copy(os.path.join(BUILD, "asmanalyzer"), BIN_OUT)
        os.chmod(BIN_OUT, 0o755)
        print(f"[w6n] binary: {BIN_OUT}", flush=True)
        # 3) standart wheel build
        super().run()

setup(
    cmdclass={"bdist_wheel": CMakeWheel},
    distclass=BinaryDistribution,
)
