#ifndef __MYMPDECIMAL_H__
#define __MYMPDECIMAL_H__


typedef uint64_t mpd_uint_t;
typedef int64_t mpd_ssize_t;

#define MPD_RDIGITS 19

const mpd_uint_t mpd_pow10[MPD_RDIGITS+1] = {
  1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000,
  10000000000ULL,100000000000ULL,1000000000000ULL,10000000000000ULL,
  100000000000000ULL,1000000000000000ULL,10000000000000000ULL,
  100000000000000000ULL,1000000000000000000ULL,10000000000000000000ULL
};

#define MPD_UINT8_C(x) ((uint8_t)x)

/* mpd_t flags */
#define MPD_POS                 MPD_UINT8_C(0)
#define MPD_NEG                 MPD_UINT8_C(1)
#define MPD_INF                 MPD_UINT8_C(2)
#define MPD_NAN                 MPD_UINT8_C(4)
#define MPD_SNAN                MPD_UINT8_C(8)
#define MPD_SPECIAL (MPD_INF|MPD_NAN|MPD_SNAN)
#define MPD_STATIC              MPD_UINT8_C(16)
#define MPD_STATIC_DATA         MPD_UINT8_C(32)
#define MPD_SHARED_DATA         MPD_UINT8_C(64)
#define MPD_CONST_DATA          MPD_UINT8_C(128)
#define MPD_DATAFLAGS (MPD_STATIC_DATA|MPD_SHARED_DATA|MPD_CONST_DATA)


/* mpd_t */
typedef struct mpd_t {
  uint8_t flags;
  mpd_ssize_t exp;
  mpd_ssize_t digits;
  mpd_ssize_t len;
  mpd_ssize_t alloc;
  mpd_uint_t *data;
} mpd_t;


#define MPD_EXP(mpd) ((mpd)->exp + (mpd)->digits - 1)
#define MPD_MSW(mpd) ((mpd)->data[(mpd)->len-1])
#define MPD_MSWDIG(mpd) ((((mpd)->digits-1) % MPD_RDIGITS) + 1)
#define MPD_IDXDIG(mpd, idx) ((idx == (mpd)->len-1) ? MPD_MSWDIG(mpd) : MPD_RDIGITS)
#define MPD_DIG2LEN(dig) ((dig / MPD_RDIGITS) + !!(dig % MPD_RDIGITS))

#define MPD_ISNEG(mpd) ((mpd)->flags & MPD_NEG)
#define MPD_ISPOS(mpd) !MPD_ISNEG(mpd)
#define MPD_ISINF(mpd) ((mpd)->flags & MPD_INF)
#define MPD_ISSPECIAL(mpd) ((mpd)->flags & MPD_SPECIAL)
#define MPD_ISZERO(mpd) (!MPD_ISSPECIAL(mpd) && (MPD_MSW(mpd) == 0))

// #define MPD_PRINTF(mpd) \
//   printf("\nflags:%02x exp:%lld digits:%lld len:%lld alloc:%lld data[msw]:%llu\n", \
//          mpd->flags, mpd->exp, mpd->digits, mpd->len, mpd->alloc, mpd->data[mpd->len-1]);


int
mpd_static_data_prepare(mpd_t *mpd, const mpd_ssize_t len) {
  assert(mpd->flags & MPD_STATIC_DATA);
  if (len <= mpd->alloc) { return 1; }

  mpd_uint_t *p = mpd->data;
  mpd->data = malloc(len * sizeof(mpd_uint_t));
  if (mpd->data == NULL) {
    mpd->data = p;
    mpd->exp = mpd->digits = mpd->len = 0;
    return 0;
  }

  memcpy(mpd->data, p, mpd->alloc * sizeof(mpd_uint_t));
  mpd->alloc = len;
  mpd->flags &= ~MPD_DATAFLAGS;
  return 1;
}

int
mpd_static_from_base100(mpd_t *mpd, const uint8_t sign, const mpd_ssize_t exp,
                        const uint8_t* bytes, const mpd_ssize_t len) {

  mpd->flags |= sign;
  mpd->digits = len << 1;
  mpd->exp = exp - mpd->digits + 1;
  mpd->len = MPD_DIG2LEN(mpd->digits);
  if (!mpd_static_data_prepare(mpd, mpd->len)) { return 0; }

  uint8_t bytemask = 0-MPD_ISNEG(mpd);
  mpd_uint_t* word = mpd->data + mpd->len - 1; *word = 0;
  mpd_ssize_t worddigits = MPD_MSWDIG(mpd);

  for (mpd_ssize_t idx=0; idx < mpd->digits; idx+=2, bytes++) {

    uint8_t digit100 = (*bytes ^ bytemask) >> 1;

    if (!worddigits) {
      *(--word) = digit100;
      worddigits = MPD_RDIGITS - 2;

    } else if (worddigits == 1) {
      *word = (*word * 10) + digit100/10;
      *(--word) = (digit100 % 10);
      worddigits = MPD_RDIGITS - 1;

    } else {
      *word = (*word * 100) + digit100;
      worddigits -= 2;
    }

  }

  return 1;
}

mpd_ssize_t
mpd_ctz(const mpd_t* mpd) {
  assert(!MPD_ISSPECIAL(mpd) && !MPD_ISZERO(mpd));

  // skip trailing 0 full words
  mpd_ssize_t idx = 0, tz = 0;
  while (!mpd->data[idx]) { idx++; }
  tz += idx * MPD_RDIGITS;

  // count 0s in last word
  mpd_uint_t word = mpd->data[idx];
  while (word % 10 == 0) { word /= 10; tz++; }

  return tz;
}

void
mpd_write_base100(const mpd_t* mpd, const mpd_ssize_t digits, uint8_t* str) {
  assert(digits <= mpd->digits);

  mpd_ssize_t ignore = mpd->digits - digits;
  mpd_ssize_t wordidx = ignore/MPD_RDIGITS;
  mpd_uint_t word = mpd->data[wordidx] / mpd_pow10[ignore % MPD_RDIGITS];
  mpd_ssize_t worddigits = MPD_IDXDIG(mpd, wordidx) - ignore;
  mpd_ssize_t lastbyteidx = (digits-1) >> 1;

  // set bytes to zero since we use addition to add digits
  memset(str, 0, lastbyteidx+1);

  for (mpd_ssize_t idx = digits-1; idx >= 0; idx--, worddigits--) {

    if (!worddigits) {
      word = mpd->data[++wordidx];
      worddigits = MPD_IDXDIG(mpd, wordidx);
    }

    uint8_t digit = word % 10;
    word /= 10;

    if (idx & 1) {
      str[idx >> 1] = digit << 1;
    } else {
      str[idx >> 1] += ((digit * 10) << 1) + 1;
    }
  }

  // Remove continuation bit from last byte
  str[lastbyteidx] ^= 1;

  if (MPD_ISNEG(mpd)) {
    while(lastbyteidx >= 0) { str[lastbyteidx] = ~str[lastbyteidx]; lastbyteidx--; }
  }
}


#endif //__MYMPDECIMAL_H__
