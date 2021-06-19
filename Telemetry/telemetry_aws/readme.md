## Consolidated Python Telemetry Utilities

### Main Scripts
- `csv_server.js`         - Node.js Express server -- Exposes endpoint for raw telemetry CSV uploads.
- `mqtt_server.py`        - Python Server -- Receives MQTT messages and writes directly to Influx.

### Configuration Files
- `csv_to_influx.py`      - Sub-process of csv_server.js. Parses CSV files and writes to Influx.
- `db.py`                 - Contains main decoder, MQTT decryption methods, and Influx database utilities

### Miscellaneous
- `index.html`            - Front-end portal for csv_server.js
- `package.json`          - Specifies Node.js dependencies for csv_server.js

### Creating a new CAN Message
To process a new CAN message, update `db.py`.
The only method that ever needs to be changed is the large `decode(msg)` method at the end of the file.
A new message can be defined as follows:
```
elif (id == <CAN_ID>):
    ret.append([<NAME_OF_DATA_POINT>, <DECRYPTION_METHOD>, <?UNITS>])
    ret.append(...)
    ...
# Note that the CAN message will start at `msg[4]`.
# Note that words in `<NAME_OF_DATA_POINT>` should be capitalized and delimited with underscores `_`.
```

### Common decryption methods
Each element of `msg` will be an encoded byte. To decrypt a set of bytes, use one of the following:
- `b2i8`    - Accepts byte-encoded substring `msg[A:B]` with length `B - A = 1`. Produces `unsigned 8-bit integer`.
- `b2i16`   - Accepts byte-encoded substring `msg[A:B]` with length `B - A = 2`. Produces `unsigned 16-bit integer`.
- `b2ui16`  - Accepts byte-encoded substring `msg[A:B]` with length `B - A = 2`. Produces `unsigned 16-bit integer`.
- `b2ui32`  - Accepts byte-encoded substring `msg[A:B]` with length `B - A = 4`. Produces `unsigned 32-bit integer`.
- `b2ui64`  - Accepts byte-encoded substring `msg[A:B]` with length `B - A = 8`. Produces `unsigned 64-bit integer`.

### Adding a message to the live console
To display a new CAN message on the live console, update `console_config.py` in the neighboring `telemetry_py` directory.
Add your message in the form `<NAME OF DATA POINT>: [<ROW>, <COL>]`
Note that `<NAME OF DATA POINT>` should be exactly the same as `NAME_OF_DATA_POINT` defined in `db.py`, but with all underscores `_` replaced with spaces ` `.
`<ROW>` and `<COL>` must be positive integers.

### Deploying changes
To migrate local changes to AWS, first push them to GitHub. SSH into the console, where there are two folders `hytech-telemetry` and `telemetry_aws`. `cd` into either one, as `hytech-telemetry` is just a symbolic link to `telemetry_aws/Telemetry/telemetry_aws`. This file structure may seem awkward but do not attempt to alter it, it's a consequence of how GitHub's spare-checkout procedure works. Checkout the branch where you've pushed your changes and pull.

Once you're sure that you've uploaded the correct version of your code, run `sudo systemctl restart <NAME_OF_SERVICE>`, where `<NAME_OF_SERVICE>` will be `hytech-csv` for `csv_server.js` and `hytech-telemetry` for `mqtt_server.py`.
To verify that the processes are running, use `systemctl status <NAME_OF_SERVICE>`.
To follow the live output on either service, use `sudo journalctl -u <NAME_OF_SERVICE>`.
