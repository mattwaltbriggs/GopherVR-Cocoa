# GopherVR-Cocoa

**Native macOS (Cocoa/AppKit) port of [GopherVR](https://github.com/mattwaltbriggs/gopherVR) — no X11 required.**

GopherVR renders [Gopher](https://en.wikipedia.org/wiki/Gopher_protocol) space as a navigable 3D environment. The original 1990s codebase ran on X11/Motif. This fork replaces the entire X11/Motif layer with native Cocoa/AppKit, so it runs directly on macOS without XQuartz.

See also: [GopherVR (X11/Motif version)](https://github.com/mattwaltbriggs/gopherVR)

## What this is

This is a from-scratch native macOS port of GopherVR that:

- **Replaces X11 and Motif** with Cocoa `NSWindow`, `NSView`, `NSTextField`, `NSTimer`, and `NSAlert`
- **Renders to a software bitmap** via Core Graphics (the existing [libvogl](https://github.com/michael-lazar/gopherVR/tree/master/libvogl) software renderer draws to a pixel buffer — no OpenGL/Metal needed)
- **Runs as a standalone `.app` bundle** — drag it to Applications and launch normally
- **Handles all mouse/keyboard interaction natively** — drag to rotate, double-click to select items, keyboard shortcuts for navigation

## What we did

This was a multi-phase port from X11/Motif to native macOS:

### Phase 1: Compile and run via XQuartz

The original code required several fixes to compile on Apple Silicon macOS:

- **Case-insensitive filesystem fix** — The `gopher/object/` headers `Stdlib.h`, `String.h`, `Locale.h` shadow system headers. Fixed with `-iquote` instead of `-I` for quoted includes.
- **libXt ABI crash fix** — OpenMotif 2.3.8 needs Homebrew's arm64 libXt. Linked against the correct version to avoid the `XtWidgetToApplicationContext` crash.
- **K&R → ANSI C conversions** — All K&R function definitions and empty-parens `()` declarations converted to ANSI C across 14+ source files for modern clang.
- **`.app` bundle** — Created `GopherVR.app` with auto-detection of the launch context via `_NSGetExecutablePath`.

### Phase 2: Replace X11/Motif with native Cocoa

This is where the real work happened. Every X11/Motif component was replaced:

#### New files created

| File | Purpose |
|------|---------|
| `gophervr/AppDelegate.h` | `GVRCanvasView` (custom `NSView` for the 3D viewport) and `AppDelegate` interface |
| `gophervr/AppDelegate.m` | Main Cocoa app — window creation, menu bar, toolbar, render timer, mouse/keyboard events, background Gopher bootstrap |
| `gophervr/main.m` | `NSApplication` entry point with `chdir()` to app Resources for Hershey font access |
| `gophervr/gophwin_cocoa.m` | Full Cocoa Gopher Menu window — history table, title button, items table, abstract text area (replaces Motif `gophwin.c`) |
| `gophervr/cocoa_dialogs.m` | Native Cocoa dialogs — text display windows, search input, about box, error alerts, URL/FTP input dialogs |
| `libvogl/drivers/cocoa.m` | Software rendering backend for libvogl — draws polygons/lines via Bresenham rasterization into a pixel buffer, exposes it as `NSImage` |

#### Stub files (replacing Motif/Xt code)

| File | Replaces |
|------|----------|
| `stub_burrower.c` | Main event loop, mouse handlers, `jumpto()` navigation |
| `stub_menus.c` | Motif `BuildMenu`/`Create_Menubar` |
| `stub_text.c` | Motif text display widgets |
| `stub_motiftools.c` | Motif toolbar, status bar, cursor, file picker |
| `stub_dialogs.c` | Motif dialog widgets |
| `stub_helpdiag.c` | Motif help system |
| `stub_error.c` | Motif error dialogs |

#### Key design decisions

- **Software rendering** — libvogl's `cocoa.m` driver renders to an in-memory RGBA pixel buffer using Bresenham line algorithm and scanline polygon fill. The buffer is copied to an `NSBitmapImageRep` → `NSImage` → displayed in `GVRCanvasView`. No OpenGL, no Metal, no GPU required.

- **Double-buffering** — The Cocoa driver maintains front and back buffers. `drawscene()` renders to the back buffer, then `swapbuf` makes it visible.

- **Background bootstrap** — Gopher network I/O (`WorldStart`) runs on a GCD background queue to keep the UI responsive. The main thread handles rendering at ~30fps via `NSTimer`.

- **Bootstrapped flag** — Set to `2` early to prevent the 3D scene's `Bootstrap()` from blocking the main thread. Background thread handles `WorldStart` instead.

- **Click coordinate mapping** — View coordinates from `convertPoint:fromView:` are scaled to vogl viewport coordinates before hit-testing via `sceneclick()`.

- **Mouse drag** — Uses quadratic accumulation matching the original X11 behavior: `dragStart` is not updated during drag, so rotation accelerates with total displacement from click point.

- **Hershey fonts** — The font file `futura.hfont` must be accessible from the working directory. The app `chdir()`s to the `.app` bundle's Resources directory at startup.

## Building from source

### Prerequisites

```bash
# macOS 11+ with Apple Silicon
# No X11, no Motif, no external dependencies — just Xcode Command Line Tools
xcode-select --install
```

### Build

```bash
# Build the libraries
make -C gopher/object -f Makefile
make -C libvogl -f Makefile.cocoa
make -C libhershey
make -C libtracker

# Build gophervr-cocoa
cd gophervr
make -f Makefile.cocoa
```

### Debug build

```bash
make -f Makefile.cocoa.debug
```

### Build the .app bundle

```bash
bash build_app.sh
open GopherVR-Cocoa.app
```

## Running

```bash
# Run from terminal
./gophervr-cocoa

# Or launch the .app bundle
open GopherVR-Cocoa.app
```

The app connects to `gopher://gopher.floodgap.com/` by default. Use File → Open Location to enter a different Gopher URL.

### Mouse controls

- **Drag** — Rotate the 3D view
- **Double-click** — Select a Gopher item (directory, document, search)
- **Right-click** — Show info about the clicked item
- **Scroll wheel** — Zoom (if supported)

### Keyboard shortcuts

| Key | Action |
|-----|--------|
| `[` | Move eye up |
| `]` | Move eye down |
| `i` | Initial viewpoint |
| `Space` | Overview |
| `m` | Show Gopher menu |
| `r` | Reload current directory |
| `o` | Open location |
| `q` | Quit |

## Repository structure

```
gophervr/              Application source
  AppDelegate.h/m      Main Cocoa app delegate and 3D canvas view
  gophwin_cocoa.m      Gopher Menu window (Cocoa replacement for gophwin.c)
  cocoa_dialogs.m      Native dialogs (text windows, search, about, errors)
  main.m               NSApplication entry point
  lcube.c              3D scene initialization
  gopherto3d.c         Gopher-to-3D object rendering
  stub_*.c             Stubs replacing Motif/Xt code
  Makefile.cocoa        Release build
  Makefile.cocoa.debug  Debug build
  build_app.sh         .app bundle builder

gopher/object/         Gopher protocol library
libvogl/               Very Ordinary GL (3D rendering library)
  drivers/cocoa.m      Native Cocoa software rendering backend
libhershey/            Hershey font library
libtracker/            Audio tracker library
lib/                   Compiled static libraries
```

## Architecture

```
┌─────────────────────────────────────────────────┐
│                  AppDelegate                     │
│  (NSWindow, menus, toolbar, render timer)       │
├─────────────────────────────────────────────────┤
│              GVRCanvasView (NSView)             │
│  (mouse/keyboard events, NSImage display)       │
├─────────────────────────────────────────────────┤
│              libvogl (3D rendering)             │
│  ├── cocoa.m (software rasterizer)             │
│  ├── bsp.c (BSP tree, sceneclick hit-testing)  │
│  └── polygons.c (polygon clipping, WTOVX/Y)    │
├─────────────────────────────────────────────────┤
│         gopherto3d.c (Gopher → 3D objects)      │
│  ├── RenderGophers() — creates 3D scene         │
│  ├── SelectGopherDir() — handles selection      │
│  └── V_OpenSessionURL() — URL navigation        │
├─────────────────────────────────────────────────┤
│          gopher/object (protocol library)        │
│  (GopherDirObj, GopherObj, network I/O)         │
└─────────────────────────────────────────────────┘
```

## Credits

- **Paul Lindner** — Original GopherVR author
- **Mark McCahill, Neophytos Iacovou** — GopherVR rendering engine
- **Cameron Kaiser** — X11/Motif macOS port, compatibility work
- **Floodgap** — Running `gopher.floodgap.com`, the default Gopher server
- **Michael Lazar** — Upstream `gopherVR` repository

## License

This project retains the original GopherVR license. See the source files for details.
