# Audiyour App

## Getting started

For this project you first need to install the Flutter which is a open source framework
by Google for building cross platform applications.

## Prerequisites

You need to have Flutter installed
    - https://docs.flutter.dev/get-started/install

You need Android Studio installed
    - https://developer.android.com/studio
    
Once you have Android Studio installed you need to download the Flutter Plugin
    - https://docs.flutter.dev/get-started/editor?tab=androidstudio

Set Flutter path on configurations within Android Studio
    - File -> Languages & Frameworks -> Flutter
    - Then set the Flutter SDK Path

Once you are ready to run you need to install a dependency from the myapp directory as admin
    - "flutter pub add flutter_blue"
    - "flutter pub get"

## Running the app

Once everything is installed you have a choice of emulating the app on a virtual android
device or using a physical Android device.

If a virtual one is to be used, first you need to create a virtual android device in the 
AVD Manager. Any device in the list can be chosen.

Once the installation is complete you launch the Android Virtual Device on the emulator
and run the app.

If a physical device is to be chosen, you connect it to Android Studio and turn on the
developer options on the phone and turn on USB Debugging.
    - https://www.samsung.com/uk/support/mobile-devices/how-do-i-turn-on-the-developer-options-menu-on-my-samsung-galaxy-device/#:~:text=1%20Go%20to%20%22Settings%22%2C,enable%20the%20Developer%20options%20menu
    
A common error when compiling is when you need to change Sdk version in app/build.gradle to 31, change version in settings.gradle 
to 1.16.10 (for moto g stylus 2021, the requirements per phone may be different)
