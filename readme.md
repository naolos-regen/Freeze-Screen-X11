# Freeze-Screen-X11

### It's my first serious C program.

##### Problem why I designed it: 

- I don't have a Logout function and I needed something to work on.
- [ ] TODO: use PAM API instead of pamtester. .. .... buffer overflow will happen a lot 


#### What dependencies

```bash
> gcc :-)
> vlc
> x11 # utils, dev, whatever tf u need for it to work*
> emacs # for editing
> pamtester # gave up with auth_script.sh
```

#### How to use it?

```bash
$ gcc -o [executable_file] freeze_x11.c -lX11
$ ./executable_file
# window opens
```

- If you try to exit just press esc, if not switch /dev/tty to kill the process.
- Type your username & your password ($USER !root).
- The current $USER where X11 is running on

- License?

#### how to install x11?

```bash
# Ensure that you have build-essential

# Fedora
sudo dnf install xorg-x11-server-devel libX11-devel libXext-devel libXrandr-devel libXtst-devel
# Debian
sudo apt-get install libx11-dev libxext-dev libxrandr-dev libxtst-dev
# Arch-Linux
sudo pacman -S libx11 libxext libxrandr libxtst
# Slackware
sudo slackpkg install x11-devel
# Void-Linux
sudo xbps-install -S libX11-devel libXext-devel libXrandr-devel libXtst-devel

```

#### Wayland?

no.
