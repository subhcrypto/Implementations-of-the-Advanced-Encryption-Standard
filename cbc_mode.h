#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "aes_algorithms.h"

// CBC (Cipher Block Chaining) mode encryption function
void CBC_Encrypt()
{
    FILE *fp1, *fp2;
    
    // Open the input file for reading in binary mode
    fp1 = fopen("filename.txt", "rb");
    // Open the output file for writing in binary mode
    fp2 = fopen("filename1.txt", "wb");
    
    // Initialization Vector (IV) for CBC mode
    uint8_t iv[16] = {
        0x1a, 0x2b, 0x3c, 0x4d, 
        0x5e, 0x6f, 0x70, 0x81, 
        0x92, 0xa3, 0xb4, 0xc5, 
        0xd6, 0xe7, 0xf8, 0x09
    };
    
    // Write the IV to the output file
    fwrite(iv, 1, 16, fp2);
    
    uint8_t buff[16]; // Buffer to store 16 bytes of data

    while(1)
    {
        int n = fread(buff, 1, 16, fp1); // Read 16 bytes from the input file into the buffer
        
        // If less than 16 bytes are read, it's the last block, and padding is needed
        if (n < 16)
        {
            uint8_t a = 16 - n; // Calculate padding value
            // Apply padding to the remaining bytes in the buffer
            for(int i = n; i < 16; i++)
            {
                buff[i] = a;
            }
            // XOR the IV with the padded block
            for (int i = 0; i < 16; i++)
            {
                iv[i] = iv[i] ^ buff[i];
            }
            AES_Encrypt(iv); // Encrypt the XORed block
            fwrite(iv, 1, 16, fp2); // Write the encrypted block to the output file
            break; // Exit the loop after processing the last block
        }
        
        // XOR the IV with the current block
        for (int i = 0; i < 16; i++)
        {
            iv[i] = iv[i] ^ buff[i];
        }
        
        AES_Encrypt(iv); // Encrypt the XORed block
        fwrite(iv, 1, 16, fp2); // Write the encrypted block to the output file
    }
    
    // Close the input and output files
    fclose(fp2);
    fclose(fp1);
}

// CBC (Cipher Block Chaining) mode decryption function
void CBC_Decrypt()
{
    FILE *fp1, *fp2;
    
    // Open the encrypted file for reading in binary mode
    fp1 = fopen("filename1.txt", "rb");
    // Open the output file for writing in binary mode
    fp2 = fopen("filename2.txt", "wb");
    
    // Check if file opening was successful
    if (fp1 == NULL)
    {
        printf("Error: open file \"filename1.txt\" failed");
    }
    
    uint8_t buff[16], temp[16], iv[16]; // Buffers to store 16 bytes of data
    fread(iv, 1, 16, fp1); // Read the IV from the input file
    int flag = 0; // Flag to indicate whether to write the previous block

    while(1)
    {
        uint8_t temp1[16]; // Temporary buffer to store the current block
        int k = fread(buff, 1, 16, fp1); // Read 16 bytes from the encrypted file into the buffer
        
        // If the flag is set, write the previous block to the output file
        if(flag == 1)
        {
            // If the end of the file is reached, handle padding removal
            if (k == 0)
            {
                int i = temp[15]; // Get the padding value
                fwrite(temp, 1, 16 - i, fp2); // Write the unpadded data to the output file
                break;
            }
            else
            {
                fwrite(temp, 1, 16, fp2); // Write the full decrypted block to the output file
            }
        }
        
        // Store the current encrypted block in temp1 for later use
        for (int i = 0; i < 16; i++)
        {
            temp1[i] = buff[i];
        }
        
        AES_Decrypt(buff); // Decrypt the current block
        
        // XOR the decrypted block with the IV to get the original plaintext
        for(int i = 0; i < 16; i++)
        {
            temp[i] = buff[i] ^ iv[i];
        }
        
        // Update the IV with the current encrypted block (temp1) for the next round
        for (int i = 0; i < 16; i++)
        {
            iv[i] = temp1[i];
        }
        
        flag = 1; // Set the flag to indicate that the previous block should be written
    }
}

// Main function to execute CBC encryption and decryption
int main()
{
    CBC_Encrypt(); // Perform CBC encryption
    CBC_Decrypt(); // Perform CBC decryption
    return 0;
}
