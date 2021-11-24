# Graphic Equalizer GUI

## Note: This was developed in Linux
## It should work in Windows, but this was not tested

## Prerequisites
- You must have Q5 Framework (GUI Toolkit) installed.
    - Ubuntu or Debian
        - sudo apt install qt5-default
    - Arch or Manjaro
        - sudo pacman -S qt5-base
    - Windows
        - https://www.qt.io/download-thank-you?os=windows


## Install through PyPi
- Optionally, create a fresh venv for installing dependencies into
    - Linux
        - `python3 -m venv venv`
        - `source venv_eq/bin/activate`
    - Windows
        - `python3 -m venv venv`
        - `venv/Scripts/Activate.ps1`
- `pip install -r requirements.txt`
- Once installed, run `python3 main.py` to launch the application