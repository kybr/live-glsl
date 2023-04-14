# live-glsl

#### how to build this project

```
brew install liblo
brew install glfw
cd this/folder
make
./main.bin
```


#### live-code w/ neovim

```
nvim foo.glsl
:source config.vim
```

This loads a vim configuration that sends the buffer after each keystroke using a Lua-based OSC messaging system. But, any editor that sends code as a blob along with a integer version number will do. The OSC message must look like this:

    /s ib <version-as-int> <code-as-blob>


