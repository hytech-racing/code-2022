### General information
Accepts .csv as input, writes to .csv as output. Note that opening a .csv in Excel may cause fields to be converted to scientific notation, which will cause the program to break. Make sure that the csv contains values in their fully expanded forms.

Compile command: `g++ --std=c++11 sd_reader.cpp -o sd_reader`
Run command: `sd_reader $OPTIONAL_INPUT_PATH $OPTIONAL_OUTPUT_PATH`

If not provided in run command, user will be prompted for input/output file paths during runtime. Files must have extension .csv, although the extension will be automatically appended if not included in file path.

### Preprocessor flags
These flags can be found at beginning of `sd_reader.cpp`;

`OUTPUT_MODE`       - If output file already exists, dictates whether new data will append to or overwrite old data.
`USE_VECTOR_HEADER` - Depending on your compiler you may have either the `<vector.h>` or `<bits/stdc++.h>`. If program fails to compile, try toggling this flag. 

### Input format
```
INTEGER time    - epoch time at which message was send
HEXCODE msg.id  - two-character hex code indicating CAN message type
INTEGER length  - length (in bytes) of can message, including leading/trailing zeroes
HEXCODE data    - CAN message hex code containing (2 * length) characters. Need not include leading zeroes.
```

### Message definitions
Each CAN message is defined as a vector containing message definition objects.

Define a message in the `can_msg_def.h` file using the following syntax:
```
CAN_MSG_DEFINITION[0x$TWO_CHARACTER_HEXCODE$] = pair<string, vector<definition>> ($NAME_OF_MESSAGE$, {
  $DEFINITION_0$
  $DEFINITION_1$
  $DEFINITION_2$
  ...
});
```

Constructors for "definition" object
```
// Single-value data field
definition(int offset, int len, bool isSigned, string field, ?string units)

// Packed booleans
definition(int offset, int len, vector<string> booleanMappings)

// Definition of constructor parameters
INTEGER  offset          - distance from start of message (in bytes);
INTEGER  len             - length of data type (in bytes)
BOOLEAN  isSigned        - whether the data can hold a negative value

STRING   field           - name of data field
?STRING  units           - optional. units of data field

STRING[] booleanMappings - names of booleans packed into field
```
