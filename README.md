# **This repository has moved to [https://git.wesk.tech/wes/Schemekit](https://git.wesk.tech/wes/Schemekit)**


## What is this?

A web browser you can control from your Scheme repl!

## Dependencies
* gtk3
* webkit2gtk
* guile-2.2

## Optional Dependencies
* gst-plugins-bad (To play videos that use certain codecs, e.g. Youtube)
* gst-libav (Required to watch videos on certain sites, e.g. Twitch.tv)
* libvpx (May also be required for videos)
* webkit2gtk >= 2.27 for performance issue fixes

## How to build
* Compile with `./build.sh`

## How to use
* Run `./schemekit` after compiling and it will launch the browser.
* Run `(open-page "https://example.org")` to see it in action.
