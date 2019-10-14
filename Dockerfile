FROM archlinux/base
RUN pacman -Syu --noconfirm pkgconf base-devel gtk3 webkit2gtk guile xorg-xauth
WORKDIR /schemekit
COPY browser.c /schemekit/browser.c
RUN gcc $(pkg-config --cflags --libs guile-2.2 gtk+-3.0 webkit2gtk-4.0) browser.c -o schemekit

CMD ["/schemekit/schemekit"]
