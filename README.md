# Product Name
> Popup command prompt just like i3-input but with more customizability and Xresources support

## Installation

```sh
make && make install
```

## Usage example

### From command line

```bash
xinp -F "Input goes here -> %s"
```
and after typing `here` and confirming the input you get
```bash
xinp -F "Input goes -> %s" # Input goes -> here
```
#### Changing font and prompt
```bash
xinp -f Inconsolata -p "cmd:"
```
#### Changing colors
```bash
xinp --bg 000000 --fg ffffff
```
### With i3

As an arbitrary workspace switcher
```
bindsym $mod+s       exec --no-startup-id xinp --i3 -f Inconsolata -l 1 -p 'workspace: ' -F 'workspace %s'
bindsym $mod+Ctrl+s  exec --no-startup-id xinp --i3 -f Inconsolata -l 1 -p 'workspace: ' -F 'move container to workspace %s'
bindsym $mod+Shift+s exec --no-startup-id xinp --i3 -f Inconsolata -l 1 -p 'workspace: ' -F 'move container to workspace %s, workspace %s'
```

## Development setup

Requires:

* gcc
* cmake
* X11
* Xft

```sh
make
./cmake-build-debug/xinp
```

## Author

Nikola Tasić –  nik@7aske.com

[7aske.com](https://7aske.com)

[https://github.com/7aske](https://github.com/7aske)

## Contributing

1. Fork it (<https://github.com/yourname/yourproject/fork>)
2. Create your feature branch (`git checkout -b feature/fooBar`)
3. Commit your changes (`git commit -am 'Add some fooBar'`)
4. Push to the branch (`git push origin feature/fooBar`)
5. Create a new Pull Request

