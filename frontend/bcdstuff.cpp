#include <QtCore>
#include "bcdstuff.h"

/**
 * @brief Convert a BCD byte to a decimal byte.
 * @param bcd
 * @return
 */
uint8_t bcd_to_decimal(uint8_t bcd) {
    return ((((bcd & 0xf0) >> 4) * 10) + ((bcd & 0x0f)));;
}

/**
 * @brief Convert a decimal byte to a BCD byte.
 * @param dec
 * @return
 */
uint8_t decimal_to_bcd(uint8_t dec) {
    uint8_t msn = static_cast<uint8_t>(dec/10);
    uint8_t lsn = static_cast<uint8_t>(dec%10);
    uint8_t value = static_cast<uint8_t>(msn << 4) | static_cast<uint8_t>(lsn);
    return value;
}
