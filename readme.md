# HDLC High-Level Data Link Control 

| Branch | Status|
| ------- | ----- |
| master | [![Build Status](https://travis-ci.com/lokraszewski/hdlco.svg?branch=master)](https://travis-ci.com/lokraszewski/hdlco) |
| develop | [![Build Status](https://travis-ci.com/lokraszewski/hdlco.svg?branch=develop)](https://travis-ci.com/lokraszewski/hdlco) |

This small library provides the means to create and serilize HDLC frames in the non-extended mode.

## Frame Structure
<table class="wikitable">
   <tbody>
      <tr>
         <th>Flag</th>
         <th>Address</th>
         <th>Control</th>
         <th>Information</th>
         <th>FCS</th>
         <th>Flag</th>
      </tr>
      <tr>
         <td>8 bits</td>
         <td>8 or more bits</td>
         <td>8</td>
         <td>Variable length, 8×<i>n</i> bits</td>
         <td>16</td>
         <td>8 bits</td>
      </tr>
   </tbody>
</table>

Note that the end flag of one frame may be (but does not have to be) the beginning (start) flag of the next frame.

Data is usually sent in multiples of 8 bits, but only some variants require this; others theoretically permit data alignments on other than 8-bit boundaries.

The frame check sequence (FCS) is a 16-bit CRC-CCITT or a 32-bit CRC-32 computed over the Address, Control, and Information fields. It provides a means by which the receiver can detect errors that may have been induced during the transmission of the frame, such as lost bits, flipped bits, and extraneous bits. However, given that the algorithms used to calculate the FCS are such that the probability of certain types of transmission errors going undetected increases with the length of the data being checked for errors, the FCS can implicitly limit the practical size of the frame.

If the receiver's calculation of the FCS does not match that of the sender's, indicating that the frame contains errors, the receiver can either send a negative acknowledge packet to the sender, or send nothing. After either receiving a negative acknowledge packet or timing out waiting for a positive acknowledge packet, the sender can retransmit the failed frame.

## Asynchronous framing
When using asynchronous serial communication such as standard RS-232 serial ports, synchronous-style bit stuffing is inappropriate for several reasons:
* Bit stuffing is not needed to ensure an adequate number of transitions, as start and stop bits provide that,
* Because the data is NRZ encoded for transmission, rather than NRZI encoded, the encoded waveform is different,
 * RS-232 sends bits in groups of 8, making adding single bits very awkward, and
* For the same reason, it is only necessary to specially code flag bytes; thus it is not necessary to worry about the bit pattern straddling multiple bytes.

Instead asynchronous framing uses "control-octet transparency", also called "byte stuffing" or "octet stuffing". The frame boundary octet is 01111110, (0x7E in hexadecimal notation). A "control escape octet", has the value 0x7D (bit sequence '10111110', as RS-232 transmits least-significant bit first). If either of these two octets appears in the transmitted data, an escape octet is sent, followed by the original data octet with bit 5 inverted. For example, the byte 0x7E would be transmitted as 0x7D 0x5E ("10111110 01111010"). Other reserved octet values (such as XON or XOFF) can be escaped in the same way if necessary.

The "abort sequence" 0x7D 0x7E ends a packet with an incomplete byte-stuff sequence, forcing the receiver to detect an error. This can be used to abort packet transmission with no chance the partial packet will be interpreted as valid by the receiver.

## Frame types:
* Information frames, or I-frames, transport user data from the network layer. In addition they can also include flow and error control information piggybacked on data.
* Supervisory Frames, or S-frames, are used for flow and error control whenever piggybacking is impossible or inappropriate, such as when a station does not have data to send. S-frames do not have information fields.
* Unnumbered frames, or U-frames, are used for various miscellaneous purposes, including link management. Some U-frames contain an information field, depending on the typ

### Control Field
<table  style="text-align:center">
   <caption>HDLC control fields</caption>
   <tbody>
      <tr>
         <th>7</th>
         <th>6</th>
         <th>5</th>
         <th>4</th>
         <th>3</th>
         <th>2</th>
         <th>1</th>
         <th>0</th>
         <th></th>
      </tr>
      <tr>
         <td colspan="3">N(R)<br>Receive sequence no.</td>
         <td>P/F</td>
         <td colspan="3">N(S)<br>Send sequence no.</td>
         <td>0</td>
         <th>I-frame</th>
      </tr>
      <tr>
         <td colspan="3">N(R)<br>Receive sequence no.</td>
         <td>P/F</td>
         <td colspan="2">type</td>
         <td>0</td>
         <td>1</td>
         <th>S-frame</th>
      </tr>
      <tr>
         <td colspan="3">type</td>
         <td>P/F</td>
         <td colspan="2">type</td>
         <td>1</td>
         <td>1</td>
         <th>U-frame</th>
      </tr>
   </tbody>
</table>

## Examples
### Create frame. 
```cpp
const auto payload = std::string("PAYLOAD");
const Frame frame(payload, Frame::Type::I);
```

### Serialize frame. 
```cpp
#include "hdlc/hdlc.h"
const Frame frame; //Some frame
const auto raw = FrameSerializer::serialize(frame); //Pack the frame into HDLC format
const auto raw_escaped = FrameSerializer::escape(raw); //Perform HDLC byte stuffing
magical_user_transmit(raw_escaped); //User implementation for transfering bytes. 
```
### De-Serialize frame. 
```cpp
#include "hdlc/hdlc.h"
const auto raw_escaped = magical_user_recieve();
const auto raw = FrameSerializer::descape(raw_escaped);
const Frame frame = FrameSerializer::deserialize(raw);
if(frame.is_empty())
{
   //Frame empty, something has gone very wrong. 
}
else
{
   //handle the frame
}

```
## Design Notes
* Currently the library only provides the means to create and serilize HDLC frames, there is no transfer implementaion or session management. This is difficult to implement since I would like for this library to be usable on both desktop and embedded platforms hence for the time being it is up to the user to implement transfer of serialized frames. 
* The underlaying storage type is vector which requires heap allocation, on embedded platforms I have tested this with FreeRTOS allocator with little issues but it may be easier to operate on buffers that have been pre-allocated. 
* The sessions are blocking the thread they are ran on. This is so that the user can implement their own threading based on the OS (bare metal, linux).
* The sessions are very minial implemenations which worked for my application feel free to fork to adapt to your needs.


## Prerequisites
* [cmake](https://cmake.org/)
* [conan](https://conan.io/)
* [serial](https://github.com/lokraszewski/serial) for example application.

## Build
```
mkdir build && cd build
cmake .. && make
```

## Running tests
```
./bin/hdlc_test
```

## TODOs ##
* Implement example packet hardware transfer.
* Implement example packet reciever and handler. 

## References
* [Data Communication Lectures of Manfred Lindner – Part HDLC](https://www.ict.tuwien.ac.at/lva/384.081/infobase/L03-HDLC_v4-4.pdf)

## Other HDLC related repositories 
* [hdlcpp](https://github.com/bang-olufsen/hdlcpp) by [Bang & Olufsen](https://github.com/bang-olufsen) -  header only C++11 HDLC implemenation. 
* [hdlcd](https://github.com/Strunzdesign/hdlcd) by [Strunzdesign](https://github.com/Strunzdesign) - The HDLC Daemon (HDLCd), desktop daemon implementaion 
