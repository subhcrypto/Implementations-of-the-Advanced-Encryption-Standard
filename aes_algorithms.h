#include <stdio.h>
#include <stdint.h>
#include "aesoperation.h"

void AES_Encrypt(uint8_t *m)
{
    word W[4][11]; // Array to store the round keys
    unsigned char key[16]= {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c}; // AES key

    // Initialize the first round key from the key array
    for ( int i = 0; i < 4; i++)
    {
        for ( int j = 0; j < 4; j++)
        {
            W[i][0].k[j] = key[4*i + j];
        }
    }

    keyExpansion(W); // Generate the round keys using the key expansion algorithm

    uint8_t S[4][4]; // State array to hold the block to be encrypted
    // Copy the input message into the state array
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            S[i][j] = m[i + 4*j];
        }
    }

    // Perform the main rounds of AES (9 rounds for AES-128)
    for (int k = 0; k < 9; k++)
    {
        // AddRoundKey step
        for (int i = 0; i < 4; i++) 
        {
            for (int j = 0; j < 4; j++) 
            {
                S[j][i] ^= W[i][k].k[j]; 
            }
        }
        
        ByteSub(S);     // SubBytes step
        ShiftRow(S);    // ShiftRows step
        MixColumn(S);   // MixColumns step
    }
    
    // Final round (without MixColumns)
    for (int i = 0; i < 4; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            S[j][i] ^= W[i][9].k[j]; 
        }
    }

    ByteSub(S);     // SubBytes step
    ShiftRow(S);    // ShiftRows step
    for (int i = 0; i < 4; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            S[j][i] ^= W[i][10].k[j]; 
        }
    }

    // Copy the encrypted state array back to the message array
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i + 4*j] = S[i][j];
        }
    }
}

void AES_Decrypt(uint8_t *m)
{
    word W[4][11]; // Array to store the round keys
    unsigned char key[16]= {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c}; // AES key

    // Initialize the first round key from the key array
    for ( int i = 0; i < 4; i++)
    {
        for ( int j = 0; j < 4; j++)
        {
            W[i][0].k[j] = key[4*i + j];
        }
    }

    keyExpansion(W); // Generate the round keys using the key expansion algorithm

    uint8_t S[4][4]; // State array to hold the block to be decrypted
    // Copy the input message into the state array
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            S[i][j] = m[i + 4*j];
        }
    }

    // Initial AddRoundKey step with the last round key
    for (int i = 0; i < 4; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            S[j][i] ^= W[i][10].k[j]; 
        }
    }

    inv_shiftRow(S);   // Inverse ShiftRows step
    inv_ByteSub(S);    // Inverse SubBytes step

    // Perform the main rounds of AES decryption (9 rounds for AES-128)
    for (int i = 0; i < 4; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            S[j][i] ^= W[i][9].k[j]; 
        }
    }

    for (int k = 8; k >= 0; k--)
    {
        inv_MixColumn(S);  // Inverse MixColumns step
        inv_shiftRow(S);   // Inverse ShiftRows step
        inv_ByteSub(S);    // Inverse SubBytes step

        // AddRoundKey step
        for (int i = 0; i < 4; i++) 
        {
            for (int j = 0; j < 4; j++) 
            {
                S[j][i] ^= W[i][k].k[j]; 
            }
        }
    }

    // Copy the decrypted state array back to the message array
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i + 4*j] = S[i][j];
        }
    }
}
