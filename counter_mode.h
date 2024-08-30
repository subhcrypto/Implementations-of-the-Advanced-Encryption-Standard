#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "aes_algorithms.h"

// Counter (CTR) mode encryption function
void CRT_Encrypt()
{
    FILE *fp1, *fp2;

    // Open the input file for reading in binary mode
    fp1 = fopen("filename.txt", "rb");
    // Open the output file for writing in binary mode
    fp2 = fopen("filename1.txt", "wb");

    // Check if files were opened successfully
    if (fp1 == NULL || fp2 == NULL) {
        printf("Error opening file.\n");
    }

    // Initialize the counter (CTR) block to all zeros
    uint8_t ctr[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buf[16]; // Buffer to store 16 bytes of data

    // Write the initial counter to the output file
    fwrite(ctr, 1, 16, fp2);

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

            // Increment the counter
            ctr[15] += 1;
            for (int i = 14; i >= 0; i--)
            {
                if (ctr[i + 1] == 0)
                {
                    ctr[i] += 1;
                }
                else
                {
                    break;
                }
            }

            AES_Encrypt(ctr); // Encrypt the counter

            // XOR the encrypted counter with the buffered data to get the ciphertext
            for (int i = 0; i < 16; i++)
            {
                buf[i] = ctr[i] ^ buf[i];
            }

            // Write the ciphertext to the output file
            fwrite(buf, 1, 16, fp2);
            break; // Exit the loop after processing the last block
        }

        // Increment the counter
        ctr[15] += 1;
        for (int i = 14; i >= 0; i--)
        {
            if (ctr[i + 1] == 0)
            {
                ctr[i] += 1;
            }
            else
            {
                break;
            }
        }

        AES_Encrypt(ctr); // Encrypt the counter

        // XOR the encrypted counter with the current block to get the ciphertext
        for (int i = 0; i < 16; i++)
        {
            buf[i] = ctr[i] ^ buf[i];
        }

        // Write the ciphertext to the output file
        fwrite(buf, 1, 16, fp2);
    }

    // Close the input and output files
    fclose(fp1);
    fclose(fp2);
}

// Counter (CTR) mode decryption function
void CRT_Decrypt()
{
    FILE *fp1, *fp2;

    // Open the encrypted file for reading in binary mode
    fp1 = fopen("filename1.txt", "rb");
    // Open the output file for writing in binary mode
    fp2 = fopen("filename2.txt", "wb");

    // Check if the files were opened successfully
    if (fp1 == NULL)
    {
        printf("Error: open file \"filename1.txt\" failed");
    }

    uint8_t buf[16], temp[16], ctr[16]; // Buffers to store 16 bytes of data

    // Read the initial counter from the input file
    fread(ctr, 1, 16, fp1);
    int flag = 0; // Flag to indicate whether to write the previous block

    while(1)
    {
        // Read 16 bytes from the encrypted file into the buffer
        int k = fread(buf, 1, 16, fp1);

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

        // Increment the counter
        ctr[15] += 1;
        for (int i = 14; i >= 0; i--)
        {
            if (ctr[i + 1] == 0)
            {
                ctr[i] += 1;
            }
            else
            {
                break;
            }
        }

        AES_Encrypt(ctr); // Encrypt the counter

        // XOR the encrypted counter with the current block to get the plaintext
        for (int i = 0; i < 16; i++)
        {
            buf[i] = ctr[i] ^ buf[i];
        }

        // Store the decrypted data in the temporary buffer
        for(int i = 0; i < 16; i++)
        {
            temp[i] = buf[i];
        }

        flag = 1; // Set the flag to indicate that the previous block should be written
    }
}


