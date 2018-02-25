/*This program gives the 64-bit optimized bitslice implementation of JH using ANSI C

   --------------------------------
   Performance

   Microprocessor: Intel CORE 2 processor (Core 2 Duo Mobile T6600 2.2GHz)
   Operating System: 64-bit Ubuntu 10.04 (Linux kernel 2.6.32-22-generic)
   Speed for long message:
   1) 45.8 cycles/byte   compiler: Intel C++ Compiler 11.1   compilation option: icc -O2
   2) 56.8 cycles/byte   compiler: gcc 4.4.3                 compilation option: gcc -O3

   --------------------------------
   Last Modified: January 16, 2011
*/

#include <crypto/c_jh.h>

#include <stdint.h>
#include <string.h>

/*typedef unsigned long long uint64;*/
typedef uint64_t uint64;

/*define data alignment for different C compilers*/
#if defined(__GNUC__)
      #define DATA_ALIGN16(x) x __attribute__ ((aligned(16)))
#else
      #define DATA_ALIGN16(x) __declspec(align(16)) x
#endif


typedef struct {
	int hashbitlen;	   	              /*the message digest size*/
	unsigned long long databitlen;    /*the message size in bits*/
	unsigned long long datasize_in_buffer;      /*the size of the message remained in buffer; assumed to be multiple of 8bits except for the last partial block at the end of the message*/
	DATA_ALIGN16(uint64 x[8][2]);     /*the 1024-bit state, ( x[i][0] || x[i][1] ) is the ith row of the state in the pseudocode*/
	unsigned char buffer[64];         /*the 512-bit message block to be hashed;*/
} hashState;


/*The initial hash value H(0)*/
const unsigned char JH224_H0[128]={0x2d,0xfe,0xdd,0x62,0xf9,0x9a,0x98,0xac,0xae,0x7c,0xac,0xd6,0x19,0xd6,0x34,0xe7,0xa4,0x83,0x10,0x5,0xbc,0x30,0x12,0x16,0xb8,0x60,0x38,0xc6,0xc9,0x66,0x14,0x94,0x66,0xd9,0x89,0x9f,0x25,0x80,0x70,0x6f,0xce,0x9e,0xa3,0x1b,0x1d,0x9b,0x1a,0xdc,0x11,0xe8,0x32,0x5f,0x7b,0x36,0x6e,0x10,0xf9,0x94,0x85,0x7f,0x2,0xfa,0x6,0xc1,0x1b,0x4f,0x1b,0x5c,0xd8,0xc8,0x40,0xb3,0x97,0xf6,0xa1,0x7f,0x6e,0x73,0x80,0x99,0xdc,0xdf,0x93,0xa5,0xad,0xea,0xa3,0xd3,0xa4,0x31,0xe8,0xde,0xc9,0x53,0x9a,0x68,0x22,0xb4,0xa9,0x8a,0xec,0x86,0xa1,0xe4,0xd5,0x74,0xac,0x95,0x9c,0xe5,0x6c,0xf0,0x15,0x96,0xd,0xea,0xb5,0xab,0x2b,0xbf,0x96,0x11,0xdc,0xf0,0xdd,0x64,0xea,0x6e};
const unsigned char JH256_H0[128]={0xeb,0x98,0xa3,0x41,0x2c,0x20,0xd3,0xeb,0x92,0xcd,0xbe,0x7b,0x9c,0xb2,0x45,0xc1,0x1c,0x93,0x51,0x91,0x60,0xd4,0xc7,0xfa,0x26,0x0,0x82,0xd6,0x7e,0x50,0x8a,0x3,0xa4,0x23,0x9e,0x26,0x77,0x26,0xb9,0x45,0xe0,0xfb,0x1a,0x48,0xd4,0x1a,0x94,0x77,0xcd,0xb5,0xab,0x26,0x2,0x6b,0x17,0x7a,0x56,0xf0,0x24,0x42,0xf,0xff,0x2f,0xa8,0x71,0xa3,0x96,0x89,0x7f,0x2e,0x4d,0x75,0x1d,0x14,0x49,0x8,0xf7,0x7d,0xe2,0x62,0x27,0x76,0x95,0xf7,0x76,0x24,0x8f,0x94,0x87,0xd5,0xb6,0x57,0x47,0x80,0x29,0x6c,0x5c,0x5e,0x27,0x2d,0xac,0x8e,0xd,0x6c,0x51,0x84,0x50,0xc6,0x57,0x5,0x7a,0xf,0x7b,0xe4,0xd3,0x67,0x70,0x24,0x12,0xea,0x89,0xe3,0xab,0x13,0xd3,0x1c,0xd7,0x69};
const unsigned char JH384_H0[128]={0x48,0x1e,0x3b,0xc6,0xd8,0x13,0x39,0x8a,0x6d,0x3b,0x5e,0x89,0x4a,0xde,0x87,0x9b,0x63,0xfa,0xea,0x68,0xd4,0x80,0xad,0x2e,0x33,0x2c,0xcb,0x21,0x48,0xf,0x82,0x67,0x98,0xae,0xc8,0x4d,0x90,0x82,0xb9,0x28,0xd4,0x55,0xea,0x30,0x41,0x11,0x42,0x49,0x36,0xf5,0x55,0xb2,0x92,0x48,0x47,0xec,0xc7,0x25,0xa,0x93,0xba,0xf4,0x3c,0xe1,0x56,0x9b,0x7f,0x8a,0x27,0xdb,0x45,0x4c,0x9e,0xfc,0xbd,0x49,0x63,0x97,0xaf,0xe,0x58,0x9f,0xc2,0x7d,0x26,0xaa,0x80,0xcd,0x80,0xc0,0x8b,0x8c,0x9d,0xeb,0x2e,0xda,0x8a,0x79,0x81,0xe8,0xf8,0xd5,0x37,0x3a,0xf4,0x39,0x67,0xad,0xdd,0xd1,0x7a,0x71,0xa9,0xb4,0xd3,0xbd,0xa4,0x75,0xd3,0x94,0x97,0x6c,0x3f,0xba,0x98,0x42,0x73,0x7f};
const unsigned char JH512_H0[128]={0x6f,0xd1,0x4b,0x96,0x3e,0x0,0xaa,0x17,0x63,0x6a,0x2e,0x5,0x7a,0x15,0xd5,0x43,0x8a,0x22,0x5e,0x8d,0xc,0x97,0xef,0xb,0xe9,0x34,0x12,0x59,0xf2,0xb3,0xc3,0x61,0x89,0x1d,0xa0,0xc1,0x53,0x6f,0x80,0x1e,0x2a,0xa9,0x5,0x6b,0xea,0x2b,0x6d,0x80,0x58,0x8e,0xcc,0xdb,0x20,0x75,0xba,0xa6,0xa9,0xf,0x3a,0x76,0xba,0xf8,0x3b,0xf7,0x1,0x69,0xe6,0x5,0x41,0xe3,0x4a,0x69,0x46,0xb5,0x8a,0x8e,0x2e,0x6f,0xe6,0x5a,0x10,0x47,0xa7,0xd0,0xc1,0x84,0x3c,0x24,0x3b,0x6e,0x71,0xb1,0x2d,0x5a,0xc1,0x99,0xcf,0x57,0xf6,0xec,0x9d,0xb1,0xf8,0x56,0xa7,0x6,0x88,0x7c,0x57,0x16,0xb1,0x56,0xe3,0xc2,0xfc,0xdf,0xe6,0x85,0x17,0xfb,0x54,0x5a,0x46,0x78,0xcc,0x8c,0xdd,0x4b};

/*42 round constants, each round constant is 32-byte (256-bit)*/
const unsigned char E8_bitslice_roundconstant[42][32]={
{0x72,0xd5,0xde,0xa2,0xdf,0x15,0xf8,0x67,0x7b,0x84,0x15,0xa,0xb7,0x23,0x15,0x57,0x81,0xab,0xd6,0x90,0x4d,0x5a,0x87,0xf6,0x4e,0x9f,0x4f,0xc5,0xc3,0xd1,0x2b,0x40},
{0xea,0x98,0x3a,0xe0,0x5c,0x45,0xfa,0x9c,0x3,0xc5,0xd2,0x99,0x66,0xb2,0x99,0x9a,0x66,0x2,0x96,0xb4,0xf2,0xbb,0x53,0x8a,0xb5,0x56,0x14,0x1a,0x88,0xdb,0xa2,0x31},
{0x3,0xa3,0x5a,0x5c,0x9a,0x19,0xe,0xdb,0x40,0x3f,0xb2,0xa,0x87,0xc1,0x44,0x10,0x1c,0x5,0x19,0x80,0x84,0x9e,0x95,0x1d,0x6f,0x33,0xeb,0xad,0x5e,0xe7,0xcd,0xdc},
{0x10,0xba,0x13,0x92,0x2,0xbf,0x6b,0x41,0xdc,0x78,0x65,0x15,0xf7,0xbb,0x27,0xd0,0xa,0x2c,0x81,0x39,0x37,0xaa,0x78,0x50,0x3f,0x1a,0xbf,0xd2,0x41,0x0,0x91,0xd3},
{0x42,0x2d,0x5a,0xd,0xf6,0xcc,0x7e,0x90,0xdd,0x62,0x9f,0x9c,0x92,0xc0,0x97,0xce,0x18,0x5c,0xa7,0xb,0xc7,0x2b,0x44,0xac,0xd1,0xdf,0x65,0xd6,0x63,0xc6,0xfc,0x23},
{0x97,0x6e,0x6c,0x3,0x9e,0xe0,0xb8,0x1a,0x21,0x5,0x45,0x7e,0x44,0x6c,0xec,0xa8,0xee,0xf1,0x3,0xbb,0x5d,0x8e,0x61,0xfa,0xfd,0x96,0x97,0xb2,0x94,0x83,0x81,0x97},
{0x4a,0x8e,0x85,0x37,0xdb,0x3,0x30,0x2f,0x2a,0x67,0x8d,0x2d,0xfb,0x9f,0x6a,0x95,0x8a,0xfe,0x73,0x81,0xf8,0xb8,0x69,0x6c,0x8a,0xc7,0x72,0x46,0xc0,0x7f,0x42,0x14},
{0xc5,0xf4,0x15,0x8f,0xbd,0xc7,0x5e,0xc4,0x75,0x44,0x6f,0xa7,0x8f,0x11,0xbb,0x80,0x52,0xde,0x75,0xb7,0xae,0xe4,0x88,0xbc,0x82,0xb8,0x0,0x1e,0x98,0xa6,0xa3,0xf4},
{0x8e,0xf4,0x8f,0x33,0xa9,0xa3,0x63,0x15,0xaa,0x5f,0x56,0x24,0xd5,0xb7,0xf9,0x89,0xb6,0xf1,0xed,0x20,0x7c,0x5a,0xe0,0xfd,0x36,0xca,0xe9,0x5a,0x6,0x42,0x2c,0x36},
{0xce,0x29,0x35,0x43,0x4e,0xfe,0x98,0x3d,0x53,0x3a,0xf9,0x74,0x73,0x9a,0x4b,0xa7,0xd0,0xf5,0x1f,0x59,0x6f,0x4e,0x81,0x86,0xe,0x9d,0xad,0x81,0xaf,0xd8,0x5a,0x9f},
{0xa7,0x5,0x6,0x67,0xee,0x34,0x62,0x6a,0x8b,0xb,0x28,0xbe,0x6e,0xb9,0x17,0x27,0x47,0x74,0x7,0x26,0xc6,0x80,0x10,0x3f,0xe0,0xa0,0x7e,0x6f,0xc6,0x7e,0x48,0x7b},
{0xd,0x55,0xa,0xa5,0x4a,0xf8,0xa4,0xc0,0x91,0xe3,0xe7,0x9f,0x97,0x8e,0xf1,0x9e,0x86,0x76,0x72,0x81,0x50,0x60,0x8d,0xd4,0x7e,0x9e,0x5a,0x41,0xf3,0xe5,0xb0,0x62},
{0xfc,0x9f,0x1f,0xec,0x40,0x54,0x20,0x7a,0xe3,0xe4,0x1a,0x0,0xce,0xf4,0xc9,0x84,0x4f,0xd7,0x94,0xf5,0x9d,0xfa,0x95,0xd8,0x55,0x2e,0x7e,0x11,0x24,0xc3,0x54,0xa5},
{0x5b,0xdf,0x72,0x28,0xbd,0xfe,0x6e,0x28,0x78,0xf5,0x7f,0xe2,0xf,0xa5,0xc4,0xb2,0x5,0x89,0x7c,0xef,0xee,0x49,0xd3,0x2e,0x44,0x7e,0x93,0x85,0xeb,0x28,0x59,0x7f},
{0x70,0x5f,0x69,0x37,0xb3,0x24,0x31,0x4a,0x5e,0x86,0x28,0xf1,0x1d,0xd6,0xe4,0x65,0xc7,0x1b,0x77,0x4,0x51,0xb9,0x20,0xe7,0x74,0xfe,0x43,0xe8,0x23,0xd4,0x87,0x8a},
{0x7d,0x29,0xe8,0xa3,0x92,0x76,0x94,0xf2,0xdd,0xcb,0x7a,0x9,0x9b,0x30,0xd9,0xc1,0x1d,0x1b,0x30,0xfb,0x5b,0xdc,0x1b,0xe0,0xda,0x24,0x49,0x4f,0xf2,0x9c,0x82,0xbf},
{0xa4,0xe7,0xba,0x31,0xb4,0x70,0xbf,0xff,0xd,0x32,0x44,0x5,0xde,0xf8,0xbc,0x48,0x3b,0xae,0xfc,0x32,0x53,0xbb,0xd3,0x39,0x45,0x9f,0xc3,0xc1,0xe0,0x29,0x8b,0xa0},
{0xe5,0xc9,0x5,0xfd,0xf7,0xae,0x9,0xf,0x94,0x70,0x34,0x12,0x42,0x90,0xf1,0x34,0xa2,0x71,0xb7,0x1,0xe3,0x44,0xed,0x95,0xe9,0x3b,0x8e,0x36,0x4f,0x2f,0x98,0x4a},
{0x88,0x40,0x1d,0x63,0xa0,0x6c,0xf6,0x15,0x47,0xc1,0x44,0x4b,0x87,0x52,0xaf,0xff,0x7e,0xbb,0x4a,0xf1,0xe2,0xa,0xc6,0x30,0x46,0x70,0xb6,0xc5,0xcc,0x6e,0x8c,0xe6},
{0xa4,0xd5,0xa4,0x56,0xbd,0x4f,0xca,0x0,0xda,0x9d,0x84,0x4b,0xc8,0x3e,0x18,0xae,0x73,0x57,0xce,0x45,0x30,0x64,0xd1,0xad,0xe8,0xa6,0xce,0x68,0x14,0x5c,0x25,0x67},
{0xa3,0xda,0x8c,0xf2,0xcb,0xe,0xe1,0x16,0x33,0xe9,0x6,0x58,0x9a,0x94,0x99,0x9a,0x1f,0x60,0xb2,0x20,0xc2,0x6f,0x84,0x7b,0xd1,0xce,0xac,0x7f,0xa0,0xd1,0x85,0x18},
{0x32,0x59,0x5b,0xa1,0x8d,0xdd,0x19,0xd3,0x50,0x9a,0x1c,0xc0,0xaa,0xa5,0xb4,0x46,0x9f,0x3d,0x63,0x67,0xe4,0x4,0x6b,0xba,0xf6,0xca,0x19,0xab,0xb,0x56,0xee,0x7e},
{0x1f,0xb1,0x79,0xea,0xa9,0x28,0x21,0x74,0xe9,0xbd,0xf7,0x35,0x3b,0x36,0x51,0xee,0x1d,0x57,0xac,0x5a,0x75,0x50,0xd3,0x76,0x3a,0x46,0xc2,0xfe,0xa3,0x7d,0x70,0x1},
{0xf7,0x35,0xc1,0xaf,0x98,0xa4,0xd8,0x42,0x78,0xed,0xec,0x20,0x9e,0x6b,0x67,0x79,0x41,0x83,0x63,0x15,0xea,0x3a,0xdb,0xa8,0xfa,0xc3,0x3b,0x4d,0x32,0x83,0x2c,0x83},
{0xa7,0x40,0x3b,0x1f,0x1c,0x27,0x47,0xf3,0x59,0x40,0xf0,0x34,0xb7,0x2d,0x76,0x9a,0xe7,0x3e,0x4e,0x6c,0xd2,0x21,0x4f,0xfd,0xb8,0xfd,0x8d,0x39,0xdc,0x57,0x59,0xef},
{0x8d,0x9b,0xc,0x49,0x2b,0x49,0xeb,0xda,0x5b,0xa2,0xd7,0x49,0x68,0xf3,0x70,0xd,0x7d,0x3b,0xae,0xd0,0x7a,0x8d,0x55,0x84,0xf5,0xa5,0xe9,0xf0,0xe4,0xf8,0x8e,0x65},
{0xa0,0xb8,0xa2,0xf4,0x36,0x10,0x3b,0x53,0xc,0xa8,0x7,0x9e,0x75,0x3e,0xec,0x5a,0x91,0x68,0x94,0x92,0x56,0xe8,0x88,0x4f,0x5b,0xb0,0x5c,0x55,0xf8,0xba,0xbc,0x4c},
{0xe3,0xbb,0x3b,0x99,0xf3,0x87,0x94,0x7b,0x75,0xda,0xf4,0xd6,0x72,0x6b,0x1c,0x5d,0x64,0xae,0xac,0x28,0xdc,0x34,0xb3,0x6d,0x6c,0x34,0xa5,0x50,0xb8,0x28,0xdb,0x71},
{0xf8,0x61,0xe2,0xf2,0x10,0x8d,0x51,0x2a,0xe3,0xdb,0x64,0x33,0x59,0xdd,0x75,0xfc,0x1c,0xac,0xbc,0xf1,0x43,0xce,0x3f,0xa2,0x67,0xbb,0xd1,0x3c,0x2,0xe8,0x43,0xb0},
{0x33,0xa,0x5b,0xca,0x88,0x29,0xa1,0x75,0x7f,0x34,0x19,0x4d,0xb4,0x16,0x53,0x5c,0x92,0x3b,0x94,0xc3,0xe,0x79,0x4d,0x1e,0x79,0x74,0x75,0xd7,0xb6,0xee,0xaf,0x3f},
{0xea,0xa8,0xd4,0xf7,0xbe,0x1a,0x39,0x21,0x5c,0xf4,0x7e,0x9,0x4c,0x23,0x27,0x51,0x26,0xa3,0x24,0x53,0xba,0x32,0x3c,0xd2,0x44,0xa3,0x17,0x4a,0x6d,0xa6,0xd5,0xad},
{0xb5,0x1d,0x3e,0xa6,0xaf,0xf2,0xc9,0x8,0x83,0x59,0x3d,0x98,0x91,0x6b,0x3c,0x56,0x4c,0xf8,0x7c,0xa1,0x72,0x86,0x60,0x4d,0x46,0xe2,0x3e,0xcc,0x8,0x6e,0xc7,0xf6},
{0x2f,0x98,0x33,0xb3,0xb1,0xbc,0x76,0x5e,0x2b,0xd6,0x66,0xa5,0xef,0xc4,0xe6,0x2a,0x6,0xf4,0xb6,0xe8,0xbe,0xc1,0xd4,0x36,0x74,0xee,0x82,0x15,0xbc,0xef,0x21,0x63},
{0xfd,0xc1,0x4e,0xd,0xf4,0x53,0xc9,0x69,0xa7,0x7d,0x5a,0xc4,0x6,0x58,0x58,0x26,0x7e,0xc1,0x14,0x16,0x6,0xe0,0xfa,0x16,0x7e,0x90,0xaf,0x3d,0x28,0x63,0x9d,0x3f},
{0xd2,0xc9,0xf2,0xe3,0x0,0x9b,0xd2,0xc,0x5f,0xaa,0xce,0x30,0xb7,0xd4,0xc,0x30,0x74,0x2a,0x51,0x16,0xf2,0xe0,0x32,0x98,0xd,0xeb,0x30,0xd8,0xe3,0xce,0xf8,0x9a},
{0x4b,0xc5,0x9e,0x7b,0xb5,0xf1,0x79,0x92,0xff,0x51,0xe6,0x6e,0x4,0x86,0x68,0xd3,0x9b,0x23,0x4d,0x57,0xe6,0x96,0x67,0x31,0xcc,0xe6,0xa6,0xf3,0x17,0xa,0x75,0x5},
{0xb1,0x76,0x81,0xd9,0x13,0x32,0x6c,0xce,0x3c,0x17,0x52,0x84,0xf8,0x5,0xa2,0x62,0xf4,0x2b,0xcb,0xb3,0x78,0x47,0x15,0x47,0xff,0x46,0x54,0x82,0x23,0x93,0x6a,0x48},
{0x38,0xdf,0x58,0x7,0x4e,0x5e,0x65,0x65,0xf2,0xfc,0x7c,0x89,0xfc,0x86,0x50,0x8e,0x31,0x70,0x2e,0x44,0xd0,0xb,0xca,0x86,0xf0,0x40,0x9,0xa2,0x30,0x78,0x47,0x4e},
{0x65,0xa0,0xee,0x39,0xd1,0xf7,0x38,0x83,0xf7,0x5e,0xe9,0x37,0xe4,0x2c,0x3a,0xbd,0x21,0x97,0xb2,0x26,0x1,0x13,0xf8,0x6f,0xa3,0x44,0xed,0xd1,0xef,0x9f,0xde,0xe7},
{0x8b,0xa0,0xdf,0x15,0x76,0x25,0x92,0xd9,0x3c,0x85,0xf7,0xf6,0x12,0xdc,0x42,0xbe,0xd8,0xa7,0xec,0x7c,0xab,0x27,0xb0,0x7e,0x53,0x8d,0x7d,0xda,0xaa,0x3e,0xa8,0xde},
{0xaa,0x25,0xce,0x93,0xbd,0x2,0x69,0xd8,0x5a,0xf6,0x43,0xfd,0x1a,0x73,0x8,0xf9,0xc0,0x5f,0xef,0xda,0x17,0x4a,0x19,0xa5,0x97,0x4d,0x66,0x33,0x4c,0xfd,0x21,0x6a},
{0x35,0xb4,0x98,0x31,0xdb,0x41,0x15,0x70,0xea,0x1e,0xf,0xbb,0xed,0xcd,0x54,0x9b,0x9a,0xd0,0x63,0xa1,0x51,0x97,0x40,0x72,0xf6,0x75,0x9d,0xbf,0x91,0x47,0x6f,0xe2}};


static void E8(hashState *state);  /*The bijective function E8, in bitslice form*/
static void F8(hashState *state);  /*The compression function F8 */

/*The API functions*/
static HashReturn Init(hashState *state, int hashbitlen);
static HashReturn Update(hashState *state, const BitSequence *data, DataLength databitlen);
static HashReturn Final(hashState *state, BitSequence *hashval);
HashReturn jh_hash(int hashbitlen, const BitSequence *data,DataLength databitlen, BitSequence *hashval);

/*swapping bit 2i with bit 2i+1 of 64-bit x*/
#define SWAP1(x)   (x) = ((((x) & 0x5555555555555555ULL) << 1) | (((x) & 0xaaaaaaaaaaaaaaaaULL) >> 1));
/*swapping bits 4i||4i+1 with bits 4i+2||4i+3 of 64-bit x*/
#define SWAP2(x)   (x) = ((((x) & 0x3333333333333333ULL) << 2) | (((x) & 0xccccccccccccccccULL) >> 2));
/*swapping bits 8i||8i+1||8i+2||8i+3 with bits 8i+4||8i+5||8i+6||8i+7 of 64-bit x*/
#define SWAP4(x)   (x) = ((((x) & 0x0f0f0f0f0f0f0f0fULL) << 4) | (((x) & 0xf0f0f0f0f0f0f0f0ULL) >> 4));
/*swapping bits 16i||16i+1||......||16i+7  with bits 16i+8||16i+9||......||16i+15 of 64-bit x*/
#define SWAP8(x)   (x) = ((((x) & 0x00ff00ff00ff00ffULL) << 8) | (((x) & 0xff00ff00ff00ff00ULL) >> 8));
/*swapping bits 32i||32i+1||......||32i+15 with bits 32i+16||32i+17||......||32i+31 of 64-bit x*/
#define SWAP16(x)  (x) = ((((x) & 0x0000ffff0000ffffULL) << 16) | (((x) & 0xffff0000ffff0000ULL) >> 16));
/*swapping bits 64i||64i+1||......||64i+31 with bits 64i+32||64i+33||......||64i+63 of 64-bit x*/
#define SWAP32(x)  (x) = (((x) << 32) | ((x) >> 32));

/*The MDS transform*/
#define L(m0,m1,m2,m3,m4,m5,m6,m7) \
      (m4) ^= (m1);                \
      (m5) ^= (m2);                \
      (m6) ^= (m0) ^ (m3);         \
      (m7) ^= (m0);                \
      (m0) ^= (m5);                \
      (m1) ^= (m6);                \
      (m2) ^= (m4) ^ (m7);         \
      (m3) ^= (m4);

/*Two Sboxes are computed in parallel, each Sbox implements S0 and S1, selected by a constant bit*/
/*The reason to compute two Sboxes in parallel is to try to fully utilize the parallel processing power*/
#define SS(m0,m1,m2,m3,m4,m5,m6,m7,cc0,cc1)   \
      m3  = ~(m3);                  \
      m7  = ~(m7);                  \
      m0 ^= ((~(m2)) & (cc0));      \
      m4 ^= ((~(m6)) & (cc1));      \
      temp0 = (cc0) ^ ((m0) & (m1));\
      temp1 = (cc1) ^ ((m4) & (m5));\
      m0 ^= ((m2) & (m3));          \
      m4 ^= ((m6) & (m7));          \
      m3 ^= ((~(m1)) & (m2));       \
      m7 ^= ((~(m5)) & (m6));       \
      m1 ^= ((m0) & (m2));          \
      m5 ^= ((m4) & (m6));          \
      m2 ^= ((m0) & (~(m3)));       \
      m6 ^= ((m4) & (~(m7)));       \
      m0 ^= ((m1) | (m3));          \
      m4 ^= ((m5) | (m7));          \
      m3 ^= ((m1) & (m2));          \
      m7 ^= ((m5) & (m6));          \
      m1 ^= (temp0 & (m0));         \
      m5 ^= (temp1 & (m4));         \
      m2 ^= temp0;                  \
      m6 ^= temp1;

/*The bijective function E8, in bitslice form*/
static void E8(hashState *state)
{
      uint64 i,roundnumber,temp0,temp1;

      for (roundnumber = 0; roundnumber < 42; roundnumber = roundnumber+7) {
            /*round 7*roundnumber+0: Sbox, MDS and Swapping layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+0])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+0])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
                  SWAP1(state->x[1][i]); SWAP1(state->x[3][i]); SWAP1(state->x[5][i]); SWAP1(state->x[7][i]);
            }

            /*round 7*roundnumber+1: Sbox, MDS and Swapping layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+1])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+1])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
                  SWAP2(state->x[1][i]); SWAP2(state->x[3][i]); SWAP2(state->x[5][i]); SWAP2(state->x[7][i]);
            }

            /*round 7*roundnumber+2: Sbox, MDS and Swapping layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+2])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+2])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
                  SWAP4(state->x[1][i]); SWAP4(state->x[3][i]); SWAP4(state->x[5][i]); SWAP4(state->x[7][i]);
            }

            /*round 7*roundnumber+3: Sbox, MDS and Swapping layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+3])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+3])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
                  SWAP8(state->x[1][i]); SWAP8(state->x[3][i]); SWAP8(state->x[5][i]); SWAP8(state->x[7][i]);
            }

            /*round 7*roundnumber+4: Sbox, MDS and Swapping layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+4])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+4])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
                  SWAP16(state->x[1][i]); SWAP16(state->x[3][i]); SWAP16(state->x[5][i]); SWAP16(state->x[7][i]);
            }

            /*round 7*roundnumber+5: Sbox, MDS and Swapping layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+5])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+5])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
                  SWAP32(state->x[1][i]); SWAP32(state->x[3][i]); SWAP32(state->x[5][i]); SWAP32(state->x[7][i]);
            }

            /*round 7*roundnumber+6: Sbox and MDS layers*/
            for (i = 0; i < 2; i++) {
                  SS(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i],((uint64*)E8_bitslice_roundconstant[roundnumber+6])[i],((uint64*)E8_bitslice_roundconstant[roundnumber+6])[i+2] );
                  L(state->x[0][i],state->x[2][i],state->x[4][i],state->x[6][i],state->x[1][i],state->x[3][i],state->x[5][i],state->x[7][i]);
            }
            /*round 7*roundnumber+6: swapping layer*/
            for (i = 1; i < 8; i = i+2) {
                  temp0 = state->x[i][0]; state->x[i][0] = state->x[i][1]; state->x[i][1] = temp0;
            }
      }

}

/*The compression function F8 */
static void F8(hashState *state)
{
      uint64  i;

      /*xor the 512-bit message with the fist half of the 1024-bit hash state*/
      for (i = 0; i < 8; i++)  state->x[i >> 1][i & 1] ^= ((uint64*)state->buffer)[i];

      /*the bijective function E8 */
      E8(state);

      /*xor the 512-bit message with the second half of the 1024-bit hash state*/
      for (i = 0; i < 8; i++)  state->x[(8+i) >> 1][(8+i) & 1] ^= ((uint64*)state->buffer)[i];
}

/*before hashing a message, initialize the hash state as H0 */
static HashReturn Init(hashState *state, int hashbitlen)
{
	  state->databitlen = 0;
	  state->datasize_in_buffer = 0;

      /*initialize the initial hash value of JH*/
      state->hashbitlen = hashbitlen;

      /*load the intital hash value into state*/
      switch (hashbitlen)
      {
            case 224: memcpy(state->x,JH224_H0,128); break;
            case 256: memcpy(state->x,JH256_H0,128); break;
            case 384: memcpy(state->x,JH384_H0,128); break;
            case 512: memcpy(state->x,JH512_H0,128); break;
      }

      return(SUCCESS);
}


/*hash each 512-bit message block, except the last partial block*/
static HashReturn Update(hashState *state, const BitSequence *data, DataLength databitlen)
{
      DataLength index; /*the starting address of the data to be compressed*/

      state->databitlen += databitlen;
      index = 0;

      /*if there is remaining data in the buffer, fill it to a full message block first*/
      /*we assume that the size of the data in the buffer is the multiple of 8 bits if it is not at the end of a message*/

      /*There is data in the buffer, but the incoming data is insufficient for a full block*/
      if ( (state->datasize_in_buffer > 0 ) && (( state->datasize_in_buffer + databitlen) < 512)  ) {
            if ( (databitlen & 7) == 0 ) {
                 memcpy(state->buffer + (state->datasize_in_buffer >> 3), data, 64-(state->datasize_in_buffer >> 3)) ;
		    }
            else memcpy(state->buffer + (state->datasize_in_buffer >> 3), data, 64-(state->datasize_in_buffer >> 3)+1) ;
            state->datasize_in_buffer += databitlen;
            databitlen = 0;
      }

      /*There is data in the buffer, and the incoming data is sufficient for a full block*/
      if ( (state->datasize_in_buffer > 0 ) && (( state->datasize_in_buffer + databitlen) >= 512)  ) {
	        memcpy( state->buffer + (state->datasize_in_buffer >> 3), data, 64-(state->datasize_in_buffer >> 3) ) ;
	        index = 64-(state->datasize_in_buffer >> 3);
	        databitlen = databitlen - (512 - state->datasize_in_buffer);
	        F8(state);
	        state->datasize_in_buffer = 0;
      }

      /*hash the remaining full message blocks*/
      for ( ; databitlen >= 512; index = index+64, databitlen = databitlen - 512) {
            memcpy(state->buffer, data+index, 64);
            F8(state);
      }

      /*store the partial block into buffer, assume that -- if part of the last byte is not part of the message, then that part consists of 0 bits*/
      if ( databitlen > 0) {
            if ((databitlen & 7) == 0)
                  memcpy(state->buffer, data+index, (databitlen & 0x1ff) >> 3);
            else
                  memcpy(state->buffer, data+index, ((databitlen & 0x1ff) >> 3)+1);
            state->datasize_in_buffer = databitlen;
      }

      return(SUCCESS);
}

/*pad the message, process the padded block(s), truncate the hash value H to obtain the message digest*/
static HashReturn Final(hashState *state, BitSequence *hashval)
{
      unsigned int i;

      if ( (state->databitlen & 0x1ff) == 0 ) {
            /*pad the message when databitlen is multiple of 512 bits, then process the padded block*/
            memset(state->buffer, 0, 64);
            state->buffer[0]  = 0x80;
            state->buffer[63] = state->databitlen & 0xff;
            state->buffer[62] = (state->databitlen >> 8)  & 0xff;
            state->buffer[61] = (state->databitlen >> 16) & 0xff;
            state->buffer[60] = (state->databitlen >> 24) & 0xff;
            state->buffer[59] = (state->databitlen >> 32) & 0xff;
            state->buffer[58] = (state->databitlen >> 40) & 0xff;
            state->buffer[57] = (state->databitlen >> 48) & 0xff;
            state->buffer[56] = (state->databitlen >> 56) & 0xff;
            F8(state);
      }
      else {
		    /*set the rest of the bytes in the buffer to 0*/
            if ( (state->datasize_in_buffer & 7) == 0)
                  for (i = (state->databitlen & 0x1ff) >> 3; i < 64; i++)  state->buffer[i] = 0;
            else
                  for (i = ((state->databitlen & 0x1ff) >> 3)+1; i < 64; i++)  state->buffer[i] = 0;

            /*pad and process the partial block when databitlen is not multiple of 512 bits, then hash the padded blocks*/
            state->buffer[((state->databitlen & 0x1ff) >> 3)] |= 1 << (7- (state->databitlen & 7));

            F8(state);
            memset(state->buffer, 0, 64);
            state->buffer[63] = state->databitlen & 0xff;
            state->buffer[62] = (state->databitlen >> 8) & 0xff;
            state->buffer[61] = (state->databitlen >> 16) & 0xff;
            state->buffer[60] = (state->databitlen >> 24) & 0xff;
            state->buffer[59] = (state->databitlen >> 32) & 0xff;
            state->buffer[58] = (state->databitlen >> 40) & 0xff;
            state->buffer[57] = (state->databitlen >> 48) & 0xff;
            state->buffer[56] = (state->databitlen >> 56) & 0xff;
            F8(state);
      }

      /*truncating the final hash value to generate the message digest*/
      switch(state->hashbitlen) {
            case 224: memcpy(hashval,(unsigned char*)state->x+64+36,28);  break;
            case 256: memcpy(hashval,(unsigned char*)state->x+64+32,32);  break;
            case 384: memcpy(hashval,(unsigned char*)state->x+64+16,48);  break;
            case 512: memcpy(hashval,(unsigned char*)state->x+64,64);     break;
      }

      return(SUCCESS);
}

/* hash a message,
   three inputs: message digest size in bits (hashbitlen); message (data); message length in bits (databitlen)
   one output:   message digest (hashval)
*/
HashReturn jh_hash(int hashbitlen, const BitSequence *data,DataLength databitlen, BitSequence *hashval)
{
      hashState state;

      if ( hashbitlen == 224 || hashbitlen == 256 || hashbitlen == 384 || hashbitlen == 512 ) {
            Init(&state, hashbitlen);
            Update(&state, data, databitlen);
            Final(&state, hashval);
            return SUCCESS;
      }
      else
            return(BAD_HASHLEN);
}
