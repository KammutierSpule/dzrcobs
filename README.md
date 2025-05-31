# DZRCOBS

**Dictionary based Compression [Reversed](https://github.com/Dirbaio/rcobs) [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing)**

## Concepts
### COBS
COBS (Consistent Overhead Byte Stuffing) is a framing method used to encode payloads for transmission over byte streams. It ensures that the data contains no zero bytes, allowing a zero byte to be safely used as a frame delimiter. COBS guarantees that the encoded frame size can be determined based on the original payload size, with minimal and consistent overhead.

### Reverse COBS
Reversed COBS is a variant of the standard COBS encoding that places the length byte at the end of the chunk instead of the beginning. This optimizes the encoding process but introduces a tradeoff to the decoder as it must have access to the entire frame before decoding.

### Dictionary based Compression
In addition to COBS encoding, a dictionary-based compression scheme is applied to reduce the overall data size of the encoded frame.

## Use cases and targets
  - Mid to high-end range microcontrollers.
  - Transmit data over slow streams (eg: UART) where there is available more CPU power than bandwith.
  - Reduce payload transmition costs (eg: over GPRS, 4G)
  - LOG and sensor data storage on non-volatile memory. Reduces storage space taking advantage of repetitive patterns.

## Usage
To integrate the code on your project, you need to consider only the following two folders:
  - [dzrcobs](/dzrcobs)
  -    [dzrcobs/include](/dzrcobs/include) header files to be added to your include paths.
  -    [dzrcobs/src](/dzrcobs/src) source code to be compiled by your project.

You may want to perform a [git sparse checkout](https://git-scm.com/docs/git-sparse-checkout) to your project with only those folders.

No CMAKE file to build project as a library is currently supported.

All other files on this repository are intended for internal use.

## License
Distributed under the 3-Clause BSD License. See accompanying file LICENSE or a copy at https://opensource.org/licenses/BSD-3-Clause

SPDX-License-Identifier: BSD-3-Clause

## Contribution
Any contribution to the project be submitted under the same license of this project.

## References
Other related project references:
  - [rCOBS](https://github.com/Dirbaio/rcobs)
