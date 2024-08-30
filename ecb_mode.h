#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "aes_algorithms.h"

// ECB (Electronic Codebook) mode encryption function
void ECB_Encrypt()
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

    while(1)
    {
        int n = fread(buf, 1, 16, fp1); // Read 16 bytes from the input file into the buffer

        // If less than 16 bytes are read, it's the last block, and padding is needed
        if (n < 16)
        {
            uint8_t a = 16 - n; // Calculate padding value
            // Apply padding to the remaining bytes in the buffer
            for(int i = n; i < 16; i++)
            {
                buf[i] = a;
            }
            AES_Encrypt(buf); // Encrypt the padded block
            fwrite(buf, 1, 16, fp2); // Write the encrypted block to the output file
            break; // Exit the loop after processing the last block
        }

        AES_Encrypt(buf); // Encrypt the 16-byte block
        fwrite(buf, 1, 16, fp2); // Write the encrypted block to the output file
    }
    
    // Close the input and output files
    fclose(fp1);
    fclose(fp2);
}

// ECB (Electronic Codebook) mode decryption function
void ECB_Decrypt()
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
    
    uint8_t buff[16], temp[16]; // Buffers to store 16 bytes of data
    int flag = 0; // Flag to indicate whether to write the previous block
    
    while(1)
    {
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

        AES_Decrypt(buff); // Decrypt the current block
        
        // Copy the decrypted data to the temporary buffer for potential future writing
        for(int i = 0; i < 16; i++)
        {
            temp[i] = buff[i];
        }

        flag = 1; // Set the flag to indicate that the previous block should be written
    }
}


