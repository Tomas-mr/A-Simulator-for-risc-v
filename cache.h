#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"pipline.h"

long time;
extern long cy2;
int lruTime, lruSlot;
int blkOffsetBits, indexMask, blk, tagMask, index, tag;
int d_miss = 0;
int d_hit = 0;
int i_miss = 0;
int i_hit = 0;

//Data Cache大小为 16KB
//Insn Cache大小为 16KB

#define DC_NUM_SETS 256
#define DC_SET_SIZE 4
#define DC_WR_BUFF_SIZE 16
#define DC_BLOCK_SIZE 128
#define MemRdLatency 100
#define MemWrLatency 100
#define MAX_TIME 0x3f3f3f3f

struct cacheBlk {
    int tag;
    int status;
    int trdy;
}dCache[DC_NUM_SETS][DC_SET_SIZE], iCache[DC_NUM_SETS][DC_SET_SIZE];

struct writeBuffer {
    int tag;
    int trdy;
} dcWrBuff[DC_WR_BUFF_SIZE];

int wrBack(int tag, int time) {
    int i, trdy = 0, minTime = MAX_TIME, minEntry = 0;
    for (i = 0; i < (DC_WR_BUFF_SIZE); i++) {
        if (dcWrBuff[i].tag == tag) {
            dcWrBuff[i].trdy = time + MemWrLatency;
            return 0;
        } // End if
        if (dcWrBuff[i].trdy < minTime) {
            minTime = dcWrBuff[i].trdy;
            minEntry = i;
        } // End if
    } // End for
    if (minTime > time)
        trdy = dcWrBuff[minEntry].trdy - time;
    dcWrBuff[minEntry].tag = tag;
    dcWrBuff[minEntry].trdy = time + trdy + MemWrLatency;
    return trdy;
}

int accessDCache(int opcd, unsigned int addr) {
    blkOffsetBits = log2(DC_BLOCK_SIZE);
    indexMask = (unsigned)((DC_NUM_SETS) - 1);
    tagMask = ~indexMask;
    blk = ((unsigned)addr) >> blkOffsetBits;
    index = (int)(blk & indexMask);
    tag = (int)(blk & tagMask);
    time = cy2;

    int pos = -1;
    int i;
    for (i = 0; i < DC_SET_SIZE; i++) {
        if ((dCache[index][i].tag == tag) && (dCache[index][i].status != 0)) {
            pos = i;
            break;
        }
        else /* Find a possible replacement line */
            if (dCache[index][i].trdy < lruTime) {
                lruTime = dCache[index][i].trdy;
                lruSlot = i;
            }
    }
    if (pos >= 0) {//hit
        if (opcd == 3) {
            if (time < dCache[index][pos].trdy)
                return 0;
            dCache[index][pos].trdy = time;
        }
        else {
            if (time < dCache[index][pos].trdy)
                return 0;
            dCache[index][pos].trdy = time;
            dCache[index][pos].status = 2;
        }
        d_hit++;
        return 1;
    }
    else {
        pos = lruSlot;
        d_miss++;
        if (opcd == 3) {
            int trdy = MemRdLatency;
            if (dCache[index][pos].status == 2) // 如果被换出的块为脏块
                trdy += wrBack(tag, time);
            dCache[index][pos].tag = tag;
            dCache[index][pos].trdy = time + trdy;
            dCache[index][pos].status = 1;
        }
        else {
            int trdy = 0;
            if (dCache[index][pos].status == 2) /* Must remote write-back old data */
                trdy = wrBack(tag, time);
            else
                dCache[index][pos].status = 2;
            /* Read in cache line we wish to update */
            trdy += MemRdLatency;
            dCache[index][pos].tag = tag;
            dCache[index][pos].trdy = time + trdy;
        }
        return 0;
    }
}


int accessICache(unsigned int addr) {
    blkOffsetBits = log2(DC_BLOCK_SIZE);
    indexMask = (unsigned)((DC_NUM_SETS)-1);
    tagMask = ~indexMask;
    blk = ((unsigned)addr) >> blkOffsetBits;
    index = (int)(blk & indexMask);
    tag = (int)(blk & tagMask);
    time = cy2;

    int pos = -1;
    int i;
    for (i = 0; i < DC_SET_SIZE; i++) {
        if ((iCache[index][i].tag == tag) && (iCache[index][i].status != 0)) {
            pos = i;
            break;
        }
        else /* Find a possible replacement line */
            if (iCache[index][i].trdy < lruTime) {
                lruTime = iCache[index][i].trdy;
                lruSlot = i;
            }
    }
    if (pos >= 0) {//hit
        if (time < iCache[index][pos].trdy)
            return 0;
        iCache[index][pos].trdy = time;
        i_hit++;
        return 1;
    }
    else {
        pos = lruSlot;
        i_miss++;
        int trdy = MemRdLatency;
        iCache[index][pos].tag = tag;
        iCache[index][pos].trdy = time + trdy;
        iCache[index][pos].status = 1;
        return 0;
    }
}