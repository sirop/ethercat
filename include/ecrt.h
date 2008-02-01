/******************************************************************************
 *
 *  $Id$
 *
 *  Copyright (C) 2006  Florian Pose, Ingenieurgemeinschaft IgH
 *
 *  This file is part of the IgH EtherCAT Master.
 *
 *  The IgH EtherCAT Master is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  The IgH EtherCAT Master is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the IgH EtherCAT Master; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  The right to use EtherCAT Technology is granted and comes free of
 *  charge under condition of compatibility of product made by
 *  Licensee. People intending to distribute/sell products based on the
 *  code, have to sign an agreement to guarantee that products using
 *  software based on IgH EtherCAT master stay compatible with the actual
 *  EtherCAT specification (which are released themselves as an open
 *  standard) as the (only) precondition to have the right to use EtherCAT
 *  Technology, IP and trade marks.
 *
 *****************************************************************************/

/** \file EtherCAT realtime interface.

  \defgroup RealtimeInterface EtherCAT realtime interface

  EtherCAT interface for realtime modules. This interface is designed for
  realtime modules that want to use EtherCAT. There are functions to request a
  master, to map process data, to communicate with slaves via CoE and to
  configure and activate the bus.
*/

/*****************************************************************************/

#ifndef __ECRT_H__
#define __ECRT_H__

#include <asm/byteorder.h>

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/******************************************************************************
 * Global definitions
 *****************************************************************************/

/** EtherCAT real-time interface major version number.
 */
#define ECRT_VER_MAJOR 1

/** EtherCAT real-time interface minor version number.
 */
#define ECRT_VER_MINOR 4

/** EtherCAT real-time interface version word generator.
 */
#define ECRT_VERSION(a, b) (((a) << 8) + (b))

/** EtherCAT real-time interface version word.
 */
#define ECRT_VERSION_MAGIC ECRT_VERSION(ECRT_VER_MAJOR, ECRT_VER_MINOR)

/******************************************************************************
 * Data types 
 *****************************************************************************/

struct ec_master;
typedef struct ec_master ec_master_t; /**< \see ec_master */

struct ec_domain;
typedef struct ec_domain ec_domain_t; /**< \see ec_domain */

struct ec_slave;
typedef struct ec_slave ec_slave_t; /**< \see ec_slave */

/*****************************************************************************/

/** Bus status.
 * This is used in ec_master_status_t.
 */
typedef enum {
    EC_BUS_FAILURE = -1, /**< At least one slave with process data exchange
                           is offline. */
    EC_BUS_OK            /**< All slaves with process data exchange are
                           online. */
}
ec_bus_status_t;

/*****************************************************************************/

/** Master status.
 * This is used for the output parameter of ecrt_master_get_status().
 */
typedef struct {
    ec_bus_status_t bus_status; /**< \see ec_bus_status_t */
    unsigned int bus_tainted; /**< non-zero, if the bus topology is invalid */
    unsigned int slaves_responding; /**< number of responding slaves */
}
ec_master_status_t;

/*****************************************************************************/

/** List entry for domain PDO registrations.
 * This type is used as a parameter for the ecrt_domain_register_pdo_list()
 * convenience function.
 */
typedef struct {
    const char *slave_address; /**< slave address string
                                 \see ec_master_parse_slave_address() */
    uint32_t vendor_id; /**< vendor ID */
    uint32_t product_code; /**< product code */
    uint16_t pdo_entry_index; /**< PDO entry index */
    uint8_t pdo_entry_subindex; /**< PDO entry subindex */
    void **data_ptr; /**< address of the process data pointer */
}
ec_pdo_reg_t;

/*****************************************************************************/

/** Direction type for PDO mapping and range registration functions.
 */
typedef enum {
    EC_DIR_OUTPUT, /**< values written by master */
    EC_DIR_INPUT   /**< values read by master */
}
ec_direction_t;

/******************************************************************************
 * Global functions
 *****************************************************************************/

ec_master_t *ecrt_request_master(unsigned int master_index);
void ecrt_release_master(ec_master_t *master);

unsigned int ecrt_version_magic(void);

/******************************************************************************
 * Master methods
 *****************************************************************************/

void ecrt_master_callbacks(ec_master_t *master, int (*request_cb)(void *),
                           void (*release_cb)(void *), void *cb_data);

ec_domain_t *ecrt_master_create_domain(ec_master_t *master);

ec_slave_t *ecrt_master_get_slave(const ec_master_t *master,
        const char *address, uint32_t vendor_id, uint32_t product_code);
ec_slave_t *ecrt_master_get_slave_by_pos(const ec_master_t *master,
        uint16_t position, uint32_t vendor_id, uint32_t product_code);

int ecrt_master_activate(ec_master_t *master);

void ecrt_master_send(ec_master_t *master);
void ecrt_master_receive(ec_master_t *master);

void ecrt_master_get_status(const ec_master_t *master, ec_master_status_t *s);

/******************************************************************************
 * Domain methods
 *****************************************************************************/

int ecrt_domain_register_pdo(ec_domain_t *domain, ec_slave_t *slave,
        uint16_t pdo_entry_index, uint8_t pdo_entry_subindex, void **data_ptr);

int ecrt_domain_register_pdo_range(ec_domain_t *domain, ec_slave_t *slave,
        ec_direction_t direction, uint16_t offset, uint16_t length,
        void **data_ptr);

int ecrt_domain_register_pdo_list(ec_domain_t *domain,
        const ec_pdo_reg_t *pdos);

void ecrt_domain_process(ec_domain_t *domain);
void ecrt_domain_queue(ec_domain_t *domain);

int ecrt_domain_state(const ec_domain_t *domain);

/******************************************************************************
 * Slave methods
 *****************************************************************************/

int ecrt_slave_conf_sdo8(ec_slave_t *slave, uint16_t sdo_index,
                         uint8_t sdo_subindex, uint8_t value);
int ecrt_slave_conf_sdo16(ec_slave_t *slave, uint16_t sdo_index,
                          uint8_t sdo_subindex, uint16_t value);
int ecrt_slave_conf_sdo32(ec_slave_t *slave, uint16_t sdo_index,
                          uint8_t sdo_subindex, uint32_t value);

void ecrt_slave_pdo_mapping_clear(ec_slave_t *, ec_direction_t);
int ecrt_slave_pdo_mapping_add(ec_slave_t *, ec_direction_t, uint16_t);
int ecrt_slave_pdo_mapping(ec_slave_t *, ec_direction_t, unsigned int, ...);

/******************************************************************************
 * Bitwise read/write macros
 *****************************************************************************/

/** Read a certain bit of an EtherCAT data byte.
 * \param DATA EtherCAT data pointer
 * \param POS bit position
 */
#define EC_READ_BIT(DATA, POS) ((*((uint8_t *) (DATA)) >> (POS)) & 0x01)

/** Write a certain bit of an EtherCAT data byte.
 * \param DATA EtherCAT data pointer
 * \param POS bit position
 * \param VAL new bit value
 */
#define EC_WRITE_BIT(DATA, POS, VAL) \
    do { \
        if (VAL) *((uint8_t *) (DATA)) |=  (1 << (POS)); \
        else     *((uint8_t *) (DATA)) &= ~(1 << (POS)); \
    } while (0)

/******************************************************************************
 * Read macros
 *****************************************************************************/

/** Read an 8-bit unsigned value from EtherCAT data.
 * \return EtherCAT data value
 */
#define EC_READ_U8(DATA) \
    ((uint8_t) *((uint8_t *) (DATA)))

/** Read an 8-bit signed value from EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \return EtherCAT data value
 */
#define EC_READ_S8(DATA) \
     ((int8_t) *((uint8_t *) (DATA)))

/** Read a 16-bit unsigned value from EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \return EtherCAT data value
 */
#define EC_READ_U16(DATA) \
     ((uint16_t) le16_to_cpup((void *) (DATA)))

/** Read a 16-bit signed value from EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \return EtherCAT data value
 */
#define EC_READ_S16(DATA) \
     ((int16_t) le16_to_cpup((void *) (DATA)))

/** Read a 32-bit unsigned value from EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \return EtherCAT data value
 */
#define EC_READ_U32(DATA) \
     ((uint32_t) le32_to_cpup((void *) (DATA)))

/** Read a 32-bit signed value from EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \return EtherCAT data value
 */
#define EC_READ_S32(DATA) \
     ((int32_t) le32_to_cpup((void *) (DATA)))

/******************************************************************************
 * Write macros
 *****************************************************************************/

/** Write an 8-bit unsigned value to EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \param VAL new value
 */
#define EC_WRITE_U8(DATA, VAL) \
    do { \
        *((uint8_t *)(DATA)) = ((uint8_t) (VAL)); \
    } while (0)

/** Write an 8-bit signed value to EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \param VAL new value
 */
#define EC_WRITE_S8(DATA, VAL) EC_WRITE_U8(DATA, VAL)

/** Write a 16-bit unsigned value to EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \param VAL new value
 */
#define EC_WRITE_U16(DATA, VAL) \
    do { \
        *((uint16_t *) (DATA)) = (uint16_t) (VAL); \
        cpu_to_le16s(DATA); \
    } while (0)

/** Write a 16-bit signed value to EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \param VAL new value
 */
#define EC_WRITE_S16(DATA, VAL) EC_WRITE_U16(DATA, VAL)

/** Write a 32-bit unsigned value to EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \param VAL new value
 */
#define EC_WRITE_U32(DATA, VAL) \
    do { \
        *((uint32_t *) (DATA)) = (uint32_t) (VAL); \
        cpu_to_le32s(DATA); \
    } while (0)

/** Write a 32-bit signed value to EtherCAT data.
 * \param DATA EtherCAT data pointer
 * \param VAL new value
 */
#define EC_WRITE_S32(DATA, VAL) EC_WRITE_U32(DATA, VAL)

/*****************************************************************************/

#endif
