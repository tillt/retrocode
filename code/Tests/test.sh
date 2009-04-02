#!/bin/sh
# 
DOSEXEC=../code.exe
UNXEXEC=../retrocode
EXEC=$UNXEXEC

rm -Rf outputs
mkdir outputs
for ee in $(ls -1 inputs); do
    $EXEC -s 8000 -c 1 inputs/$ee outputs/${ee}.s8000_c1.qcp --save true

    $EXEC -s 8000 -c 1 inputs/$ee outputs/${ee}.s8000_c1.mmf --save true
    $EXEC -s 16000 -c 1 inputs/$ee outputs/${ee}.s8000_c1.mmf --save true

    $EXEC -s 8000 -c 1 -cc 1 inputs/$ee outputs/${ee}.s8000_c1_cc1.wav --save true
    $EXEC -s 8000 -c 1 -cc 4 inputs/$ee outputs/${ee}.s8000_c1_cc4.wav --save true
    $EXEC -s 44100 -c 1 inputs/$ee outputs/${ee}.s44100_c1.wav --save true

    $EXEC -s 8000 -c 1 -br 12200 inputs/$ee outputs/${ee}.s8000_c1_br12200.amr --save true
    $EXEC -s 8000 -c 1 -br 7400 inputs/$ee outputs/${ee}.s8000_c1_br7400.amr --save true
    $EXEC -s 16000 -c 1 -br 23850 inputs/$ee outputs/${ee}.s16000_c1_br23850.awb --save true

    $EXEC -s 22050 -c 1 inputs/$ee outputs/${ee}.s22050_c1.rmf --save true
    $EXEC -s 8000 -c 1 inputs/$ee outputs/${ee}.s8000_c1.rmf --save true

    $EXEC -s 16000 -c 1 -cc 1 inputs/$ee outputs/${ee}.s16000_c1_cc1.pmd --save true
    $EXEC -s 16000 -c 1 -cc 1 -lc 15 inputs/$ee outputs/${ee}.s16000_c1_cc1_lc15.pmd --save true
    $EXEC -s 8000 -c 1 -cc 3 inputs/$ee outputs/${ee}.s8000_c1_cc3.pmd --save true

    $EXEC -s 8000 -c 1 inputs/$ee outputs/${ee}.s8000_c1.vox --save true

    $EXEC -s 8000 -c 1 inputs/$ee outputs/${ee}.s8000_c1.mfm --save true

    $EXEC -s 22050 -c 1 -br 96000 inputs/$ee outputs/${ee}.s22050_c1_br56000.mp3 --save true
    $EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c1_br96000.mp3 --save true
    $EXEC -s 44100 -c 2 -br 128000 inputs/$ee outputs/${ee}.s44100_c1_br128000.mp3 --save true

    $EXEC -s 44100 -c 1 -br 64000 inputs/$ee outputs/${ee}.s44100_c1_br64000.swf --save true
    
    $EXEC -s 44100 -c 1 -br 48000 inputs/$ee outputs/${ee}.s44100_c1_br48000.aac --save true
    $EXEC -s 44100 -c 2 -br 48000 inputs/$ee outputs/${ee}.s44100_c2_br48000.aac --save true
    $EXEC -s 44100 -c 2 -br 128000 inputs/$ee outputs/${ee}.s44100_c2_br128000.aac --save true
    $EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c2_br96000.aac --save true
    $EXEC -s 44100 -c 1 -br 48000 -ac 0 inputs/$ee outputs/${ee}.s44100_c1_br48000_ac0.aac --save true
    #this does not work
    #$EXEC -s 44100 -c 1 -br 48000 -ac 2 inputs/$ee outputs/${ee}.s44100_c1_br48000_ac2.aac --save true
    $EXEC -s 44100 -c 1 -br 48000 -ac 3 inputs/$ee outputs/${ee}.s44100_c1_br48000_ac3.aac --save true
    $EXEC -s 44100 -c 1 -br 48000 -ac 4 inputs/$ee outputs/${ee}.s44100_c1_br48000_ac4.aac --save true

    #for these files are different even if you convert using the same version and parameters
    #$EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c2_br96000.mp4 --save true

    #$EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c2_br96000.3gp --save true
    #$EXEC -s 44100 -c 2 -br 64000 -dt 0 inputs/$ee outputs/${ee}.s44100_c2_br64000_dt0.3gp --save true

    #these are causing retrocode to crash
    #$EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c2_br96000.wma --save true

    #$EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c2_br96000.ra --save true

    #$EXEC -s 44100 -c 2 -br 96000 inputs/$ee outputs/${ee}.s44100_c2_br96000.ogg --save true
done

for ee in $(ls -1 golden); do
    if [ -f outputs/$ee ]; then
        if cmp outputs/$ee golden/$ee; then
            echo $ee OK
        fi
    else
        echo $ee was not succesfully created
    fi
done
