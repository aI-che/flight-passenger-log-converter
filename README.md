# Flight Passenger Log Converter

A command-line tool written in C for converting and processing flight passenger log data.

## Features

* CSV → Binary conversion
* Binary → XML conversion
* XML validation using XSD
* XML encoding conversion
* UTF-8, UTF-16LE and UTF-16BE encoding support
* Configurable CSV separators
* Command-line interface
* Flight passenger record processing

The application uses the `libxml2` library for XML parsing, generation and XSD validation.

## Supported Conversions

| Type | Conversion                    |
| ---- | ----------------------------- |
| `1`  | CSV → Binary                  |
| `2`  | Binary → XML                  |
| `3`  | XML → XSD Validation          |
| `4`  | XML → XML Encoding Conversion |

## Requirements

* GCC
* `libxml2`
* `libxml2-dev` on Debian/Ubuntu-based Linux distributions

## Installation

On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install gcc libxml2-dev
```

## Build

Compile the program with:

```bash
gcc flightTool.c -o flightTool $(xml2-config --cflags --libs)
```

## Usage

```bash
./flightTool <input_file> <output_file> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-encoding <1|2|3>]
```

### Arguments

| Argument            | Description                                         |
| ------------------- | --------------------------------------------------- |
| `<input_file>`      | Source file                                         |
| `<output_file>`     | Target file                                         |
| `<conversion_type>` | Conversion operation to perform                     |
| `-separator`        | CSV separator type                                  |
| `-opsys`            | Operating system option                             |
| `-encoding`         | XML encoding type, required for conversion type `4` |

## Separator Options

```text
1 = Comma (,)
2 = Tab
3 = Semicolon (;)
```

## Operating System Options

```text
1 = Windows
2 = Linux
3 = macOS
```

## Encoding Options

```text
1 = UTF-16LE
2 = UTF-16BE
3 = UTF-8
```

## Examples

### 1. CSV → Binary

```bash
./flightTool passengers.csv passengers.bin 1 -separator 1 -opsys 2
```

### 2. Binary → XML

```bash
./flightTool passengers.bin passengers.xml 2
```

### 3. XML Validation with XSD

```bash
./flightTool passengers.xml flightlogs.xsd 3
```

### 4. XML Encoding Conversion

UTF-8 → UTF-16LE:

```bash
./flightTool passengers.xml passengers_utf16.xml 4 -encoding 1
```

UTF-8 → UTF-16BE:

```bash
./flightTool passengers.xml passengers_utf16be.xml 4 -encoding 2
```

UTF-8 → UTF-8:

```bash
./flightTool passengers.xml passengers_utf8.xml 4 -encoding 3
```

## Passenger Record

The binary records contain the following information:

* Ticket ID
* Timestamp
* Baggage weight
* Loyalty points
* Status
* Destination
* Cabin class
* Seat number
* Application version
* Passenger name

## Help

To display the command-line help:

```bash
./flightTool -h
```

or:

```bash
./flightTool --help
```

## Project Structure

```text
flight-passenger-log-converter/
├── flightTool.c
├── flightlogs.xsd
├── README.md
├── LICENSE
└── .gitignore
```

## Technologies

* C
* GCC
* libxml2
* XML
* XSD
* Binary file processing

## License

This project is licensed under the MIT License.

