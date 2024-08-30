#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "aes_algorithms.h"

// OFB (Output Feedback) mode encryption function
void OFB_Encrypt()
{
    FILE *fp1, *fp2;
    
    // Open the input file for reading in binary mode
    fp1 = fopen("filename.txt", "rb");
    // Open the output file for writing in binary mode
    fp2 = fopen("filename1.txt", "wb");
    
    // Check if file opening was successful
    if (fp1 == NULL || fp2 == NULL) {
        printf("Error opening file.\n");
    }

    uint8_t buf[16]; // Buffer to store 16 bytes of data

    // Initialization Vector (IV) for OFB mode
    uint8_t iv[16] = {
        0x1a, 0x2b, 0x3c, 0x4d, 
        0x5e, 0x6f, 0x70, 0x81, 
        0x92, 0xa3, 0xb4, 0xc5, 
        0xd6, 0xe7, 0xf8, 0x09
    };

    // Write the IV to the output file
    fwrite(iv, 1, 16, fp2);

    while(1)
    {
        // Read 16 bytes from the input file into the buffer
        int n = fread(buf, 1, 16, fp1);
        
        // If less than 16 bytes are read, it's the last block, and padding is needed
        if (n < 16)
        {
            uint8_t a = 16 - n; // Calculate padding value
            
            // Apply padding to the remaining bytes in the buffer
            for(int i = n; i < 16; i++)
            {
                buf[i] = a;
            }
            
            AES_Encrypt(iv); // Encrypt the IV
            
            // XOR the encrypted IV with the padded buffer to get the ciphertext
            for(int i = 0; i < 16; i++)
            {
                buf[i] = buf[i] ^ iv[i];
            }
            
            // Write the ciphertext to the output file
            fwrite(buf, 1, 16, fp2);
            break; // Exit the loop after processing the last block
        }

        AES_Encrypt(iv); // Encrypt the IV
        
        // XOR the encrypted IV with the current block to get the ciphertext
        for(int i = 0; i < 16; i++)
        {
            buf[i] = buf[i] ^ iv[i];
        }
        
        // Write the ciphertext to the output file
        fwrite(buf, 1, 16, fp2);
    }

    // Close the input and output files
    fclose(fp1);
    fclose(fp2);
}

// OFB (Output Feedback) mode decryption function
void OFB_Decrypt()
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
        // Read 16 bytes from the encrypted file into the buffer
        int k = fread(buff, 1, 16, fp1);
        
        // If the flag is set, write the previous block to the output file
        if(flag == 1)
        {
            // If the end of the file is reached, handle padding removal
            if (k == 0)
            {
                int i = temp[15]; // Get the padding value
                fwrite(temp, 1, 16 - i, fp2); // Write the unpadded data to the output file
                break; // Exit the loop after processing the last block
            }
            else
            {
                fwrite(temp, 1, 16, fp2); // Write the full decrypted block to the output file
            }
        }
        
        AES_Encrypt(iv); // Encrypt the IV
        
        // XOR the encrypted IV with the current block to get the plaintext
        for(int i = 0; i < 16; i++)
        {
            temp[i] = buff[i] ^ iv[i];
        }

        flag = 1; // Set the flag to indicate that the previous block should be written
    }
}

