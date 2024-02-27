# QMK on Zephyr

This project makes [QMK](https://qmk.fm/) as a library, and build on top of [Zephyr RTOS](https://www.zephyrproject.org/).

## Build

First, follow the getting start guide from the official [Zephyr guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html). This project uses zephyr 3.2.0.

After that, build this project with

```
$ west -p auto -b <your board>
```
