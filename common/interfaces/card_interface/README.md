
The communication between device and desktop application occurs via NFC.
We are using adafruit_pn532 for nfc communication.
Maximum data packet size supported is 256 bytes (`PN532_PACKBUFF_SIZE` defined in `adafruit_pn532.c`).
The communication protocol uses 1 byte checksum for maintaining data integrity.

# Payload structure (for APDUs)
The payload of the NFC packet is serialised based on TLV structure as defined below:
 * [TAG][Length][Value] where,
 * Tag : Unique Identifier
 * Length : Length of Value. (skipped if length of value = 1)
 * Value : The data to be sent
 * Example for Name - [0xE0][16][Data from wallet->wallet_name]

# NFC communication packet structure
| Frame byte index | Data / Value |
| --- | --- |
| 0 | PN532_PREAMBLE (0x00) |
| 1 | PN532_STARTCODE1 (0x00) |
| 2 | PN532_STARTCODE2 (0xFF) |
| 3 | `2 + data_len + 1` |
| 4 | &lt;checksum complement of `2 + data_len + 1`&gt; |
| 5 | PN532_HOSTTOPN532 (0xD4) / PN532_PN532TOHOST (0xD5) |
| 6 | PN532_COMMAND_INDATAEXCHANGE (0x40) |
| 7 | `in_listed_tag` |
| 8 - ... (`data_len`) | &lt;data to transfer&gt; |
| (n-1) | &lt;Checksum data&gt; |
| n | PN532_POSTAMBLE (0x00) |

`HEADER_SEQUENCE_LENGTH` defined in `ada_pn532.h`

`CHECKSUM_SEQUENCE_LENGTH` defined in `ada_pn532.h`

`PN532_COMMAND_INDATAEXCHANGE` defined in `ada_pn532.h`

`PN532_PREAMBLE` defined in `ada_pn532.h`

`PN532_STARTCODE1` defined in `ada_pn532.h`

`PN532_STARTCODE2` defined in `ada_pn532.h`

`PN532_HOSTTOPN532` defined in  `ada_pn532.h`
