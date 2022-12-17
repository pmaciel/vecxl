# vecxl
:space_invader: Vector graphics and lasers (Vectrex emulator vecx + Helios Laser DAC)

This project connects vector graphics to lasers. The starting point is `vecx` (the portable Vectrex emulator) and Helios Laser DAC for USB-ILDA laser interfacing.


Setup
=====

Requirements
---

- `sdl2`, `sdl2_gfx`, `sdl2_image`
- `libusb`

macOS
-----

```brew install sdl2 sdl2_gfx sdl2_image libusb```


Usage
=====

```vecxl game.bin```


Authors
=======

- [pmaciel](https://github.com/pmaciel)

vecx authors:

- Valavan Manohararajah - original author
- [John Hawthorn](https://twitter.com/jhawthorn) - SDL port
- [Nikita Zimin](https://twitter.com/nzeemin) - audio
- [Optixx](https://twitter.com/optixx) - SDL2 port

Helios Laser DAC is authored by Bitlasers

