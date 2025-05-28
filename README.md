# Shijima-NX

Shimeji desktop pet runner for Nintendo Switch, built with [libtesla](https://github.com/pixelomer/libtesla) and [libshijima](https://github.com/pixelomer/libshijima). Intended to be used with [Tesla-Menu](https://github.com/WerWolv/Tesla-Menu).

## Usage

Extract the contents of the release zip to the root of your SD card. Then, from the Tesla menu, launch Shijima-NX. You can interact with the shimeji using the touch screen. To close Shijima-NX, press L-DPadDown-RStick.

Interaction with other games/applets will be lost when you first launch Shijima-NX. To resolve this, hold the home button to bring up the side menu, then press it again to close it. Games will start receiving input again.

The release zip contains the default shimeji shipped with Shimeji-ee. If you want to use your own mascot, you need to generate Shijima-NX files for it using `make-config.sh`.
