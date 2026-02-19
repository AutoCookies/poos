#!/usr/bin/env python3
import struct,sys
img=sys.argv[1]
size=128*1024*1024
start=2048
sectors=size//512
part_secs=sectors-start
b=bytearray(size)
# mbr
pe=bytearray(16); pe[4]=0x06; pe[8:12]=struct.pack('<I',start); pe[12:16]=struct.pack('<I',part_secs)
b[446:462]=pe; b[510]=0x55; b[511]=0xAA
# fat16 boot sector
off=start*512
bps=512; spc=4; rsvd=1; fats=1; root_cnt=512; spf=128
bs=bytearray(512)
bs[0:3]=b'\xeb\x3c\x90'; bs[3:11]=b'POOSFAT '
struct.pack_into('<H',bs,11,bps); bs[13]=spc; struct.pack_into('<H',bs,14,rsvd); bs[16]=fats; struct.pack_into('<H',bs,17,root_cnt)
struct.pack_into('<H',bs,19,part_secs if part_secs<65536 else 0); bs[21]=0xF8; struct.pack_into('<H',bs,22,spf); struct.pack_into('<H',bs,24,63); struct.pack_into('<H',bs,26,16)
struct.pack_into('<I',bs,28,start); struct.pack_into('<I',bs,32,part_secs); bs[36]=0x80; bs[38]=0x29; struct.pack_into('<I',bs,39,0x12345678); bs[43:54]=b'POOS DISK  '; bs[54:62]=b'FAT16   '
bs[510]=0x55; bs[511]=0xAA
b[off:off+512]=bs
fat_off=off+512
# FAT entries
struct.pack_into('<H',b,fat_off+0,0xFFF8); struct.pack_into('<H',b,fat_off+2,0xFFFF)
open(img,'wb').write(b)
