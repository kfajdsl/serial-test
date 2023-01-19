#ifdef __cplusplus
extern "C" {
#endif
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Calculate CRC8 checksum of data
 * @param bytes Pointer to data buffer.
 * @param len Length of data buffer.
 * @return CRC8 checksum
 */
uint8_t crc8(uint8_t* bytes, size_t len); 



/**
 * Encode data with zero delimited COBS. Includes a start delimeter.
 * @param data Pointer to data buffer.
 * @param len Length of data buffer.
 * @param[out] buf Buffer to write encoded data to. sizeof(buf) MUST BE >= len + 2.
 */
void cobs_encode(const uint8_t* data, size_t len, uint8_t* buf);

/**
 * Decode zero delimited COBS data, including a start delimeter.
 * @param data Pointer to encoded data buffer.
 * @param len Length of encoded data buffer.
 * @param[out] buf Buffer to write decoded data to. sizeof(buf) MUST BE >= len - 2
 */
void cobs_decode(const uint8_t* encoded, size_t len, uint8_t* buf);


/**
 * Create a packet suitable for serial comms. Uses COBS framing and includes a CRC8 checksum.
 * @param msg Pointer to data struct. **Last byte of struct must be reserved for checksum.**
 * @param msg_len Length of msg struct, including checksum byte.
 * @param[out] buf Buffer to write packet data to. sizeof(buf) MUST BE >= msg_len + 2
 */
void packetize(const void* msg, size_t msg_len, uint8_t* buf);


/**
 * Decode a packet created with packetize().
 * @param packet Pointer to packet buffer. 
 * @param msg_len Length of **msg struct**, including checksum byte. Not the length of packet.
 *                Should be the same as used in packetize().
 * @param[out] msg Buffer to write decoded data to (probably a struct).
 */
bool depacketize(const uint8_t* packet, size_t msg_len, void* msg);

#ifdef __cplusplus
}
#endif
