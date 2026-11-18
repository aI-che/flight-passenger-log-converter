# Flight Passenger Log Converter

A **C** command-line tool that converts flight passenger logs **CSV → Binary → XML**, validates the XML against an **XSD** schema, and converts its encoding. It uses **libxml2**.

## Features

| Type | Operation |
| --- | --- |
| `1` | CSV → Binary |
| `2` | Binary → XML |
| `3` | Validate XML with XSD |
| `4` | Convert XML encoding (UTF-8 / UTF-16LE / UTF-16BE) |

CSV separator (comma, tab or semicolon) and line-ending style (Windows, Linux or macOS) are configurable.

## Build

```bash
sudo apt install gcc libxml2-dev
gcc flighttool.c -o flightTool $(xml2-config --cflags --libs)
```

## Usage

```bash
./flightTool <input> <output> <type> [-separator 1|2|3] [-opsys 1|2|3] [-encoding 1|2|3]
```

```bash
./flightTool passengers.csv passengers.bin 1 -separator 1 -opsys 2   # CSV → Binary
./flightTool passengers.bin passengers.xml 2                         # Binary → XML
./flightTool passengers.xml flightlogs.xsd 3                         # Validate
./flightTool passengers.xml out_utf16.xml 4 -encoding 1              # → UTF-16LE
```

Run `./flightTool -h` to see all options.

## Tech

C · libxml2 · XML / XSD · Binary file processing

## License

MIT
