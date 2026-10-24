# COMP3271 Computer Graphics

Welcome to the COMP3271 in year 2026-2027!

Here is the code repository of HKU course COMP3271. Any commit and issue will be welcome.

Instructor: [Prof. Taku Komura](https://i.cs.hku.hk/~taku/)

TA: @[Litian Zhang](https://litzzzhang.github.io), Email: litzzzhang@connect.hku.hk

## Instruction

### Environment Setup

Please follow the instructions in [Environment_Setup.md](Environment_Setup.md) to install Git, prepare a compiler, install xmake, and set up your IDE. After the setup, you should be able to run `xmake run test` and see a colorful triangle.

If you encounter any problems during the setup, please read the [xmake FAQ](Environment_Setup.md#xmake-faq) first, or contact the TA (litzzzhang@connect.hku.hk) for help.

### Assignments

All assignments share the same codebase and build system:

- Assignment 1: [Interactive Triangle Drawing and 3D Scene Editor](Assignment1/README.md)
- Assignment 2: [Shader Lab](Assignment2/README.md)
- Assignment 3: TBA

Each assignment has its own folder containing a `README.md` with the handout and
an `xmake.lua` with its build target. To compile and run an assignment, enter
the `code/` directory and run:

```shell
xmake run <target_name>
```

Replace `<target_name>` with the target of the assignment (for example,
`assignment1_part_a` or `assignment2`), and follow its `README.md`.

Any commit and issue will be welcome. Don't hesitate to seek help via email.
