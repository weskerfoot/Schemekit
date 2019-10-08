## What is this?

A web browser you can control from your Scheme repl!

## Dependencies
* gtk3
* webkit2gtk-4.0
* guile-2.2

## How to build
* Compile with `./build.sh`

## How to use
* Run `./a.out` after compiling
* Run `(load "test.scm")` to start up the browser in another thread
* Run `(open-page "https://example.org")` to see it in action.
