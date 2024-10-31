#!/usr/bin/env python3

import platform
import os
import subprocess


def main():
    PLATFORM = platform.platform()

    match PLATFORM:
        case "Darwin":
            if os.system("brew --version") != 0:
                print("\e[0;31merror:\e[0m homebrew is not installed")
                print("you can get it at brew.sh")
                exit(1)

            for package in ["assimp", "sdl2", "meson"]:
                print(f"\e[0;35minfo:\e[0m installing {package}")
                if subprocess.run(["brew", "install", package]).returncode != 0:
                    print(f"\e[0;31merror:\e[0m cannot install {package}")
                    exit(1)

            print("\e[0;35minfo:\e[0m all packages installed")

        case "Linux":
            ...
        case _:
            print("\e[0;31merror:\e[0m unsupported platform")
            exit(1)


if __name__ == "__main__":
    main()
