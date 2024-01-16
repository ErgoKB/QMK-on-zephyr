/*
 * This software is experimental and a work in progress.
 * Under no circumstances should these files be used in relation to any critical system(s).
 * Use of these files is at your own risk.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This files are free to use from http://engsta.com/stm32-flash-memory-eeprom-emulator/ by
 * Artur F.
 *
 * Modifications for QMK and STM32F303 by Yiancar
 *
 * This library assumes 8-bit data locations. To add a new MCU, please provide the flash
 * page size and the total flash size in Kb. The number of available pages must be a multiple
 * of 2. Only half of the pages account for the total EEPROM size.
 * This library also assumes that the pages are not used by the firmware.
 */

#pragma once

#include <stdint.h>

// shim for emulated flash settings
#define FEE_PAGE_SIZE CONFIG_FEE_PAGE_SIZE
#define FEE_PAGE_COUNT CONFIG_FEE_PAGE_COUNT
#define FEE_PAGE_BASE_ADDRESS 0

// shim for derived flash settings
#define FEE_DENSITY_BYTES (FEE_PAGE_COUNT * FEE_PAGE_SIZE / 2)
#define FEE_WRITE_LOG_BYTES (FEE_PAGE_COUNT * FEE_PAGE_SIZE - FEE_DENSITY_BYTES)
/* Start of the emulated eeprom compacted flash area */
#define FEE_COMPACTED_BASE_ADDRESS FEE_PAGE_BASE_ADDRESS
/* End of the emulated eeprom compacted flash area */
#define FEE_COMPACTED_LAST_ADDRESS                                             \
  (FEE_COMPACTED_BASE_ADDRESS + FEE_DENSITY_BYTES)
/* Start of the emulated eeprom write log */
#define FEE_WRITE_LOG_BASE_ADDRESS FEE_COMPACTED_LAST_ADDRESS
/* End of the emulated eeprom write log */
#define FEE_WRITE_LOG_LAST_ADDRESS                                             \
  (FEE_WRITE_LOG_BASE_ADDRESS + FEE_WRITE_LOG_BYTES)

typedef enum {
  FLASH_BUSY = 1,
  FLASH_ERROR_PG,
  FLASH_ERROR_WRP,
  FLASH_ERROR_OPT,
  FLASH_COMPLETE,
  FLASH_TIMEOUT,
  FLASH_BAD_ADDRESS
} FLASH_Status;

uint16_t EEPROM_Init(void);
void     EEPROM_Erase(void);
uint8_t  EEPROM_WriteDataByte(uint16_t Address, uint8_t DataByte);
uint8_t  EEPROM_WriteDataWord(uint16_t Address, uint16_t DataWord);
uint8_t  EEPROM_ReadDataByte(uint16_t Address);
uint16_t EEPROM_ReadDataWord(uint16_t Address);

void print_eeprom(void);
