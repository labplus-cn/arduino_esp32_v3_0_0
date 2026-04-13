/*
 * QR Code generator library (C) - MIT License
 * Copyright (c) Project Nayuki. https://www.nayuki.io/page/qr-code-generator-library
 */
#include "qrcodegen.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define LV_ASSERT(x) assert(x)
#define testable static

static const char *ALPHANUMERIC_CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

static const int8_t ECC_CODEWORDS_PER_BLOCK[4][41] = {
    {-1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
    {-1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28},
    {-1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
    {-1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
};

#define REED_SOLOMON_DEGREE_MAX 30

static const int8_t NUM_ERROR_CORRECTION_BLOCKS[4][41] = {
    {-1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},
    {-1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},
    {-1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},
    {-1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81},
};

static const int PENALTY_N1 =  3;
static const int PENALTY_N2 =  3;
static const int PENALTY_N3 = 40;
static const int PENALTY_N4 = 10;

// Forward declarations
testable void appendBitsToBuffer(unsigned int val, int numBits, uint8_t buffer[], int *bitLen);
testable void addEccAndInterleave(uint8_t data[], int version, enum qrcodegen_Ecc ecl, uint8_t result[]);
testable int  getNumDataCodewords(int version, enum qrcodegen_Ecc ecl);
testable int  getNumRawDataModules(int ver);
testable void calcReedSolomonGenerator(int degree, uint8_t result[]);
testable void calcReedSolomonRemainder(const uint8_t data[], int dataLen, const uint8_t generator[], int degree, uint8_t result[]);
testable uint8_t finiteFieldMultiply(uint8_t x, uint8_t y);
testable void initializeFunctionModules(int version, uint8_t qrcode[]);
static  void drawWhiteFunctionModules(uint8_t qrcode[], int version);
static  void drawFormatBits(enum qrcodegen_Ecc ecl, enum qrcodegen_Mask mask, uint8_t qrcode[]);
testable int  getAlignmentPatternPositions(int version, uint8_t result[7]);
static  void fillRectangle(int left, int top, int width, int height, uint8_t qrcode[]);
static  void drawCodewords(const uint8_t data[], int dataLen, uint8_t qrcode[]);
static  void applyMask(const uint8_t functionModules[], uint8_t qrcode[], enum qrcodegen_Mask mask);
static  long getPenaltyScore(const uint8_t qrcode[]);
static  void addRunToHistory(unsigned char run, unsigned char history[7]);
static  bool hasFinderLikePattern(const unsigned char runHistory[7]);
testable bool getModule(const uint8_t qrcode[], int x, int y);
testable void setModule(uint8_t qrcode[], int x, int y, bool isBlack);
testable void setModuleBounded(uint8_t qrcode[], int x, int y, bool isBlack);
static  bool getBit(int x, int i);
testable int  calcSegmentBitLength(enum qrcodegen_Mode mode, size_t numChars);
testable int  getTotalBits(const struct qrcodegen_Segment segs[], size_t len, int version);
static  int  numCharCountBits(enum qrcodegen_Mode mode, int version);

/*---- High-level encoding ----*/

bool qrcodegen_encodeText(const char *text, uint8_t tempBuffer[], uint8_t qrcode[],
    enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl)
{
    size_t textLen = strlen(text);
    if (textLen == 0)
        return qrcodegen_encodeSegmentsAdvanced(NULL, 0, ecl, minVersion, maxVersion, mask, boostEcl, tempBuffer, qrcode);
    size_t bufLen = qrcodegen_BUFFER_LEN_FOR_VERSION(maxVersion);
    struct qrcodegen_Segment seg;
    if (qrcodegen_isNumeric(text)) {
        if (qrcodegen_calcSegmentBufferSize(qrcodegen_Mode_NUMERIC, textLen) > bufLen) goto fail;
        seg = qrcodegen_makeNumeric(text, tempBuffer);
    } else if (qrcodegen_isAlphanumeric(text)) {
        if (qrcodegen_calcSegmentBufferSize(qrcodegen_Mode_ALPHANUMERIC, textLen) > bufLen) goto fail;
        seg = qrcodegen_makeAlphanumeric(text, tempBuffer);
    } else {
        if (textLen > bufLen) goto fail;
        for (size_t i = 0; i < textLen; i++) tempBuffer[i] = (uint8_t)text[i];
        seg.mode = qrcodegen_Mode_BYTE;
        seg.bitLength = calcSegmentBitLength(seg.mode, textLen);
        if (seg.bitLength == -1) goto fail;
        seg.numChars = (int)textLen;
        seg.data = tempBuffer;
    }
    return qrcodegen_encodeSegmentsAdvanced(&seg, 1, ecl, minVersion, maxVersion, mask, boostEcl, tempBuffer, qrcode);
fail:
    qrcode[0] = 0;
    return false;
}

bool qrcodegen_encodeBinary(uint8_t dataAndTemp[], size_t dataLen, uint8_t qrcode[],
    enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl)
{
    struct qrcodegen_Segment seg;
    seg.mode = qrcodegen_Mode_BYTE;
    seg.bitLength = calcSegmentBitLength(seg.mode, dataLen);
    if (seg.bitLength == -1) { qrcode[0] = 0; return false; }
    seg.numChars = (int)dataLen;
    seg.data = dataAndTemp;
    return qrcodegen_encodeSegmentsAdvanced(&seg, 1, ecl, minVersion, maxVersion, mask, boostEcl, dataAndTemp, qrcode);
}

bool qrcodegen_encodeSegments(const struct qrcodegen_Segment segs[], size_t len,
    enum qrcodegen_Ecc ecl, uint8_t tempBuffer[], uint8_t qrcode[])
{
    return qrcodegen_encodeSegmentsAdvanced(segs, len, ecl,
        qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, -1, true, tempBuffer, qrcode);
}

bool qrcodegen_encodeSegmentsAdvanced(const struct qrcodegen_Segment segs[], size_t len, enum qrcodegen_Ecc ecl,
    int minVersion, int maxVersion, int mask, bool boostEcl, uint8_t tempBuffer[], uint8_t qrcode[])
{
    LV_ASSERT(segs != NULL || len == 0);
    LV_ASSERT(qrcodegen_VERSION_MIN <= minVersion && minVersion <= maxVersion && maxVersion <= qrcodegen_VERSION_MAX);
    LV_ASSERT(0 <= (int)ecl && (int)ecl <= 3 && -1 <= mask && mask <= 7);
    int version, dataUsedBits;
    for (version = minVersion; ; version++) {
        int dataCapacityBits = getNumDataCodewords(version, ecl) * 8;
        dataUsedBits = getTotalBits(segs, len, version);
        if (dataUsedBits != -1 && dataUsedBits <= dataCapacityBits) break;
        if (version >= maxVersion) { qrcode[0] = 0; return false; }
    }
    for (int i = (int)qrcodegen_Ecc_MEDIUM; i <= (int)qrcodegen_Ecc_HIGH; i++) {
        if (boostEcl && dataUsedBits <= getNumDataCodewords(version, (enum qrcodegen_Ecc)i) * 8)
            ecl = (enum qrcodegen_Ecc)i;
    }
    memset(qrcode, 0, qrcodegen_BUFFER_LEN_FOR_VERSION(version) * sizeof(qrcode[0]));
    int bitLen = 0;
    for (size_t i = 0; i < len; i++) {
        const struct qrcodegen_Segment *seg = &segs[i];
        appendBitsToBuffer((int)seg->mode, 4, qrcode, &bitLen);
        appendBitsToBuffer(seg->numChars, numCharCountBits(seg->mode, version), qrcode, &bitLen);
        for (int j = 0; j < seg->bitLength; j++)
            appendBitsToBuffer((seg->data[j >> 3] >> (7 - (j & 7))) & 1, 1, qrcode, &bitLen);
    }
    int dataCapacityBits = getNumDataCodewords(version, ecl) * 8;
    int terminatorBits = dataCapacityBits - bitLen;
    if (terminatorBits > 4) terminatorBits = 4;
    appendBitsToBuffer(0, terminatorBits, qrcode, &bitLen);
    appendBitsToBuffer(0, (8 - bitLen % 8) % 8, qrcode, &bitLen);
    for (uint8_t padByte = 0xEC; bitLen < dataCapacityBits; padByte ^= 0xEC ^ 0x11)
        appendBitsToBuffer(padByte, 8, qrcode, &bitLen);
    addEccAndInterleave(qrcode, version, ecl, tempBuffer);
    initializeFunctionModules(version, qrcode);
    drawCodewords(tempBuffer, getNumRawDataModules(version) / 8, qrcode);
    drawWhiteFunctionModules(qrcode, version);
    initializeFunctionModules(version, tempBuffer);
    if (mask == qrcodegen_Mask_AUTO) {
        long minPenalty = LONG_MAX;
        for (int i = 0; i < 8; i++) {
            enum qrcodegen_Mask msk = (enum qrcodegen_Mask)i;
            applyMask(tempBuffer, qrcode, msk);
            drawFormatBits(ecl, msk, qrcode);
            long penalty = getPenaltyScore(qrcode);
            if (penalty < minPenalty) { mask = msk; minPenalty = penalty; }
            applyMask(tempBuffer, qrcode, msk);
        }
    }
    applyMask(tempBuffer, qrcode, mask);
    drawFormatBits(ecl, mask, qrcode);
    return true;
}

/*---- ECC and interleave ----*/

testable void appendBitsToBuffer(unsigned int val, int numBits, uint8_t buffer[], int *bitLen) {
    for (int i = numBits - 1; i >= 0; i--, (*bitLen)++)
        buffer[*bitLen >> 3] |= ((val >> i) & 1) << (7 - (*bitLen & 7));
}

testable void addEccAndInterleave(uint8_t data[], int version, enum qrcodegen_Ecc ecl, uint8_t result[]) {
    int numBlocks = NUM_ERROR_CORRECTION_BLOCKS[(int)ecl][version];
    int blockEccLen = ECC_CODEWORDS_PER_BLOCK[(int)ecl][version];
    int rawCodewords = getNumRawDataModules(version) / 8;
    int dataLen = getNumDataCodewords(version, ecl);
    int numShortBlocks = numBlocks - rawCodewords % numBlocks;
    int shortBlockDataLen = rawCodewords / numBlocks - blockEccLen;
    uint8_t generator[REED_SOLOMON_DEGREE_MAX];
    calcReedSolomonGenerator(blockEccLen, generator);
    const uint8_t *dat = data;
    for (int i = 0; i < numBlocks; i++) {
        int datLen = shortBlockDataLen + (i < numShortBlocks ? 0 : 1);
        uint8_t *ecc = &data[dataLen];
        calcReedSolomonRemainder(dat, datLen, generator, blockEccLen, ecc);
        for (int j = 0, k = i; j < datLen; j++, k += numBlocks) {
            if (j == shortBlockDataLen) k -= numShortBlocks;
            result[k] = dat[j];
        }
        for (int j = 0, k = dataLen + i; j < blockEccLen; j++, k += numBlocks)
            result[k] = ecc[j];
        dat += datLen;
    }
}

testable int getNumDataCodewords(int version, enum qrcodegen_Ecc ecl) {
    return getNumRawDataModules(version) / 8
        - ECC_CODEWORDS_PER_BLOCK[(int)ecl][version]
        * NUM_ERROR_CORRECTION_BLOCKS[(int)ecl][version];
}

testable int getNumRawDataModules(int ver) {
    int result = (16 * ver + 128) * ver + 64;
    if (ver >= 2) {
        int numAlign = ver / 7 + 2;
        result -= (25 * numAlign - 10) * numAlign - 55;
        if (ver >= 7) result -= 36;
    }
    return result;
}

testable void calcReedSolomonGenerator(int degree, uint8_t result[]) {
    memset(result, 0, degree * sizeof(result[0]));
    result[degree - 1] = 1;
    uint8_t root = 1;
    for (int i = 0; i < degree; i++) {
        for (int j = 0; j < degree; j++) {
            result[j] = finiteFieldMultiply(result[j], root);
            if (j + 1 < degree) result[j] ^= result[j + 1];
        }
        root = finiteFieldMultiply(root, 0x02);
    }
}

testable void calcReedSolomonRemainder(const uint8_t data[], int dataLen,
    const uint8_t generator[], int degree, uint8_t result[]) {
    memset(result, 0, degree * sizeof(result[0]));
    for (int i = 0; i < dataLen; i++) {
        uint8_t factor = data[i] ^ result[0];
        memmove(&result[0], &result[1], (degree - 1) * sizeof(result[0]));
        result[degree - 1] = 0;
        for (int j = 0; j < degree; j++)
            result[j] ^= finiteFieldMultiply(generator[j], factor);
    }
}

testable uint8_t finiteFieldMultiply(uint8_t x, uint8_t y) {
    uint8_t z = 0;
    for (int i = 7; i >= 0; i--) {
        z = (z << 1) ^ ((z >> 7) * 0x11D);
        z ^= ((y >> i) & 1) * x;
    }
    return z;
}

/*---- Drawing function modules ----*/

testable void initializeFunctionModules(int version, uint8_t qrcode[]) {
    int qrsize = version * 4 + 17;
    memset(qrcode, 0, ((qrsize * qrsize + 7) / 8 + 1) * sizeof(qrcode[0]));
    qrcode[0] = (uint8_t)qrsize;
    fillRectangle(6, 0, 1, qrsize, qrcode);
    fillRectangle(0, 6, qrsize, 1, qrcode);
    fillRectangle(0, 0, 9, 9, qrcode);
    fillRectangle(qrsize - 8, 0, 8, 9, qrcode);
    fillRectangle(0, qrsize - 8, 9, 8, qrcode);
    uint8_t alignPatPos[7];
    int numAlign = getAlignmentPatternPositions(version, alignPatPos);
    for (int i = 0; i < numAlign; i++)
        for (int j = 0; j < numAlign; j++)
            if (!((i == 0 && j == 0) || (i == 0 && j == numAlign-1) || (i == numAlign-1 && j == 0)))
                fillRectangle(alignPatPos[i]-2, alignPatPos[j]-2, 5, 5, qrcode);
    if (version >= 7) {
        fillRectangle(qrsize - 11, 0, 3, 6, qrcode);
        fillRectangle(0, qrsize - 11, 6, 3, qrcode);
    }
}

static void drawWhiteFunctionModules(uint8_t qrcode[], int version) {
    int qrsize = qrcodegen_getSize(qrcode);
    for (int i = 7; i < qrsize - 7; i += 2) {
        setModule(qrcode, 6, i, false);
        setModule(qrcode, i, 6, false);
    }
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            int dist = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
            if (dist == 2 || dist == 4) {
                setModuleBounded(qrcode, 3+dx, 3+dy, false);
                setModuleBounded(qrcode, qrsize-4+dx, 3+dy, false);
                setModuleBounded(qrcode, 3+dx, qrsize-4+dy, false);
            }
        }
    }
    uint8_t alignPatPos[7];
    int numAlign = getAlignmentPatternPositions(version, alignPatPos);
    for (int i = 0; i < numAlign; i++) {
        for (int j = 0; j < numAlign; j++) {
            if ((i==0&&j==0)||(i==0&&j==numAlign-1)||(i==numAlign-1&&j==0)) continue;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    setModule(qrcode, alignPatPos[i]+dx, alignPatPos[j]+dy, dx==0&&dy==0);
        }
    }
    if (version >= 7) {
        int rem = version;
        for (int i = 0; i < 12; i++) rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
        long bits = (long)version << 12 | rem;
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 3; j++) {
                int k = qrsize - 11 + j;
                setModule(qrcode, k, i, (bits & 1) != 0);
                setModule(qrcode, i, k, (bits & 1) != 0);
                bits >>= 1;
            }
    }
}

static void drawFormatBits(enum qrcodegen_Ecc ecl, enum qrcodegen_Mask mask, uint8_t qrcode[]) {
    static const int table[] = {1, 0, 3, 2};
    int data = table[(int)ecl] << 3 | (int)mask;
    int rem = data;
    for (int i = 0; i < 10; i++) rem = (rem << 1) ^ ((rem >> 9) * 0x537);
    int bits = (data << 10 | rem) ^ 0x5412;
    for (int i = 0; i <= 5; i++) setModule(qrcode, 8, i, getBit(bits, i));
    setModule(qrcode, 8, 7, getBit(bits, 6));
    setModule(qrcode, 8, 8, getBit(bits, 7));
    setModule(qrcode, 7, 8, getBit(bits, 8));
    for (int i = 9; i < 15; i++) setModule(qrcode, 14-i, 8, getBit(bits, i));
    int qrsize = qrcodegen_getSize(qrcode);
    for (int i = 0; i < 8; i++) setModule(qrcode, qrsize-1-i, 8, getBit(bits, i));
    for (int i = 8; i < 15; i++) setModule(qrcode, 8, qrsize-15+i, getBit(bits, i));
    setModule(qrcode, 8, qrsize-8, true);
}

testable int getAlignmentPatternPositions(int version, uint8_t result[7]) {
    if (version == 1) return 0;
    int numAlign = version / 7 + 2;
    int step = (version == 32) ? 26 : (version * 4 + numAlign * 2 + 1) / (numAlign * 2 - 2) * 2;
    for (int i = numAlign-1, pos = version*4+10; i >= 1; i--, pos -= step) result[i] = pos;
    result[0] = 6;
    return numAlign;
}

static void fillRectangle(int left, int top, int width, int height, uint8_t qrcode[]) {
    for (int dy = 0; dy < height; dy++)
        for (int dx = 0; dx < width; dx++)
            setModule(qrcode, left+dx, top+dy, true);
}

/*---- Drawing data modules and masking ----*/

static void drawCodewords(const uint8_t data[], int dataLen, uint8_t qrcode[]) {
    int qrsize = qrcodegen_getSize(qrcode);
    int i = 0;
    for (int right = qrsize-1; right >= 1; right -= 2) {
        if (right == 6) right = 5;
        for (int vert = 0; vert < qrsize; vert++) {
            for (int j = 0; j < 2; j++) {
                int x = right - j;
                bool upward = ((right + 1) & 2) == 0;
                int y = upward ? qrsize-1-vert : vert;
                if (!getModule(qrcode, x, y) && i < dataLen * 8) {
                    setModule(qrcode, x, y, getBit(data[i >> 3], 7 - (i & 7)));
                    i++;
                }
            }
        }
    }
}

static void applyMask(const uint8_t functionModules[], uint8_t qrcode[], enum qrcodegen_Mask mask) {
    int qrsize = qrcodegen_getSize(qrcode);
    for (int y = 0; y < qrsize; y++) {
        for (int x = 0; x < qrsize; x++) {
            if (getModule(functionModules, x, y)) continue;
            bool invert;
            switch ((int)mask) {
                case 0: invert = (x+y)%2==0; break;
                case 1: invert = y%2==0; break;
                case 2: invert = x%3==0; break;
                case 3: invert = (x+y)%3==0; break;
                case 4: invert = (x/3+y/2)%2==0; break;
                case 5: invert = x*y%2+x*y%3==0; break;
                case 6: invert = (x*y%2+x*y%3)%2==0; break;
                case 7: invert = ((x+y)%2+x*y%3)%2==0; break;
                default: return;
            }
            setModule(qrcode, x, y, getModule(qrcode, x, y) ^ invert);
        }
    }
}

static long getPenaltyScore(const uint8_t qrcode[]) {
    int qrsize = qrcodegen_getSize(qrcode);
    long result = 0;
    for (int y = 0; y < qrsize; y++) {
        unsigned char runHistory[7] = {0};
        bool color = false; unsigned char runX = 0;
        for (int x = 0; x < qrsize; x++) {
            if (getModule(qrcode, x, y) == color) {
                runX++; if (runX == 5) result += PENALTY_N1; else if (runX > 5) result++;
            } else {
                addRunToHistory(runX, runHistory);
                if (!color && hasFinderLikePattern(runHistory)) result += PENALTY_N3;
                color = getModule(qrcode, x, y); runX = 1;
            }
        }
        addRunToHistory(runX, runHistory);
        if (color) addRunToHistory(0, runHistory);
        if (hasFinderLikePattern(runHistory)) result += PENALTY_N3;
    }
    for (int x = 0; x < qrsize; x++) {
        unsigned char runHistory[7] = {0};
        bool color = false; unsigned char runY = 0;
        for (int y = 0; y < qrsize; y++) {
            if (getModule(qrcode, x, y) == color) {
                runY++; if (runY == 5) result += PENALTY_N1; else if (runY > 5) result++;
            } else {
                addRunToHistory(runY, runHistory);
                if (!color && hasFinderLikePattern(runHistory)) result += PENALTY_N3;
                color = getModule(qrcode, x, y); runY = 1;
            }
        }
        addRunToHistory(runY, runHistory);
        if (color) addRunToHistory(0, runHistory);
        if (hasFinderLikePattern(runHistory)) result += PENALTY_N3;
    }
    for (int y = 0; y < qrsize-1; y++)
        for (int x = 0; x < qrsize-1; x++) {
            bool c = getModule(qrcode, x, y);
            if (c == getModule(qrcode,x+1,y) && c == getModule(qrcode,x,y+1) && c == getModule(qrcode,x+1,y+1))
                result += PENALTY_N2;
        }
    int black = 0;
    for (int y = 0; y < qrsize; y++)
        for (int x = 0; x < qrsize; x++)
            if (getModule(qrcode, x, y)) black++;
    int total = qrsize * qrsize;
    int k = (int)((labs(black * 20L - total * 10L) + total - 1) / total) - 1;
    result += k * PENALTY_N4;
    return result;
}

static void addRunToHistory(unsigned char run, unsigned char history[7]) {
    memmove(&history[1], &history[0], 6 * sizeof(history[0]));
    history[0] = run;
}

static bool hasFinderLikePattern(const unsigned char h[7]) {
    unsigned char n = h[1];
    return n > 0 && h[2]==n && h[4]==n && h[5]==n && h[3]==n*3 && (h[0]>=n*4 || h[6]>=n*4);
}

/*---- Basic QR Code info ----*/

int qrcodegen_getSize(const uint8_t qrcode[]) { return qrcode[0]; }

bool qrcodegen_getModule(const uint8_t qrcode[], int x, int y) {
    int qrsize = qrcode[0];
    return (0<=x && x<qrsize && 0<=y && y<qrsize) && getModule(qrcode, x, y);
}

testable bool getModule(const uint8_t qrcode[], int x, int y) {
    int qrsize = qrcode[0];
    int index = y * qrsize + x;
    return getBit(qrcode[(index >> 3) + 1], index & 7);
}

testable void setModule(uint8_t qrcode[], int x, int y, bool isBlack) {
    int qrsize = qrcode[0];
    int index = y * qrsize + x;
    int bitIndex = index & 7, byteIndex = (index >> 3) + 1;
    if (isBlack) qrcode[byteIndex] |= 1 << bitIndex;
    else         qrcode[byteIndex] &= (1 << bitIndex) ^ 0xFF;
}

testable void setModuleBounded(uint8_t qrcode[], int x, int y, bool isBlack) {
    int qrsize = qrcode[0];
    if (0<=x && x<qrsize && 0<=y && y<qrsize) setModule(qrcode, x, y, isBlack);
}

static bool getBit(int x, int i) { return ((x >> i) & 1) != 0; }

/*---- Segment handling ----*/

bool qrcodegen_isAlphanumeric(const char *text) {
    for (; *text != '\0'; text++)
        if (strchr(ALPHANUMERIC_CHARSET, *text) == NULL) return false;
    return true;
}

bool qrcodegen_isNumeric(const char *text) {
    for (; *text != '\0'; text++)
        if (*text < '0' || *text > '9') return false;
    return true;
}

size_t qrcodegen_calcSegmentBufferSize(enum qrcodegen_Mode mode, size_t numChars) {
    int temp = calcSegmentBitLength(mode, numChars);
    if (temp == -1) return SIZE_MAX;
    return ((size_t)temp + 7) / 8;
}

testable int calcSegmentBitLength(enum qrcodegen_Mode mode, size_t numChars) {
    if (numChars > (unsigned int)INT16_MAX) return -1;
    long result = (long)numChars;
    if      (mode == qrcodegen_Mode_NUMERIC)      result = (result * 10 + 2) / 3;
    else if (mode == qrcodegen_Mode_ALPHANUMERIC) result = (result * 11 + 1) / 2;
    else if (mode == qrcodegen_Mode_BYTE)         result *= 8;
    else if (mode == qrcodegen_Mode_KANJI)        result *= 13;
    else if (mode == qrcodegen_Mode_ECI && numChars == 0) result = 3 * 8;
    else return -1;
    if ((unsigned int)result > (unsigned int)INT16_MAX) return -1;
    return (int)result;
}

struct qrcodegen_Segment qrcodegen_makeBytes(const uint8_t data[], size_t len, uint8_t buf[]) {
    struct qrcodegen_Segment result;
    result.mode = qrcodegen_Mode_BYTE;
    result.bitLength = calcSegmentBitLength(result.mode, len);
    result.numChars = (int)len;
    if (len > 0) memcpy(buf, data, len * sizeof(buf[0]));
    result.data = buf;
    return result;
}

struct qrcodegen_Segment qrcodegen_makeNumeric(const char *digits, uint8_t buf[]) {
    struct qrcodegen_Segment result;
    size_t len = strlen(digits);
    result.mode = qrcodegen_Mode_NUMERIC;
    int bitLen = calcSegmentBitLength(result.mode, len);
    result.numChars = (int)len;
    if (bitLen > 0) memset(buf, 0, ((size_t)bitLen + 7) / 8 * sizeof(buf[0]));
    result.bitLength = 0;
    unsigned int accumData = 0; int accumCount = 0;
    for (; *digits != '\0'; digits++) {
        accumData = accumData * 10 + (unsigned int)(*digits - '0');
        if (++accumCount == 3) { appendBitsToBuffer(accumData, 10, buf, &result.bitLength); accumData = 0; accumCount = 0; }
    }
    if (accumCount > 0) appendBitsToBuffer(accumData, accumCount * 3 + 1, buf, &result.bitLength);
    result.data = buf;
    return result;
}

struct qrcodegen_Segment qrcodegen_makeAlphanumeric(const char *text, uint8_t buf[]) {
    struct qrcodegen_Segment result;
    size_t len = strlen(text);
    result.mode = qrcodegen_Mode_ALPHANUMERIC;
    int bitLen = calcSegmentBitLength(result.mode, len);
    result.numChars = (int)len;
    if (bitLen > 0) memset(buf, 0, ((size_t)bitLen + 7) / 8 * sizeof(buf[0]));
    result.bitLength = 0;
    unsigned int accumData = 0; int accumCount = 0;
    for (; *text != '\0'; text++) {
        accumData = accumData * 45 + (unsigned int)(strchr(ALPHANUMERIC_CHARSET, *text) - ALPHANUMERIC_CHARSET);
        if (++accumCount == 2) { appendBitsToBuffer(accumData, 11, buf, &result.bitLength); accumData = 0; accumCount = 0; }
    }
    if (accumCount > 0) appendBitsToBuffer(accumData, 6, buf, &result.bitLength);
    result.data = buf;
    return result;
}

testable int getTotalBits(const struct qrcodegen_Segment segs[], size_t len, int version) {
    long result = 0;
    for (size_t i = 0; i < len; i++) {
        int ccbits = numCharCountBits(segs[i].mode, version);
        if (segs[i].numChars >= (1L << ccbits)) return -1;
        result += 4L + ccbits + segs[i].bitLength;
        if (result > INT16_MAX) return -1;
    }
    return (int)result;
}

static int numCharCountBits(enum qrcodegen_Mode mode, int version) {
    int i = (version + 7) / 17;
    switch (mode) {
        case qrcodegen_Mode_NUMERIC:      { static const int t[]={10,12,14}; return t[i]; }
        case qrcodegen_Mode_ALPHANUMERIC: { static const int t[]={ 9,11,13}; return t[i]; }
        case qrcodegen_Mode_BYTE:         { static const int t[]={ 8,16,16}; return t[i]; }
        case qrcodegen_Mode_KANJI:        { static const int t[]={ 8,10,12}; return t[i]; }
        case qrcodegen_Mode_ECI:          return 0;
        default: return -1;
    }
}
