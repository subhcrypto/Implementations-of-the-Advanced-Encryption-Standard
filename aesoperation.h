#include <stdio.h>
#include <stdint.h>
#include "keyexpansion.h" // Include header for key expansion functions

// Function to multiply two bytes in GF(2^8)
uint16_t Mul(uint8_t x, uint8_t y)
{
    uint16_t result = 0x0000; // Initialize result to zero
    for (int i = 0; i < 8; i++) // Iterate over each bit
    {
        uint8_t c = (y >> i) & 0x01; // Extract the i-th bit of y
        uint16_t temp;
        temp = c * x << i; // Multiply x by the bit and shift left by i
        result = result ^ temp; // XOR the temporary result with the cumulative result
    }
    return result; // Return the multiplied value
}

// Function to perform reduction modulo the AES polynomial (x^8 + x^4 + x^3 + x + 1)
uint8_t Reduction(uint16_t x)
{
    uint8_t a = x & 0xff; // Lower 8 bits of x
    uint8_t b = x >> 8;   // Upper 8 bits of x
    int i = 0;
    while(i < 2) // Perform reduction twice to handle up to 16-bit input
    {
        uint16_t c = Mul(b, 0x1b); // Multiply the upper byte by the AES reduction polynomial
        a = a ^ (c & 0xff);        // XOR the lower byte with the result
        b = (c >> 8);               // Update the upper byte
        i = i + 1;                  // Increment loop counter
    }
    return a; // Return the reduced byte
}

// Function to multiply a byte by 0x02 in GF(2^8)
uint8_t MulByTwo(uint8_t a) // Multiplication by 0x02 in GF(2^8)
{
    uint16_t b = a;
    b = b << 1; // Shift left by 1 (equivalent to multiplying by x)
    uint8_t x, y;
    x = b & 0xff;        // Extract the lower 8 bits
    y = (b >> 8) & 0xff; // Extract the overflow bit
    y = (y * 0x1b) ^ x;  // If overflow, reduce by the AES polynomial
    return y; // Return the result of multiplication
}

// Function to multiply a byte by 0x03 in GF(2^8)
uint8_t MulByThree(uint8_t a) // Multiplication by 0x03 in GF(2^8)
{
    uint16_t b = a;
    b = b ^ (b << 1); // Equivalent to a * x + a
    uint8_t x, y;
    x = b & 0xff;        // Extract the lower 8 bits
    y = (b >> 8) & 0xff; // Extract the overflow bit
    y = (y * 0x1b) ^ x;  // If overflow, reduce by the AES polynomial
    return y; // Return the result of multiplication
}

// Function to perform the MixColumns transformation on the state matrix
void MixColumn(uint8_t S[4][4])
{
    uint8_t T[4]; // Temporary array to store transformed column
    for (int i = 0; i < 4; i++) // Iterate over each column
    {
        // Apply the MixColumns transformation using GF(2^8) multiplication
        T[0] = MulByTwo(S[0][i]) ^ MulByThree(S[1][i]) ^ S[2][i] ^ S[3][i];
        T[1] = S[0][i] ^ MulByTwo(S[1][i]) ^ MulByThree(S[2][i]) ^ S[3][i];
        T[2] = S[0][i] ^ S[1][i] ^ MulByTwo(S[2][i]) ^ MulByThree(S[3][i]);
        T[3] = MulByThree(S[0][i]) ^ S[1][i] ^ S[2][i] ^ MulByTwo(S[3][i]);
        
        // Update the state matrix with the transformed column
        for (int j = 0; j < 4; j++)
        {
            S[j][i] = T[j];
        }
    }
}

// Function to apply the S-Box substitution to each byte of the state matrix
void ByteSub(uint8_t S[4][4])
{
    for (int i = 0; i < 4; i++) // Iterate over each row
    {
        for (int j = 0; j < 4; j++) // Iterate over each column
        {
            S_Box(&S[i][j]); // Substitute the byte using the S-Box
        }
    }
}

// Function to perform the ShiftRows transformation on the state matrix
void ShiftRow(uint8_t S[4][4])
{
    for (int i = 1; i < 4; i++) // Start from the second row (i = 1)
    {
        uint8_t x[i]; // Temporary array to store bytes to be shifted
        for (int j = 0; j < i; j++)
        {
            x[j] = S[i][j]; // Store the first 'i' bytes
        }
        for (int j = 0; j < (4 - i); j++)
        {
            S[i][j] = S[i][(i + j) % 4]; // Shift the remaining bytes to the left
        }
        int k = 0;
        for (int j = (4 - i); j < 4; j++)
        {
            S[i][j] = x[k]; // Place the stored bytes at the end of the row
            k++;
        }
    }
}

// Function to apply the inverse S-Box substitution to a single byte
void inv_SBox(uint8_t *s)
{
    // Define the inverse S-Box as a 16x16 lookup table
    static const uint8_t inv_sbox[16][16] = {
        {0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB},
        {0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB},
        {0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E},
        {0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25},
        {0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92},
        {0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84},
        {0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06},
        {0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B},
        {0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73},
        {0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E},
        {0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B},
        {0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4},
        {0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F},
        {0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF},
        {0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61},
        {0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D}
    };
    
    unsigned char x, y;
    y = (*s) & 0x0f;    // Extract the lower 4 bits (column index)
    x = (*s) >> 4;      // Extract the upper 4 bits (row index)
    *s = inv_sbox[x][y]; // Substitute the byte using the inverse S-Box
}

// Function to apply the inverse S-Box substitution to each byte of the state matrix
void inv_ByteSub(uint8_t S[4][4])
{
    for (int i = 0; i < 4; i++) // Iterate over each row
    {
        for (int j = 0; j < 4; j++) // Iterate over each column
        {
            inv_SBox(&S[i][j]); // Substitute the byte using the inverse S-Box
        }
    }
}

// Function to perform the inverse ShiftRows transformation on the state matrix
void inv_shiftRow(uint8_t S[4][4])
{
    for (int i = 1; i < 4; i++) // Start from the second row (i = 1)
    {
        uint8_t x[i]; // Temporary array to store bytes to be shifted
        for (int j = 0; j < (4 - i); j++)
        {
            x[j] = S[i][j]; // Store the first (4 - i) bytes
        }
        for (int j = 0; j < i; j++)
        {
            S[i][j] = S[i][(4 - i + j) % 4]; // Shift the remaining bytes to the right
        }
        int k = 0;
        for (int j = i; j < 4; j++)
        {
            S[i][j] = x[k]; // Place the stored bytes at the end of the row
            k++;
        }
    }
}

// Function to perform the inverse MixColumns transformation on the state matrix
void inv_MixColumn(uint8_t S[4][4])
{
    // Define the inverse MixColumns matrix
    uint8_t mixmat[4][4] = {
        {0x0e, 0x0b, 0x0d, 0x09},
        {0x09, 0x0e, 0x0b, 0x0d},
        {0x0d, 0x09, 0x0e, 0x0b},
        {0x0b, 0x0d, 0x09, 0x0e}
    };
    
    uint8_t temp[4][4]; // Temporary matrix to store the result
    for(int j = 0; j < 4; j++) // Iterate over each column
    {
        for (int i = 0; i < 4; i++) // Iterate over each row of the mix matrix
        {
            temp[i][j] = 0; // Initialize the temporary byte to zero
            for (int k = 0; k < 4; k++) // Perform matrix multiplication
            {
                // Multiply corresponding bytes and perform reduction
                temp[i][j] = temp[i][j] ^ Reduction(Mul(mixmat[i][k], S[k][j]));
            } 
        }
        for(int m = 0; m < 4; m++) // Update the state matrix with the transformed column
        {
            S[m][j] = temp[m][j];
        }
    }
}
