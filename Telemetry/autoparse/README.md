# HyTech Automatic Telemetry Server

Important: before using this module, please ensure that you are using a version of the telemetry control unit which uses millisecond timestamp precision.

This module contains:

- Code to parse CAN Library "JavaDoc"
- Bash scripts for telemetry functions:
	- [parse_folder]('./exe/parse_folder') - Parse SD Data Folder
	- [console]('./exe/console') - Live Console
	- [influx_server]('./exe/influx_server') - Live InfluxDB Telemetry Server
	- [upload]('./exe/upload') - CSV-to-InfluxDB Uploads
- AWS Webserver (available [here]('http://ec2-3-134-2-166.us-east-2.compute.amazonaws.com:5000/))
	- Status / refresh / branch switching
	- CSV upload
	- Grafana

Always run this code in a linux environment (LinuxOS or WSL)

## Creating / Using the Parser

For the initial build, run `make install` and `make CANParser`

For all future builds you can just run `make CANParser`

Parse a single file using `./CANParser -i <input filepath> -o <output filepath>`

If `--pipelined` flag is used, input filepath defaults to stdin, output filepath defaults to stdout.

If `--pipelined` flag is omitted, output filepath defaults to `{input filepath}_parsed.csv`

## Using Bash Scripts

Navigate to the [exe]('./exe') folder and run `sh setup.sh`
If this gives you some trouble, try:

```
chmod +x setup.sh
./setup.sh
chmod -x setup.sh
```
Note: This script runs chmod on a set of files, which will cause git diffs. To discard these changes, run `sh setup.sh --undo`. Again, if this causes trouble, run
```
chmod +x setup.sh
./setup.sh --undo
chmod -x setup.sh
```

NOTE: If someone wants to figure out how to resolve the pathname so that scripts can be run from anywhere, be my guest, but otherwise bash scripts should ALWAYS be run from the [exe]('./exe') folder.

### Script: parse_folder

```
Usage: ./parse_folder [directory]
```

Parses every CSV in a directory, directs ouput to a new set of CSV files located at `./log/{directory basename}`

### Script: console

```
Usage: ./console
```

Creates log file at `./log/{timestamp}.csv`

Modify the console display by editing [console_config.py](./app/console_config.py'). Each entry takes the form
```
<display name>: [<row>, <column>]
```
Where the display name is derived from CSV output using the formula
```
display_name = raw_variable_name.toUpperCase().replace('_', ' ')

Example: bms_average_temperature => BMS AVERAGE TEMPERATURE
```

## Refreshing AWS

To update AWS Parser config without logging in:
- Go to the [status page]('http://ec2-3-134-2-166.us-east-2.compute.amazonaws.com:5000/status.html)
- Select a branch
- Press `Refresh Config`

It may take a while for your changes to be reported. Please do not spam the `Refresh Config` button, but feel free to spam your browser refresh. Alternatively, show some restraint and the page will auto-refresh every 60 seconds

Sometimes you will need to make more rigorous changes or investigate a failure. In these instances you will need to SSH into the server. There are two services running:
- hytech-telemetry - Live Influx Server
- hytech-csv - Upload / Status check Server

Note: hytech-csv depends on a set of scripts located at `~/upload_scripts` for managing configuration refresh. 

A few useful commands for running/debugging the services:
- `sudo systemctl (start/kill/restart) <service name>` - run/stop/restart a service
- `systemctl status <service name>` - locate systemd file, verify service running, view stdout clip
- `journalctl -u <service name>` - view stdout of service
	- optionally supply `-f` flag to continue logging output as it is generated

## The CAN Parser

This section will detail how exactly to write JavaDoc annotations on CAN libraries

There are three types of annotation - class-level, variable-level, and function-level (only used for packed flags). The first annotation must always be the corresponding type identifier (`@Parseclass`, `@Parse`, or `@Parseflag`), as any annotations preceding these labels will be disregarded.

### Full List of Annotations

| Class-Level			| Variable-Level	| Function-Level			|
|-----------------------|-------------------|---------------------------|
| `@Parseclass`			| `@Parse`			| `@Parseflag(set, name?)`	|
| `@ID(id,name?)`		| `@Name(str)`		|							|
| `@Prefix(str, id?)`	| `@Getter(fname)`	|							|
| `@Custom(fname)`		| `@Hex`			|							|
| 						| `@Unit(str)`		|							|
| 						| `@Scale(factor, precision?)`	|				|
| 						| `@Flagprefix(str?)`	|						|
| 						| `@Flaglist(name[(fname)]?, ...)`	|			|
| 						| `@Flagset`		|							|

### General formatting rules

In this implementation of JavaDoc, you can place a single-line or multi-line comment above the code of interest (you may separate the two using whitespace-only lines). You may also place a single-line comment directly to the right of the code of interest. 

Some Valid Examples:
```cpp
// JAVADOC
code

/* JAVADOC */
code

/** JAVADOC */
code

/* JAVADOC
MORE JAVADOC */
code

/**
 * JAVADOC
 * MORE JAVADOC
 */
code

code // JAVADOC
code /* JAVADOC */
code /** JAVADOC */
```

Some Invalid examples:
```cpp
// JAVADOC
// MORE JAVADOC
code

/* JAVADOC */ code

/** JAVADOC
 * MORE JAVADOC
 */ code

code /* JAVADOC
		MORE JAVADOC */
```

Another point to note: annotations that do not accept a parameter may be followed by a space or empty parentheses. The exceptions are `@Parse` and `@Parseclass`, which must be followed by a space.

### Class-Level Annotations

`@Parseclass` - Indicates beginning of class-level annotation

`@ID(id,name?)` - CAN ID associated with message
- `name`: CAN message name associated with ID (defaults to class name)
- If no @ID label present, assumes CAN ID = `ID_{classname.toUpperCase()}`

`@Prefix(str, id?)` - Prefixes all variable names with `str`
- id: Binds prefix to specific CAN ID. 
- If omitted, sets default prefix for all CAN IDs associated with class
- ID-specific prefix will override default

`@Custom(fname)` - Custom parser function for all CAN IDs associated with class
- DO NOT include parentheses in `fname`
- Implement custom function in any .cpp file in [user](./user) directory. That file should `#include "UserDefined.h"`.
- Function prototype: `CUSTOMFUNC(<fname>, <Classname> & data)`
- Custom function should output using `show(formatstring, ...params)`, a macro that overwrites `fprintf`

### Variable-Level Annotations

`@Parse` - Indicates beginning of variable-level annotation

`@Name(str)` - Name to use when outputting variable (defaults to actual name of variable in code)

`@Getter(fname)` - Function to get value of variable (defaults to `get_{varname}`)
- Omit parentheses

`@Hex` - Variable should be written in hexadecimal form
- In flag-related variables, if `@Hex` is not specified then only individual flags will be written, not the hex format of the containing variable

`@Unit(str)` - Unit of measurement for this value

`@Scale(factor, precision?)` - Factor by which actual value has been scaled up
- Precision defaults to log10(factor)

`@Flagprefix(str?)` - `str` will be added as prefix to all packed booleans associated with this variable.
- If str is omitted, prefix will be `varname`

`@Flaglist(name[(fname)]?, ...)` - List of booleans packed within this variable
- If list item is simply in form `name`, assumes getter function is `get_{name}`
- If list item is in form `name(fname)`, boolean `name` is obtained by calling `fname()`
- DO NOT include parentheses in fname

`@Flagset` - Simply marks a variable as containing packed booleans (utility for `@Parseflag` annotation)

### Function-Level Annotations

`@Parseflag(set, name?)` indicates that the function returns a boolean associated with a flag variable (marked using `@Flagprefix`, `@Flaglist`, and/or `@Flagset`). `set` must match the name of the container variable. `name` indicates the name of the packed value, and may be omitted if the method name follows the format `get_{flagname}`, in which case `name` will default to `flagname`.