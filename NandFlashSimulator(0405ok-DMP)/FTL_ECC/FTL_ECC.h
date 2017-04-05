/*********************************************************
*     File Name : FTL_ECC.h
*     YIN  @ 2017.04.01
*     Description  : Initial create
**********************************************************/
#ifndef _FTL_ECC_H_
#define _FTL_ECC_H_

/**
 * nand_calculate_ecc - [NAND Interface] Calculate 3-byte ECC for 256/512-byte
 *			 block
 * @buf:	input buffer with raw data
 * @code:	output buffer with ECC
 */
void nand_calculate_ecc(const unsigned char *buf,
                        unsigned char *ecc);


/**
 * nand_correct_data - [NAND Interface] Detect and correct bit error(s)
 * @buf:	raw data read from the chip
 * @read_ecc:	ECC from the chip
 * @calc_ecc:	the ECC calculated from raw data
 *
 * Detect and correct a 1 bit error for 256/512 byte block
 */
int nand_correct_data(unsigned char *buf,
                      unsigned char *read_ecc, unsigned char *test_ecc);




#endif

