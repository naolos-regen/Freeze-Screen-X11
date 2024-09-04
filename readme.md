# Freeze-Screen-X11

It's my first serious C program.

Problem why I designed it: 

- I don't have a Logout function and I needed something to work on.

How to use it?

```bash
$ gcc -o [executable_file] freeze_x11.c -lX11
$ ./executable_file
```

- If you try to exit just press esc, if not switch /dev/tty to kill the process.
- Type your username & your password ($USER !root).

- License?
