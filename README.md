# QMK on Zephyr

QMK on Zephyr is a open-source keyboard firmware. It is a [Zephyr RTOS](https://www.zephyrproject.org/) application that integrates another famous open-source keyboard [QMK](https://qmk.fm/), and adopt various base functionality provided by Zephyr RTOS (i.e., BLE stack) to build a more powerful keyboard firmware.

## Contribute

We welcome contributions to QMK on Zephyr in any area. Look for or file a issue for an interesting feature, bug, or documentation improvements to start the discussion.

### Coding Style

For C files, please follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

### Commit Message Style

For general documentation for how to write git commit messages, check out [How to Write a Git Commit Message](https://cbea.ms/git-commit/).

## Development

Currently, this project is based on Zephyr 3.2.0. To set up the development

- First, follow the official [Zephyr guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).
- Clone this project including submodule.
- Run build via

```
$ west build -p auto -b <your board>

```

This project is now targeting on Phoenix Pro as the development target, and we will gradually add support for different keyboards.
At this point, we aim to make generic changes (non Phoenix Pro specific changes) be landed on the main branch, and the branch `phoenix_pro` are then rebased onto the main branch accordingly.
After we configure the project to be able to build for different keyboard, we plan to remove `phoenix_pro` branch.
