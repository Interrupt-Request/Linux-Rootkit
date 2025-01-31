/*
    * This file contains the assembler code for embedding a binary file 
*/
        .section .init.rodata, "a" //;Linked as part of the rodata segment and initialization code, section is allocatable 
        .global embedded_binaryBlob_start //;Global variable label embedded_binaryBlob_start
embedded_binaryBlob_start:
        .incbin "/Rootkit/build/reflectiveLoader" //;Incbin directive
        .global embedded_binaryBlob_end //;Global variable label embedded_binaryBlob_end
embedded_binaryBlob_end: