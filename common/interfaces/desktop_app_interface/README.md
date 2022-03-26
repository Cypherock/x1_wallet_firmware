
The communication between device and desktop application occurs via a USB.
Maximum data packet size supported is 32 bytes (defined in `communication.h`).
The communication protocol uses CRC16 for maintaining data integrity.


## USB communication packet structure
### Before byte stuffing
| Frame byte index | Data / Value |
| --- | --- |
| 0 | START_OF_FRAME_U16 (0x5A5A) or START_OF_FRAME (0xAA) |
| 1 | &lt;command type&gt; [link](link to protocol doc) |
| 2 | &lt;data length&gt; + 2 bytes of CRC16 data |
| 3 - 4 | &lt;packet number for this frame&gt; (in big endian) |
| 5 - 6 | &lt;total packet number&gt; (in big endian) |
| 7 - ... (`data_len`) | &lt;data to transfer&gt; |
| (n-1) - (n) | CRC16 data (in big endian) |

### After byte stuffing (actual data ready to transfer)
| Frame byte index | Data / Value |
| --- | --- |
| 0 | START_OF_FRAME_U16 (0x5A5A) or START_OF_FRAME (0xAA) |
| 1 | &lt;command type&gt; [link](link to protocol doc) |
| 2 | &lt;byte stuffed data length&gt; |
| 3 - ... | &lt;byte stuffed array&gt; (byte stuffing of data to transfer + CRC16 data) |

***NOTE: START_OF_FRAME is used for older packet structure where 1-byte command_type is accepted while
with the new start of frame i.e. START_OF_FRAME_U16, 4-byte command-type is used.