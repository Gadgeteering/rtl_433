/** @file
    LUCKY Wireless Transducer - only compatible with Lucky Fishfinder FF718LiC-W, FF1108-1CW, FFW718.

    Copyright (C) 2022 Pedro Caza

   'n=fishfinder,m=OOK_PWM,s=175,l=575,bits=20,r=700'

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/
/**
LUCKY Wireless Transducer - Wireless Sonar Transducer

    https://www.luckysonar.com/wp-content/uploads/2021/11/FF1108-W.pdf

*/

#include "decoder.h"
#include <math.h>       /* pow */


static int lucky_transducer_callback(r_device *decoder, bitbuffer_t *bitbuffer)
{
    uint8_t *b = bitbuffer->bb[0];
    int device_id          = 0;
    uint8_t bytes[2];

    // temperature row needs to have at least 20 bits (plus a few more due to bit stuffing)
    if (bitbuffer->bits_per_row[0] < 16) return DECODE_ABORT_LENGTH;
    if (bitbuffer->bits_per_row[0] > 15) {
        device_id = bytes[0] + (bytes[1] << 8);
        printf("bytes %x %x",bytes[0],bytes[1]);
    }
    bitbuffer_extract_bytes(bitbuffer,0,0,bytes,16);
    device_id = bytes[0] + (bytes[1] << 8);
    printf("bytes %x %x",bytes[0],bytes[1]);
    bitbuffer_print(bitbuffer); 

    printf("Debug Mode Level %d \n",decoder->verbose);
    // TODO: Use repeat signal for error checking / correction!
    if (decoder->verbose == 0) {
        printf("Debug Mode Level %d \n",decoder->verbose);   
    }

    // The protocol uses bit-stuffing double
    // Also, each byte is represented with least significant bit first -> swap them!
    bitbuffer_t bits = {0};
    int temperature = 0;
    

    if (decoder->verbose == 0) {
        bitbuffer_print(bitbuffer);
        //fprintf(stderr, "  Row 0 = Input, Row 1 = Zero bit stuffing, Row 2 = Stripped delimiters, Row 3 = Decoded nibbles\n");
    }
    /*
    for (uint16_t k = bitbuffer->bits_per_row[0]-5; k != 0 ; k--) {
        int bit = bitrow_get_bit(b, k);
        printf("---%d bit %d \n",k,bit);
     }
     */
    if (bitbuffer->bits_per_row[0] > 19) {
    int power = 0;
    for (uint16_t k = bitbuffer->bits_per_row[0]-5; k < 1000 ; k-= 2) {
        int bit = bitrow_get_bit(b, k);
        int bit2 = bitrow_get_bit(b, k-1);
       // printf("bit %d bit2 %d  %d",bit,bit2,k);
        
        if (bit == bit2) {
            if (bit == 1) temperature = temperature + pow (2,power);   
        }
        else {
            if (decoder->verbose == 0) {
            fprintf(stderr, "Checksum error in LUCKY Wireless Transducer.\n");
            return DECODE_FAIL_SANITY; 
            }
        }
        //printf("Temp %d   %d\n",temperature, pow(2,power));
        power++;
    }
    }

    //b = bits.bb[0];
    
    uint16_t bitcount = bits.bits_per_row[0];

    

    

    //fahrenheit2celsius(float fahrenheit)

        // Device ID starts at byte 4:
        
        int heating_mode       = 0;    // highest bit indicates automatic (2-point) / analogue mode
        int target_temperature = 0;  // highest bit indicates auto(2-point) / analogue mode
        int water_preheated    = 0; // bit 4 indicates water: 1=Pre-heat, 0=no pre-heated water
        int battery_low        = 0;      // if not zero, battery is low

        /* clang-format off */
        data_t *data = data_make(
                "model",        "",                     DATA_STRING, "LUCKY-Wireless-Transducer",
                "id",           "Device ID",            DATA_FORMAT, "0x%04X", DATA_INT, device_id,
                "heating",      "Heating Mode",         DATA_STRING, (heating_mode == 0 && target_temperature == 0) ? "OFF" : heating_mode ? "ON (2-point)" : "ON (analogue)",
                "temp", "Water Temp.",  DATA_FORMAT, "%d", DATA_INT, temperature,
                "water",        "Pre-heated Water",     DATA_STRING, water_preheated ? "ON" : "off",
                "battery_ok",   "Battery",              DATA_INT,    !battery_low,
                NULL);
        /* clang-format on */
        decoder_output_data(decoder, data);

        return 1;
    }

    
 

static char *output_fields[] = {
        "model",
        "id",
        "heating",
        "temp",
        "water",
        "battery_ok",
        NULL,
};

r_device lucky_transducer = {
        .name        = "LUCKY Wireless Transducer - Lucky Fishfinder Transducer FF1108, FF718L",
        .modulation  = OOK_PULSE_PWM,
        .short_width = 200,  // half-bit width 836 us
        .long_width  = 600, // bit width 1648 us
        .reset_limit = 4000,
        .tolerance   = 120, // us
        .decode_fn   = &lucky_transducer_callback,
        .fields      = output_fields,
};
