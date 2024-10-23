# TVpilot - TV Show Schedule Tracker

TVpilot downloads and caches TV show information from epguides.com and allows you to track
broadcast dates for previous and upcoming episodes for the shows you configure. Individual episodes can be
flagged as missing or watched. Periodically selecting 'Download' refreshes the show
information from epguides.com and adds newly announced episodes to the database.

## Build Dependencies

The project is configured to build a 32-bit Windows target.

The Boost C++ libraries must be in the include path.
The header files and libraries for both **libcurl** and **libxml2** must also be available.

Before building, configure the appropriate INCLUDE and LIBRARY settings in the project
properties.

As far as know there is no version dependence on any of these libraries, the version
numbers linked to below just happen to be the ones I used.

## Third Party Libraries Download Locations

These four DLLs must be available at runtime. I put them in the same folder as the
executable but they can be in any of the standard Windows DLL search locations.

- libcurl.dll
- libxml2.dll
- zlib1.dll
- iconv.dll

### Curl

Prebuilt binary available from https://curl.se/download.html

- [curl](https://curl.se/download.html)

### libxml2, iconv & zlib

Prebuilt binaries available from https://www.zlatkovic.com/pub/libxml/

- [libxml2](https://www.zlatkovic.com/pub/libxml/libxml2-2.7.8.win32.zip)
- [zlib](https://www.zlatkovic.com/pub/libxml/zlib-1.2.5.win32.zip)
- [iconv](https://www.zlatkovic.com/pub/libxml/iconv-1.9.2.win32.zip)
