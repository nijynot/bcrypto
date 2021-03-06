/*!
 * ecc.c - elliptic curves for libtorsion
 * Copyright (c) 2020, Christopher Jeffrey (MIT License).
 * https://github.com/bcoin-org/libtorsion
 *
 * Parts of this software are based on indutny/elliptic:
 *   Copyright (c) 2014, Fedor Indutny (MIT License).
 *   https://github.com/indutny/elliptic
 *
 * Formulas from DJB and Tanja Lange [EFD].
 *
 * References:
 *
 *   [GECC] Guide to Elliptic Curve Cryptography
 *     D. Hankerson, A. Menezes, and S. Vanstone
 *     https://tinyurl.com/guide-to-ecc
 *
 *   [GLV] Faster Point Multiplication on Elliptic Curves
 *     R. Gallant, R. Lambert, and S. Vanstone
 *     https://link.springer.com/content/pdf/10.1007/3-540-44647-8_11.pdf
 *
 *   [MONT1] Montgomery curves and the Montgomery ladder
 *     Daniel J. Bernstein, Tanja Lange
 *     https://eprint.iacr.org/2017/293.pdf
 *
 *   [SQUARED] Elligator Squared
 *     Mehdi Tibouchi
 *     https://eprint.iacr.org/2014/043.pdf
 *
 *   [SEC1] SEC 1: Elliptic Curve Cryptography, Version 2.0
 *     Certicom Research
 *     http://www.secg.org/sec1-v2.pdf
 *
 *   [EFD] Explicit-Formulas Database
 *     Daniel J. Bernstein, Tanja Lange
 *     https://hyperelliptic.org/EFD/index.html
 *
 *   [SAFE] SafeCurves: choosing safe curves for elliptic-curve cryptography
 *     Daniel J. Bernstein
 *     https://safecurves.cr.yp.to/
 *
 *   [SSWU1] Efficient Indifferentiable Hashing into Ordinary Elliptic Curves
 *     E. Brier, J. Coron, T. Icart, D. Madore, H. Randriam, M. Tibouchi
 *     https://eprint.iacr.org/2009/340.pdf
 *
 *   [SSWU2] Rational points on certain hyperelliptic curves over finite fields
 *     Maciej Ulas
 *     https://arxiv.org/abs/0706.1448
 *
 *   [H2EC] Hashing to Elliptic Curves
 *     A. Faz-Hernandez, S. Scott, N. Sullivan, R. S. Wahby, C. A. Wood
 *     https://git.io/JeWz6
 *     https://github.com/cfrg/draft-irtf-cfrg-hash-to-curve
 *
 *   [SVDW1] Construction of Rational Points on Elliptic Curves
 *     A. Shallue, C. E. van de Woestijne
 *     https://works.bepress.com/andrew_shallue/1/download/
 *
 *   [SVDW2] Indifferentiable Hashing to Barreto-Naehrig Curves
 *     Pierre-Alain Fouque, Mehdi Tibouchi
 *     https://www.di.ens.fr/~fouque/pub/latincrypt12.pdf
 *
 *   [SVDW3] Covert ECDH over secp256k1
 *     Pieter Wuille
 *     https://gist.github.com/sipa/29118d3fcfac69f9930d57433316c039
 *
 *   [MONT2] Montgomery Curve (wikipedia)
 *     https://en.wikipedia.org/wiki/Montgomery_curve
 *
 *   [SIDE2] Weierstrass Elliptic Curves and Side-Channel Attacks
 *     Eric Brier, Marc Joye
 *     http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.2.273&rep=rep1&type=pdf
 *
 *   [SIDE3] Unified Point Addition Formulae and Side-Channel Attacks
 *     Douglas Stebila, Nicolas Theriault
 *     https://eprint.iacr.org/2005/419.pdf
 *
 *   [MONT3] Montgomery Curves and their arithmetic
 *     C. Costello, B. Smith
 *     https://eprint.iacr.org/2017/212.pdf
 *
 *   [ELL2] Elliptic-curve points indistinguishable from uniform random strings
 *     D. Bernstein, M. Hamburg, A. Krasnova, T. Lange
 *     https://elligator.cr.yp.to/elligator-20130828.pdf
 *
 *   [RFC7748] Elliptic Curves for Security
 *     A. Langley, M. Hamburg, S. Turner
 *     https://tools.ietf.org/html/rfc7748
 *
 *   [TWISTED] Twisted Edwards Curves
 *     D. Bernstein, P. Birkner, M. Joye, T. Lange, C. Peters
 *     https://eprint.iacr.org/2008/013.pdf
 *
 *   [RFC8032] Edwards-Curve Digital Signature Algorithm (EdDSA)
 *     S. Josefsson, SJD AB, I. Liusvaara
 *     https://tools.ietf.org/html/rfc8032
 *
 *   [SCHNORR] Schnorr Signatures for secp256k1
 *     Pieter Wuille
 *     https://github.com/sipa/bips/blob/bip-schnorr/bip-schnorr.mediawiki
 *
 *   [CASH] Schnorr Signature specification
 *     Mark B. Lundeberg
 *     https://github.com/bitcoincashorg/bitcoincash.org/blob/master/spec/2019-05-15-schnorr.md
 *
 *   [JCEN12] Efficient Software Implementation of Public-Key Cryptography
 *            on Sensor Networks Using the MSP430X Microcontroller
 *     C. P. L. Gouvea, L. B. Oliveira, J. Lopez
 *     http://conradoplg.cryptoland.net/files/2010/12/jcen12.pdf
 *
 *   [FIPS186] Federal Information Processing Standards Publication
 *     National Institute of Standards and Technology
 *     https://tinyurl.com/fips-186-3
 *
 *   [FIPS186] Suite B Implementer's Guide to FIPS 186-3 (ECDSA)
 *     https://tinyurl.com/fips186-guide
 *
 *   [RFC6979] Deterministic Usage of the Digital Signature
 *             Algorithm (DSA) and Elliptic Curve Digital
 *             Signature Algorithm (ECDSA)
 *     T. Pornin
 *     https://tools.ietf.org/html/rfc6979
 *
 *   [EDDSA] High-speed high-security signatures
 *     D. J. Bernstein, N. Duif, T. Lange, P. Schwabe, B. Yang
 *     https://ed25519.cr.yp.to/ed25519-20110926.pdf
 *
 *   [RFC8032] Edwards-Curve Digital Signature Algorithm (EdDSA)
 *     S. Josefsson, I. Liusvaara
 *     https://tools.ietf.org/html/rfc8032
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#ifdef TORSION_TEST
#include <stdio.h>
#endif

#include <torsion/drbg.h>
#include <torsion/ecc.h>
#include <torsion/hash.h>
#include <torsion/util.h>

#include "fields/p192.h"
#include "fields/p224.h"
#include "fields/p256.h"
#include "fields/p384.h"
#include "fields/p521.h"
#include "fields/secp256k1.h"
#include "fields/p25519.h"
#include "fields/p448.h"
#include "fields/p251.h"

#include "asn1.h"
#include "mpn.h"

#if CHAR_BIT != 8
#error "sane char widths please"
#endif

#if (-1 & 3) != 3
#error "twos complement please"
#endif

#ifdef TORSION_USE_64BIT
typedef uint64_t fe_word_t;
#define FIELD_WORD_SIZE 64
#define MAX_FIELD_WORDS 9
#else
typedef uint32_t fe_word_t;
#define FIELD_WORD_SIZE 32
#define MAX_FIELD_WORDS 19
#endif

#define MAX_FIELD_BITS 521
#define MAX_FIELD_SIZE 66
#define MAX_FIELD_LIMBS \
  ((MAX_FIELD_BITS + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS)

#define MAX_SCALAR_BITS 521
#define MAX_SCALAR_SIZE 66
#define MAX_SCALAR_LIMBS \
  ((MAX_SCALAR_BITS + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS)
#define MAX_REDUCE_LIMBS ((MAX_SCALAR_LIMBS + 1) * 4)

#define MAX_PUB_SIZE (1 + MAX_FIELD_SIZE * 2)
#define MAX_SIG_SIZE (MAX_FIELD_SIZE + MAX_SCALAR_SIZE)
#define MAX_DER_SIZE (9 + MAX_SIG_SIZE)

#define WND_WIDTH 4
#define WND_SIZE (1 << WND_WIDTH) /* 16 */
#define WND_STEPS(bits) (((bits) + WND_WIDTH - 1) / WND_WIDTH) /* 64 */
#define MAX_WNDS_SIZE (WND_STEPS(MAX_SCALAR_BITS) * WND_SIZE) /* 1024 */

#define NAF_WIDTH 4
#define NAF_SIZE (1 << (NAF_WIDTH - 1)) /* 8 */

#define NAF_WIDTH_PRE 8
#define NAF_SIZE_PRE (1 << (NAF_WIDTH_PRE - 1)) /* 128 */

/*
 * Scalar Field
 */

typedef mp_limb_t sc_t[MAX_SCALAR_LIMBS];

typedef struct _scalar_field_s {
  int endian;
  size_t size;
  size_t bits;
  mp_size_t shift;
  mp_limb_t n[MAX_REDUCE_LIMBS];
  mp_limb_t nh[MAX_REDUCE_LIMBS];
  mp_limb_t m[MAX_REDUCE_LIMBS];
  mp_size_t limbs;
  unsigned char raw[MAX_SCALAR_SIZE];
} scalar_field_t;

typedef struct _scalar_def_s {
  size_t bits;
  const unsigned char n[MAX_FIELD_SIZE];
} scalar_def_t;

/*
 * Prime Field
 */

typedef void fe_add_func(fe_word_t *out1, const fe_word_t *arg1, const fe_word_t *arg2);
typedef void fe_sub_func(fe_word_t *out1, const fe_word_t *arg1, const fe_word_t *arg2);
typedef void fe_opp_func(fe_word_t *out1, const fe_word_t *arg1);
typedef void fe_mul_func(fe_word_t *out1, const fe_word_t *arg1, const fe_word_t *arg2);
typedef void fe_sqr_func(fe_word_t *out1, const fe_word_t *arg1);
typedef void fe_from_montgomery_func(fe_word_t *out1, const fe_word_t *arg1);
typedef void fe_nonzero_func(fe_word_t *out1, const fe_word_t *arg1);
typedef void fe_selectznz_func(fe_word_t *out1, unsigned char arg1, const fe_word_t *arg2, const fe_word_t *arg3);
typedef void fe_to_bytes_func(uint8_t *out1, const fe_word_t *arg1);
typedef void fe_from_bytes_func(fe_word_t *out1, const uint8_t *arg1);
typedef void fe_carry_func(fe_word_t *out1, const fe_word_t *arg1);
typedef void fe_invert_func(fe_word_t *out, const fe_word_t *in);
typedef int fe_sqrt_func(fe_word_t *out, const fe_word_t *in);
typedef int fe_isqrt_func(fe_word_t *out, const fe_word_t *u, const fe_word_t *v);
typedef void fe_scmul_121666(fe_word_t *out1, const fe_word_t *arg1);

typedef fe_word_t fe_t[MAX_FIELD_WORDS];

typedef struct _prime_field_s {
  int endian;
  size_t size;
  size_t bits;
  size_t shift;
  size_t words;
  size_t adj_size;
  mp_limb_t p[MAX_REDUCE_LIMBS];
  mp_size_t limbs;
  mp_size_t adj_limbs;
  unsigned char mask;
  unsigned char raw[MAX_FIELD_SIZE];
  scalar_field_t sc;
  fe_add_func *add;
  fe_sub_func *sub;
  fe_opp_func *opp;
  fe_mul_func *mul;
  fe_sqr_func *square;
  fe_from_montgomery_func *from_montgomery;
  fe_nonzero_func *nonzero;
  fe_selectznz_func *selectznz;
  fe_to_bytes_func *to_bytes;
  fe_from_bytes_func *from_bytes;
  fe_carry_func *carry;
  fe_invert_func *invert;
  fe_sqrt_func *sqrt;
  fe_isqrt_func *isqrt;
  fe_scmul_121666 *scmul_121666;
  fe_t zero;
  fe_t one;
  fe_t two;
  fe_t three;
  fe_t four;
  fe_t mone;
} prime_field_t;

typedef struct _prime_def_s {
  size_t bits;
  size_t words;
  const unsigned char p[MAX_FIELD_SIZE];
  fe_add_func *add;
  fe_sub_func *sub;
  fe_opp_func *opp;
  fe_mul_func *mul;
  fe_sqr_func *square;
  fe_from_montgomery_func *from_montgomery;
  fe_nonzero_func *nonzero;
  fe_selectznz_func *selectznz;
  fe_to_bytes_func *to_bytes;
  fe_from_bytes_func *from_bytes;
  fe_carry_func *carry;
  fe_invert_func *invert;
  fe_sqrt_func *sqrt;
  fe_isqrt_func *isqrt;
  fe_scmul_121666 *scmul_121666;
} prime_def_t;

/*
 * Short Weierstrass
 */

/* wge = weierstrass group element (affine) */
typedef struct _wge_s {
  fe_t x;
  fe_t y;
  int inf;
} wge_t;

/* jge = jacobian group element */
typedef struct _jge_s {
  fe_t x;
  fe_t y;
  fe_t z;
} jge_t;

typedef struct _wei_s {
  int hash;
  prime_field_t fe;
  scalar_field_t sc;
  unsigned int h;
  mp_limb_t pmodn[MAX_REDUCE_LIMBS];
  fe_t red_n;
  fe_t a;
  fe_t b;
  fe_t c;
  fe_t z;
  fe_t ai;
  fe_t zi;
  fe_t i2;
  fe_t i3;
  int zero_a;
  int three_a;
  wge_t g;
  sc_t blind;
  wge_t unblind;
  wge_t windows[MAX_WNDS_SIZE];
  wge_t points[NAF_SIZE_PRE];
  int endo;
  fe_t beta;
  sc_t lambda;
  sc_t b1;
  sc_t b2;
  sc_t g1;
  sc_t g2;
  wge_t endo_points[NAF_SIZE_PRE];
} wei_t;

typedef struct _wei_def_s {
  const char *id;
  int hash;
  const prime_def_t *fe;
  const scalar_def_t *sc;
  const unsigned char a[MAX_FIELD_SIZE];
  const unsigned char b[MAX_FIELD_SIZE];
  unsigned int h;
  int z;
  const unsigned char c[MAX_FIELD_SIZE];
  const unsigned char x[MAX_FIELD_SIZE];
  const unsigned char y[MAX_FIELD_SIZE];
  int endo;
  const unsigned char beta[MAX_FIELD_SIZE];
  const unsigned char lambda[MAX_SCALAR_SIZE];
  const unsigned char b1[MAX_SCALAR_SIZE];
  const unsigned char b2[MAX_SCALAR_SIZE];
  const unsigned char g1[MAX_SCALAR_SIZE];
  const unsigned char g2[MAX_SCALAR_SIZE];
} wei_def_t;

typedef struct _wei_scratch_s {
  jge_t wnd_normal[32 * 4]; /* 27kb */
  wge_t wnd_endo[64 * 4];
  int32_t naf[64 * (MAX_SCALAR_BITS + 1)]; /* 65kb */
  wge_t points[64];
  sc_t coeffs[64];
} wei_scratch_t;

/*
 * Montgomery
 */

typedef void clamp_func(unsigned char *raw);

/* mge = montgomery group element (affine) */
typedef struct _mge_s {
  fe_t x;
  fe_t y;
  int inf;
} mge_t;

/* pge = projective group element (x/z) */
typedef struct _pge_s {
  fe_t x;
  fe_t z;
} pge_t;

typedef struct _mont_s {
  const char *prefix;
  prime_field_t fe;
  scalar_field_t sc;
  unsigned int h;
  fe_t a;
  fe_t b;
  fe_t z;
  int invert;
  fe_t c;
  fe_t bi;
  fe_t i4;
  fe_t a24;
  fe_t a0;
  fe_t b0;
  mge_t g;
  clamp_func *clamp;
} mont_t;

typedef struct _mont_def_s {
  const char *id;
  const char *prefix;
  const prime_def_t *fe;
  const scalar_def_t *sc;
  const unsigned char a[MAX_FIELD_SIZE];
  const unsigned char b[MAX_FIELD_SIZE];
  unsigned int h;
  int z;
  int invert;
  const unsigned char c[MAX_FIELD_SIZE];
  const unsigned char x[MAX_FIELD_SIZE];
  const unsigned char y[MAX_FIELD_SIZE];
  clamp_func *clamp;
} mont_def_t;

/*
 * Edwards
 */

/* xge = extended group element */
typedef struct _xge_s {
  fe_t x;
  fe_t y;
  fe_t z;
  fe_t t;
} xge_t;

typedef struct _edwards_s {
  int hash;
  int context;
  const char *prefix;
  prime_field_t fe;
  scalar_field_t sc;
  unsigned int h;
  fe_t a;
  fe_t d;
  fe_t k;
  fe_t z;
  int invert;
  fe_t c;
  fe_t A;
  fe_t B;
  fe_t Bi;
  fe_t A0;
  fe_t B0;
  int mone_a;
  int one_a;
  xge_t g;
  sc_t blind;
  xge_t unblind;
  xge_t windows[MAX_WNDS_SIZE];
  xge_t points[NAF_SIZE_PRE];
  clamp_func *clamp;
} edwards_t;

typedef struct _edwards_def_s {
  const char *id;
  int hash;
  int context;
  const char *prefix;
  const prime_def_t *fe;
  const scalar_def_t *sc;
  const unsigned char a[MAX_FIELD_SIZE];
  const unsigned char d[MAX_FIELD_SIZE];
  unsigned int h;
  int z;
  int invert;
  const unsigned char c[MAX_FIELD_SIZE];
  const unsigned char x[MAX_FIELD_SIZE];
  const unsigned char y[MAX_FIELD_SIZE];
  clamp_func *clamp;
} edwards_def_t;

typedef struct _edwards_scratch_s {
  xge_t wnd[32 * 4]; /* 36kb */
  int32_t naf[32 * (MAX_SCALAR_BITS + 1)]; /* 65kb */
  xge_t points[64];
  sc_t coeffs[64];
} edwards_scratch_t;

/*
 * Helpers
 */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static uint32_t
bytes_zero(const unsigned char *a, size_t size) {
  /* Compute (a == 0) in constant time. */
  uint32_t z = 0;
  size_t i;

  for (i = 0; i < size; i++)
    z |= (uint32_t)a[i];

  return (z - 1) >> 31;
}

static uint32_t
bytes_equal(const unsigned char *a,
            const unsigned char *b,
            size_t size) {
  /* Compute (a == b) in constant time. */
  uint32_t z = 0;
  size_t i;

  for (i = 0; i < size; i++)
    z |= (uint32_t)a[i] ^ (uint32_t)b[i];

  return (z - 1) >> 31;
}

static uint32_t
bytes_lt(const unsigned char *a,
         const unsigned char *b,
         int size,
         int endian) {
  /* Compute (a < b) in constant time. */
  int le = (endian == -1);
  int i = le ? size - 1 : 0;
  uint32_t eq = 1;
  uint32_t lt = 0;
  uint32_t x, y;

  for (; le ? i >= 0 : i < size; le ? i-- : i++) {
    x = a[i];
    y = b[i];
    lt = ((eq ^ 1) & lt) | (eq & ((x - y) >> 31));
    eq &= ((x ^ y) - 1) >> 31;
  }

  return lt & (eq ^ 1);
}

static uint32_t
bytes_lte(const unsigned char *a,
          const unsigned char *b,
          int size,
          int endian) {
  /* Compute (a <= b) in constant time. */
  int le = (endian == -1);
  int i = le ? size - 1 : 0;
  uint32_t eq = 1;
  uint32_t lt = 0;
  uint32_t x, y;

  for (; le ? i >= 0 : i < size; le ? i-- : i++) {
    x = a[i];
    y = b[i];
    lt = ((eq ^ 1) & lt) | (eq & ((x - y) >> 31));
    eq &= ((x ^ y) - 1) >> 31;
  }

  return lt | eq;
}

#ifndef __has_builtin
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
#define __has_builtin(x) 1
#else
#define __has_builtin(x) 0
#endif
#endif

#if __has_builtin(__builtin_clz)
#define bit_length(x) \
  ((sizeof(unsigned int) * CHAR_BIT - __builtin_clz(x)) * ((x) != 0))
#else
static size_t
bit_length(uint32_t x) {
  /* http://aggregate.org/MAGIC/#Leading%20Zero%20Count */
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x -= (x >> 1) & 0x55555555u;
  x = ((x >> 2) & 0x33333333u) + (x & 0x33333333u);
  x = ((x >> 4) + x) & 0x0f0f0f0fu;
  x += x >> 8;
  x += x >> 16;
  return x & 0x0000003fu;
}
#endif

/*
 * Scalar
 */

static void
sc_reduce(scalar_field_t *sc, sc_t r, const sc_t ap);

static void
fe_export(prime_field_t *fe, unsigned char *raw, const fe_t a);

static void
sc_zero(scalar_field_t *sc, sc_t r) {
  mpn_zero(r, sc->limbs);
}

static void
sc_cleanse(scalar_field_t *sc, sc_t r) {
  mpn_cleanse(r, sc->limbs);
}

static int
sc_import(scalar_field_t *sc, sc_t r, const unsigned char *raw) {
  mpn_import(r, sc->limbs, raw, sc->size, sc->endian);
  return bytes_lt(raw, sc->raw, sc->size, sc->endian);
}

static int
sc_import_weak(scalar_field_t *sc, sc_t r, const unsigned char *raw) {
  /* Weak reduction if we're aligned to 8 bits. */
  const mp_limb_t *np = sc->n;
  mp_size_t nn = sc->limbs;
  mp_limb_t sp[MAX_SCALAR_LIMBS];
  mp_limb_t cy;

  mpn_import(r, sc->limbs, raw, sc->size, sc->endian);

  cy = mpn_sub_n(sp, r, np, nn);

  mpn_cnd_select(cy == 0, r, r, sp, nn);

  cleanse(sp, sizeof(sp));

  return cy != 0;
}

static int
sc_import_strong(scalar_field_t *sc, sc_t r, const unsigned char *raw) {
  /* Otherwise, a full reduction. */
  mp_limb_t rp[MAX_REDUCE_LIMBS];

  mpn_import(rp, sc->shift * 2, raw, sc->size, sc->endian);

  sc_reduce(sc, r, rp);

  cleanse(rp, sizeof(rp));

  return bytes_lt(raw, sc->raw, sc->size, sc->endian);
}

static int
sc_import_reduce(scalar_field_t *sc, sc_t r, const unsigned char *raw) {
  if ((sc->bits & 7) == 0)
    return sc_import_weak(sc, r, raw);
  return sc_import_strong(sc, r, raw);
}

static void
sc_export(scalar_field_t *sc, unsigned char *raw, const sc_t a) {
  mpn_export(raw, sc->size, a, sc->limbs, sc->endian);
}

static void
sc_set(scalar_field_t *sc, sc_t r, const sc_t a) {
  mpn_copyi(r, a, sc->limbs);
}

static void
sc_swap(scalar_field_t *sc, sc_t a, sc_t b, unsigned int flag) {
  mpn_cnd_swap(flag != 0, a, b, sc->limbs);
}

static void
sc_select(scalar_field_t *sc, sc_t r,
          const sc_t a, const sc_t b,
          unsigned int flag) {
  mpn_cnd_select(flag != 0, r, a, b, sc->limbs);
}

#ifdef TORSION_TEST
static void
sc_print(scalar_field_t *sc, const sc_t a) {
  mpn_print(a, sc->limbs, 16);
  printf("\n");
}
#endif

static int
sc_set_fe(scalar_field_t *sc, prime_field_t *fe, sc_t r, const fe_t a) {
  unsigned char tmp[MAX_FIELD_SIZE];
  fe_export(fe, tmp, a);
  return sc_import_reduce(sc, r, tmp);
}

static void
sc_set_word(scalar_field_t *sc, sc_t r, uint32_t word) {
  r[0] = word;
  mpn_zero(r + 1, sc->limbs - 1);
}

static int
sc_equal(scalar_field_t *sc, const sc_t a, const sc_t b) {
  mp_limb_t z = 0;
  mp_size_t i;

  for (i = 0; i < sc->limbs; i++)
    z |= a[i] ^ b[i];

  return z == 0;
}

static int
sc_cmp_var(scalar_field_t *sc, const sc_t a, const sc_t b) {
  return mpn_cmp(a, b, sc->limbs);
}

static int
sc_is_zero(scalar_field_t *sc, const sc_t a) {
  mp_limb_t z = 0;
  mp_size_t i;

  for (i = 0; i < sc->limbs; i++)
    z |= a[i];

  return z == 0;
}

static int
sc_is_high(scalar_field_t *sc, const sc_t a) {
  mp_limb_t tmp[MAX_SCALAR_LIMBS];
  mp_limb_t cy = mpn_sub_n(tmp, a, sc->nh, sc->limbs);

  mpn_cleanse(tmp, sc->limbs);

  return (cy == 0) & (sc_equal(sc, a, sc->nh) ^ 1);
}

static int
sc_is_high_var(scalar_field_t *sc, const sc_t a) {
  return sc_cmp_var(sc, a, sc->nh) > 0;
}

static void
sc_neg(scalar_field_t *sc, sc_t r, const sc_t a) {
  const mp_limb_t *np = sc->n;
  mp_size_t nn = sc->limbs;
  mp_limb_t zero = sc_is_zero(sc, a);
  mp_size_t i;

  mpn_sub_n(r, np, a, nn);

  for (i = 0; i < nn; i++)
    r[i] &= -(zero ^ 1);
}

static void
sc_neg_cond(scalar_field_t *sc, sc_t r, const sc_t a, unsigned int flag) {
  sc_t m;
  sc_neg(sc, m, a);
  sc_select(sc, r, a, m, flag);
  sc_cleanse(sc, m);
}

static void
sc_add(scalar_field_t *sc, sc_t r, const sc_t ap, const sc_t bp) {
  const mp_limb_t *np = sc->n;
  mp_size_t nn = sc->limbs + 1;
  mp_limb_t up[MAX_SCALAR_LIMBS + 1];
  mp_limb_t vp[MAX_SCALAR_LIMBS + 1];
  mp_limb_t cy;

  assert(np[nn - 1] == 0);

  mpn_copyi(up, ap, sc->limbs);
  mpn_copyi(vp, bp, sc->limbs);

  up[nn - 1] = 0;
  vp[nn - 1] = 0;

  /* r = a + b */
  mpn_add_n(up, up, vp, nn);

  /* r = r - n if u >= n */
  cy = mpn_sub_n(vp, up, np, nn);
  mpn_cnd_select(cy == 0, r, up, vp, sc->limbs);
}

static void
sc_sub(scalar_field_t *sc, sc_t r, const sc_t a, const sc_t b) {
  sc_t mb;
  sc_neg(sc, mb, b);
  sc_add(sc, r, a, mb);
  sc_cleanse(sc, mb);
}

static void
sc_mul_word(scalar_field_t *sc, sc_t r, const sc_t a, unsigned int word) {
  /* Only constant-time if `word` is constant. */
  int bits = bit_length(word);
  int i;

  if (word > 1 && (word & (word - 1)) == 0) {
    sc_add(sc, r, a, a);

    for (i = 1; i < bits - 1; i++)
      sc_add(sc, r, r, r);
  } else {
    sc_t c;

    sc_set(sc, c, a);
    sc_zero(sc, r);

    for (i = bits - 1; i >= 0; i--) {
      if (i != bits - 1)
        sc_add(sc, r, r, r);

      if ((word >> i) & 1)
        sc_add(sc, r, r, c);
    }
  }
}

static void
sc_reduce(scalar_field_t *sc, sc_t r, const mp_limb_t *ap) {
  /* Barrett reduction. */
  const mp_limb_t *np = sc->n;
  const mp_limb_t *mp = sc->m;
  mp_size_t sh = sc->shift;
  mp_limb_t qp[MAX_REDUCE_LIMBS];
  mp_limb_t up[MAX_REDUCE_LIMBS];
  mp_limb_t *hp = qp;
  mp_limb_t cy;

  /* q = a * m */
  mpn_mul_n(qp, ap, mp, sh);

  /* h = q >> k */
  hp += sh;

  /* u = a - h * n */
  mpn_mul_n(up, hp, np, sh);
  cy = mpn_sub_n(up, ap, up, sh * 2);
  assert(cy == 0);

  /* u = u - n if u >= n */
  cy = mpn_sub_n(qp, up, np, sh);
  mpn_cnd_select(cy == 0, r, up, qp, sc->limbs);
}

static void
sc_mul(scalar_field_t *sc, sc_t r, const sc_t a, const sc_t b) {
  mp_limb_t ap[MAX_REDUCE_LIMBS];
  mp_size_t an = sc->limbs * 2;

  mpn_zero(ap + an, sc->shift * 2 - an);
  mpn_mul_n(ap, a, b, sc->limbs);

  sc_reduce(sc, r, ap);
}

static void
sc_sqr(scalar_field_t *sc, sc_t r, const sc_t a) {
  mp_limb_t ap[MAX_REDUCE_LIMBS];
  mp_size_t an = sc->limbs * 2;

  mpn_zero(ap + an, sc->shift * 2 - an);
  mpn_sqr(ap, a, sc->limbs);

  sc_reduce(sc, r, ap);
}

static void
sc_mulshift(scalar_field_t *sc, sc_t r,
            const sc_t a, const sc_t b,
            size_t shift) {
  mp_limb_t scratch[MAX_SCALAR_LIMBS * 2 + 1];
  mp_limb_t *rp = scratch;
  mp_size_t rn = sc->limbs * 2;
  mp_size_t nn = sc->limbs;
  mp_size_t i = shift - 1;
  mp_size_t limbs = shift / GMP_NUMB_BITS;
  mp_size_t left = shift % GMP_NUMB_BITS;
  mp_limb_t bit;

  assert(shift > sc->bits);

  mpn_mul_n(rp, a, b, nn);
  rp[rn] = 0;

  bit = rp[i / GMP_NUMB_BITS] >> (i % GMP_NUMB_BITS);

  rp += limbs;
  rn -= limbs;

  assert(rn > 0);

  if (left != 0) {
    mpn_rshift(rp, rp, rn, left);
    rn -= (rp[rn - 1] == 0);
  }

  rn += 1;
  mpn_add_1(rp, rp, rn, bit & 1);
  rn -= (rp[rn - 1] == 0);

  assert(rn <= nn);

  mpn_zero(r + rn, nn - rn);
  mpn_copyi(r, rp, rn);

  mpn_cleanse(scratch, ARRAY_SIZE(scratch));
}

static int
sc_invert_var(scalar_field_t *sc, sc_t r, const sc_t a) {
  return mpn_invert_n(r, a, sc->n, sc->limbs);
}

static void
sc_pow(scalar_field_t *sc, sc_t r, const sc_t a, const sc_t e) {
  /* Used for inversion if not available otherwise. */
  mp_size_t i;
  sc_t b;

  sc_set_word(sc, b, 1);

  for (i = sc->bits - 1; i >= 0; i--) {
    mp_limb_t bit = e[i / GMP_NUMB_BITS] >> (i % GMP_NUMB_BITS);

    sc_sqr(sc, b, b);

    if (bit & 1)
      sc_mul(sc, b, b, a);
  }

  sc_set(sc, r, b);
}

static int
sc_invert(scalar_field_t *sc, sc_t r, const sc_t a) {
  int ret = sc_is_zero(sc, a) ^ 1;
  sc_t e;

#ifdef TORSION_TEST
  sc_t a0;
  sc_set(sc, a0, a);
#endif

  /* e = n - 2 */
  mpn_copyi(e, sc->n, sc->limbs);
  mpn_sub_1(e, e, sc->limbs, 2);

  sc_pow(sc, r, a, e);

#ifdef TORSION_TEST
  assert(sc_invert_var(sc, a0, a0) == ret);
  assert(sc_equal(sc, r, a0));
#endif

  return ret;
}

static size_t
sc_bitlen_var(scalar_field_t *sc, const sc_t a) {
  return mpn_bitlen(a, sc->limbs);
}

static mp_limb_t
sc_get_bits(scalar_field_t *sc, const sc_t a, size_t i, size_t w) {
  mp_limb_t mask = ((mp_limb_t)1 << w) - 1;
  return (a[i / GMP_NUMB_BITS] >> (i % GMP_NUMB_BITS)) & mask;
}

static mp_limb_t
sc_get_bit(scalar_field_t *sc, const sc_t k, size_t i) {
  return (k[i / GMP_NUMB_BITS] >> (i % GMP_NUMB_BITS)) & 1;
}

static size_t
sc_maxlen_var(scalar_field_t *sc,
              const sc_t k1, const sc_t k2,
              const sc_t k3, const sc_t k4) {
  size_t len1 = sc_bitlen_var(sc, k1);
  size_t len2 = sc_bitlen_var(sc, k2);
  size_t len3 = sc_bitlen_var(sc, k3);
  size_t len4 = sc_bitlen_var(sc, k4);
  size_t max = 0;

  if (len1 > max)
    max = len1;

  if (len2 > max)
    max = len2;

  if (len3 > max)
    max = len3;

  if (len4 > max)
    max = len4;

  return max;
}

static int
sc_minimize(scalar_field_t *sc, sc_t r, const sc_t a) {
  int high = sc_is_high(sc, a);
  sc_neg_cond(sc, r, a, high);
  return high;
}

static void
sc_naf_var(scalar_field_t *sc,
           int32_t *naf,
           const sc_t x,
           int32_t sign,
           size_t width,
           size_t max) {
  /* Computing the width-w NAF of a positive integer.
   *
   * [GECC] Algorithm 3.35, Page 100, Section 3.3.
   */
  mp_limb_t k[MAX_SCALAR_LIMBS + 2];
  mp_size_t nn = sc->limbs;
  mp_size_t kn = sc->limbs;
  mp_limb_t cy;
  int32_t pow = 1 << (width + 1);
  size_t i = 0;
  int32_t z;

  mpn_copyi(k, x, nn);

  k[nn] = 0;

  while (kn > 0 && k[kn - 1] == 0)
    kn -= 1;

  while (kn > 0) {
    z = 0;

    if (k[0] & 1) {
      z = k[0] & (pow - 1);

      if (z & (pow >> 1))
        z -= pow;

      if (z < 0) {
        kn += 1;
        cy = mpn_add_1(k, k, kn, -z);
      } else {
        cy = mpn_sub_1(k, k, kn, z);
      }

      assert(kn <= nn + 1);
      assert(cy == 0);

      kn -= (k[kn - 1] == 0);
    }

    naf[i++] = z * sign;

    if (kn > 0) {
      mpn_rshift(k, k, kn, 1);
      kn -= (k[kn - 1] == 0);
    }
  }

  assert(i <= max);

  for (; i < max; i++)
    naf[i] = 0;
}

static void
sc_jsf_var(scalar_field_t *sc,
           int32_t *naf,
           const sc_t x1,
           int32_t s1,
           const sc_t x2,
           int32_t s2,
           size_t max) {
  /* Joint sparse form.
   *
   * [GECC] Algorithm 3.50, Page 111, Section 3.3.
   */
  mp_limb_t k1[MAX_SCALAR_LIMBS];
  mp_limb_t k2[MAX_SCALAR_LIMBS];
  mp_size_t nn = sc->limbs;
  mp_size_t n1 = sc->limbs;
  mp_size_t n2 = sc->limbs;
  size_t i = 0;
  int32_t d1 = 0;
  int32_t d2 = 0;

  /* JSF->NAF conversion table. */
  static const int32_t table[9] = {
    -3, /* -1 -1 */
    -1, /* -1 0 */
    -5, /* -1 1 */
    -7, /* 0 -1 */
    0, /* 0 0 */
    7, /* 0 1 */
    5, /* 1 -1 */
    1, /* 1 0 */
    3  /* 1 1 */
  };

  mpn_copyi(k1, x1, nn);
  mpn_copyi(k2, x2, nn);

  while (n1 > 0 && k1[n1 - 1] == 0)
    n1 -= 1;

  while (n2 > 0 && k2[n2 - 1] == 0)
    n2 -= 1;

  while (mpn_cmp_limb(k1, n1, -d1) > 0
      || mpn_cmp_limb(k2, n2, -d2) > 0) {
    /* First phase. */
    int32_t m14 = ((k1[0] & 3) + d1) & 3;
    int32_t m24 = ((k2[0] & 3) + d2) & 3;
    int32_t u1 = 0;
    int32_t u2 = 0;

    if (m14 == 3)
      m14 = -1;

    if (m24 == 3)
      m24 = -1;

    if (m14 & 1) {
      int32_t m8 = ((k1[0] & 7) + d1) & 7;

      if ((m8 == 3 || m8 == 5) && m24 == 2)
        u1 = -m14;
      else
        u1 = m14;
    }

    if (m24 & 1) {
      int32_t m8 = ((k2[0] & 7) + d2) & 7;

      if ((m8 == 3 || m8 == 5) && m14 == 2)
        u2 = -m24;
      else
        u2 = m24;
    }

    /* JSF -> NAF conversion. */
    naf[i] = table[(u1 * s1 + 1) * 3 + (u2 * s2 + 1)];

    /* Second phase. */
    if (2 * d1 == u1 + 1)
      d1 = 1 - d1;

    if (2 * d2 == u2 + 1)
      d2 = 1 - d2;

    if (n1 > 0) {
      mpn_rshift(k1, k1, n1, 1);
      n1 -= (k1[n1 - 1] == 0);
    }

    if (n2 > 0) {
      mpn_rshift(k2, k2, n2, 1);
      n2 -= (k2[n2 - 1] == 0);
    }

    i += 1;
  }

  assert(i <= max);

  for (; i < max; i++)
    naf[i] = 0;
}

static void
sc_random(scalar_field_t *sc, sc_t k, drbg_t *rng) {
  unsigned char bytes[MAX_SCALAR_SIZE];

  for (;;) {
    drbg_generate(rng, bytes, sc->size);

    if (!sc_import(sc, k, bytes))
      continue;

    if (sc_is_zero(sc, k))
      continue;

    break;
  }

  cleanse(bytes, sizeof(bytes));
}

/*
 * Field Element
 */

static void
fe_zero(prime_field_t *fe, fe_t r) {
  memset(r, 0, fe->words * sizeof(fe_word_t));
}

static void
fe_cleanse(prime_field_t *fe, fe_t r) {
  cleanse(r, fe->words * sizeof(fe_word_t));
}

static int
fe_import(prime_field_t *fe, fe_t r, const unsigned char *raw) {
  if (fe->from_montgomery) {
    /* Use a constant time barrett reduction
     * to montgomerize the field element.
     */
    mp_limb_t xp[MAX_REDUCE_LIMBS];
    mp_size_t shift = fe->shift / GMP_NUMB_BITS;
    mp_size_t left = fe->shift % GMP_NUMB_BITS;
    mp_size_t xn = fe->limbs + shift + (left != 0);

    /* We can only handle 2*(max+1) limbs. */
    assert(xn <= fe->sc.shift);

    /* x = (x << shift) mod p */
    mpn_zero(xp, fe->sc.shift * 2);
    mpn_import(xp + shift, fe->limbs, raw, fe->size, fe->endian);

    /* Align if necessary. */
    if (left != 0)
      assert(mpn_lshift(xp, xp, xn, left) == 0);

    sc_reduce(&fe->sc, xp, xp);

    if (GMP_NUMB_BITS == FIELD_WORD_SIZE) {
      /* Import directly. */
      assert(sizeof(mp_limb_t) == sizeof(fe_word_t));
      assert((size_t)fe->limbs == fe->words);
      memcpy(r, xp, fe->limbs * sizeof(mp_limb_t));
    } else {
      /* Export as little endian. */
      unsigned char tmp[MAX_FIELD_SIZE];
      mpn_export_le(tmp, fe->size, xp, fe->limbs);
      fe->from_bytes(r, tmp);
    }
  } else {
    if (fe->endian == 1) {
      unsigned char tmp[MAX_FIELD_SIZE];
      size_t i;

      /* Swap endianness. */
      for (i = 0; i < fe->size; i++)
        tmp[i] = raw[fe->size - 1 - i];

      fe->from_bytes(r, tmp);
    } else {
      fe->from_bytes(r, raw);
    }
  }

  return bytes_lt(raw, fe->raw, fe->size, fe->endian);
}

static int
fe_import_be(prime_field_t *fe, fe_t r, const unsigned char *raw) {
  if (fe->endian == -1) {
    unsigned char tmp[MAX_FIELD_SIZE];
    size_t i;

    for (i = 0; i < fe->size; i++)
      tmp[i] = raw[fe->size - 1 - i];

    return fe_import(fe, r, tmp);
  }

  return fe_import(fe, r, raw);
}

static int
fe_import_uniform(prime_field_t *fe, fe_t r, const unsigned char *raw) {
  int ret;

  if (fe->mask != 0xff) {
    unsigned char tmp[MAX_FIELD_SIZE];

    memcpy(tmp, raw, fe->size);

    if (fe->endian == -1)
      tmp[fe->size - 1] &= fe->mask;
    else
      tmp[0] &= fe->mask;

    ret = fe_import(fe, r, tmp);
  } else {
    ret = fe_import(fe, r, raw);
  }

  if (fe->carry)
    fe->carry(r, r);

  return ret;
}

static void
fe_export(prime_field_t *fe, unsigned char *raw, const fe_t a) {
  if (fe->from_montgomery) {
    fe_t tmp;

    /* Demontgomerize. */
    fe->from_montgomery(tmp, a);

    if (fe->size != fe->words * (FIELD_WORD_SIZE / 8)) {
      /* Fiat accepts bytes serialized as full
       * words. In particular, this affects the
       * P224 64 bit backend. This is a non-issue
       * during deserialization as fiat will zero
       * the remaining limbs.
       */
      unsigned char buf[MAX_FIELD_SIZE];

      assert(fe->bits == 224);
      assert(FIELD_WORD_SIZE == 64);

      fe->to_bytes(buf, tmp);
      memcpy(raw, buf, fe->size);
    } else {
      fe->to_bytes(raw, tmp);
    }
  } else {
    fe->to_bytes(raw, a);
  }

  if (fe->endian == 1) {
    int i = 0;
    int j = fe->size - 1;

    while (i < j) {
      unsigned char t = raw[j];

      raw[j] = raw[i];
      raw[i] = t;

      i += 1;
      j -= 1;
    }
  }
}

static void
fe_swap(prime_field_t *fe, fe_t a, fe_t b, unsigned int flag) {
  fe_word_t cond = (flag != 0);
  fe_word_t mask = -cond;
  size_t i;

  for (i = 0; i < fe->words; i++) {
    fe_word_t word = (a[i] ^ b[i]) & mask;

    a[i] ^= word;
    b[i] ^= word;
  }
}

static void
fe_select(prime_field_t *fe, fe_t r,
          const fe_t a, const fe_t b,
          unsigned int flag) {
  fe->selectznz(r, flag != 0, a, b);
}

static void
fe_set(prime_field_t *fe, fe_t r, const fe_t a) {
  memcpy(r, a, sizeof(fe_t));
}

static int
fe_set_limbs(prime_field_t *fe, fe_t r, const mp_limb_t *p, mp_size_t n) {
  unsigned char tmp[MAX_FIELD_SIZE];

  assert(n <= fe->limbs);

  mpn_export(tmp, fe->size, p, n, fe->endian);

  return fe_import(fe, r, tmp);
}

static void
fe_get_limbs(prime_field_t *fe, mp_limb_t *r, const fe_t a) {
  unsigned char tmp[MAX_FIELD_SIZE];

  fe_export(fe, tmp, a);

  mpn_import(r, fe->limbs, tmp, fe->size, fe->endian);
}

#ifdef TORSION_TEST
static void
fe_print(prime_field_t *fe, const fe_t a) {
  mp_limb_t xp[MAX_FIELD_LIMBS];

  fe_get_limbs(fe, xp, a);

  mpn_print(xp, fe->limbs, 16);
  printf("\n");
}
#endif

static int
fe_set_sc(prime_field_t *fe, scalar_field_t *sc, fe_t r, const sc_t a) {
  unsigned char tmp[MAX_SCALAR_SIZE];
  int ret;

  sc_export(sc, tmp, a);

  ret = fe_import(fe, r, tmp);

  cleanse(tmp, sizeof(tmp));

  return ret;
}

static void
fe_set_word(prime_field_t *fe, fe_t r, uint32_t word) {
  if (fe->from_montgomery) {
    unsigned char tmp[MAX_FIELD_SIZE];

    memset(tmp, 0x00, sizeof(tmp));

    if (fe->endian == 1) {
      tmp[fe->size - 4] = (word >> 24) & 0xff;
      tmp[fe->size - 3] = (word >> 16) & 0xff;
      tmp[fe->size - 2] = (word >> 8) & 0xff;
      tmp[fe->size - 1] = word & 0xff;
    } else {
      tmp[0] = word & 0xff;
      tmp[1] = (word >> 8) & 0xff;
      tmp[2] = (word >> 16) & 0xff;
      tmp[3] = (word >> 24) & 0xff;
    }

    fe_import(fe, r, tmp);
  } else {
    /* Note: the limit of the word size here depends
     * on how saturated the field implementation is.
     */
    fe_zero(fe, r);
    r[0] = word;
  }
}

static int
fe_is_zero(prime_field_t *fe, const fe_t a) {
  fe_word_t z = 0;

  if (fe->nonzero) {
    fe->nonzero(&z, a);
  } else {
    unsigned char tmp[MAX_FIELD_SIZE];
    size_t i;

    fe->to_bytes(tmp, a);

    for (i = 0; i < fe->size; i++)
      z |= (fe_word_t)tmp[i];
  }

  return z == 0;
}

static int
fe_equal(prime_field_t *fe, const fe_t a, const fe_t b) {
  fe_t c;

  fe->sub(c, a, b);

  if (fe->carry)
    fe->carry(c, c);

  return fe_is_zero(fe, c);
}

static int
fe_is_odd(prime_field_t *fe, const fe_t a) {
  int sign;

  if (fe->from_montgomery) {
    fe_t tmp;
    fe->from_montgomery(tmp, a);
    sign = tmp[0] & 1;
  } else {
    unsigned char tmp[MAX_FIELD_SIZE];
    fe->to_bytes(tmp, a);
    sign = tmp[0] & 1;
  }

  return sign;
}

static void
fe_neg(prime_field_t *fe, fe_t r, const fe_t a) {
  fe->opp(r, a);

  if (fe->carry)
    fe->carry(r, r);
}

static void
fe_neg_cond(prime_field_t *fe, fe_t r, const fe_t a, unsigned int flag) {
  fe_t b;
  fe_neg(fe, b, a);
  fe_select(fe, r, a, b, flag);
}

static void
fe_set_odd(prime_field_t *fe, fe_t r, const fe_t a, unsigned int odd) {
  fe_neg_cond(fe, r, a, fe_is_odd(fe, a) ^ (odd != 0));
}

static void
fe_add(prime_field_t *fe, fe_t r, const fe_t a, const fe_t b) {
  fe->add(r, a, b);

  if (fe->carry)
    fe->carry(r, r);
}

static void
fe_sub(prime_field_t *fe, fe_t r, const fe_t a, const fe_t b) {
  fe->sub(r, a, b);

  if (fe->carry)
    fe->carry(r, r);
}

static void
fe_mul_word(prime_field_t *fe, fe_t r, const fe_t a, unsigned int word) {
  /* Only constant-time if `word` is constant. */
  int bits = bit_length(word);
  int i;

  if (word > 1 && (word & (word - 1)) == 0) {
    fe_add(fe, r, a, a);

    for (i = 1; i < bits - 1; i++)
      fe_add(fe, r, r, r);
  } else {
    fe_t c;

    fe_set(fe, c, a);
    fe_zero(fe, r);

    for (i = bits - 1; i >= 0; i--) {
      if (i != bits - 1)
        fe_add(fe, r, r, r);

      if ((word >> i) & 1)
        fe_add(fe, r, r, c);
    }
  }
}

static void
fe_mul(prime_field_t *fe, fe_t r, const fe_t a, const fe_t b) {
  fe->mul(r, a, b);
}

static void
fe_sqr(prime_field_t *fe, fe_t r, const fe_t a) {
  fe->square(r, a);
}

static void
fe_mul121666(prime_field_t *fe, fe_t r, const fe_t a) {
  assert(fe->scmul_121666 != NULL);
  fe->scmul_121666(r, a);
}

static void
fe_pow(prime_field_t *fe, fe_t r, const fe_t a, const mp_limb_t *e) {
  /* Used for inversion and legendre if not available otherwise. */
  mp_size_t i;
  fe_t b;

  fe_set(fe, b, fe->one);

  for (i = fe->bits - 1; i >= 0; i--) {
    mp_limb_t bit = e[i / GMP_NUMB_BITS] >> (i % GMP_NUMB_BITS);

    fe_sqr(fe, b, b);

    if (bit & 1)
      fe_mul(fe, b, b, a);
  }

  fe_set(fe, r, b);
}

static int
fe_invert_var(prime_field_t *fe, fe_t r, const fe_t a) {
  mp_limb_t rp[MAX_FIELD_LIMBS];
  int ret;

  fe_get_limbs(fe, rp, a);

  ret = mpn_invert_n(rp, rp, fe->p, fe->limbs);

  assert(fe_set_limbs(fe, r, rp, fe->limbs));

  return ret;
}

static int
fe_invert(prime_field_t *fe, fe_t r, const fe_t a) {
  int ret = fe_is_zero(fe, a) ^ 1;

#ifdef TORSION_TEST
  fe_t a0;
  fe_set(fe, a0, a);
#endif

  if (fe->invert) {
    /* Fast inversion chain. */
    fe->invert(r, a);
  } else {
    /* Fermat's little theorem. */
    mp_limb_t e[MAX_FIELD_LIMBS];

    /* e = p - 2 */
    mpn_copyi(e, fe->p, fe->limbs);
    mpn_sub_1(e, e, fe->limbs, 2);

    fe_pow(fe, r, a, e);
  }

#ifdef TORSION_TEST
  assert(fe_invert_var(fe, a0, a0) == ret);
  assert(fe_equal(fe, r, a0));
#endif

  return ret;
}

static int
fe_sqrt(prime_field_t *fe, fe_t r, const fe_t a) {
  int ret;

  if (fe->sqrt) {
    /* Fast square root chain. */
    ret = fe->sqrt(r, a);
  } else {
    /* Handle p = 3 mod 4 and p = 5 mod 8. */
    mp_limb_t e[MAX_FIELD_LIMBS + 1];
    fe_t b, b2;

    if ((fe->p[0] & 3) == 3) {
      /* b = a^((p + 1) / 4) mod p */
      mpn_copyi(e, fe->p, fe->limbs + 1);
      mpn_add_1(e, e, fe->limbs + 1, 1);
      mpn_rshift(e, e, fe->limbs + 1, 2);
      fe_pow(fe, b, a, e);
    } else if ((fe->p[0] & 7) == 5) {
      fe_t a2, c;

      /* a2 = a * 2 mod p */
      fe_add(fe, a2, a, a);

      /* c = a2^((p - 5) / 8) mod p */
      mpn_copyi(e, fe->p, fe->limbs);
      mpn_sub_1(e, e, fe->limbs, 5);
      mpn_rshift(e, e, fe->limbs, 3);
      fe_pow(fe, c, a2, e);

      /* b = (c^2 * a2 - 1) * a * c mod p */
      fe_sqr(fe, b, c);
      fe_mul(fe, b, b, a2);
      fe_sub(fe, b, b, fe->one);
      fe_mul(fe, b, b, a);
      fe_mul(fe, b, b, c);
    } else {
      assert(0 && "no sqrt implementation");
    }

    /* b2 = b^2 mod p */
    fe_sqr(fe, b2, b);

    ret = fe_equal(fe, b2, a);

    fe_set(fe, r, b);
  }

  return ret;
}

static int
fe_is_square_var(prime_field_t *fe, const fe_t a) {
  mp_limb_t ap[MAX_FIELD_LIMBS];
  mpz_t an, pn;

  fe_get_limbs(fe, ap, a);

  mpz_roinit_n(an, ap, fe->limbs);
  mpz_roinit_n(pn, fe->p, fe->limbs);

  return mpz_jacobi(an, pn) >= 0;
}

static int
fe_is_square(prime_field_t *fe, const fe_t a) {
  int ret;

  if (fe->sqrt && fe->bits != 224) {
    /* Fast square root chain. */
    fe_t tmp;
    ret = fe->sqrt(tmp, a);
  } else {
    /* Euler's criterion. */
    mp_limb_t e[MAX_FIELD_LIMBS];
    int x, y, z;
    fe_t b;

    /* e = (p - 1) / 2 */
    mpn_copyi(e, fe->p, fe->limbs);
    mpn_sub_1(e, e, fe->limbs, 1);
    mpn_rshift(e, e, fe->limbs, 1);

    fe_pow(fe, b, a, e);

    x = fe_is_zero(fe, a);
    y = fe_equal(fe, b, fe->one);
    z = fe_equal(fe, b, fe->mone);

    assert(x + y + z == 1);

    ret = x | y;
  }

  return ret;
}

static int
fe_isqrt(prime_field_t *fe, fe_t r, const fe_t u, const fe_t v) {
  int ret = 1;

  if (fe->isqrt) {
    /* Fast inverse square root chain. */
    ret &= fe->isqrt(r, u, v);
  } else {
    fe_t z;

    ret &= fe_invert(fe, z, v);

    fe_mul(fe, z, z, u);

    ret &= fe_sqrt(fe, r, z);
  }

  return ret;
}

static void
fe_random(prime_field_t *fe, fe_t x, drbg_t *rng) {
  unsigned char bytes[MAX_FIELD_SIZE];

  for (;;) {
    drbg_generate(rng, bytes, fe->size);

    if (!fe_import(fe, x, bytes))
      continue;

    if (fe_is_zero(fe, x))
      continue;

    break;
  }

  cleanse(bytes, sizeof(bytes));
}

/*
 * Scalar Field
 */

static void
scalar_field_set(scalar_field_t *sc,
                 const unsigned char *modulus,
                 size_t bits,
                 int endian) {
  sc->endian = endian;
  sc->limbs = (bits + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
  sc->size = (bits + 7) / 8;
  sc->bits = bits;
  sc->shift = (sc->limbs + 1) * 2;

  mpn_import_be(sc->n, ARRAY_SIZE(sc->n), modulus, sc->size);

  mpn_rshift(sc->nh, sc->n, ARRAY_SIZE(sc->n), 1);

  /* Compute the barrett reduction constant `m`:
   *
   *   m = (1 << (bits * 2)) / n
   */
  {
    mp_limb_t x[(MAX_SCALAR_LIMBS + 1) * 2 + 1];

    mpn_zero(sc->m, ARRAY_SIZE(sc->m));
    mpn_zero(x, ARRAY_SIZE(x));

    x[sc->shift] = 1;

    mpn_tdiv_qr(sc->m, x, 0, x, sc->shift + 1, sc->n, sc->limbs);
  }

  mpn_export(sc->raw, sc->size, sc->n, ARRAY_SIZE(sc->n), sc->endian);
}

static void
scalar_field_init(scalar_field_t *sc, const scalar_def_t *def, int endian) {
  scalar_field_set(sc, def->n, def->bits, endian);
}

/*
 * Prime Field
 */

static void
prime_field_init(prime_field_t *fe, const prime_def_t *def, int endian) {
  fe->endian = endian;
  fe->limbs = (def->bits + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
  fe->size = (def->bits + 7) / 8;
  fe->bits = def->bits;
  fe->shift = def->bits;
  fe->words = def->words;
  fe->adj_size = fe->size + ((fe->bits & 7) == 0);
  fe->adj_limbs = ((fe->adj_size * 8) + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
  fe->mask = 0xff;

  if ((fe->shift % FIELD_WORD_SIZE) != 0)
    fe->shift += FIELD_WORD_SIZE - (fe->shift % FIELD_WORD_SIZE);

  if ((fe->bits & 7) != 0) {
    unsigned int ignore = fe->size * 8 - fe->bits;
    unsigned int mask = (1 << (8 - ignore)) - 1;

    fe->mask = mask;
  }

  mpn_import_be(fe->p, ARRAY_SIZE(fe->p), def->p, fe->size);

  mpn_export(fe->raw, fe->size, fe->p, ARRAY_SIZE(fe->p), fe->endian);

  scalar_field_set(&fe->sc, def->p, def->bits, endian);

  fe->add = def->add;
  fe->sub = def->sub;
  fe->opp = def->opp;
  fe->mul = def->mul;
  fe->square = def->square;
  fe->from_montgomery = def->from_montgomery;
  fe->nonzero = def->nonzero;
  fe->selectznz = def->selectznz;
  fe->to_bytes = def->to_bytes;
  fe->from_bytes = def->from_bytes;
  fe->carry = def->carry;
  fe->invert = def->invert;
  fe->sqrt = def->sqrt;
  fe->isqrt = def->isqrt;
  fe->scmul_121666 = def->scmul_121666;

  fe_set_word(fe, fe->zero, 0);
  fe_set_word(fe, fe->one, 1);
  fe_set_word(fe, fe->two, 2);
  fe_set_word(fe, fe->three, 3);
  fe_set_word(fe, fe->four, 4);
  fe_neg(fe, fe->mone, fe->one);
}

/*
 * Short Weierstrass
 */

static void
wge_to_jge(wei_t *ec, jge_t *r, const wge_t *a);

static void
jge_add_var(wei_t *ec, jge_t *r, const jge_t *a, const jge_t *b);

static void
jge_sub_var(wei_t *ec, jge_t *r, const jge_t *a, const jge_t *b);

static void
jge_mixed_add_var(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b);

static void
jge_mixed_sub_var(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b);

static void
jge_dbl(wei_t *ec, jge_t *r, const jge_t *p);

static void
jge_mixed_add(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b);

static void
jge_to_wge(wei_t *ec, wge_t *r, const jge_t *p);

/*
 * Short Weierstrass Affine Point
 */

static void
wge_zero(wei_t *ec, wge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_zero(fe, r->x);
  fe_zero(fe, r->y);
  r->inf = 1;
}

static void
wge_cleanse(wei_t *ec, wge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_cleanse(fe, r->x);
  fe_cleanse(fe, r->y);
  r->inf = 1;
}

static int
wge_validate(wei_t *ec, const wge_t *p) {
  /* [GECC] Page 89, Section 3.2.2. */
  prime_field_t *fe = &ec->fe;
  fe_t lhs, rhs, ax;

  /* y^2 = x^3 + a * x + b */
  fe_sqr(fe, lhs, p->y);
  fe_sqr(fe, rhs, p->x);
  fe_mul(fe, rhs, rhs, p->x);
  fe_mul(fe, ax, ec->a, p->x);
  fe_add(fe, rhs, rhs, ax);
  fe_add(fe, rhs, rhs, ec->b);

  return fe_equal(fe, lhs, rhs) | p->inf;
}

static int
wge_set_x(wei_t *ec, wge_t *r, const fe_t x, int sign) {
  /* [GECC] Page 89, Section 3.2.2. */
  prime_field_t *fe = &ec->fe;
  fe_t y, ax;
  int ret;

  /* y^2 = x^3 + a * x + b */
  fe_sqr(fe, y, x);
  fe_mul(fe, y, y, x);
  fe_mul(fe, ax, ec->a, x);
  fe_add(fe, y, y, ax);
  fe_add(fe, y, y, ec->b);

  ret = fe_sqrt(fe, y, y);

  if (sign != -1)
    fe_set_odd(fe, y, y, sign);

  fe_set(fe, r->x, x);
  fe_set(fe, r->y, y);
  r->inf = 0;

#ifdef TORSION_TEST
  assert(wge_validate(ec, r) == ret);
#endif

  return ret;
}

static void
wge_set_xy(wei_t *ec, wge_t *r, const fe_t x, const fe_t y) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, x);
  fe_set(fe, r->y, y);
  r->inf = 0;
}

static int
wge_import(wei_t *ec, wge_t *r, const unsigned char *raw, size_t len) {
  /* [SEC1] Page 11, Section 2.3.4. */
  prime_field_t *fe = &ec->fe;
  int form;

  if (len == 0)
    return 0;

  form = raw[0];

  switch (form) {
    case 0x02:
    case 0x03: {
      if (len != 1 + fe->size)
        return 0;

      if (!fe_import(fe, r->x, raw + 1))
        return 0;

      if (!wge_set_x(ec, r, r->x, form & 1))
        return 0;

      return 1;
    }
    case 0x04:
    case 0x06:
    case 0x07: {
      if (len != 1 + fe->size * 2)
        return 0;

      if (!fe_import(fe, r->x, raw + 1))
        return 0;

      if (!fe_import(fe, r->y, raw + 1 + fe->size))
        return 0;

      r->inf = 0;

      if (form != 0x04 && form != (0x06 | fe_is_odd(fe, r->y)))
        return 0;

      if (!wge_validate(ec, r))
        return 0;

      return 1;
    }
    default: {
      return 0;
    }
  }
}

static int
wge_export(wei_t *ec,
          unsigned char *raw,
          size_t *len,
          const wge_t *p,
          int compact) {
  /* [SEC1] Page 10, Section 2.3.3. */
  prime_field_t *fe = &ec->fe;

  if (p->inf)
    return 0;

  if (compact) {
    raw[0] = 0x02 | fe_is_odd(fe, p->y);
    fe_export(fe, raw + 1, p->x);

    if (len != NULL)
      *len = 1 + fe->size;
  } else {
    raw[0] = 0x04;
    fe_export(fe, raw + 1, p->x);
    fe_export(fe, raw + 1 + fe->size, p->y);

    if (len != NULL)
      *len = 1 + fe->size * 2;
  }

  return 1;
}

static int
wge_import_x(wei_t *ec, wge_t *r, const unsigned char *raw) {
  /* [SCHNORR] "Specification". */
  prime_field_t *fe = &ec->fe;

  if (!fe_import(fe, r->x, raw))
    return 0;

  return wge_set_x(ec, r, r->x, -1);
}

static int
wge_export_x(wei_t *ec, unsigned char *raw, const wge_t *p) {
  /* [SCHNORR] "Specification". */
  prime_field_t *fe = &ec->fe;

  if (p->inf)
    return 0;

  fe_export(fe, raw, p->x);

  return 1;
}

static void
wge_swap(wei_t *ec, wge_t *a, wge_t *b, unsigned int flag) {
  prime_field_t *fe = &ec->fe;
  int cond = (flag != 0);
  int inf1 = a->inf;
  int inf2 = b->inf;

  fe_swap(fe, a->x, b->x, flag);
  fe_swap(fe, a->y, b->y, flag);

  a->inf = (inf1 & (cond ^ 1)) | (inf2 & cond);
  b->inf = (inf2 & (cond ^ 1)) | (inf1 & cond);
}

static void
wge_select(wei_t *ec,
           wge_t *r,
           const wge_t *a,
           const wge_t *b,
           unsigned int flag) {
  prime_field_t *fe = &ec->fe;
  int cond = (flag != 0);
  int mask0 = cond - 1;
  int mask1 = ~mask0;

  fe_select(fe, r->x, a->x, b->x, flag);
  fe_select(fe, r->y, a->y, b->y, flag);

  r->inf = (a->inf & mask0) | (b->inf & mask1);
}

static void
wge_set(wei_t *ec, wge_t *r, const wge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_set(fe, r->y, a->y);
  r->inf = a->inf;
}

static int
wge_equal(wei_t *ec, const wge_t *a, const wge_t *b) {
  prime_field_t *fe = &ec->fe;
  int both = a->inf & b->inf;
  int ret = 1;

  /* P = O, Q = O */
  ret &= (a->inf ^ b->inf) ^ 1;

  /* X1 = X2 */
  ret &= fe_equal(fe, a->x, b->x) | both;

  /* Y1 = Y2 */
  ret &= fe_equal(fe, a->y, b->y) | both;

  return ret;
}

static int
wge_is_zero(wei_t *ec, const wge_t *a) {
  return a->inf;
}

static int
wge_is_square(wei_t *ec, const wge_t *p) {
  return fe_is_square(&ec->fe, p->y) & (p->inf ^ 1);
}

static int
wge_is_square_var(wei_t *ec, const wge_t *p) {
  if (p->inf)
    return 0;

  return fe_is_square_var(&ec->fe, p->y);
}

static int
wge_equal_x(wei_t *ec, const wge_t *p, const fe_t x) {
  return fe_equal(&ec->fe, p->x, x) & (p->inf ^ 1);
}

static void
wge_neg(wei_t *ec, wge_t *r, const wge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_neg(fe, r->y, a->y);
  r->inf = a->inf;
}

static void
wge_neg_cond(wei_t *ec, wge_t *r, const wge_t *a, unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_neg_cond(fe, r->y, a->y, flag);
  r->inf = a->inf;
}

static void
wge_dbl_var(wei_t *ec, wge_t *r, const wge_t *p) {
  /* [GECC] Page 80, Section 3.1.2.
   *
   * Addition Law (doubling):
   *
   *   l = (3 * x1^2 + a) / (2 * y1)
   *   x3 = l^2 - 2 * x1
   *   y3 = l * (x1 - x3) - y1
   *
   * 1I + 2M + 2S + 3A + 2*2 + 1*3
   */
  prime_field_t *fe = &ec->fe;
  fe_t l, t, x3, y3;

  /* P = O */
  if (p->inf) {
    wge_zero(ec, r);
    return;
  }

  /* Y1 = 0 */
  if (ec->h > 1 && fe_is_zero(fe, p->y)) {
    wge_zero(ec, r);
    return;
  }

  /* L = (3 * X1^2 + a) / (2 * Y1) */
  fe_sqr(fe, l, p->x);
  fe_add(fe, t, l, l);
  fe_add(fe, l, t, l);
  fe_add(fe, l, l, ec->a);
  fe_add(fe, t, p->y, p->y);
  fe_invert_var(fe, t, t);
  fe_mul(fe, l, l, t);

  /* X3 = L^2 - 2 * X1 */
  fe_sqr(fe, x3, l);
  fe_sub(fe, x3, x3, p->x);
  fe_sub(fe, x3, x3, p->x);

  /* Y3 = L * (X1 - X3) - Y1 */
  fe_sub(fe, t, p->x, x3);
  fe_mul(fe, y3, l, t);
  fe_sub(fe, y3, y3, p->y);

  fe_set(fe, r->x, x3);
  fe_set(fe, r->y, y3);
  r->inf = 0;
}

static void
wge_add_var(wei_t *ec, wge_t *r, const wge_t *a, const wge_t *b) {
  /* [GECC] Page 80, Section 3.1.2.
   *
   * Addition Law:
   *
   *   l = (y1 - y2) / (x1 - x2)
   *   x3 = l^2 - x1 - x2
   *   y3 = l * (x1 - x3) - y1
   *
   * 1I + 2M + 1S + 6A
   */
  prime_field_t *fe = &ec->fe;
  fe_t l, t, x3, y3;

  /* O + P = P */
  if (a->inf) {
    wge_set(ec, r, b);
    return;
  }

  /* P + O = P */
  if (b->inf) {
    wge_set(ec, r, a);
    return;
  }

  /* P + P, P + -P */
  if (fe_equal(fe, a->x, b->x)) {
    /* P + -P = O */
    if (!fe_equal(fe, a->y, b->y)) {
      wge_zero(ec, r);
      return;
    }

    /* P + P = 2P */
    wge_dbl_var(ec, r, a);
    return;
  }

  /* X1 != X2, Y1 = Y2 */
  if (fe_equal(fe, a->y, b->y)) {
    /* X3 = -X1 - X2 */
    fe_neg(fe, x3, a->x);
    fe_sub(fe, x3, x3, b->x);

    /* Y3 = -Y1 */
    fe_neg(fe, y3, a->y);

    /* Skip the inverse. */
    fe_set(fe, r->x, x3);
    fe_set(fe, r->y, y3);
    r->inf = 0;

    return;
  }

  /* L = (Y1 - Y2) / (X1 - X2) */
  fe_sub(fe, l, a->y, b->y);
  fe_sub(fe, t, a->x, b->x);
  fe_invert_var(fe, t, t);
  fe_mul(fe, l, l, t);

  /* X3 = L^2 - X1 - X2 */
  fe_sqr(fe, x3, l);
  fe_sub(fe, x3, x3, a->x);
  fe_sub(fe, x3, x3, b->x);

  /* Y3 = L * (X1 - X3) - Y1 */
  fe_sub(fe, t, a->x, x3);
  fe_mul(fe, y3, l, t);
  fe_sub(fe, y3, y3, a->y);

  fe_set(fe, r->x, x3);
  fe_set(fe, r->y, y3);
  r->inf = 0;
}

static void
wge_sub_var(wei_t *ec, wge_t *r, const wge_t *a, const wge_t *b) {
  wge_t c;
  wge_neg(ec, &c, b);
  wge_add_var(ec, r, a, &c);
}

static void
wge_dbl(wei_t *ec, wge_t *r, const wge_t *p) {
  jge_t j;

  wge_to_jge(ec, &j, p);
  jge_dbl(ec, &j, &j);
  jge_to_wge(ec, r, &j);
}

static void
wge_add(wei_t *ec, wge_t *r, const wge_t *a, const wge_t *b) {
  jge_t j;

  wge_to_jge(ec, &j, a);
  jge_mixed_add(ec, &j, &j, b);
  jge_to_wge(ec, r, &j);
}

static void
wge_sub(wei_t *ec, wge_t *r, const wge_t *a, const wge_t *b) {
  wge_t c;
  wge_neg(ec, &c, b);
  wge_add(ec, r, a, &c);
}

static void
wge_to_jge(wei_t *ec, jge_t *r, const wge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_select(fe, r->x, a->x, fe->one, a->inf);
  fe_select(fe, r->y, a->y, fe->one, a->inf);
  fe_select(fe, r->z, fe->one, fe->zero, a->inf);
}

static void
wge_wnd_points_var(wei_t *ec, wge_t *out, const wge_t *p) {
  scalar_field_t *sc = &ec->sc;
  size_t i, j;
  wge_t g;

  wge_set(ec, &g, p);

  for (i = 0; i < WND_STEPS(sc->bits); i++) {
    wge_zero(ec, &out[i * WND_SIZE]);

    for (j = 1; j < WND_SIZE; j++)
      wge_add_var(ec, &out[i * WND_SIZE + j], &out[i * WND_SIZE + j - 1], &g);

    for (j = 0; j < WND_WIDTH; j++)
      wge_dbl_var(ec, &g, &g);
  }
}

static void
wge_naf_points_var(wei_t *ec, wge_t *points, const wge_t *p, size_t width) {
  size_t size = 1 << (width - 1);
  wge_t dbl;
  size_t i;

  wge_dbl_var(ec, &dbl, p);
  wge_set(ec, &points[0], p);

  for (i = 1; i < size; i++)
    wge_add_var(ec, &points[i], &points[i - 1], &dbl);
}

static void
wge_jsf_points_var(wei_t *ec, jge_t *points, const wge_t *p1, const wge_t *p2) {
  /* Create comb for JSF. */
  wge_to_jge(ec, &points[0], p1); /* 1 */
  jge_mixed_add_var(ec, &points[1], &points[0], p2); /* 3 */
  jge_mixed_sub_var(ec, &points[2], &points[0], p2); /* 5 */
  wge_to_jge(ec, &points[3], p2); /* 7 */
}

static void
wge_endo_beta(wei_t *ec, wge_t *r, const wge_t *p) {
  prime_field_t *fe = &ec->fe;

  fe_mul(fe, r->x, p->x, ec->beta);
  fe_set(fe, r->y, p->y);
  r->inf = p->inf;
}

static void
wge_jsf_points_endo(wei_t *ec, wge_t *points, const wge_t *p1) {
  /* Runs in constant time despite _var calls. */
  wge_t p2;

  /* Split point. */
  wge_endo_beta(ec, &p2, p1);

  /* Create comb for JSF. */
  wge_set(ec, &points[0], p1); /* 1 */
  wge_add_var(ec, &points[1], p1, &p2); /* 3 */
  wge_sub_var(ec, &points[2], p1, &p2); /* 5 */
  wge_set(ec, &points[3], &p2); /* 7 */
}

#ifdef TORSION_TEST
static void
wge_print(wei_t *ec, const wge_t *p) {
  prime_field_t *fe = &ec->fe;

  if (wge_is_zero(ec, p)) {
    printf("(infinity)\n");
  } else {
    mp_limb_t xp[MAX_FIELD_LIMBS];
    mp_limb_t yp[MAX_FIELD_LIMBS];

    fe_get_limbs(fe, xp, p->x);
    fe_get_limbs(fe, yp, p->y);

    printf("(");
    mpn_print(xp, fe->limbs, 16);
    printf(", ");
    mpn_print(yp, fe->limbs, 16);
    printf(")\n");
  }
}
#endif

/*
 * Short Weierstrass Jacobian Point
 */

static void
jge_zero(wei_t *ec, jge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, fe->one);
  fe_set(fe, r->y, fe->one);
  fe_zero(fe, r->z);
}

static void
jge_cleanse(wei_t *ec, jge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_cleanse(fe, r->x);
  fe_cleanse(fe, r->y);
  fe_cleanse(fe, r->z);
}

static void
jge_swap(wei_t *ec, jge_t *a, jge_t *b, unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_swap(fe, a->x, b->x, flag);
  fe_swap(fe, a->y, b->y, flag);
  fe_swap(fe, a->z, b->z, flag);
}

static void
jge_select(wei_t *ec,
           jge_t *r,
           const jge_t *a,
           const jge_t *b,
           unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_select(fe, r->x, a->x, b->x, flag);
  fe_select(fe, r->y, a->y, b->y, flag);
  fe_select(fe, r->z, a->z, b->z, flag);
}

static void
jge_set(wei_t *ec, jge_t *r, const jge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_set(fe, r->y, a->y);
  fe_set(fe, r->z, a->z);
}

static int
jge_is_zero(wei_t *ec, const jge_t *a) {
  prime_field_t *fe = &ec->fe;

  return fe_is_zero(fe, a->z);
}

static int
jge_equal(wei_t *ec, const jge_t *a, const jge_t *b) {
  prime_field_t *fe = &ec->fe;
  fe_t z1, z2, e1, e2;
  int inf1 = jge_is_zero(ec, a);
  int inf2 = jge_is_zero(ec, b);
  int both = inf1 & inf2;
  int ret = 1;

  /* P = O, Q = O */
  ret &= (inf1 ^ inf2) ^ 1;

  /* X1 * Z2^2 == X2 * Z1^2 */
  fe_sqr(fe, z1, a->z);
  fe_sqr(fe, z2, b->z);
  fe_mul(fe, e1, a->x, z2);
  fe_mul(fe, e2, b->x, z1);

  ret &= fe_equal(fe, e1, e2) | both;

  /* Y1 * Z2^3 == Y2 * Z1^3 */
  fe_mul(fe, z1, z1, a->z);
  fe_mul(fe, z2, z2, b->z);
  fe_mul(fe, e1, a->y, z2);
  fe_mul(fe, e2, b->y, z1);

  ret &= fe_equal(fe, e1, e2) | both;

  return ret;
}

static int
jge_is_square(wei_t *ec, const jge_t *p) {
  prime_field_t *fe = &ec->fe;
  fe_t yz;

  fe_mul(fe, yz, p->y, p->z);

  return fe_is_square(fe, yz)
       & (jge_is_zero(ec, p) ^ 1);
}

static int
jge_is_square_var(wei_t *ec, const jge_t *p) {
  prime_field_t *fe = &ec->fe;
  fe_t yz;

  if (jge_is_zero(ec, p))
    return 0;

  fe_mul(fe, yz, p->y, p->z);

  return fe_is_square_var(fe, yz);
}

static int
jge_equal_x(wei_t *ec, const jge_t *p, const fe_t x) {
  prime_field_t *fe = &ec->fe;
  fe_t xz;

  fe_sqr(fe, xz, p->z);
  fe_mul(fe, xz, xz, x);

  return fe_equal(fe, p->x, xz)
       & (jge_is_zero(ec, p) ^ 1);
}

#ifdef ECC_WITH_TRICK
static int
jge_equal_r(wei_t *ec, const jge_t *p, const sc_t x) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  mp_limb_t cp[MAX_FIELD_LIMBS + 1];
  mp_size_t cn = fe->limbs + 1;
  fe_t zz, rx, rn;

  assert(fe->limbs >= sc->limbs);

  if (jge_is_zero(ec, p))
    return 0;

  fe_sqr(fe, zz, p->z);

  assert(fe_set_sc(fe, sc, rx, x));
  fe_mul(fe, rx, rx, zz);

  if (fe_equal(fe, p->x, rx))
    return 1;

  mpn_zero(cp + sc->limbs, cn - sc->limbs);
  mpn_copyi(cp, x, sc->limbs);

  fe_mul(fe, rn, ec->red_n, zz);

  assert(sc->n[cn - 1] == 0);
  assert(fe->p[cn - 1] == 0);

  for (;;) {
    mpn_add_n(cp, cp, sc->n, cn);

    if (mpn_cmp(cp, fe->p, cn) >= 0)
      return 0;

    fe_add(fe, rx, rx, rn);

    if (fe_equal(fe, p->x, rx))
      break;
  }

  return 1;
}
#endif

static void
jge_neg(wei_t *ec, jge_t *r, const jge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_neg(fe, r->y, a->y);
  fe_set(fe, r->z, a->z);
}

static void
jge_neg_cond(wei_t *ec, jge_t *r, const jge_t *a, unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_neg_cond(fe, r->y, a->y, flag);
  fe_set(fe, r->z, a->z);
}

static void
jge_dblj(wei_t *ec, jge_t *r, const jge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#doubling-dbl-1998-cmo-2
   * 3M + 6S + 4A + 1*a + 2*2 + 1*3 + 1*4 + 1*8
   */
  prime_field_t *fe = &ec->fe;
  fe_t xx, yy, zz, s, m, t;

  /* XX = X1^2 */
  fe_sqr(fe, xx, p->x);

  /* YY = Y1^2 */
  fe_sqr(fe, yy, p->y);

  /* ZZ = Z1^2 */
  fe_sqr(fe, zz, p->z);

  /* S = 4 * X1 * YY */
  fe_mul(fe, s, p->x, yy);
  fe_add(fe, s, s, s);
  fe_add(fe, s, s, s);

  /* M = 3 * XX + a * ZZ^2 */
  fe_add(fe, m, xx, xx);
  fe_add(fe, m, m, xx);
  fe_sqr(fe, t, zz);
  fe_mul(fe, t, t, ec->a);
  fe_add(fe, m, m, t);

  /* T = M^2 - 2 * S */
  fe_sqr(fe, t, m);
  fe_sub(fe, t, t, s);
  fe_sub(fe, t, t, s);

  /* Z3 = 2 * Y1 * Z1 */
  fe_mul(fe, r->z, p->z, p->y);
  fe_add(fe, r->z, r->z, r->z);

  /* X3 = T */
  fe_set(fe, r->x, t);

  /* Y3 = M * (S - T) - 8 * YY^2 */
  fe_sub(fe, xx, s, t);
  fe_sqr(fe, zz, yy);
  fe_add(fe, zz, zz, zz);
  fe_add(fe, zz, zz, zz);
  fe_add(fe, zz, zz, zz);
  fe_mul(fe, r->y, m, xx);
  fe_sub(fe, r->y, r->y, zz);
}

static void
jge_dbl0(wei_t *ec, jge_t *r, const jge_t *p) {
  /* Assumes a = 0.
   * https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#doubling-dbl-2009-l
   * 2M + 5S + 6A + 3*2 + 1*3 + 1*8
   */
  prime_field_t *fe = &ec->fe;
  fe_t a, b, c, d, e, f;

  /* A = X1^2 */
  fe_sqr(fe, a, p->x);

  /* B = Y1^2 */
  fe_sqr(fe, b, p->y);

  /* C = B^2 */
  fe_sqr(fe, c, b);

  /* D = 2 * ((X1 + B)^2 - A - C) */
  fe_add(fe, d, p->x, b);
  fe_sqr(fe, d, d);
  fe_sub(fe, d, d, a);
  fe_sub(fe, d, d, c);
  fe_add(fe, d, d, d);

  /* E = 3 * A */
  fe_add(fe, e, a, a);
  fe_add(fe, e, e, a);

  /* F = E^2 */
  fe_sqr(fe, f, e);

  /* Z3 = 2 * Y1 * Z1 */
  fe_mul(fe, r->z, p->z, p->y);
  fe_add(fe, r->z, r->z, r->z);

  /* X3 = F - 2 * D */
  fe_add(fe, r->x, d, d);
  fe_sub(fe, r->x, f, r->x);

  /* Y3 = E * (D - X3) - 8 * C */
  fe_add(fe, c, c, c);
  fe_add(fe, c, c, c);
  fe_add(fe, c, c, c);
  fe_sub(fe, d, d, r->x);
  fe_mul(fe, r->y, e, d);
  fe_sub(fe, r->y, r->y, c);
}

static void
jge_dbl3(wei_t *ec, jge_t *r, const jge_t *p) {
  /* Assumes a = -3.
   * https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html#doubling-dbl-2001-b
   * 3M + 5S + 8A + 1*3 + 1*4 + 2*8
   */
  prime_field_t *fe = &ec->fe;
  fe_t delta, gamma, beta, alpha, t1, t2;

  /* delta = Z1^2 */
  fe_sqr(fe, delta, p->z);

  /* gamma = Y1^2 */
  fe_sqr(fe, gamma, p->y);

  /* beta = X1 * gamma */
  fe_mul(fe, beta, p->x, gamma);

  /* alpha = 3 * (X1 - delta) * (X1 + delta) */
  fe_sub(fe, t1, p->x, delta);
  fe_add(fe, t2, p->x, delta);
  fe_add(fe, alpha, t1, t1);
  fe_add(fe, alpha, alpha, t1);
  fe_mul(fe, alpha, alpha, t2);

  /* Z3 = (Y1 + Z1)^2 - gamma - delta */
  fe_add(fe, r->z, p->y, p->z);
  fe_sqr(fe, r->z, r->z);
  fe_sub(fe, r->z, r->z, gamma);
  fe_sub(fe, r->z, r->z, delta);

  /* X3 = alpha^2 - 8 * beta */
  fe_add(fe, t1, beta, beta);
  fe_add(fe, t1, t1, t1);
  fe_add(fe, t2, t1, t1);
  fe_sqr(fe, r->x, alpha);
  fe_sub(fe, r->x, r->x, t2);

  /* Y3 = alpha * (4 * beta - X3) - 8 * gamma^2 */
  fe_sub(fe, r->y, t1, r->x);
  fe_mul(fe, r->y, r->y, alpha);
  fe_sqr(fe, gamma, gamma);
  fe_add(fe, gamma, gamma, gamma);
  fe_add(fe, gamma, gamma, gamma);
  fe_add(fe, gamma, gamma, gamma);
  fe_sub(fe, r->y, r->y, gamma);
}

static void
jge_dbl_var(wei_t *ec, jge_t *r, const jge_t *p) {
  prime_field_t *fe = &ec->fe;

  /* P = O */
  if (jge_is_zero(ec, p)) {
    jge_zero(ec, r);
    return;
  }

  /* Y1 = 0 */
  if (ec->h > 1 && fe_is_zero(fe, p->y)) {
    jge_zero(ec, r);
    return;
  }

  if (ec->zero_a)
    jge_dbl0(ec, r, p);
  else if (ec->three_a)
    jge_dbl3(ec, r, p);
  else
    jge_dblj(ec, r, p);
}

static void
jge_add_var(wei_t *ec, jge_t *r, const jge_t *a, const jge_t *b) {
  /* No assumptions.
   * https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#addition-add-1998-cmo-2
   * 12M + 4S + 6A + 1*2
   */
  prime_field_t *fe = &ec->fe;
  fe_t z1z1, z2z2, u1, u2, s1, s2, h, r0, hh, hhh, v;

  /* O + P = P */
  if (jge_is_zero(ec, a)) {
    jge_set(ec, r, b);
    return;
  }

  /* P + O = P */
  if (jge_is_zero(ec, b)) {
    jge_set(ec, r, a);
    return;
  }

  /* Z1Z1 = Z1^2 */
  fe_sqr(fe, z1z1, a->z);

  /* Z2Z2 = Z2^2 */
  fe_sqr(fe, z2z2, b->z);

  /* U1 = X1 * Z2Z2 */
  fe_mul(fe, u1, a->x, z2z2);

  /* U2 = X2 * Z1Z1 */
  fe_mul(fe, u2, b->x, z1z1);

  /* S1 = Y1 * Z2 * Z2Z2 */
  fe_mul(fe, s1, a->y, b->z);
  fe_mul(fe, s1, s1, z2z2);

  /* S2 = Y2 * Z1 * Z1Z1 */
  fe_mul(fe, s2, b->y, a->z);
  fe_mul(fe, s2, s2, z1z1);

  /* H = U2 - U1 */
  fe_sub(fe, h, u2, u1);

  /* r = S2 - S1 */
  fe_sub(fe, r0, s2, s1);

  /* H = 0 */
  if (fe_is_zero(fe, h)) {
    if (!fe_is_zero(fe, r0)) {
      jge_zero(ec, r);
      return;
    }

    jge_dbl_var(ec, r, a);
    return;
  }

  /* HH = H^2 */
  fe_sqr(fe, hh, h);

  /* HHH = H * HH */
  fe_mul(fe, hhh, h, hh);

  /* V = U1 * HH */
  fe_mul(fe, v, u1, hh);

  /* Z3 = Z1 * Z2 * H */
  fe_mul(fe, r->z, a->z, b->z);
  fe_mul(fe, r->z, r->z, h);

  /* X3 = r^2 - HHH - 2 * V */
  fe_sqr(fe, r->x, r0);
  fe_sub(fe, r->x, r->x, hhh);
  fe_sub(fe, r->x, r->x, v);
  fe_sub(fe, r->x, r->x, v);

  /* Y3 = r * (V - X3) - S1 * HHH */
  fe_sub(fe, u1, v, r->x);
  fe_mul(fe, u2, s1, hhh);
  fe_mul(fe, r->y, r0, u1);
  fe_sub(fe, r->y, r->y, u2);
}

static void
jge_sub_var(wei_t *ec, jge_t *r, const jge_t *a, const jge_t *b) {
  jge_t c;
  jge_neg(ec, &c, b);
  jge_add_var(ec, r, a, &c);
}

static void
jge_mixed_add_var(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b) {
  /* Assumes Z2 = 1.
   * https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#addition-madd
   * 8M + 3S + 6A + 5*2
   */
  prime_field_t *fe = &ec->fe;
  fe_t z1z1, u2, s2, h, r0, i, j, v;

  /* O + P = P */
  if (jge_is_zero(ec, a)) {
    wge_to_jge(ec, r, b);
    return;
  }

  /* P + O = P */
  if (wge_is_zero(ec, b)) {
    jge_set(ec, r, a);
    return;
  }

  /* Z1Z1 = Z1^2 */
  fe_sqr(fe, z1z1, a->z);

  /* U2 = X2 * Z1Z1 */
  fe_mul(fe, u2, b->x, z1z1);

  /* S2 = Y2 * Z1 * Z1Z1 */
  fe_mul(fe, s2, b->y, a->z);
  fe_mul(fe, s2, s2, z1z1);

  /* H = U2 - X1 */
  fe_sub(fe, h, u2, a->x);

  /* r = 2 * (S2 - Y1) */
  fe_sub(fe, r0, s2, a->y);
  fe_add(fe, r0, r0, r0);

  /* H = 0 */
  if (fe_is_zero(fe, h)) {
    if (!fe_is_zero(fe, r0)) {
      jge_zero(ec, r);
      return;
    }

    jge_dbl_var(ec, r, a);
    return;
  }

  /* I = (2 * H)^2 */
  fe_add(fe, i, h, h);
  fe_sqr(fe, i, i);

  /* J = H * I */
  fe_mul(fe, j, h, i);

  /* V = X1 * I */
  fe_mul(fe, v, a->x, i);

  /* X3 = r^2 - J - 2 * V */
  fe_sqr(fe, r->x, r0);
  fe_sub(fe, r->x, r->x, j);
  fe_sub(fe, r->x, r->x, v);
  fe_sub(fe, r->x, r->x, v);

  /* Y3 = r * (V - X3) - 2 * Y1 * J */
  fe_sub(fe, u2, v, r->x);
  fe_mul(fe, s2, a->y, j);
  fe_add(fe, s2, s2, s2);
  fe_mul(fe, r->y, r0, u2);
  fe_sub(fe, r->y, r->y, s2);

  /* Z3 = 2 * Z1 * H */
  fe_mul(fe, r->z, a->z, h);
  fe_add(fe, r->z, r->z, r->z);
}

static void
jge_mixed_sub_var(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b) {
  wge_t c;
  wge_neg(ec, &c, b);
  jge_mixed_add_var(ec, r, a, &c);
}

static void
jge_dbl(wei_t *ec, jge_t *r, const jge_t *p) {
  prime_field_t *fe = &ec->fe;
  int inf = 0;

  /* P = O */
  inf |= jge_is_zero(ec, p);

  /* Y1 = 0 */
  if (ec->h > 1)
    inf |= fe_is_zero(fe, p->y);

  if (ec->zero_a)
    jge_dbl0(ec, r, p);
  else if (ec->three_a)
    jge_dbl3(ec, r, p);
  else
    jge_dblj(ec, r, p);

  fe_select(fe, r->x, r->x, fe->one, inf);
  fe_select(fe, r->y, r->y, fe->one, inf);
  fe_select(fe, r->z, r->z, fe->zero, inf);
}

static void
jge_add(wei_t *ec, jge_t *r, const jge_t *a, const jge_t *b) {
  /* Strongly unified Jacobian addition (Brier and Joye).
   *
   * [SIDE2] Page 6, Section 3.
   * [SIDE3] Page 4, Section 3.
   *
   * The above documents use projective coordinates[1]. The
   * formula below was heavily adapted from libsecp256k1[2].
   *
   * [1] https://hyperelliptic.org/EFD/g1p/auto-shortw-projective.html#addition-add-2002-bj
   * [2] https://github.com/bitcoin-core/secp256k1/blob/ee9e68c/src/group_impl.h#L525
   *
   * 11M + 8S + 7A + 1*a + 2*4 + 1*3 + 2*2 (a != 0)
   * 11M + 6S + 6A + 2*4 + 1*3 + 2*2 (a = 0)
   */
  prime_field_t *fe = &ec->fe;
  fe_t z1z1, z2z2, u1, u2, s1, s2, z, t, m, l, w, h;
  int degenerate, inf1, inf2, inf3;

  /* Save some stack space. */
#define ll l
#define f m
#define r0 z1z1
#define g z2z2
#define x3 u2
#define y3 s2
#define z3 t

  /* Z1Z1 = Z1^2 */
  fe_sqr(fe, z1z1, a->z);

  /* Z2Z2 = Z2^2 */
  fe_sqr(fe, z2z2, b->z);

  /* U1 = X1 * Z2Z2 */
  fe_mul(fe, u1, a->x, z2z2);

  /* U2 = X2 * Z1Z1 */
  fe_mul(fe, u2, b->x, z1z1);

  /* S1 = Y1 * Z2Z2 * Z2 */
  fe_mul(fe, s1, a->y, z2z2);
  fe_mul(fe, s1, s1, b->z);

  /* S2 = Y2 * Z1Z1 * Z1 */
  fe_mul(fe, s2, b->y, z1z1);
  fe_mul(fe, s2, s2, a->z);

  /* Z = Z1 * Z2 */
  fe_mul(fe, z, a->z, b->z);

  /* T = U1 + U2 */
  fe_add(fe, t, u1, u2);

  /* M = S1 + S2 */
  fe_add(fe, m, s1, s2);

  /* R = T^2 - U1 * U2 */
  fe_sqr(fe, r0, t);
  fe_mul(fe, l, u1, u2);
  fe_sub(fe, r0, r0, l);

  /* R = R + a * Z^4 (if a != 0) */
  if (!ec->zero_a) {
    fe_sqr(fe, l, z);
    fe_sqr(fe, l, l);
    fe_mul(fe, l, l, ec->a);
    fe_add(fe, r0, r0, l);
  }

  /* Check for degenerate case (X1 != X2, Y1 = -Y2). */
  degenerate = fe_is_zero(fe, m) & fe_is_zero(fe, r0);

  /* M = U1 - U2 (if degenerate) */
  fe_sub(fe, l, u1, u2);
  fe_select(fe, m, m, l, degenerate);

  /* R = S1 - S2 (if degenerate) */
  fe_sub(fe, l, s1, s2);
  fe_select(fe, r0, r0, l, degenerate);

  /* L = M^2 */
  fe_sqr(fe, l, m);

  /* G = T * L */
  fe_mul(fe, g, t, l);

  /* LL = L^2 */
  fe_sqr(fe, ll, l);

  /* LL = 0 (if degenerate) */
  fe_select(fe, ll, ll, fe->zero, degenerate);

  /* W = R^2 */
  fe_sqr(fe, w, r0);

  /* F = Z * M */
  fe_mul(fe, f, m, z);

  /* H = 3 * G - 2 * W */
  fe_add(fe, h, g, g);
  fe_add(fe, h, h, g);
  fe_sub(fe, h, h, w);
  fe_sub(fe, h, h, w);

  /* X3 = 4 * (W - G) */
  fe_sub(fe, x3, w, g);
  fe_add(fe, x3, x3, x3);
  fe_add(fe, x3, x3, x3);

  /* Y3 = 4 * (R * H - LL) */
  fe_mul(fe, y3, r0, h);
  fe_sub(fe, y3, y3, ll);
  fe_add(fe, y3, y3, y3);
  fe_add(fe, y3, y3, y3);

  /* Z3 = 2 * F */
  fe_add(fe, z3, f, f);

  /* Check for infinity. */
  inf1 = fe_is_zero(fe, a->z);
  inf2 = fe_is_zero(fe, b->z);
  inf3 = fe_is_zero(fe, z3) & ((inf1 | inf2) ^ 1);

  /* Case 1: O + P = P */
  fe_select(fe, x3, x3, b->x, inf1);
  fe_select(fe, y3, y3, b->y, inf1);
  fe_select(fe, z3, z3, b->z, inf1);

  /* Case 2: P + O = P */
  fe_select(fe, x3, x3, a->x, inf2);
  fe_select(fe, y3, y3, a->y, inf2);
  fe_select(fe, z3, z3, a->z, inf2);

  /* Case 3: P + -P = O */
  fe_select(fe, x3, x3, fe->one, inf3);
  fe_select(fe, y3, y3, fe->one, inf3);
  fe_select(fe, z3, z3, fe->zero, inf3);

  /* R = (X3, Y3, Z3) */
  fe_set(fe, r->x, x3);
  fe_set(fe, r->y, y3);
  fe_set(fe, r->z, z3);

#undef ll
#undef f
#undef r0
#undef g
#undef x3
#undef y3
#undef z3
}

static void
jge_sub(wei_t *ec, jge_t *r, const jge_t *a, const jge_t *b) {
  jge_t c;
  jge_neg(ec, &c, b);
  jge_add(ec, r, a, &c);
}

static void
jge_mixed_add(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b) {
  /* Strongly unified mixed addition (Brier and Joye).
   *
   * [SIDE2] Page 6, Section 3.
   * [SIDE3] Page 4, Section 3.
   *
   * 7M + 7S + 7A + 1*a + 2*4 + 1*3 + 2*2 (a != 0)
   * 7M + 5S + 6A + 2*4 + 1*3 + 2*2 (a = 0)
   */
  prime_field_t *fe = &ec->fe;
  fe_t z1z1, u2, s2, t, m, l, g, w, h;
  int degenerate, inf1, inf2, inf3;

  /* Save some stack space. */
#define u1 a->x
#define s1 a->y
#define ll l
#define f m
#define r0 z1z1
#define x3 u2
#define y3 s2
#define z3 t

  /* Z1Z1 = Z1^2 */
  fe_sqr(fe, z1z1, a->z);

  /* U1 = X1 */

  /* U2 = X2 * Z1Z1 */
  fe_mul(fe, u2, b->x, z1z1);

  /* S1 = Y1 */

  /* S2 = Y2 * Z1Z1 * Z1 */
  fe_mul(fe, s2, b->y, z1z1);
  fe_mul(fe, s2, s2, a->z);

  /* T = U1 + U2 */
  fe_add(fe, t, u1, u2);

  /* M = S1 + S2 */
  fe_add(fe, m, s1, s2);

  /* R = T^2 - U1 * U2 */
  fe_sqr(fe, r0, t);
  fe_mul(fe, l, u1, u2);
  fe_sub(fe, r0, r0, l);

  /* R = R + a * Z1^4 (if a != 0) */
  if (!ec->zero_a) {
    fe_sqr(fe, l, a->z);
    fe_sqr(fe, l, l);
    fe_mul(fe, l, l, ec->a);
    fe_add(fe, r0, r0, l);
  }

  /* Check for degenerate case (X1 != X2, Y1 = -Y2). */
  degenerate = fe_is_zero(fe, m) & fe_is_zero(fe, r0);

  /* M = U1 - U2 (if degenerate) */
  fe_sub(fe, l, u1, u2);
  fe_select(fe, m, m, l, degenerate);

  /* R = S1 - S2 (if degenerate) */
  fe_sub(fe, l, s1, s2);
  fe_select(fe, r0, r0, l, degenerate);

  /* L = M^2 */
  fe_sqr(fe, l, m);

  /* G = T * L */
  fe_mul(fe, g, t, l);

  /* LL = L^2 */
  fe_sqr(fe, ll, l);

  /* LL = 0 (if degenerate) */
  fe_select(fe, ll, ll, fe->zero, degenerate);

  /* W = R^2 */
  fe_sqr(fe, w, r0);

  /* F = Z1 * M */
  fe_mul(fe, f, m, a->z);

  /* H = 3 * G - 2 * W */
  fe_add(fe, h, g, g);
  fe_add(fe, h, h, g);
  fe_sub(fe, h, h, w);
  fe_sub(fe, h, h, w);

  /* X3 = 4 * (W - G) */
  fe_sub(fe, x3, w, g);
  fe_add(fe, x3, x3, x3);
  fe_add(fe, x3, x3, x3);

  /* Y3 = 4 * (R * H - LL) */
  fe_mul(fe, y3, r0, h);
  fe_sub(fe, y3, y3, ll);
  fe_add(fe, y3, y3, y3);
  fe_add(fe, y3, y3, y3);

  /* Z3 = 2 * F */
  fe_add(fe, z3, f, f);

  /* Check for infinity. */
  inf1 = fe_is_zero(fe, a->z);
  inf2 = b->inf;
  inf3 = fe_is_zero(fe, z3) & ((inf1 | inf2) ^ 1);

  /* Case 1: O + P = P */
  fe_select(fe, x3, x3, b->x, inf1);
  fe_select(fe, y3, y3, b->y, inf1);
  fe_select(fe, z3, z3, fe->one, inf1);

  /* Case 2: P + O = P */
  fe_select(fe, x3, x3, a->x, inf2);
  fe_select(fe, y3, y3, a->y, inf2);
  fe_select(fe, z3, z3, a->z, inf2);

  /* Case 3: P + -P = O */
  fe_select(fe, x3, x3, fe->one, inf3);
  fe_select(fe, y3, y3, fe->one, inf3);
  fe_select(fe, z3, z3, fe->zero, inf3);

  /* R = (X3, Y3, Z3) */
  fe_set(fe, r->x, x3);
  fe_set(fe, r->y, y3);
  fe_set(fe, r->z, z3);

#undef u1
#undef s1
#undef ll
#undef f
#undef r0
#undef x3
#undef y3
#undef z3
}

static void
jge_mixed_sub(wei_t *ec, jge_t *r, const jge_t *a, const wge_t *b) {
  wge_t c;
  wge_neg(ec, &c, b);
  jge_mixed_add(ec, r, a, &c);
}

static void
jge_to_wge(wei_t *ec, wge_t *r, const jge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#scaling-z
   * 1I + 3M + 1S
   */
  prime_field_t *fe = &ec->fe;
  fe_t a, aa;

  /* A = 1 / Z1 */
  r->inf = fe_invert(fe, a, p->z) ^ 1;

  /* AA = A^2 */
  fe_sqr(fe, aa, a);

  /* X3 = X1 * AA */
  fe_mul(fe, r->x, p->x, aa);

  /* Y3 = Y1 * AA * A */
  fe_mul(fe, r->y, p->y, aa);
  fe_mul(fe, r->y, r->y, a);
}

static void
jge_to_wge_var(wei_t *ec, wge_t *r, const jge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#scaling-z
   * 1I + 3M + 1S
   */
  prime_field_t *fe = &ec->fe;
  fe_t a, aa;

  /* P = O */
  if (jge_is_zero(ec, p)) {
    wge_zero(ec, r);
    return;
  }

  /* Z = 1 */
  if (fe_equal(fe, p->z, fe->one)) {
    fe_set(fe, r->x, p->x);
    fe_set(fe, r->y, p->y);
    r->inf = 0;
    return;
  }

  /* A = 1 / Z1 */
  fe_invert_var(fe, a, p->z);

  /* AA = A^2 */
  fe_sqr(fe, aa, a);

  /* X3 = X1 * AA */
  fe_mul(fe, r->x, p->x, aa);

  /* Y3 = Y1 * AA * A */
  fe_mul(fe, r->y, p->y, aa);
  fe_mul(fe, r->y, r->y, a);
  r->inf = 0;
}

static int
jge_validate(wei_t *ec, const jge_t *p) {
  /* [GECC] Example 3.20, Page 88, Section 3. */
  prime_field_t *fe = &ec->fe;
  fe_t lhs, x3, z2, z4, z6, rhs;

  /* y^2 = x^3 + a * x * z^4 + b * z^6 */
  fe_sqr(fe, lhs, p->y);
  fe_sqr(fe, x3, p->x);
  fe_mul(fe, x3, x3, p->x);
  fe_sqr(fe, z2, p->z);
  fe_sqr(fe, z4, z2);
  fe_mul(fe, z6, z4, z2);
  fe_mul(fe, rhs, ec->b, z6);
  fe_add(fe, rhs, rhs, x3);
  fe_mul(fe, x3, ec->a, z4);
  fe_mul(fe, x3, x3, p->x);
  fe_add(fe, rhs, rhs, x3);

  return fe_equal(fe, lhs, rhs)
       | jge_is_zero(ec, p);
}

static void
jge_naf_points_var(wei_t *ec, jge_t *points, const wge_t *p, size_t width) {
  size_t size = 1 << (width - 1);
  jge_t dbl;
  size_t i;

  wge_to_jge(ec, &points[0], p);
  jge_dbl_var(ec, &dbl, &points[0]);

  for (i = 1; i < size; i++)
    jge_add_var(ec, &points[i], &points[i - 1], &dbl);
}

#ifdef TORSION_TEST
static void
jge_print(wei_t *ec, const jge_t *p) {
  prime_field_t *fe = &ec->fe;

  if (jge_is_zero(ec, p)) {
    printf("(infinity)\n");
  } else {
    mp_limb_t xp[MAX_FIELD_LIMBS];
    mp_limb_t yp[MAX_FIELD_LIMBS];
    mp_limb_t zp[MAX_FIELD_LIMBS];

    fe_get_limbs(fe, xp, p->x);
    fe_get_limbs(fe, yp, p->y);
    fe_get_limbs(fe, zp, p->z);

    printf("(");
    mpn_print(xp, fe->limbs, 16);
    printf(", ");
    mpn_print(yp, fe->limbs, 16);
    printf(", ");
    mpn_print(zp, fe->limbs, 16);
    printf(")\n");
  }
}
#endif

/*
 * Short Weierstrass Curve
 */

static void
wei_init(wei_t *ec, const wei_def_t *def) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  fe_t m3;

  ec->hash = def->hash;
  ec->h = def->h;

  prime_field_init(fe, def->fe, 1);
  scalar_field_init(sc, def->sc, 1);

  sc_reduce(sc, ec->pmodn, fe->p);

  fe_set_limbs(fe, ec->red_n, sc->n, sc->limbs);
  fe_import(fe, ec->a, def->a);
  fe_import(fe, ec->b, def->b);
  fe_import(fe, ec->c, def->c);

  if (def->z < 0) {
    fe_set_word(fe, ec->z, -def->z);
    fe_neg(fe, ec->z, ec->z);
  } else {
    fe_set_word(fe, ec->z, def->z);
  }

  fe_invert_var(fe, ec->ai, ec->a);
  fe_invert_var(fe, ec->zi, ec->z);
  fe_invert_var(fe, ec->i2, fe->two);
  fe_invert_var(fe, ec->i3, fe->three);

  fe_neg(fe, m3, fe->three);

  ec->zero_a = fe_is_zero(fe, ec->a);
  ec->three_a = fe_equal(fe, ec->a, m3);

  fe_import(fe, ec->g.x, def->x);
  fe_import(fe, ec->g.y, def->y);
  ec->g.inf = 0;

  sc_zero(sc, ec->blind);
  wge_zero(ec, &ec->unblind);

  wge_wnd_points_var(ec, ec->windows, &ec->g);
  wge_naf_points_var(ec, ec->points, &ec->g, NAF_WIDTH_PRE);

  ec->endo = def->endo;

  if (ec->endo) {
    size_t i;

    fe_import(fe, ec->beta, def->beta);
    sc_import(sc, ec->lambda, def->lambda);
    sc_import(sc, ec->b1, def->b1);
    sc_import(sc, ec->b2, def->b2);
    sc_import(sc, ec->g1, def->g1);
    sc_import(sc, ec->g2, def->g2);

    for (i = 0; i < NAF_SIZE_PRE; i++)
      wge_endo_beta(ec, &ec->endo_points[i], &ec->points[i]);
  }
}

static void
wei_endo_split(wei_t *ec,
               sc_t k1,
               int32_t *s1,
               sc_t k2,
               int32_t *s2,
               const sc_t k) {
  /* Balanced length-two representation of a multiplier.
   *
   * [GECC] Algorithm 3.74, Page 127, Section 3.5.
   *
   * Computation:
   *
   *   c1 = round(b2 * k / n)
   *   c2 = round(-b1 * k / n)
   *   k1 = k - c1 * a1 - c2 * a2
   *   k2 = -c1 * b1 - c2 * b2
   *
   * It is possible to precompute[1] values in order
   * to avoid the round division[2][3][4].
   *
   * This involves precomputing `g1` and `g2` as:
   *
   *   d = a1 * b2 - b1 * a2
   *   t = ceil(log2(d)) + 16
   *   g1 = round((2^t * b2) / d)
   *   g2 = round((2^t * b1) / d)
   *
   * Where `d` is equal to `n`.
   *
   * `c1` and `c2` can then be computed as follows:
   *
   *   t = ceil(log2(n)) + 16
   *   c1 = (k * g1) >> t
   *   c2 = -((k * g2) >> t)
   *   k1 = k - c1 * a1 - c2 * a2
   *   k2 = -c1 * b1 - c2 * b2
   *
   * Where `>>` is an _unsigned_ right shift. Also
   * note that the last bit discarded in the shift
   * must be stored. If it is 1, then add 1 to the
   * integer (absolute addition).
   *
   * libsecp256k1 modifies the computation further:
   *
   *   t = ceil(log2(n)) + 16
   *   c1 = ((k * g1) >> t) * -b1
   *   c2 = ((k * -g2) >> t) * -b2
   *   k2 = c1 + c2
   *   k1 = k2 * -lambda + k
   *
   * Once the multiply and shift are complete, we
   * can use modular arithmetic for the rest of
   * the calculations (the mul+shift is done in
   * the integers, not mod n). This is nice as it
   * allows us to re-use existing scalar functions,
   * and our decomposition becomes a constant-time
   * calculation.
   *
   * Since the above computation is done mod n,
   * the resulting scalars must be reduced. Sign
   * correction is necessary outside of this
   * function.
   *
   * [1] [JCEN12] Page 5, Section 4.3.
   * [2] https://github.com/bitcoin-core/secp256k1/blob/0b70241/src/scalar_impl.h#L259
   * [3] https://github.com/bitcoin-core/secp256k1/pull/21
   * [4] https://github.com/bitcoin-core/secp256k1/pull/127
   */
  scalar_field_t *sc = &ec->sc;
  sc_t c1, c2;
  int32_t h1, h2;

  sc_mulshift(sc, c1, k, ec->g1, sc->bits + 16);
  sc_mulshift(sc, c2, k, ec->g2, sc->bits + 16); /* -g2 */

  sc_mul(sc, c1, c1, ec->b1); /* -b1 */
  sc_mul(sc, c2, c2, ec->b2); /* -b2 */

  sc_add(sc, k2, c1, c2);
  sc_mul(sc, k1, k2, ec->lambda); /* -lambda */
  sc_add(sc, k1, k1, k);

  h1 = sc_minimize(sc, k1, k1);
  h2 = sc_minimize(sc, k2, k2);

  sc_cleanse(sc, c1);
  sc_cleanse(sc, c2);

  *s1 = -h1 | 1;
  *s2 = -h2 | 1;
}

static void
wei_jmul_g(wei_t *ec, jge_t *r, const sc_t k) {
  scalar_field_t *sc = &ec->sc;
  size_t i, j, b;
  sc_t k0;
  wge_t p;

  /* Blind if available. */
  sc_add(sc, k0, k, ec->blind);

  /* Multiply in constant time. */
  wge_zero(ec, &p);
  wge_to_jge(ec, r, &ec->unblind);

  for (i = 0; i < WND_STEPS(sc->bits); i++) {
    b = sc_get_bits(sc, k0, i * WND_WIDTH, WND_WIDTH);

    for (j = 0; j < WND_SIZE; j++)
      wge_select(ec, &p, &p, &ec->windows[i * WND_SIZE + j], j == b);

    jge_mixed_add(ec, r, r, &p);
  }

  /* Cleanse. */
  sc_cleanse(sc, k0);

  cleanse(&b, sizeof(b));
}

static void
wei_mul_g(wei_t *ec, wge_t *r, const sc_t k) {
  jge_t j;
  wei_jmul_g(ec, &j, k);
  jge_to_wge(ec, r, &j);
}

static void
wei_jmul_normal(wei_t *ec, jge_t *r, const wge_t *p, const sc_t k) {
  scalar_field_t *sc = &ec->sc;
  mp_size_t start = WND_STEPS(sc->bits) - 1;
  jge_t table[WND_SIZE];
  mp_size_t i, j, b;
  jge_t t;

  jge_zero(ec, &table[0]);
  wge_to_jge(ec, &table[1], p);

  for (i = 2; i < WND_SIZE; i += 2) {
    jge_dbl(ec, &table[i], &table[i >> 1]);
    jge_mixed_add(ec, &table[i + 1], &table[i], p);
  }

  jge_zero(ec, r);
  jge_zero(ec, &t);

  for (i = start; i >= 0; i--) {
    if (i != start) {
      for (j = 0; j < WND_WIDTH; j++)
        jge_dbl(ec, r, r);
    }

    b = sc_get_bits(sc, k, i * WND_WIDTH, WND_WIDTH);

    for (j = 0; j < WND_SIZE; j++)
      jge_select(ec, &t, &t, &table[j], j == b);

    jge_add(ec, r, r, &t);
  }

  cleanse(&b, sizeof(b));
}

static void
wei_jmul_endo(wei_t *ec, jge_t *r, const wge_t *p, const sc_t k) {
  scalar_field_t *sc = &ec->sc;
  mp_size_t bits = (sc->bits + 1) >> 1;
  mp_size_t start = WND_STEPS(bits) - 1;
  jge_t table1[WND_SIZE];
  jge_t table2[WND_SIZE];
  mp_size_t i, j, b;
  int32_t s1, s2;
  wge_t p1, p2;
  sc_t k1, k2;
  jge_t t;

  wge_set(ec, &p1, p);
  wge_endo_beta(ec, &p2, &p1);
  wei_endo_split(ec, k1, &s1, k2, &s2, k);
  wge_neg_cond(ec, &p1, &p1, (s1 >> 31) & 1);
  wge_neg_cond(ec, &p2, &p2, (s2 >> 31) & 1);

#ifdef TORSION_TEST
  assert(sc_bitlen_var(sc, k1) <= (size_t)bits);
  assert(sc_bitlen_var(sc, k2) <= (size_t)bits);
#endif

  jge_zero(ec, &table1[0]);
  jge_zero(ec, &table2[0]);

  wge_to_jge(ec, &table1[1], &p1);
  wge_to_jge(ec, &table2[1], &p2);

  for (i = 2; i < WND_SIZE; i += 2) {
    jge_dbl(ec, &table1[i], &table1[i >> 1]);
    jge_dbl(ec, &table2[i], &table2[i >> 1]);

    jge_mixed_add(ec, &table1[i + 1], &table1[i], &p1);
    jge_mixed_add(ec, &table2[i + 1], &table2[i], &p2);
  }

  jge_zero(ec, r);
  jge_zero(ec, &t);

  for (i = start; i >= 0; i--) {
    if (i != start) {
      for (j = 0; j < WND_WIDTH; j++)
        jge_dbl(ec, r, r);
    }

    b = sc_get_bits(sc, k1, i * WND_WIDTH, WND_WIDTH);

    for (j = 0; j < WND_SIZE; j++)
      jge_select(ec, &t, &t, &table1[j], j == b);

    jge_add(ec, r, r, &t);

    b = sc_get_bits(sc, k2, i * WND_WIDTH, WND_WIDTH);

    for (j = 0; j < WND_SIZE; j++)
      jge_select(ec, &t, &t, &table2[j], j == b);

    jge_add(ec, r, r, &t);
  }

  sc_cleanse(sc, k1);
  sc_cleanse(sc, k2);

  cleanse(&b, sizeof(b));
  cleanse(&s1, sizeof(s1));
  cleanse(&s2, sizeof(s2));
}

static void
wei_jmul(wei_t *ec, jge_t *r, const wge_t *p, const sc_t k) {
  if (ec->endo)
    wei_jmul_endo(ec, r, p, k);
  else
    wei_jmul_normal(ec, r, p, k);
}

static void
wei_mul(wei_t *ec, wge_t *r, const wge_t *p, const sc_t k) {
  jge_t j;
  wei_jmul(ec, &j, p, k);
  jge_to_wge(ec, r, &j);
}

static void
wei_jmul_double_normal_var(wei_t *ec,
                           jge_t *r,
                           const sc_t k1,
                           const wge_t *p2,
                           const sc_t k2) {
  /* Multiple point multiplication, also known
   * as "Shamir's trick" (with interleaved NAFs).
   *
   * [GECC] Algorithm 3.48, Page 109, Section 3.3.3.
   *        Algorithm 3.51, Page 112, Section 3.3.
   */
  scalar_field_t *sc = &ec->sc;
  wge_t *wnd1 = ec->points;
  jge_t wnd2[NAF_SIZE];
  int32_t naf1[MAX_SCALAR_BITS + 1];
  int32_t naf2[MAX_SCALAR_BITS + 1];
  size_t max1 = sc_bitlen_var(sc, k1) + 1;
  size_t max2 = sc_bitlen_var(sc, k2) + 1;
  size_t max = max1 > max2 ? max1 : max2;
  jge_t acc;
  size_t i;

  sc_naf_var(sc, naf1, k1, 1, NAF_WIDTH_PRE, max);
  sc_naf_var(sc, naf2, k2, 1, NAF_WIDTH, max);

  jge_naf_points_var(ec, wnd2, p2, NAF_WIDTH);

  /* Multiply and add. */
  jge_zero(ec, &acc);

  for (i = max; i-- > 0;) {
    int32_t z1 = naf1[i];
    int32_t z2 = naf2[i];

    if (i != max - 1)
      jge_dbl_var(ec, &acc, &acc);

    if (z1 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd1[(z1 - 1) >> 1]);
    else if (z1 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd1[(-z1 - 1) >> 1]);

    if (z2 > 0)
      jge_add_var(ec, &acc, &acc, &wnd2[(z2 - 1) >> 1]);
    else if (z2 < 0)
      jge_sub_var(ec, &acc, &acc, &wnd2[(-z2 - 1) >> 1]);
  }

  jge_set(ec, r, &acc);
}

static void
wei_jmul_double_endo_var(wei_t *ec,
                         jge_t *r,
                         const sc_t k1,
                         const wge_t *p2,
                         const sc_t k2) {
  /* Point multiplication with efficiently computable endomorphisms.
   *
   * [GECC] Algorithm 3.77, Page 129, Section 3.5.
   * [GLV] Page 193, Section 3 (Using Efficient Endomorphisms).
   */
  scalar_field_t *sc = &ec->sc;
  wge_t *wnd1 = ec->points;
  wge_t *wnd2 = ec->endo_points;
  wge_t wnd3[4];
  int32_t naf1[MAX_SCALAR_BITS + 1];
  int32_t naf2[MAX_SCALAR_BITS + 1];
  int32_t naf3[MAX_SCALAR_BITS + 1];
  sc_t c1, c2, c3, c4;
  int32_t s1, s2, s3, s4;
  size_t i, max;
  jge_t acc;

  assert(ec->endo == 1);

  /* Split scalars. */
  wei_endo_split(ec, c1, &s1, c2, &s2, k1);
  wei_endo_split(ec, c3, &s3, c4, &s4, k2);

  /* Compute max length. */
  max = sc_maxlen_var(sc, c1, c2, c3, c4) + 1;

  /* Compute NAFs. */
  sc_naf_var(sc, naf1, c1, s1, NAF_WIDTH_PRE, max);
  sc_naf_var(sc, naf2, c2, s2, NAF_WIDTH_PRE, max);
  sc_jsf_var(sc, naf3, c3, s3, c4, s4, max);

  /* Create comb for JSF. */
  wge_jsf_points_endo(ec, wnd3, p2);

  /* Multiply and add. */
  jge_zero(ec, &acc);

  for (i = max; i-- > 0;) {
    int32_t z1 = naf1[i];
    int32_t z2 = naf2[i];
    int32_t z3 = naf3[i];

    if (i != max - 1)
      jge_dbl_var(ec, &acc, &acc);

    if (z1 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd1[(z1 - 1) >> 1]);
    else if (z1 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd1[(-z1 - 1) >> 1]);

    if (z2 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd2[(z2 - 1) >> 1]);
    else if (z2 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd2[(-z2 - 1) >> 1]);

    if (z3 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd3[(z3 - 1) >> 1]);
    else if (z3 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd3[(-z3 - 1) >> 1]);
  }

  jge_set(ec, r, &acc);
}

static void
wei_jmul_double_var(wei_t *ec,
                    jge_t *r,
                    const sc_t k1,
                    const wge_t *p2,
                    const sc_t k2) {
  if (ec->endo)
    wei_jmul_double_endo_var(ec, r, k1, p2, k2);
  else
    wei_jmul_double_normal_var(ec, r, k1, p2, k2);
}

static void
wei_mul_double_var(wei_t *ec,
                     wge_t *r,
                     const sc_t k1,
                     const wge_t *p2,
                     const sc_t k2) {
  jge_t j;
  wei_jmul_double_var(ec, &j, k1, p2, k2);
  jge_to_wge_var(ec, r, &j);
}

static void
wei_jmul_multi_normal_var(wei_t *ec,
                          jge_t *r,
                          const sc_t k0,
                          const wge_t *points,
                          const sc_t *coeffs,
                          size_t len,
                          wei_scratch_t *scratch) {
  /* Multiple point multiplication, also known
   * as "Shamir's trick" (with interleaved NAFs).
   *
   * [GECC] Algorithm 3.48, Page 109, Section 3.3.3.
   *        Algorithm 3.51, Page 112, Section 3.3.
   */
  scalar_field_t *sc = &ec->sc;
  wge_t *wnd0 = ec->points;
  size_t max = sc_bitlen_var(sc, k0) + 1;
  int32_t naf0[MAX_SCALAR_BITS + 1];
  jge_t *wnds[32];
  int32_t *nafs[32];
  size_t i, j;
  jge_t acc;

  assert((len & 1) == 0);
  assert(len <= 64);

  /* Setup scratch. */
  for (i = 0; i < 32; i++) {
    wnds[i] = &scratch->wnd_normal[i * 4];
    nafs[i] = &scratch->naf[i * (MAX_SCALAR_BITS + 1)];
  }

  /* Compute max scalar size. */
  for (i = 0; i < len; i++) {
    size_t bits = sc_bitlen_var(sc, coeffs[i]) + 1;

    if (bits > max)
      max = bits;
  }

  /* Compute NAFs. */
  sc_naf_var(sc, naf0, k0, 1, NAF_WIDTH_PRE, max);

  for (i = 0; i < len; i += 2) {
    const wge_t *p1 = &points[i + 0];
    const wge_t *p2 = &points[i + 1];
    const sc_t *k1 = &coeffs[i + 0];
    const sc_t *k2 = &coeffs[i + 1];

    wge_jsf_points_var(ec, wnds[i >> 1], p1, p2);
    sc_jsf_var(sc, nafs[i >> 1], *k1, 1, *k2, 1, max);
  }

  len >>= 1;

  /* Multiply and add. */
  jge_zero(ec, &acc);

  for (i = max; i-- > 0;) {
    int32_t z0 = naf0[i];

    if (i != max - 1)
      jge_dbl_var(ec, &acc, &acc);

    if (z0 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd0[(z0 - 1) >> 1]);
    else if (z0 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd0[(-z0 - 1) >> 1]);

    for (j = 0; j < len; j++) {
      int32_t z = nafs[j][i];

      if (z > 0)
        jge_add_var(ec, &acc, &acc, &wnds[j][(z - 1) >> 1]);
      else if (z < 0)
        jge_sub_var(ec, &acc, &acc, &wnds[j][(-z - 1) >> 1]);
    }
  }

  jge_set(ec, r, &acc);
}

static void
wei_jmul_multi_endo_var(wei_t *ec,
                        jge_t *r,
                        const sc_t k0,
                        const wge_t *points,
                        const sc_t *coeffs,
                        size_t len,
                        wei_scratch_t *scratch) {
  /* Multiple point multiplication, also known
   * as "Shamir's trick" (with interleaved NAFs).
   *
   * [GECC] Algorithm 3.48, Page 109, Section 3.3.3.
   *        Algorithm 3.51, Page 112, Section 3.3.
   */
  scalar_field_t *sc = &ec->sc;
  wge_t *wnd0 = ec->points;
  wge_t *wnd1 = ec->endo_points;
  size_t max = ((sc->bits + 1) >> 1) + 1;
  int32_t naf0[MAX_SCALAR_BITS + 1];
  int32_t naf1[MAX_SCALAR_BITS + 1];
  wge_t *wnds[64];
  int32_t *nafs[64];
  sc_t k1, k2;
  int32_t s1, s2;
  size_t i, j;
  jge_t acc;

  assert(ec->endo == 1);
  assert(len <= 64);

  /* Setup scratch. */
  for (i = 0; i < 64; i++) {
    wnds[i] = &scratch->wnd_endo[i * 4];
    nafs[i] = &scratch->naf[i * (MAX_SCALAR_BITS + 1)];
  }

  /* Split scalar. */
  wei_endo_split(ec, k1, &s1, k2, &s2, k0);

  /* Compute NAFs. */
  sc_naf_var(sc, naf0, k1, s1, NAF_WIDTH_PRE, max);
  sc_naf_var(sc, naf1, k2, s2, NAF_WIDTH_PRE, max);

  for (i = 0; i < len; i++) {
    /* Split scalar. */
    wei_endo_split(ec, k1, &s1, k2, &s2, coeffs[i]);

    /* Compute NAF.*/
    sc_jsf_var(sc, nafs[i], k1, s1, k2, s2, max);

    /* Create comb for JSF. */
    wge_jsf_points_endo(ec, wnds[i], &points[i]);

#ifdef TORSION_TEST
    /* Check max size.*/
    assert(sc_bitlen_var(sc, k1) + 1 <= max);
    assert(sc_bitlen_var(sc, k2) + 1 <= max);
#endif
  }

  /* Multiply and add. */
  jge_zero(ec, &acc);

  for (i = max; i-- > 0;) {
    int32_t z0 = naf0[i];
    int32_t z1 = naf1[i];

    if (i != max - 1)
      jge_dbl_var(ec, &acc, &acc);

    if (z0 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd0[(z0 - 1) >> 1]);
    else if (z0 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd0[(-z0 - 1) >> 1]);

    if (z1 > 0)
      jge_mixed_add_var(ec, &acc, &acc, &wnd1[(z1 - 1) >> 1]);
    else if (z1 < 0)
      jge_mixed_sub_var(ec, &acc, &acc, &wnd1[(-z1 - 1) >> 1]);

    for (j = 0; j < len; j++) {
      int32_t z = nafs[j][i];

      if (z > 0)
        jge_mixed_add_var(ec, &acc, &acc, &wnds[j][(z - 1) >> 1]);
      else if (z < 0)
        jge_mixed_sub_var(ec, &acc, &acc, &wnds[j][(-z - 1) >> 1]);
    }
  }

  jge_set(ec, r, &acc);
}

static void
wei_jmul_multi_var(wei_t *ec,
                   jge_t *r,
                   const sc_t k0,
                   const wge_t *points,
                   const sc_t *coeffs,
                   size_t len,
                   wei_scratch_t *scratch) {
  if (ec->endo)
    wei_jmul_multi_endo_var(ec, r, k0, points, coeffs, len, scratch);
  else
    wei_jmul_multi_normal_var(ec, r, k0, points, coeffs, len, scratch);
}

static void
wei_mul_multi_var(wei_t *ec,
                   wge_t *r,
                   const sc_t k0,
                   const wge_t *points,
                   const sc_t *coeffs,
                   size_t len,
                   wei_scratch_t *scratch) {
  jge_t j;
  wei_jmul_multi_var(ec, &j, k0, points, coeffs, len, scratch);
  jge_to_wge_var(ec, r, &j);
}

static void
wei_randomize(wei_t *ec, const unsigned char *entropy) {
  scalar_field_t *sc = &ec->sc;
  sc_t blind;
  wge_t unblind;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  sc_random(sc, blind, &rng);

  wei_mul_g(ec, &unblind, blind);

  sc_neg(sc, ec->blind, blind);
  wge_set(ec, &ec->unblind, &unblind);

  sc_cleanse(sc, blind);
  wge_cleanse(ec, &unblind);
  cleanse(&rng, sizeof(rng));
}

static void
wei_solve_y2(wei_t *ec, fe_t r, const fe_t x) {
  /* [GECC] Page 89, Section 3.2.2. */
  prime_field_t *fe = &ec->fe;
  fe_t y2, ax;

  /* y^2 = x^3 + a * x + b */
  fe_sqr(fe, y2, x);
  fe_mul(fe, y2, y2, x);
  fe_mul(fe, ax, ec->a, x);
  fe_add(fe, y2, y2, ax);
  fe_add(fe, r, y2, ec->b);
}

static void
wei_sswu(wei_t *ec, wge_t *p, const fe_t u) {
  /* Simplified Shallue-Woestijne-Ulas Method.
   *
   * Distribution: 3/8.
   *
   * [SSWU1] Page 15-16, Section 7. Appendix G.
   * [SSWU2] Page 5, Theorem 2.3.
   * [H2EC] "Simplified Shallue-van de Woestijne-Ulas Method".
   *
   * Assumptions:
   *
   *   - a != 0, b != 0.
   *   - Let z be a non-square in F(p).
   *   - z != -1.
   *   - The polynomial g(x) - z is irreducible over F(p).
   *   - g(b / (z * a)) is square in F(p).
   *   - u != 0, u != +-sqrt(-1 / z).
   *
   * Map:
   *
   *   g(x) = x^3 + a * x + b
   *   t1 = 1 / (z^2 * u^4 + z * u^2)
   *   x1 = (-b / a) * (1 + t1)
   *   x1 = b / (z * a), if t1 = 0
   *   x2 = z * u^2 * x1
   *   x = x1, if g(x1) is square
   *     = x2, otherwise
   *   y = sign(u) * abs(sqrt(g(x)))
   */
  prime_field_t *fe = &ec->fe;
  fe_t z2, ba, bza, u2, u4, t1, x1, x2, y1, y2;
  int zero, alpha;

  fe_sqr(fe, z2, ec->z);
  fe_neg(fe, ba, ec->b);
  fe_mul(fe, ba, ba, ec->ai);
  fe_mul(fe, bza, ec->b, ec->zi);
  fe_mul(fe, bza, bza, ec->ai);

  fe_sqr(fe, u2, u);
  fe_sqr(fe, u4, u2);

  fe_mul(fe, x1, ec->z, u2);
  fe_mul(fe, t1, z2, u4);
  fe_add(fe, t1, t1, x1);
  zero = fe_invert(fe, t1, t1) ^ 1;

  fe_add(fe, t1, t1, fe->one);
  fe_mul(fe, x1, ba, t1);

  fe_select(fe, x1, x1, bza, zero);

  fe_mul(fe, x2, ec->z, u2);
  fe_mul(fe, x2, x2, x1);

  wei_solve_y2(ec, y1, x1);
  wei_solve_y2(ec, y2, x2);

  alpha = fe_is_square(fe, y1);

  fe_select(fe, x1, x1, x2, alpha ^ 1);
  fe_select(fe, y1, y1, y2, alpha ^ 1);
  fe_sqrt(fe, y1, y1);

  fe_set_odd(fe, y1, y1, fe_is_odd(fe, u));

  wge_set_xy(ec, p, x1, y1);
}

static int
wei_sswui(wei_t *ec, fe_t u, const wge_t *p, unsigned int hint) {
  /* Inverting the Map (Simplified Shallue-Woestijne-Ulas).
   *
   * Assumptions:
   *
   *   - a^2 * x^2 - 2 * a * b * x - 3 * b^2 is square in F(p).
   *   - If r < 3 then x != -b / a.
   *
   * Unlike SVDW, the preimages here are evenly
   * distributed (more or less). SSWU covers ~3/8
   * of the curve points. Each preimage has a 1/2
   * chance of mapping to either x1 or x2.
   *
   * Assuming the point is within that set, each
   * point has a 1/4 chance of inverting to any
   * of the preimages. This means we can simply
   * randomly select a preimage if one exists.
   *
   * However, the [SVDW2] sampling method seems
   * slighly faster in practice for [SQUARED].
   *
   * Map:
   *
   *   c = sqrt(a^2 * x^2 - 2 * a * b * x - 3 * b^2)
   *   u1 = -(a * x + b - c) / (2 * (a * x + b) * z)
   *   u2 = -(a * x + b + c) / (2 * (a * x + b) * z)
   *   u3 = -(a * x + b - c) / (2 * b * z)
   *   u4 = -(a * x + b + c) / (2 * b * z)
   *   r = random integer in [1,4]
   *   u = sign(y) * abs(sqrt(ur))
   */
  prime_field_t *fe = &ec->fe;
  fe_t a2x2, abx2, b23, axb, c, n0, n1, d0, d1;
  unsigned int r = hint & 3;
  unsigned s0, s1;

  fe_sqr(fe, n0, ec->a);
  fe_sqr(fe, n1, p->x);
  fe_mul(fe, a2x2, n0, n1);

  fe_mul(fe, abx2, ec->a, ec->b);
  fe_mul(fe, abx2, abx2, p->x);
  fe_add(fe, abx2, abx2, abx2);

  fe_sqr(fe, b23, ec->b);
  fe_mul_word(fe, b23, b23, 3);

  fe_mul(fe, axb, ec->a, p->x);
  fe_add(fe, axb, axb, ec->b);

  fe_sub(fe, c, a2x2, abx2);
  fe_sub(fe, c, c, b23);
  s0 = fe_sqrt(fe, c, c);

  fe_sub(fe, n0, axb, c);
  fe_neg(fe, n0, n0);

  fe_add(fe, n1, axb, c);
  fe_neg(fe, n1, n1);

  fe_mul(fe, d0, axb, ec->z);
  fe_add(fe, d0, d0, d0);

  fe_mul(fe, d1, ec->b, ec->z);
  fe_add(fe, d1, d1, d1);

  fe_select(fe, n0, n0, n1, r & 1); /* r = 1 or 3 */
  fe_select(fe, d0, d0, d1, r >> 1); /* r = 2 or 3 */

  s1 = fe_isqrt(fe, u, n0, d0);

  fe_set_odd(fe, u, u, fe_is_odd(fe, p->y));

  return s0 & s1 & (p->inf ^ 1);
}

static void
wei_svdwf(wei_t *ec, fe_t x, fe_t y, const fe_t u) {
  /* Shallue-van de Woestijne Method.
   *
   * Distribution: 9/16.
   *
   * [SVDW1] Section 5.
   * [SVDW2] Page 8, Section 3.
   *         Page 15, Section 6, Algorithm 1.
   * [H2EC] "Shallue-van de Woestijne Method".
   *
   * Assumptions:
   *
   *   - p = 1 (mod 3).
   *   - a = 0, b != 0.
   *   - Let z be a unique element in F(p).
   *   - g((sqrt(-3 * z^2) - z) / 2) is square in F(p).
   *   - u != 0, u != +-sqrt(-g(z)).
   *
   * Map:
   *
   *   g(x) = x^3 + b
   *   c = sqrt(-3 * z^2)
   *   t1 = u^2 + g(z)
   *   t2 = 1 / (u^2 * t1)
   *   t3 = u^4 * t2 * c
   *   x1 = (c - z) / 2 - t3
   *   x2 = t3 - (c + z) / 2
   *   x3 = z - t1^3 * t2 / (3 * z^2)
   *   x = x1, if g(x1) is square
   *     = x2, if g(x2) is square
   *     = x3, otherwise
   *   y = sign(u) * abs(sqrt(g(x)))
   */
  prime_field_t *fe = &ec->fe;
  fe_t gz, z3, u2, u4, t1, t2, t3, t4, x1, x2, x3, y1, y2, y3;
  unsigned int alpha, beta;

  wei_solve_y2(ec, gz, ec->z);

  fe_sqr(fe, z3, ec->zi);
  fe_mul(fe, z3, z3, ec->i3);

  fe_sqr(fe, u2, u);
  fe_sqr(fe, u4, u2);

  fe_add(fe, t1, u2, gz);

  fe_mul(fe, t2, u2, t1);
  fe_invert(fe, t2, t2);

  fe_mul(fe, t3, u4, t2);
  fe_mul(fe, t3, t3, ec->c);

  fe_sqr(fe, t4, t1);
  fe_mul(fe, t4, t4, t1);

  fe_sub(fe, x1, ec->c, ec->z);
  fe_mul(fe, x1, x1, ec->i2);
  fe_sub(fe, x1, x1, t3);

  fe_add(fe, y1, ec->c, ec->z);
  fe_mul(fe, y1, y1, ec->i2);
  fe_sub(fe, x2, t3, y1);

  fe_mul(fe, y1, t4, t2);
  fe_mul(fe, y1, y1, z3);
  fe_sub(fe, x3, ec->z, y1);

  wei_solve_y2(ec, y1, x1);
  wei_solve_y2(ec, y2, x2);
  wei_solve_y2(ec, y3, x3);

  alpha = fe_is_square(fe, y1);
  beta = fe_is_square(fe, y2);

  fe_select(fe, x1, x1, x2, (alpha ^ 1) & beta);
  fe_select(fe, y1, y1, y2, (alpha ^ 1) & beta);
  fe_select(fe, x1, x1, x3, (alpha ^ 1) & (beta ^ 1));
  fe_select(fe, y1, y1, y3, (alpha ^ 1) & (beta ^ 1));

  fe_set(fe, x, x1);
  fe_set(fe, y, y1);
}

static void
wei_svdw(wei_t *ec, wge_t *p, const fe_t u) {
  prime_field_t *fe = &ec->fe;
  fe_t x, y;

  wei_svdwf(ec, x, y, u);

  fe_sqrt(fe, y, y);
  fe_set_odd(fe, y, y, fe_is_odd(fe, u));

  wge_set_xy(ec, p, x, y);
}

static int
wei_svdwi(wei_t *ec, fe_t u, const wge_t *p, unsigned int hint) {
  /* Inverting the Map (Shallue-van de Woestijne).
   *
   * [SQUARED] Algorithm 1, Page 8, Section 3.3.
   * [SVDW2] Page 12, Section 5.
   * [SVDW3] "Inverting the map".
   *
   * Assumptions:
   *
   *   - If r = 1 then x != -(c + z) / 2.
   *   - If r = 2 then x != (c - z) / 2.
   *   - If r > 2 then (t0 - t1 + t2) is square in F(p).
   *   - f(f^-1(x)) = x where f is the map function.
   *
   * We use the sampling method from [SVDW2],
   * _not_ [SQUARED]. This seems to have a
   * better distribution in practice.
   *
   * Note that [SVDW3] also appears to be
   * incorrect in terms of distribution.
   *
   * The distribution of f(u), assuming u is
   * random, is (1/2, 1/4, 1/4).
   *
   * To mirror this, f^-1(x) should simply
   * pick (1/2, 1/4, 1/8, 1/8).
   *
   * To anyone running the forward map, our
   * strings will appear to be random.
   *
   * Map:
   *
   *   g(x) = x^3 + b
   *   c = sqrt(-3 * z^2)
   *   t0 = 9 * (x^2 * z^2 + z^4)
   *   t1 = 18 * x * z^3
   *   t2 = 12 * g(z) * (x - z)
   *   t3 = sqrt(t0 - t1 + t2)
   *   t4 = t3 * z
   *   u1 = g(z) * (c - 2 * x - z) / (c + 2 * x + z)
   *   u2 = g(z) * (c + 2 * x + z) / (c - 2 * x - z)
   *   u3 = (3 * (z^3 - x * z^2) - 2 * g(z) + t4) / 2
   *   u4 = (3 * (z^3 - x * z^2) - 2 * g(z) - t4) / 2
   *   r = random integer in [1,4]
   *   u = sign(y) * abs(sqrt(ur))
   */
  prime_field_t *fe = &ec->fe;
  fe_t z2, z3, z4, gz, c0, c1, t4, t5, n0, n1, n2, n3, d0;
  uint32_t r = hint & 3;
  uint32_t s0, s1, s2, s3;

  fe_sqr(fe, z2, ec->z);
  fe_mul(fe, z3, z2, ec->z);
  fe_sqr(fe, z4, z2);
  fe_add(fe, gz, z3, ec->b);

  fe_sqr(fe, n0, p->x);
  fe_mul(fe, n0, n0, z2);
  fe_add(fe, n0, n0, z4);
  fe_mul_word(fe, n0, n0, 9);

  fe_mul(fe, n1, p->x, z3);
  fe_mul_word(fe, n1, n1, 18);

  fe_sub(fe, n2, p->x, ec->z);
  fe_mul(fe, n2, n2, gz);
  fe_mul_word(fe, n2, n2, 12);

  fe_sub(fe, t4, n0, n1);
  fe_add(fe, t4, t4, n2);
  s0 = fe_sqrt(fe, t4, t4);
  s1 = ((r - 2) >> 31) | s0;
  fe_mul(fe, t4, t4, ec->z);

  fe_mul(fe, n0, p->x, z2);
  fe_add(fe, n1, gz, gz);
  fe_sub(fe, t5, z3, n0);
  fe_mul_word(fe, t5, t5, 3);
  fe_sub(fe, t5, t5, n1);

  fe_add(fe, n0, p->x, p->x);
  fe_add(fe, n0, n0, ec->z);

  fe_sub(fe, c0, ec->c, n0);
  fe_add(fe, c1, ec->c, n0);

  fe_mul(fe, n0, gz, c0);
  fe_mul(fe, n1, gz, c1);
  fe_add(fe, n2, t5, t4);
  fe_sub(fe, n3, t5, t4);
  fe_set(fe, d0, fe->two);

  fe_select(fe, n0, n0, n1, ((r ^ 1) - 1) >> 31); /* r = 1 */
  fe_select(fe, n0, n0, n2, ((r ^ 2) - 1) >> 31); /* r = 2 */
  fe_select(fe, n0, n0, n3, ((r ^ 3) - 1) >> 31); /* r = 3 */
  fe_select(fe, d0, d0, c1, ((r ^ 0) - 1) >> 31); /* r = 0 */
  fe_select(fe, d0, d0, c0, ((r ^ 1) - 1) >> 31); /* r = 1 */

  s2 = fe_isqrt(fe, u, n0, d0);

  wei_svdwf(ec, n0, n1, u);

  s3 = fe_equal(fe, n0, p->x);

  fe_set_odd(fe, u, u, fe_is_odd(fe, p->y));

  return s1 & s2 & s3 & (p->inf ^ 1);
}

static void
wei_point_from_uniform(wei_t *ec, wge_t *p, const unsigned char *bytes) {
  prime_field_t *fe = &ec->fe;
  fe_t u;

  fe_import_uniform(fe, u, bytes);

  if (ec->zero_a)
    wei_svdw(ec, p, u);
  else
    wei_sswu(ec, p, u);

  fe_cleanse(fe, u);
}

static int
wei_point_to_uniform(wei_t *ec,
                     unsigned char *bytes,
                     const wge_t *p,
                     unsigned int hint) {
  prime_field_t *fe = &ec->fe;
  fe_t u;
  int ret;

  if (ec->zero_a)
    ret = wei_svdwi(ec, u, p, hint);
  else
    ret = wei_sswui(ec, u, p, hint);

  fe_export(fe, bytes, u);
  fe_cleanse(fe, u);

  bytes[0] |= (hint >> 8) & ~fe->mask;

  return ret;
}

static void
wei_point_from_hash(wei_t *ec, wge_t *p, const unsigned char *bytes) {
  /* [H2EC] "Roadmap". */
  wge_t p1, p2;

  wei_point_from_uniform(ec, &p1, bytes);
  wei_point_from_uniform(ec, &p2, bytes + ec->fe.size);

  wge_add(ec, p, &p1, &p2);

  wge_cleanse(ec, &p1);
  wge_cleanse(ec, &p2);
}

static void
wei_point_to_hash(wei_t *ec,
                  unsigned char *bytes,
                  const wge_t *p,
                  const unsigned char *entropy) {
  /* [SQUARED] Algorithm 1, Page 8, Section 3.3. */
  prime_field_t *fe = &ec->fe;
  unsigned int hint;
  wge_t p1, p2;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  for (;;) {
    drbg_generate(&rng, bytes, fe->size);

    if (!bytes_lt(bytes, fe->raw, fe->size, fe->endian))
      continue;

    wei_point_from_uniform(ec, &p1, bytes);

    /* Avoid 2-torsion points. */
    if (ec->h > 1 && fe_is_zero(fe, p1.y))
      continue;

    wge_sub(ec, &p2, p, &p1);

    drbg_generate(&rng, &hint, sizeof(hint));

    if (!wei_point_to_uniform(ec, bytes + fe->size, &p2, hint))
      continue;

    break;
  }

  cleanse(&rng, sizeof(rng));
  cleanse(&hint, sizeof(hint));

  wge_cleanse(ec, &p1);
  wge_cleanse(ec, &p2);
}

/*
 * Montgomery
 */

static void
mont_mul_a24(mont_t *ec, fe_t r, const fe_t a);

static void
_mont_to_edwards(prime_field_t *fe, xge_t *r,
                 const mge_t *p, const fe_t c,
                 int invert, int isogeny);

static void
pge_import(mont_t *ec, pge_t *r, const unsigned char *raw);

static int
pge_to_mge(mont_t *ec, mge_t *r, const pge_t *p, int sign);

/*
 * Montgomery Affine Point
 */

static void
mge_zero(mont_t *ec, mge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_zero(fe, r->x);
  fe_zero(fe, r->y);
  r->inf = 1;
}

static void
mge_cleanse(mont_t *ec, mge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_cleanse(fe, r->x);
  fe_cleanse(fe, r->y);
  r->inf = 1;
}

static int
mge_validate(mont_t *ec, const mge_t *p) {
  /* [MONT3] Page 3, Section 2. */
  /* B * y^2 = x^3 + A * x^2 + x */
  prime_field_t *fe = &ec->fe;
  fe_t lhs, rhs, x2, x3;

  fe_sqr(fe, lhs, p->y);
  fe_mul(fe, lhs, ec->b, lhs);
  fe_sqr(fe, x2, p->x);
  fe_mul(fe, x3, x2, p->x);
  fe_mul(fe, x2, ec->a, x2);
  fe_add(fe, rhs, x3, x2);
  fe_add(fe, rhs, rhs, p->x);

  return fe_equal(fe, lhs, rhs) | p->inf;
}

static int
mge_set_x(mont_t *ec, mge_t *r, const fe_t x, int sign) {
  /* [MONT3] Page 3, Section 2. */
  /* B * y^2 = x^3 + A * x^2 + x */
  prime_field_t *fe = &ec->fe;
  fe_t y, x2, x3;
  int ret;

  fe_sqr(fe, x2, x);
  fe_mul(fe, x3, x2, x);
  fe_mul(fe, x2, ec->a, x2);
  fe_add(fe, y, x3, x2);
  fe_add(fe, y, y, x);
  fe_mul(fe, y, y, ec->bi);

  ret = fe_sqrt(fe, y, y);

  if (sign != -1)
    fe_set_odd(fe, y, y, sign);

  fe_set(fe, r->x, x);
  fe_set(fe, r->y, y);
  r->inf = 0;

#ifdef TORSION_TEST
  assert(mge_validate(ec, r) == ret);
#endif

  return ret;
}

static void
mge_set_xy(mont_t *ec, mge_t *r, const fe_t x, const fe_t y) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, x);
  fe_set(fe, r->y, y);
  r->inf = 0;
}

static int
mge_import(mont_t *ec, mge_t *r, const unsigned char *raw, int sign) {
  pge_t p;
  pge_import(ec, &p, raw);
  return pge_to_mge(ec, r, &p, sign);
}

static int
mge_export(mont_t *ec,
          unsigned char *raw,
          const mge_t *p) {
  /* [RFC7748] Section 5. */
  prime_field_t *fe = &ec->fe;

  if (p->inf)
    return 0;

  fe_export(fe, raw, p->x);

  return 1;
}

static void
mge_swap(mont_t *ec, mge_t *a, mge_t *b, unsigned int flag) {
  prime_field_t *fe = &ec->fe;
  int cond = (flag != 0);
  int inf1 = a->inf;
  int inf2 = b->inf;

  fe_swap(fe, a->x, b->x, flag);
  fe_swap(fe, a->y, b->y, flag);

  a->inf = (inf1 & (cond ^ 1)) | (inf2 & cond);
  b->inf = (inf2 & (cond ^ 1)) | (inf1 & cond);
}

static void
mge_set(mont_t *ec, mge_t *r, const mge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_set(fe, r->y, a->y);
  r->inf = a->inf;
}

static int
mge_equal(mont_t *ec, const mge_t *a, const mge_t *b) {
  prime_field_t *fe = &ec->fe;
  int both = a->inf & b->inf;
  int ret = 1;

  /* P = O, Q = O */
  ret &= (a->inf ^ b->inf) ^ 1;

  /* X1 = X2 */
  ret &= fe_equal(fe, a->x, b->x) | both;

  /* Y1 = Y2 */
  ret &= fe_equal(fe, a->y, b->y) | both;

  return ret;
}

static int
mge_is_zero(mont_t *ec, const mge_t *a) {
  return a->inf;
}

static void
mge_neg(mont_t *ec, mge_t *r, const mge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_neg(fe, r->y, a->y);
  r->inf = a->inf;
}

static void
mge_dbl(mont_t *ec, mge_t *r, const mge_t *p) {
  /* [MONT1] Page 8, Section 4.3.2.
   *
   * Addition Law (doubling):
   *
   *   l = (3 * x1^2 + 2 * a * x1 + 1) / (2 * b * y1)
   *   x3 = b * l^2 - a - 2 * x1
   *   y3 = l * (x1 - x3) - y1
   *
   * 1I + 3M + 2S + 7A + 1*a + 1*b + 1*b + 2*2 + 1*3
   */
  prime_field_t *fe = &ec->fe;
  int inf = p->inf | fe_is_zero(fe, p->y);
  fe_t l, t, x3, y3;

  /* L = (3 * X1^2 + 2 * a * X1 + 1) / (2 * b * Y1) */
  fe_add(fe, x3, ec->a, ec->a);
  fe_mul(fe, x3, x3, p->x);
  fe_add(fe, x3, x3, fe->one);
  fe_sqr(fe, t, p->x);
  fe_add(fe, l, t, t);
  fe_add(fe, l, l, t);
  fe_add(fe, l, l, x3);
  fe_add(fe, t, p->y, p->y);
  fe_mul(fe, t, t, ec->b);
  fe_invert(fe, t, t);
  fe_mul(fe, l, l, t);

  /* X3 = b * L^2 - a - 2 * X1 */
  fe_sqr(fe, x3, l);
  fe_mul(fe, x3, x3, ec->b);
  fe_sub(fe, x3, x3, ec->a);
  fe_sub(fe, x3, x3, p->x);
  fe_sub(fe, x3, x3, p->x);

  /* Y3 = L * (X1 - X3) - Y1 */
  fe_sub(fe, y3, p->x, x3);
  fe_mul(fe, y3, y3, l);
  fe_sub(fe, y3, y3, p->y);

  fe_select(fe, r->x, x3, fe->zero, inf);
  fe_select(fe, r->y, y3, fe->zero, inf);
  r->inf = inf;
}

static void
mge_add(mont_t *ec, mge_t *r, const mge_t *a, const mge_t *b) {
  /* [MONT1] Page 8, Section 4.3.2.
   *
   * Addition Law:
   *
   *   l = (y2 - y1) / (x2 - x1)
   *   x3 = b * l^2 - a - x1 - x2
   *   y3 = l * (x1 - x3) - y1
   *
   * If we detect a doubling, we
   * switch the lambda to:
   *
   *   l = (3 * x1^2 + 2 * a * x1 + 1) / (2 * b * y1)
   *
   * 1I + 3M + 2S + 9A + 1*a + 2*b + 2*2 + 1*3
   */
  prime_field_t *fe = &ec->fe;
  fe_t h, r0, m, z, l, x3, y3;
  int dbl, neg, inf;

  /* H = X2 - X1 */
  fe_sub(fe, h, b->x, a->x);

  /* R = Y2 - Y1 */
  fe_sub(fe, r0, b->y, a->y);

  /* M = (3 * X1^2) + (2 * a * X1) + 1 */
  fe_add(fe, x3, ec->a, ec->a);
  fe_mul(fe, x3, x3, a->x);
  fe_add(fe, x3, x3, fe->one);
  fe_sqr(fe, z, a->x);
  fe_add(fe, m, z, z);
  fe_add(fe, m, m, z);
  fe_add(fe, m, m, x3);

  /* Z = 2 * b * Y1 */
  fe_add(fe, z, a->y, a->y);
  fe_mul(fe, z, z, ec->b);

  /* Check for doubling (X1 = X2, Y1 = Y2). */
  dbl = fe_is_zero(fe, h) & fe_is_zero(fe, r0);

  /* R = M (if dbl) */
  fe_select(fe, r0, r0, m, dbl);

  /* H = Z (if dbl) */
  fe_select(fe, h, h, z, dbl);

  /* Check for negation (X1 = X2, Y1 = -Y2). */
  neg = fe_is_zero(fe, h) & ((a->inf | b->inf) ^ 1);

  /* L = R / H */
  fe_invert(fe, h, h);
  fe_mul(fe, l, r0, h);

  /* X3 = b * L^2 - a - X1 - X2 */
  fe_sqr(fe, x3, l);
  fe_mul(fe, x3, x3, ec->b);
  fe_sub(fe, x3, x3, ec->a);
  fe_sub(fe, x3, x3, a->x);
  fe_sub(fe, x3, x3, b->x);

  /* Y3 = L * (X1 - X3) - Y1 */
  fe_sub(fe, y3, a->x, x3);
  fe_mul(fe, y3, y3, l);
  fe_sub(fe, y3, y3, a->y);

  /* Check for infinity. */
  inf = neg | (a->inf & b->inf);

  /* Case 1: O + P = P */
  fe_select(fe, x3, x3, b->x, a->inf);
  fe_select(fe, y3, y3, b->y, a->inf);

  /* Case 2: P + O = P */
  fe_select(fe, x3, x3, a->x, b->inf);
  fe_select(fe, y3, y3, a->y, b->inf);

  /* Case 3 & 4: P + -P = O, O + O = O */
  fe_select(fe, x3, x3, fe->zero, inf);
  fe_select(fe, y3, y3, fe->zero, inf);

  fe_set(fe, r->x, x3);
  fe_set(fe, r->y, y3);
  r->inf = inf;
}

static void
mge_sub(mont_t *ec, mge_t *r, const mge_t *a, const mge_t *b) {
  mge_t c;
  mge_neg(ec, &c, b);
  mge_add(ec, r, a, &c);
}

static void
mge_to_pge(mont_t *ec, pge_t *r, const mge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_select(fe, r->x, a->x, fe->one, a->inf);
  fe_select(fe, r->z, fe->one, fe->zero, a->inf);
}

static void
mge_to_xge(mont_t *ec, xge_t *r, const mge_t *p) {
  _mont_to_edwards(&ec->fe, r, p, ec->c, ec->invert, 1);
}

#ifdef TORSION_TEST
static void
mge_print(mont_t *ec, const mge_t *p) {
  prime_field_t *fe = &ec->fe;

  if (mge_is_zero(ec, p)) {
    printf("(infinity)\n");
  } else {
    mp_limb_t xp[MAX_FIELD_LIMBS];
    mp_limb_t yp[MAX_FIELD_LIMBS];

    fe_get_limbs(fe, xp, p->x);
    fe_get_limbs(fe, yp, p->y);

    printf("(");
    mpn_print(xp, fe->limbs, 16);
    printf(", ");
    mpn_print(yp, fe->limbs, 16);
    printf(")\n");
  }
}
#endif

/*
 * Montgomery Projective Point
 */

static void
pge_zero(mont_t *ec, pge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, fe->one);
  fe_zero(fe, r->z);
}

static void
pge_cleanse(mont_t *ec, pge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_cleanse(fe, r->x);
  fe_cleanse(fe, r->z);
}

static int
pge_validate(mont_t *ec, const pge_t *p) {
  prime_field_t *fe = &ec->fe;
  fe_t x2, x3, z2, ax2, xz2, y2;

  /* B * y^2 * z = x^3 + A * x^2 * z + x * z^2 */
  fe_sqr(fe, x2, p->x);
  fe_mul(fe, x3, x2, p->x);
  fe_sqr(fe, z2, p->z);
  fe_mul(fe, ax2, ec->a, x2);
  fe_mul(fe, ax2, ax2, p->z);
  fe_mul(fe, xz2, p->x, z2);
  fe_add(fe, y2, x3, ax2);
  fe_add(fe, y2, y2, xz2);
  fe_mul(fe, y2, y2, ec->bi);
  fe_mul(fe, y2, y2, p->z);

  /* sqrt(y^2 * z^4) = y * z^2 */
  return fe_is_square(fe, y2);
}

static void
pge_import(mont_t *ec, pge_t *r, const unsigned char *raw) {
  /* [RFC7748] Section 5. */
  prime_field_t *fe = &ec->fe;

  fe_import_uniform(fe, r->x, raw);
  fe_set(fe, r->z, fe->one);
}

static int
pge_export(mont_t *ec,
          unsigned char *raw,
          const pge_t *p) {
  /* [RFC7748] Section 5. */
  prime_field_t *fe = &ec->fe;
  int ret;
  fe_t x;

  ret = fe_invert(fe, x, p->z);

  fe_mul(fe, x, x, p->x);
  fe_export(fe, raw, x);

  return ret;
}

static void
pge_swap(mont_t *ec, pge_t *a, pge_t *b, unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_swap(fe, a->x, b->x, flag);
  fe_swap(fe, a->z, b->z, flag);
}

static void
pge_set(mont_t *ec, pge_t *r, const pge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_set(fe, r->z, a->z);
}

static int
pge_is_zero(mont_t *ec, const pge_t *a) {
  prime_field_t *fe = &ec->fe;

  return fe_is_zero(fe, a->z);
}

static int
pge_equal(mont_t *ec, const pge_t *a, const pge_t *b) {
  prime_field_t *fe = &ec->fe;
  fe_t e1, e2;

  /* X1 * Z2 == X2 * Z1 */
  fe_mul(fe, e1, a->x, b->z);
  fe_mul(fe, e2, b->x, a->z);

  return fe_equal(fe, e1, e2);
}

static void
pge_dbl(mont_t *ec, pge_t *r, const pge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#doubling-dbl-1987-m-3
   * 2M + 2S + 4A + 1*a24
   */
  prime_field_t *fe = &ec->fe;
  fe_t a, aa, b, bb, c;

  /* A = X1 + Z1 */
  fe_add(fe, a, p->x, p->z);

  /* AA = A^2 */
  fe_sqr(fe, aa, a);

  /* B = X1 - Z1 */
  fe_sub(fe, b, p->x, p->z);

  /* BB = B^2 */
  fe_sqr(fe, bb, b);

  /* C = AA - BB */
  fe_sub(fe, c, aa, bb);

  /* X3 = AA * BB */
  fe_mul(fe, r->x, aa, bb);

  /* Z3 = C * (BB + a24 * C) */
  mont_mul_a24(ec, r->z, c);
  fe_add(fe, r->z, r->z, bb);
  fe_mul(fe, r->z, r->z, c);
}

static void
pge_ladder(mont_t *ec,
           pge_t *p4,
           pge_t *p5,
           const pge_t *p1,
           const pge_t *p2,
           const pge_t *p3) {
  /* https://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#ladder-ladd-1987-m-3
   * 6M + 4S + 8A + 1*a24
   */
  prime_field_t *fe = &ec->fe;
  fe_t a, aa, b, bb, e, c, d, da, cb;

  assert(p1 != p5);
  assert(p4 != p5);

  /* A = X2 + Z2 */
  fe_add(fe, a, p2->x, p2->z);

  /* AA = A^2 */
  fe_sqr(fe, aa, a);

  /* B = X2 - Z2 */
  fe_sub(fe, b, p2->x, p2->z);

  /* BB = B^2 */
  fe_sqr(fe, bb, b);

  /* E = AA - BB */
  fe_sub(fe, e, aa, bb);

  /* C = X3 + Z3 */
  fe_add(fe, c, p3->x, p3->z);

  /* D = X3 - Z3 */
  fe_sub(fe, d, p3->x, p3->z);

  /* DA = D * A */
  fe_mul(fe, da, d, a);

  /* CB = C * B */
  fe_mul(fe, cb, c, b);

  /* X5 = Z1 * (DA + CB)^2 */
  fe_add(fe, p5->x, da, cb);
  fe_sqr(fe, p5->x, p5->x);
  fe_mul(fe, p5->x, p5->x, p1->z);

  /* Z5 = X1 * (DA - CB)^2 */
  fe_sub(fe, p5->z, da, cb);
  fe_sqr(fe, p5->z, p5->z);
  fe_mul(fe, p5->z, p5->z, p1->x);

  /* X4 = AA * BB */
  fe_mul(fe, p4->x, aa, bb);

  /* Z4 = E * (BB + a24 * E) */
  mont_mul_a24(ec, p4->z, e);
  fe_add(fe, p4->z, p4->z, bb);
  fe_mul(fe, p4->z, p4->z, e);
}

static int
pge_to_mge(mont_t *ec, mge_t *r, const pge_t *p, int sign) {
  /* https://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#scaling-scale
   * 1I + 1M
   */
  prime_field_t *fe = &ec->fe;
  fe_t a;

  /* A = 1 / Z1 */
  r->inf = fe_invert(fe, a, p->z) ^ 1;

  /* X3 = X1 * A */
  fe_mul(fe, r->x, p->x, a);

  return mge_set_x(ec, r, r->x, sign);
}

static void
pge_mulh(mont_t *ec, pge_t *r, const pge_t *p) {
  int bits = bit_length(ec->h);
  int i;

  pge_set(ec, r, p);

  for (i = 0; i < bits - 1; i++)
    pge_dbl(ec, r, r);
}

static int
pge_is_small(mont_t *ec, const pge_t *p) {
  pge_t r;

  pge_mulh(ec, &r, p);

  return pge_is_zero(ec, &r)
      & (pge_is_zero(ec, p) ^ 1);
}

#ifdef TORSION_TEST
static void
pge_print(mont_t *ec, const pge_t *p) {
  prime_field_t *fe = &ec->fe;

  if (pge_is_zero(ec, p)) {
    printf("(infinity)\n");
  } else {
    mp_limb_t xp[MAX_FIELD_LIMBS];
    mp_limb_t zp[MAX_FIELD_LIMBS];

    fe_get_limbs(fe, xp, p->x);
    fe_get_limbs(fe, zp, p->z);

    printf("(");
    mpn_print(xp, fe->limbs, 16);
    printf(", ");
    mpn_print(zp, fe->limbs, 16);
    printf(")\n");
  }
}
#endif

/*
 * Montgomery Curve
 */

static void
mont_init_isomorphism(mont_t *ec, const mont_def_t *def);

static void
mont_init(mont_t *ec, const mont_def_t *def) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;

  ec->h = def->h;
  ec->clamp = def->clamp;

  prime_field_init(fe, def->fe, -1);
  scalar_field_init(sc, def->sc, -1);

  fe_import_be(fe, ec->a, def->a);
  fe_import_be(fe, ec->b, def->b);

  if (def->z < 0) {
    fe_set_word(fe, ec->z, -def->z);
    fe_neg(fe, ec->z, ec->z);
  } else {
    fe_set_word(fe, ec->z, def->z);
  }

  mont_init_isomorphism(ec, def);

  fe_invert_var(fe, ec->bi, ec->b);
  fe_invert_var(fe, ec->i4, fe->four);

  /* a24 = (a + 2) / 4 */
  fe_add(fe, ec->a24, ec->a, fe->two);
  fe_mul(fe, ec->a24, ec->a24, ec->i4);

  /* a0 = a / b */
  fe_mul(fe, ec->a0, ec->a, ec->bi);

  /* b0 = 1 / b^2 */
  fe_sqr(fe, ec->b0, ec->bi);

  fe_import_be(fe, ec->g.x, def->x);
  fe_import_be(fe, ec->g.y, def->y);
  ec->g.inf = 0;
}

static void
mont_init_isomorphism(mont_t *ec, const mont_def_t *def) {
  /* Trick: recover isomorphism from scaling factor `c`.
   *
   * Normal:
   *
   *   c = sqrt((A + 2) / (B * a))
   *   a = (A + 2) / (B * c^2)
   *   d = a * (A - 2) / (A + 2)
   *
   * Inverted:
   *
   *   c = sqrt((A - 2) / (B * a))
   *   a = (A - 2) / (B * c^2)
   *   d = a * (A + 2) / (A - 2)
   */
  prime_field_t *fe = &ec->fe;

  ec->invert = def->invert;
  fe_import_be(fe, ec->c, def->c);
}

static void
mont_clamp(mont_t *ec, unsigned char *out, const unsigned char *in) {
  memcpy(out, in, ec->sc.size);
  ec->clamp(out);
}

static void
mont_mul_a24(mont_t *ec, fe_t r, const fe_t a) {
  prime_field_t *fe = &ec->fe;

  if (fe->scmul_121666)
    fe_mul121666(fe, r, a);
  else
    fe_mul(fe, r, a, ec->a24);
}

static void
mont_mul(mont_t *ec, pge_t *r, const pge_t *p, const sc_t k) {
  /* Multiply with the Montgomery Ladder.
   *
   * [MONT3] Algorithm 7, Page 16, Section 5.3.
   *         Algorithm 8, Page 16, Section 5.3.
   *
   * [RFC7748] Page 7, Section 5.
   *
   * Note that any clamping is meant to
   * be done _outside_ of this function.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  mp_limb_t swap = 0;
  mp_limb_t bit = 0;
  mp_size_t i;
  pge_t a, b;

  assert((size_t)sc->limbs * GMP_NUMB_BITS >= fe->bits);

  pge_zero(ec, &a);
  pge_set(ec, &b, p);

  /* Climb the ladder. */
  for (i = fe->bits - 1; i >= 0; i--) {
    bit = sc_get_bit(sc, k, i);

    /* Maybe swap. */
    pge_swap(ec, &a, &b, swap ^ bit);

    /* Single coordinate add+double. */
    pge_ladder(ec, &a, &b, p, &a, &b);

    swap = bit;
  }

  /* Finalize loop. */
  pge_swap(ec, &a, &b, swap);
  pge_set(ec, r, &a);

  /* Cleanse. */
  cleanse(&bit, sizeof(bit));
  cleanse(&swap, sizeof(swap));
}

static void
mont_mul_g(mont_t *ec, pge_t *r, const sc_t k) {
  pge_t g;
  mge_to_pge(ec, &g, &ec->g);
  mont_mul(ec, r, &g, k);
}

static void
mont_solve_y0(mont_t *ec, fe_t y2, const fe_t x) {
  /* y^2 = x^3 + A * x^2 + B * x */
  prime_field_t *fe = &ec->fe;
  fe_t x2, x3, ax2, bx;

  fe_sqr(fe, x2, x);
  fe_mul(fe, x3, x2, x);
  fe_mul(fe, ax2, ec->a0, x2);
  fe_mul(fe, bx, ec->b0, x);
  fe_add(fe, y2, x3, ax2);
  fe_add(fe, y2, y2, bx);
}

static void
mont_elligator2(mont_t *ec, mge_t *r, const fe_t u) {
  /* Elligator 2.
   *
   * Distribution: 1/2.
   *
   * [ELL2] Page 11, Section 5.2.
   * [H2EC] "Elligator 2 Method".
   *        "Mappings for Montgomery curves".
   * [SAFE] "Indistinguishability from uniform random strings".
   *
   * Assumptions:
   *
   *   - y^2 = x^3 + A * x^2 + B * x.
   *   - A != 0, B != 0.
   *   - A^2 - 4 * B is non-zero and non-square in F(p).
   *   - Let z be a non-square in F(p).
   *   - u != +-sqrt(-1 / z).
   *
   * Note that Elligator 2 is defined over the form:
   *
   *   y'^2 = x'^3 + A' * x'^2 + B' * x'
   *
   * Instead of:
   *
   *   B * y^2 = x^3 + A * x^2 + x
   *
   * Where:
   *
   *   A' = A / B
   *   B' = 1 / B^2
   *   x' = x / B
   *   y' = y / B
   *
   * And:
   *
   *   x = B * x'
   *   y = B * y'
   *
   * This is presumably the result of Elligator 2
   * being designed in long Weierstrass form. If
   * we want to support B != 1, we need to do the
   * conversion.
   *
   * Map:
   *
   *   g(x) = x^3 + A * x^2 + B * x
   *   x1 = -A / (1 + z * u^2)
   *   x1 = -A, if x1 = 0
   *   x2 = -x1 - A
   *   x = x1, if g(x1) is square
   *     = x2, otherwise
   *   y = sign(u) * abs(sqrt(g(x)))
   */
  prime_field_t *fe = &ec->fe;
  fe_t lhs, rhs, x1, x2, y1, y2;
  int alpha;

  fe_neg(fe, lhs, ec->a0);
  fe_sqr(fe, rhs, u);
  fe_mul(fe, rhs, rhs, ec->z);
  fe_add(fe, rhs, rhs, fe->one);

  fe_select(fe, rhs, rhs, fe->one, fe_is_zero(fe, rhs));

  fe_invert(fe, rhs, rhs);
  fe_mul(fe, x1, lhs, rhs);
  fe_neg(fe, x2, x1);
  fe_sub(fe, x2, x2, ec->a0);

  mont_solve_y0(ec, y1, x1);
  mont_solve_y0(ec, y2, x2);

  alpha = fe_is_square(fe, y1);

  fe_select(fe, x1, x1, x2, alpha ^ 1);
  fe_select(fe, y1, y1, y2, alpha ^ 1);
  fe_sqrt(fe, y1, y1);

  fe_set_odd(fe, y1, y1, fe_is_odd(fe, u));

  fe_mul(fe, x1, x1, ec->b);
  fe_mul(fe, y1, y1, ec->b);

  mge_set_xy(ec, r, x1, y1);
}

static int
mont_invert2(mont_t *ec, fe_t u, const mge_t *p, unsigned int hint) {
  /* Inverting the Map (Elligator 2).
   *
   * [ELL2] Page 12, Section 5.3.
   *
   * Assumptions:
   *
   *   - -z * x * (x + A) is square in F(p).
   *   - If r = 1 then x != 0.
   *   - If r = 2 then x != -A.
   *
   * Map:
   *
   *   u1 = -(x + A) / (x * z)
   *   u2 = -x / ((x + A) * z)
   *   r = random integer in [1,2]
   *   u = sign(y) * abs(sqrt(ur))
   *
   * Note that `0 / 0` can only occur if `A == 0`
   * (this violates the assumptions of Elligator 2).
   */
  prime_field_t *fe = &ec->fe;
  fe_t x0, y0, n, d;
  int ret;

  fe_mul(fe, x0, p->x, ec->bi);
  fe_mul(fe, y0, p->y, ec->bi);

  fe_add(fe, n, x0, ec->a0);
  fe_set(fe, d, x0);

  fe_swap(fe, n, d, hint & 1);

  fe_neg(fe, n, n);
  fe_mul(fe, d, d, ec->z);

  ret = fe_isqrt(fe, u, n, d);

  fe_set_odd(fe, u, u, fe_is_odd(fe, y0));

  return ret & (p->inf ^ 1);
}

static void
mont_point_from_uniform(mont_t *ec, mge_t *p, const unsigned char *bytes) {
  prime_field_t *fe = &ec->fe;
  fe_t u;

  fe_import_uniform(fe, u, bytes);

  mont_elligator2(ec, p, u);

  fe_cleanse(fe, u);
}

static int
mont_point_to_uniform(mont_t *ec,
                      unsigned char *bytes,
                      const mge_t *p,
                      unsigned int hint) {
  prime_field_t *fe = &ec->fe;
  fe_t u;
  int ret;

  ret = mont_invert2(ec, u, p, hint);

  fe_export(fe, bytes, u);
  fe_cleanse(fe, u);

  bytes[fe->size - 1] |= (hint >> 8) & ~fe->mask;

  return ret;
}

static void
mont_point_from_hash(mont_t *ec, mge_t *p, const unsigned char *bytes) {
  /* [H2EC] "Roadmap". */
  mge_t p1, p2;

  mont_point_from_uniform(ec, &p1, bytes);
  mont_point_from_uniform(ec, &p2, bytes + ec->fe.size);

  mge_add(ec, p, &p1, &p2);

  mge_cleanse(ec, &p1);
  mge_cleanse(ec, &p2);
}

static void
mont_point_to_hash(mont_t *ec,
                   unsigned char *bytes,
                   const mge_t *p,
                   const unsigned char *entropy) {
  /* [SQUARED] Algorithm 1, Page 8, Section 3.3. */
  prime_field_t *fe = &ec->fe;
  unsigned int hint;
  mge_t p1, p2;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  for (;;) {
    drbg_generate(&rng, bytes, fe->size);

    if (!bytes_lt(bytes, fe->raw, fe->size, fe->endian))
      continue;

    mont_point_from_uniform(ec, &p1, bytes);

    /* Avoid 2-torsion points. */
    if (fe_is_zero(fe, p1.y))
      continue;

    mge_sub(ec, &p2, p, &p1);

    drbg_generate(&rng, &hint, sizeof(hint));

    if (!mont_point_to_uniform(ec, bytes + fe->size, &p2, hint))
      continue;

    break;
  }

  cleanse(&rng, sizeof(rng));
  cleanse(&hint, sizeof(hint));

  mge_cleanse(ec, &p1);
  mge_cleanse(ec, &p2);
}

/*
 * Edwards
 */

static void
edwards_mul_a(edwards_t *ec, fe_t r, const fe_t x);

static void
_edwards_to_mont(prime_field_t *fe, mge_t *r,
                 const xge_t *p, const fe_t c,
                 int invert, int isogeny);

/*
 * Edwards Extended Point
 */

static void
xge_zero(edwards_t *ec, xge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_zero(fe, r->x);
  fe_set(fe, r->y, fe->one);
  fe_set(fe, r->z, fe->one);
  fe_zero(fe, r->t);
}

static void
xge_cleanse(edwards_t *ec, xge_t *r) {
  prime_field_t *fe = &ec->fe;

  fe_cleanse(fe, r->x);
  fe_cleanse(fe, r->y);
  fe_cleanse(fe, r->z);
  fe_cleanse(fe, r->t);
}

static int
xge_validate(edwards_t *ec, const xge_t *p) {
  /* [TWISTED] Definition 2.1, Page 3, Section 2. */
  /*           Page 11, Section 6. */
  /* (a * x^2 + y^2) * z^2 = z^4 + d * x^2 * y^2 */
  prime_field_t *fe = &ec->fe;
  fe_t lhs, rhs, x2, y2, ax2, z2, z4;

  fe_sqr(fe, x2, p->x);
  fe_sqr(fe, y2, p->y);
  fe_sqr(fe, z2, p->z);
  fe_sqr(fe, z4, z2);

  edwards_mul_a(ec, ax2, x2);
  fe_add(fe, lhs, ax2, y2);
  fe_mul(fe, lhs, lhs, z2);

  fe_mul(fe, rhs, x2, y2);
  fe_mul(fe, rhs, rhs, ec->d);
  fe_add(fe, rhs, rhs, z4);

  fe_mul(fe, x2, p->t, p->z);
  fe_mul(fe, y2, p->x, p->y);

  return fe_equal(fe, lhs, rhs)
       & fe_equal(fe, x2, y2)
       & (fe_is_zero(fe, p->z) ^ 1);
}

static void
xge_set_xy(edwards_t *ec, xge_t *r, const fe_t x, const fe_t y) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, x);
  fe_set(fe, r->y, y);
  fe_set(fe, r->z, fe->one);
  fe_mul(fe, r->t, x, y);
}

static int
xge_set_x(edwards_t *ec, xge_t *r, const fe_t x, int sign) {
  /* y^2 = (a * x^2 - 1) / (d * x^2 - 1) */
  prime_field_t *fe = &ec->fe;
  fe_t y, x2, lhs, rhs;
  int ret;

  fe_sqr(fe, x2, x);
  edwards_mul_a(ec, lhs, x2);
  fe_sub(fe, lhs, lhs, fe->one);
  fe_mul(fe, rhs, x2, ec->d);
  fe_sub(fe, rhs, rhs, fe->one);

  ret = fe_isqrt(fe, y, lhs, rhs);

  if (sign != -1)
    fe_set_odd(fe, y, y, sign);

  xge_set_xy(ec, r, x, y);

#ifdef TORSION_TEST
  assert(xge_validate(ec, r) == ret);
#endif

  if (sign != -1)
    ret &= (fe_is_zero(fe, y) & (sign != 0)) ^ 1;

  return ret;
}

static int
xge_set_y(edwards_t *ec, xge_t *r, const fe_t y, int sign) {
  /* [RFC8032] Section 5.1.3 & 5.2.3. */
  /* x^2 = (y^2 - 1) / (d * y^2 - a) */
  prime_field_t *fe = &ec->fe;
  fe_t x, y2, lhs, rhs;
  int ret;

  fe_sqr(fe, y2, y);
  fe_sub(fe, lhs, y2, fe->one);
  fe_mul(fe, rhs, ec->d, y2);
  fe_sub(fe, rhs, rhs, ec->a);

  ret = fe_isqrt(fe, x, lhs, rhs);

  if (sign != -1)
    fe_set_odd(fe, x, x, sign);

  xge_set_xy(ec, r, x, y);

#ifdef TORSION_TEST
  assert(xge_validate(ec, r) == ret);
#endif

  if (sign != -1)
    ret &= (fe_is_zero(fe, x) & (sign != 0)) ^ 1;

  return ret;
}

static int
xge_import(edwards_t *ec, xge_t *r, const unsigned char *raw) {
  prime_field_t *fe = &ec->fe;
  int sign;

  /* Quirk: we need an extra byte (p448). */
  if ((fe->bits & 7) == 0) {
    if ((raw[fe->size] & 0x7f) != 0x00)
      return 0;

    if (!fe_import(fe, r->y, raw))
      return 0;

    sign = raw[fe->size] >> 7;
  } else {
    unsigned char tmp[MAX_FIELD_SIZE];

    memcpy(tmp, raw, fe->size);

    tmp[fe->size - 1] &= 0x7f;

    if (!fe_import(fe, r->y, tmp))
      return 0;

    sign = raw[fe->size - 1] >> 7;
  }

  return xge_set_y(ec, r, r->y, sign);
}

static void
xge_export(edwards_t *ec,
          unsigned char *raw,
          const xge_t *p) {
  /* [RFC8032] Section 5.1.2. */
  prime_field_t *fe = &ec->fe;
  fe_t x, y, z;

  assert(fe_invert(fe, z, p->z));

  fe_mul(fe, x, p->x, z);
  fe_mul(fe, y, p->y, z);

  fe_export(fe, raw, y);

  /* Quirk: we need an extra byte (p448). */
  if ((fe->bits & 7) == 0)
    raw[fe->size] = fe_is_odd(fe, x) << 7;
  else
    raw[fe->size - 1] |= fe_is_odd(fe, x) << 7;
}

static void
xge_swap(edwards_t *ec, xge_t *a, xge_t *b, unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_swap(fe, a->x, b->x, flag);
  fe_swap(fe, a->y, b->y, flag);
  fe_swap(fe, a->z, b->z, flag);
  fe_swap(fe, a->t, b->t, flag);
}

static void
xge_select(edwards_t *ec,
           xge_t *r,
           const xge_t *a,
           const xge_t *b,
           unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_select(fe, r->x, a->x, b->x, flag);
  fe_select(fe, r->y, a->y, b->y, flag);
  fe_select(fe, r->z, a->z, b->z, flag);
  fe_select(fe, r->t, a->t, b->t, flag);
}

static void
xge_set(edwards_t *ec, xge_t *r, const xge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_set(fe, r->x, a->x);
  fe_set(fe, r->y, a->y);
  fe_set(fe, r->z, a->z);
  fe_set(fe, r->t, a->t);
}

static int
xge_is_zero(edwards_t *ec, const xge_t *a) {
  prime_field_t *fe = &ec->fe;

  return fe_is_zero(fe, a->x)
       & fe_equal(fe, a->y, a->z);
}

static int
xge_equal(edwards_t *ec, const xge_t *a, const xge_t *b) {
  prime_field_t *fe = &ec->fe;
  fe_t e1, e2;
  int ret = 1;

  /* X1 * Z2 == X2 * Z1 */
  fe_mul(fe, e1, a->x, b->z);
  fe_mul(fe, e2, b->x, a->z);

  ret &= fe_equal(fe, e1, e2);

  /* Y1 * Z2 == Y2 * Z1 */
  fe_mul(fe, e1, a->y, b->z);
  fe_mul(fe, e2, b->y, a->z);

  ret &= fe_equal(fe, e1, e2);

  return ret;
}

static void
xge_neg(edwards_t *ec, xge_t *r, const xge_t *a) {
  prime_field_t *fe = &ec->fe;

  fe_neg(fe, r->x, a->x);
  fe_set(fe, r->y, a->y);
  fe_set(fe, r->z, a->z);
  fe_neg(fe, r->t, a->t);
}

static void
xge_neg_cond(edwards_t *ec, xge_t *r, const xge_t *a, unsigned int flag) {
  prime_field_t *fe = &ec->fe;

  fe_neg_cond(fe, r->x, a->x, flag);
  fe_set(fe, r->y, a->y);
  fe_set(fe, r->z, a->z);
  fe_neg_cond(fe, r->t, a->t, flag);
}

static void
xge_dbl(edwards_t *ec, xge_t *r, const xge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-twisted-extended.html#doubling-dbl-2008-hwcd
   * 4M + 4S + 6A + 1*a + 1*2
   */
  prime_field_t *fe = &ec->fe;
  fe_t a, b, c, d, e, g, f, h;

  /* A = X1^2 */
  fe_sqr(fe, a, p->x);

  /* B = Y1^2 */
  fe_sqr(fe, b, p->y);

  /* C = 2 * Z1^2 */
  fe_sqr(fe, c, p->z);
  fe_add(fe, c, c, c);

  /* D = a * A */
  edwards_mul_a(ec, d, a);

  /* E = (X1 + Y1)^2 - A - B */
  fe_add(fe, e, p->x, p->y);
  fe_sqr(fe, e, e);
  fe_sub(fe, e, e, a);
  fe_sub(fe, e, e, b);

  /* G = D + B */
  fe_add(fe, g, d, b);

  /* F = G - C */
  fe_sub(fe, f, g, c);

  /* H = D - B */
  fe_sub(fe, h, d, b);

  /* X3 = E * F */
  fe_mul(fe, r->x, e, f);

  /* Y3 = G * H */
  fe_mul(fe, r->y, g, h);

  /* T3 = E * H */
  fe_mul(fe, r->t, e, h);

  /* Z3 = F * G */
  fe_mul(fe, r->z, f, g);
}

static void
xge_add_a(edwards_t *ec, xge_t *r, const xge_t *a, const xge_t *b) {
  /* https://hyperelliptic.org/EFD/g1p/auto-twisted-extended.html#addition-add-2008-hwcd
   * 9M + 7A + 1*a + 1*d
   */
  prime_field_t *fe = &ec->fe;
  fe_t A, B, c, d, e, f, g, h;

  /* A = X1 * X2 */
  fe_mul(fe, A, a->x, b->x);

  /* B = Y1 * Y2 */
  fe_mul(fe, B, a->y, b->y);

  /* C = T1 * d * T2 */
  fe_mul(fe, c, a->t, b->t);
  fe_mul(fe, c, c, ec->d);

  /* D = Z1 * Z2 */
  fe_mul(fe, d, a->z, b->z);

  /* E = (X1 + Y1) * (X2 + Y2) - A - B */
  fe_add(fe, f, a->x, a->y);
  fe_add(fe, g, b->x, b->y);
  fe_mul(fe, e, f, g);
  fe_sub(fe, e, e, A);
  fe_sub(fe, e, e, B);

  /* F = D - C */
  fe_sub(fe, f, d, c);

  /* G = D + C */
  fe_add(fe, g, d, c);

  /* H = B - a * A */
  edwards_mul_a(ec, h, A);
  fe_sub(fe, h, B, h);

  /* X3 = E * F */
  fe_mul(fe, r->x, e, f);

  /* Y3 = G * H */
  fe_mul(fe, r->y, g, h);

  /* T3 = E * H */
  fe_mul(fe, r->t, e, h);

  /* Z3 = F * G */
  fe_mul(fe, r->z, f, g);
}

static void
xge_add_m1(edwards_t *ec, xge_t *r, const xge_t *a, const xge_t *b) {
  /* Assumes a = -1.
   *
   * https://hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html#addition-add-2008-hwcd-3
   * 8M + 8A + 1*k + 1*2
   */
  prime_field_t *fe = &ec->fe;
  fe_t A, B, c, d, e, f, g, h;

  /* A = (Y1 - X1) * (Y2 - X2) */
  fe_sub(fe, c, a->y, a->x);
  fe_sub(fe, d, b->y, b->x);
  fe_mul(fe, A, c, d);

  /* B = (Y1 + X1) * (Y2 + X2) */
  fe_add(fe, c, a->y, a->x);
  fe_add(fe, d, b->y, b->x);
  fe_mul(fe, B, c, d);

  /* C = T1 * k * T2 */
  fe_mul(fe, c, a->t, b->t);
  fe_mul(fe, c, c, ec->k);

  /* D = Z1 * 2 * Z2 */
  fe_mul(fe, d, a->z, b->z);
  fe_add(fe, d, d, d);

  /* E = B - A */
  fe_sub(fe, e, B, A);

  /* F = D - C */
  fe_sub(fe, f, d, c);

  /* G = D + C */
  fe_add(fe, g, d, c);

  /* H = B + A */
  fe_add(fe, h, B, A);

  /* X3 = E * F */
  fe_mul(fe, r->x, e, f);

  /* Y3 = G * H */
  fe_mul(fe, r->y, g, h);

  /* T3 = E * H */
  fe_mul(fe, r->t, e, h);

  /* Z3 = F * G */
  fe_mul(fe, r->z, f, g);
}

static void
xge_add(edwards_t *ec, xge_t *r, const xge_t *a, const xge_t *b) {
  if (ec->mone_a)
    xge_add_m1(ec, r, a, b);
  else
    xge_add_a(ec, r, a, b);
}

static void
xge_sub(edwards_t *ec, xge_t *r, const xge_t *a, const xge_t *b) {
  xge_t c;
  xge_neg(ec, &c, b);
  xge_add(ec, r, a, &c);
}

static void
xge_normalize(edwards_t *ec, xge_t *r, const xge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#scaling-z
   * 1I + 2M (+ 1M if extended)
   */
  prime_field_t *fe = &ec->fe;
  fe_t a;

  /* A = 1 / Z1 */
  assert(fe_invert(fe, a, p->z));

  /* X3 = X1 * A */
  fe_mul(fe, r->x, p->x, a);

  /* Y3 = Y1 * A */
  fe_mul(fe, r->y, p->y, a);

  /* Z3 = 1 */
  fe_set(fe, r->z, fe->one);

  /* T3 = T1 * A */
  fe_mul(fe, r->t, p->t, a);
}

static void
xge_normalize_var(edwards_t *ec, xge_t *r, const xge_t *p) {
  /* https://hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#scaling-z
   * 1I + 2M (+ 1M if extended)
   */
  prime_field_t *fe = &ec->fe;
  fe_t a;

  /* Z = 1 */
  if (fe_equal(fe, p->z, fe->one)) {
    fe_set(fe, r->x, p->x);
    fe_set(fe, r->y, p->y);
    fe_set(fe, r->z, fe->one);
    fe_set(fe, r->t, p->t);
    return;
  }

  /* A = 1 / Z1 */
  assert(fe_invert_var(fe, a, p->z));

  /* X3 = X1 * A */
  fe_mul(fe, r->x, p->x, a);

  /* Y3 = Y1 * A */
  fe_mul(fe, r->y, p->y, a);

  /* Z3 = 1 */
  fe_set(fe, r->z, fe->one);

  /* T3 = T1 * A */
  fe_mul(fe, r->t, p->t, a);
}

static void
xge_mulh(edwards_t *ec, xge_t *r, const xge_t *p) {
  int bits = bit_length(ec->h);
  int i;

  xge_set(ec, r, p);

  for (i = 0; i < bits - 1; i++)
    xge_dbl(ec, r, r);
}

static int
xge_is_small(edwards_t *ec, const xge_t *p) {
  xge_t r;

  xge_mulh(ec, &r, p);

  return xge_is_zero(ec, &r)
      & (xge_is_zero(ec, p) ^ 1);
}

static void
xge_wnd_points(edwards_t *ec, xge_t *out, const xge_t *p) {
  scalar_field_t *sc = &ec->sc;
  size_t i, j;
  xge_t g;

  xge_set(ec, &g, p);

  for (i = 0; i < WND_STEPS(sc->bits); i++) {
    xge_zero(ec, &out[i * WND_SIZE]);

    for (j = 1; j < WND_SIZE; j++)
      xge_add(ec, &out[i * WND_SIZE + j], &out[i * WND_SIZE + j - 1], &g);

    for (j = 0; j < WND_WIDTH; j++)
      xge_dbl(ec, &g, &g);
  }
}

static void
xge_naf_points(edwards_t *ec, xge_t *points, const xge_t *p, size_t width) {
  size_t size = 1 << (width - 1);
  xge_t dbl;
  size_t i;

  xge_dbl(ec, &dbl, p);
  xge_set(ec, &points[0], p);

  for (i = 1; i < size; i++)
    xge_add(ec, &points[i], &points[i - 1], &dbl);
}

static void
xge_jsf_points(edwards_t *ec, xge_t *points, const xge_t *p1, const xge_t *p2) {
  /* Create comb for JSF. */
  xge_set(ec, &points[0], p1); /* 1 */
  xge_add(ec, &points[1], p1, p2); /* 3 */
  xge_sub(ec, &points[2], p1, p2); /* 5 */
  xge_set(ec, &points[3], p2); /* 7 */
}

static void
xge_to_mge(edwards_t *ec, mge_t *r, const xge_t *p) {
  _edwards_to_mont(&ec->fe, r, p, ec->c, ec->invert, 1);
}

#ifdef TORSION_TEST
static void
xge_print(edwards_t *ec, const xge_t *p) {
  prime_field_t *fe = &ec->fe;

  if (xge_is_zero(ec, p)) {
    printf("(infinity)\n");
  } else {
    mp_limb_t xp[MAX_FIELD_LIMBS];
    mp_limb_t yp[MAX_FIELD_LIMBS];
    mp_limb_t zp[MAX_FIELD_LIMBS];

    fe_get_limbs(fe, xp, p->x);
    fe_get_limbs(fe, yp, p->y);
    fe_get_limbs(fe, zp, p->z);

    printf("(");
    mpn_print(xp, fe->limbs, 16);
    printf(", ");
    mpn_print(yp, fe->limbs, 16);
    printf(", ");
    mpn_print(zp, fe->limbs, 16);
    printf(")\n");
  }
}
#endif

/*
 * Edwards Curve
 */

static void
edwards_init_isomorphism(edwards_t *ec, const edwards_def_t *def);

static void
edwards_init(edwards_t *ec, const edwards_def_t *def) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;

  ec->hash = def->hash;
  ec->context = def->context;
  ec->prefix = def->prefix;
  ec->h = def->h;
  ec->clamp = def->clamp;

  prime_field_init(fe, def->fe, -1);
  scalar_field_init(sc, def->sc, -1);

  fe_import_be(fe, ec->a, def->a);
  fe_import_be(fe, ec->d, def->d);
  fe_add(fe, ec->k, ec->d, ec->d);

  if (def->z < 0) {
    fe_set_word(fe, ec->z, -def->z);
    fe_neg(fe, ec->z, ec->z);
  } else {
    fe_set_word(fe, ec->z, def->z);
  }

  edwards_init_isomorphism(ec, def);

  ec->mone_a = fe_equal(fe, ec->a, fe->mone);
  ec->one_a = fe_equal(fe, ec->a, fe->one);

  fe_import_be(fe, ec->g.x, def->x);
  fe_import_be(fe, ec->g.y, def->y);
  fe_set(fe, ec->g.z, fe->one);
  fe_mul(fe, ec->g.t, ec->g.x, ec->g.y);

  sc_zero(sc, ec->blind);
  xge_zero(ec, &ec->unblind);

  xge_wnd_points(ec, ec->windows, &ec->g);
  xge_naf_points(ec, ec->points, &ec->g, NAF_WIDTH_PRE);
}

static void
edwards_init_isomorphism(edwards_t *ec, const edwards_def_t *def) {
  /* Trick: recover isomorphism from scaling factor `c`.
   *
   * Normal:
   *
   *   c = sqrt((A + 2) / (B * a))
   *   A = 2 * (a + d) / (a - d)
   *   B = (A + 2) / (a * c^2)
   *
   * Inverted:
   *
   *   c = sqrt((A - 2) / (B * a))
   *   A = 2 * (d + a) / (d - a)
   *   B = (A - 2) / (a * c^2)
   */
  prime_field_t *fe = &ec->fe;
  fe_t u, v;

  ec->invert = def->invert;
  fe_import_be(fe, ec->c, def->c);

  if (!ec->invert) {
    fe_add(fe, u, ec->a, ec->d);
    fe_sub(fe, v, ec->a, ec->d);
  } else {
    fe_add(fe, u, ec->d, ec->a);
    fe_sub(fe, v, ec->d, ec->a);
  }

  fe_add(fe, u, u, u);
  fe_invert_var(fe, v, v);
  fe_mul(fe, ec->A, u, v);

  if (!ec->invert)
    fe_add(fe, u, ec->A, fe->two);
  else
    fe_sub(fe, u, ec->A, fe->two);

  fe_sqr(fe, v, ec->c);
  fe_mul(fe, v, v, ec->a);
  fe_invert_var(fe, v, v);
  fe_mul(fe, ec->B, u, v);
  fe_invert_var(fe, ec->Bi, ec->B);

  /* A0 = A / B */
  fe_mul(fe, ec->A0, ec->A, ec->Bi);

  /* B0 = 1 / B^2 */
  fe_sqr(fe, ec->B0, ec->Bi);
}

static void
edwards_clamp(edwards_t *ec, unsigned char *out, const unsigned char *in) {
  memcpy(out, in, ec->sc.size);
  ec->clamp(out);
}

static void
edwards_mul_a(edwards_t *ec, fe_t r, const fe_t x) {
  prime_field_t *fe = &ec->fe;

  if (ec->mone_a)
    fe_neg(fe, r, x); /* a = -1 */
  else if (ec->one_a)
    fe_set(fe, r, x); /* a = 1 */
  else
    fe_mul(fe, r, x, ec->a);
}

static void
edwards_mul_g(edwards_t *ec, xge_t *r, const sc_t k) {
  scalar_field_t *sc = &ec->sc;
  size_t i, j, b;
  sc_t k0;
  xge_t p;

  /* Blind if available. */
  sc_add(sc, k0, k, ec->blind);

  /* Multiply in constant time. */
  xge_zero(ec, &p);
  xge_set(ec, r, &ec->unblind);

  for (i = 0; i < WND_STEPS(sc->bits); i++) {
    b = sc_get_bits(sc, k0, i * WND_WIDTH, WND_WIDTH);

    for (j = 0; j < WND_SIZE; j++)
      xge_select(ec, &p, &p, &ec->windows[i * WND_SIZE + j], j == b);

    xge_add(ec, r, r, &p);
  }

  /* Cleanse. */
  sc_cleanse(sc, k0);

  cleanse(&b, sizeof(b));
}

static void
edwards_mul(edwards_t *ec, xge_t *r, const xge_t *p, const sc_t k) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  mp_size_t start = WND_STEPS(fe->bits) - 1;
  xge_t table[WND_SIZE];
  mp_size_t i, j, b;
  xge_t t;

  xge_zero(ec, &table[0]);
  xge_set(ec, &table[1], p);

  for (i = 2; i < WND_SIZE; i += 2) {
    xge_dbl(ec, &table[i], &table[i >> 1]);
    xge_add(ec, &table[i + 1], &table[i], p);
  }

  xge_zero(ec, r);
  xge_zero(ec, &t);

  for (i = start; i >= 0; i--) {
    if (i != start) {
      for (j = 0; j < WND_WIDTH; j++)
        xge_dbl(ec, r, r);
    }

    b = sc_get_bits(sc, k, i * WND_WIDTH, WND_WIDTH);

    for (j = 0; j < WND_SIZE; j++)
      xge_select(ec, &t, &t, &table[j], j == b);

    xge_add(ec, r, r, &t);
  }

  cleanse(&b, sizeof(b));
}

static void
edwards_mul_double_var(edwards_t *ec,
                        xge_t *r,
                        const sc_t k1,
                        const xge_t *p2,
                        const sc_t k2) {
  /* Multiple point multiplication, also known
   * as "Shamir's trick" (with interleaved NAFs).
   *
   * [GECC] Algorithm 3.48, Page 109, Section 3.3.3.
   *        Algorithm 3.51, Page 112, Section 3.3.
   */
  scalar_field_t *sc = &ec->sc;
  xge_t *wnd1 = ec->points;
  xge_t wnd2[NAF_SIZE];
  int32_t naf1[MAX_SCALAR_BITS + 1];
  int32_t naf2[MAX_SCALAR_BITS + 1];
  size_t max1 = sc_bitlen_var(sc, k1) + 1;
  size_t max2 = sc_bitlen_var(sc, k2) + 1;
  size_t max = max1 > max2 ? max1 : max2;
  xge_t acc;
  size_t i;

  sc_naf_var(sc, naf1, k1, 1, NAF_WIDTH_PRE, max);
  sc_naf_var(sc, naf2, k2, 1, NAF_WIDTH, max);

  xge_naf_points(ec, wnd2, p2, NAF_WIDTH);

  /* Multiply and add. */
  xge_zero(ec, &acc);

  for (i = max; i-- > 0;) {
    int32_t z1 = naf1[i];
    int32_t z2 = naf2[i];

    if (i != max - 1)
      xge_dbl(ec, &acc, &acc);

    if (z1 > 0)
      xge_add(ec, &acc, &acc, &wnd1[(z1 - 1) >> 1]);
    else if (z1 < 0)
      xge_sub(ec, &acc, &acc, &wnd1[(-z1 - 1) >> 1]);

    if (z2 > 0)
      xge_add(ec, &acc, &acc, &wnd2[(z2 - 1) >> 1]);
    else if (z2 < 0)
      xge_sub(ec, &acc, &acc, &wnd2[(-z2 - 1) >> 1]);
  }

  xge_set(ec, r, &acc);
}

static void
edwards_mul_multi_var(edwards_t *ec,
                       xge_t *r,
                       const sc_t k0,
                       const xge_t *points,
                       const sc_t *coeffs,
                       size_t len,
                       edwards_scratch_t *scratch) {
  /* Multiple point multiplication, also known
   * as "Shamir's trick" (with interleaved NAFs).
   *
   * [GECC] Algorithm 3.48, Page 109, Section 3.3.3.
   *        Algorithm 3.51, Page 112, Section 3.3.
   */
  scalar_field_t *sc = &ec->sc;
  xge_t *wnd0 = ec->points;
  size_t max = sc_bitlen_var(sc, k0) + 1;
  int32_t naf0[MAX_SCALAR_BITS + 1];
  xge_t *wnds[32];
  int32_t *nafs[32];
  size_t i, j;
  xge_t acc;

  assert((len & 1) == 0);
  assert(len <= 64);

  /* Setup scratch. */
  for (i = 0; i < 32; i++) {
    wnds[i] = &scratch->wnd[i * 4];
    nafs[i] = &scratch->naf[i * (MAX_SCALAR_BITS + 1)];
  }

  /* Compute max scalar size. */
  for (i = 0; i < len; i++) {
    size_t bits = sc_bitlen_var(sc, coeffs[i]) + 1;

    if (bits > max)
      max = bits;
  }

  /* Compute NAFs. */
  sc_naf_var(sc, naf0, k0, 1, NAF_WIDTH_PRE, max);

  for (i = 0; i < len; i += 2) {
    const xge_t *p1 = &points[i + 0];
    const xge_t *p2 = &points[i + 1];
    const sc_t *k1 = &coeffs[i + 0];
    const sc_t *k2 = &coeffs[i + 1];

    xge_jsf_points(ec, wnds[i >> 1], p1, p2);
    sc_jsf_var(sc, nafs[i >> 1], *k1, 1, *k2, 1, max);
  }

  len >>= 1;

  /* Multiply and add. */
  xge_zero(ec, &acc);

  for (i = max; i-- > 0;) {
    int32_t z0 = naf0[i];

    if (i != max - 1)
      xge_dbl(ec, &acc, &acc);

    if (z0 > 0)
      xge_add(ec, &acc, &acc, &wnd0[(z0 - 1) >> 1]);
    else if (z0 < 0)
      xge_sub(ec, &acc, &acc, &wnd0[(-z0 - 1) >> 1]);

    for (j = 0; j < len; j++) {
      int32_t z = nafs[j][i];

      if (z > 0)
        xge_add(ec, &acc, &acc, &wnds[j][(z - 1) >> 1]);
      else if (z < 0)
        xge_sub(ec, &acc, &acc, &wnds[j][(-z - 1) >> 1]);
    }
  }

  xge_set(ec, r, &acc);
}

static void
edwards_randomize(edwards_t *ec, const unsigned char *entropy) {
  scalar_field_t *sc = &ec->sc;
  sc_t blind;
  xge_t unblind;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  sc_random(sc, blind, &rng);

  edwards_mul_g(ec, &unblind, blind);

  sc_neg(sc, ec->blind, blind);
  xge_set(ec, &ec->unblind, &unblind);

  sc_cleanse(sc, blind);
  xge_cleanse(ec, &unblind);
  cleanse(&rng, sizeof(rng));
}

static void
edwards_solve_y0(edwards_t *ec, fe_t y2, const fe_t x) {
  /* y^2 = x^3 + A * x^2 + B * x */
  prime_field_t *fe = &ec->fe;
  fe_t x2, x3, ax2, bx;

  fe_sqr(fe, x2, x);
  fe_mul(fe, x3, x2, x);
  fe_mul(fe, ax2, ec->A0, x2);
  fe_mul(fe, bx, ec->B0, x);
  fe_add(fe, y2, x3, ax2);
  fe_add(fe, y2, y2, bx);
}

static void
edwards_elligator2(edwards_t *ec, xge_t *r, const fe_t u) {
  /* Elligator 2.
   *
   * Distribution: 1/2.
   *
   * [ELL2] Page 11, Section 5.2.
   * [H2EC] "Elligator 2 Method".
   *        "Mappings for Montgomery curves".
   * [SAFE] "Indistinguishability from uniform random strings".
   *
   * Assumptions:
   *
   *   - y^2 = x^3 + A * x^2 + B * x.
   *   - A != 0, B != 0.
   *   - A^2 - 4 * B is non-zero and non-square in F(p).
   *   - Let z be a non-square in F(p).
   *   - u != +-sqrt(-1 / z).
   *
   * Note that Elligator 2 is defined over the form:
   *
   *   y'^2 = x'^3 + A' * x'^2 + B' * x'
   *
   * Instead of:
   *
   *   B * y^2 = x^3 + A * x^2 + x
   *
   * Where:
   *
   *   A' = A / B
   *   B' = 1 / B^2
   *   x' = x / B
   *   y' = y / B
   *
   * And:
   *
   *   x = B * x'
   *   y = B * y'
   *
   * This is presumably the result of Elligator 2
   * being designed in long Weierstrass form. If
   * we want to support B != 1, we need to do the
   * conversion.
   *
   * Map:
   *
   *   g(x) = x^3 + A * x^2 + B * x
   *   x1 = -A / (1 + z * u^2)
   *   x1 = -A, if x1 = 0
   *   x2 = -x1 - A
   *   x = x1, if g(x1) is square
   *     = x2, otherwise
   *   y = sign(u) * abs(sqrt(g(x)))
   */
  prime_field_t *fe = &ec->fe;
  fe_t lhs, rhs, x1, x2, y1, y2;
  mge_t m;
  int alpha;

  fe_neg(fe, lhs, ec->A0);
  fe_sqr(fe, rhs, u);
  fe_mul(fe, rhs, rhs, ec->z);
  fe_add(fe, rhs, rhs, fe->one);

  fe_select(fe, rhs, rhs, fe->one, fe_is_zero(fe, rhs));

  fe_invert(fe, rhs, rhs);
  fe_mul(fe, x1, lhs, rhs);
  fe_neg(fe, x2, x1);
  fe_sub(fe, x2, x2, ec->A0);

  edwards_solve_y0(ec, y1, x1);
  edwards_solve_y0(ec, y2, x2);

  alpha = fe_is_square(fe, y1);

  fe_select(fe, x1, x1, x2, alpha ^ 1);
  fe_select(fe, y1, y1, y2, alpha ^ 1);
  fe_sqrt(fe, y1, y1);

  fe_set_odd(fe, y1, y1, fe_is_odd(fe, u));

  fe_mul(fe, x1, x1, ec->B);
  fe_mul(fe, y1, y1, ec->B);

  fe_set(fe, m.x, x1);
  fe_set(fe, m.y, y1);
  m.inf = 0;

  _mont_to_edwards(fe, r, &m, ec->c, ec->invert, 0);
}

static int
edwards_invert2(edwards_t *ec, fe_t u, const xge_t *p, unsigned int hint) {
  /* Inverting the Map (Elligator 2).
   *
   * [ELL2] Page 12, Section 5.3.
   *
   * Assumptions:
   *
   *   - -z * x * (x + A) is square in F(p).
   *   - If r = 1 then x != 0.
   *   - If r = 2 then x != -A.
   *
   * Map:
   *
   *   u1 = -(x + A) / (x * z)
   *   u2 = -x / ((x + A) * z)
   *   r = random integer in [1,2]
   *   u = sign(y) * abs(sqrt(ur))
   *
   * Note that `0 / 0` can only occur if `A == 0`
   * (this violates the assumptions of Elligator 2).
   */
  prime_field_t *fe = &ec->fe;
  fe_t x0, y0, n, d;
  mge_t m;
  int ret;

  _edwards_to_mont(fe, &m, p, ec->c, ec->invert, 0);

  fe_mul(fe, x0, m.x, ec->Bi);
  fe_mul(fe, y0, m.y, ec->Bi);

  fe_add(fe, n, x0, ec->A0);
  fe_set(fe, d, x0);

  fe_swap(fe, n, d, hint & 1);

  fe_neg(fe, n, n);
  fe_mul(fe, d, d, ec->z);

  ret = fe_isqrt(fe, u, n, d);

  fe_set_odd(fe, u, u, fe_is_odd(fe, y0));

  return ret & (m.inf ^ 1);
}

static void
edwards_point_from_uniform(edwards_t *ec, xge_t *p,
                           const unsigned char *bytes) {
  prime_field_t *fe = &ec->fe;
  fe_t u;

  fe_import_uniform(fe, u, bytes);

  edwards_elligator2(ec, p, u);

  fe_cleanse(fe, u);
}

static int
edwards_point_to_uniform(edwards_t *ec,
                      unsigned char *bytes,
                      const xge_t *p,
                      unsigned int hint) {
  prime_field_t *fe = &ec->fe;
  fe_t u;
  int ret;

  ret = edwards_invert2(ec, u, p, hint);

  fe_export(fe, bytes, u);
  fe_cleanse(fe, u);

  bytes[fe->size - 1] |= (hint >> 8) & ~fe->mask;

  return ret;
}

static void
edwards_point_from_hash(edwards_t *ec, xge_t *p, const unsigned char *bytes) {
  /* [H2EC] "Roadmap". */
  xge_t p1, p2;

  edwards_point_from_uniform(ec, &p1, bytes);
  edwards_point_from_uniform(ec, &p2, bytes + ec->fe.size);

  xge_add(ec, p, &p1, &p2);

  xge_cleanse(ec, &p1);
  xge_cleanse(ec, &p2);
}

static void
edwards_point_to_hash(edwards_t *ec,
                      unsigned char *bytes,
                      const xge_t *p,
                      const unsigned char *entropy) {
  /* [SQUARED] Algorithm 1, Page 8, Section 3.3. */
  prime_field_t *fe = &ec->fe;
  unsigned int hint;
  xge_t p1, p2;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  for (;;) {
    drbg_generate(&rng, bytes, fe->size);

    if (!bytes_lt(bytes, fe->raw, fe->size, fe->endian))
      continue;

    edwards_point_from_uniform(ec, &p1, bytes);

    /* Avoid 2-torsion points. */
    if (fe_is_zero(fe, p1.x))
      continue;

    xge_sub(ec, &p2, p, &p1);

    drbg_generate(&rng, &hint, sizeof(hint));

    if (!edwards_point_to_uniform(ec, bytes + fe->size, &p2, hint))
      continue;

    break;
  }

  cleanse(&rng, sizeof(rng));
  cleanse(&hint, sizeof(hint));

  xge_cleanse(ec, &p1);
  xge_cleanse(ec, &p2);
}

/*
 * Isomorphism (low-level functions)
 */

static void
_mont_to_edwards(prime_field_t *fe, xge_t *r,
                 const mge_t *p, const fe_t c,
                 int invert, int isogeny) {
  /* [RFC7748] Section 4.1 & 4.2. */
  /* [MONT3] Page 6, Section 2.5. */
  /* [TWISTED] Theorem 3.2, Page 4, Section 3. */
  int inf = p->inf;
  int tor = fe_is_zero(fe, p->x) & (inf ^ 1);
  fe_t xx, xz, yy, yz;

  if (isogeny && fe->bits == 448) {
    /* 4-isogeny maps for M(2-4d,1)->E(1,d):
     *
     *   x = 4 * v * (u^2 - 1) / (u^4 - 2 * u^2 + 4 * v^2 + 1)
     *   y = -(u^5 - 2 * u^3 - 4 * u * v^2 + u) /
     *        (u^5 - 2 * u^2 * v^2 - 2 * u^3 - 2 * v^2 + u)
     *
     * Undefined for u = 0 and v = 0.
     *
     * Exceptional Cases:
     *   - O -> (0, 1)
     *   - (0, 0) -> (0, 1)
     *
     * Unexceptional Cases:
     *   - (-1, +-sqrt(A - 2)) -> (0, 1)
     *   - (1, +-sqrt(A + 2)) -> (0, -1)
     *
     * The point (1, v) is invalid on Curve448.
     */
    fe_t u2, u3, u4, u5, v2, a, b, c, d, e, f, g, h;

    fe_sqr(fe, u2, p->x);
    fe_mul(fe, u3, u2, p->x);
    fe_mul(fe, u4, u3, p->x);
    fe_mul(fe, u5, u4, p->x);
    fe_sqr(fe, v2, p->y);

    fe_mul_word(fe, a, p->y, 4);
    fe_sub(fe, b, u2, fe->one);
    fe_mul_word(fe, c, u2, 2);
    fe_mul_word(fe, d, v2, 4);
    fe_mul_word(fe, e, u3, 2);
    fe_mul(fe, f, p->x, v2);
    fe_mul_word(fe, f, f, 4);
    fe_mul(fe, g, u2, v2);
    fe_mul_word(fe, g, g, 2);
    fe_mul_word(fe, h, v2, 2);

    fe_mul(fe, xx, a, b);

    fe_sub(fe, xz, u4, c);
    fe_add(fe, xz, xz, d);
    fe_add(fe, xz, xz, fe->one);

    fe_sub(fe, yy, u5, e);
    fe_sub(fe, yy, yy, f);
    fe_add(fe, yy, yy, p->x);
    fe_neg(fe, yy, yy);

    fe_sub(fe, yz, u5, g);
    fe_sub(fe, yz, yz, e);
    fe_sub(fe, yz, yz, h);
    fe_add(fe, yz, yz, p->x);

    /* Handle 2-torsion as infinity. */
    inf |= tor;
  } else if (invert) {
    /* Isomorphic maps for M(-A,-B)->E(a,d):
     *
     *   x = +-sqrt((A - 2) / (B * a)) * u / v
     *   y = (u + 1) / (u - 1)
     *
     * Undefined for u = 1 or v = 0.
     *
     * Exceptional Cases:
     *   - O -> (0, 1)
     *   - (0, 0) -> (0, -1)
     *   - (1, +-sqrt((A + 2) / B)) -> (+-sqrt(1 / d), oo)
     *
     * Unexceptional Cases:
     *   - (-1, +-sqrt((A - 2) / B)) -> (+-sqrt(1 / a), 0)
     *
     * The point (1, v) is invalid on Curve448.
     */
    fe_mul(fe, xx, c, p->x);
    fe_select(fe, xz, p->y, fe->one, tor);
    fe_add(fe, yy, p->x, fe->one);
    fe_sub(fe, yz, p->x, fe->one);
  } else {
    /* Isomorphic maps for M(A,B)->E(a,d):
     *
     *   x = +-sqrt((A + 2) / (B * a)) * u / v
     *   y = (u - 1) / (u + 1)
     *
     * Undefined for u = -1 or v = 0.
     *
     * Exceptional Cases:
     *   - O -> (0, 1)
     *   - (0, 0) -> (0, -1)
     *   - (-1, +-sqrt((A - 2) / B)) -> (+-sqrt(1 / d), oo)
     *
     * Unexceptional Cases:
     *   - (1, +-sqrt((A + 2) / B)) -> (+-sqrt(1 / a), 0)
     *
     * The point (-1, v) is invalid on Curve25519.
     */
    fe_mul(fe, xx, c, p->x);
    fe_select(fe, xz, p->y, fe->one, tor);
    fe_sub(fe, yy, p->x, fe->one);
    fe_add(fe, yz, p->x, fe->one);
  }

  /* Completed point. */
  fe_mul(fe, r->x, xx, yz);
  fe_mul(fe, r->y, yy, xz);
  fe_mul(fe, r->z, xz, yz);
  fe_mul(fe, r->t, xx, yy);

  /* Handle infinity. */
  fe_select(fe, r->x, r->x, fe->zero, inf);
  fe_select(fe, r->y, r->y, fe->one, inf);
  fe_select(fe, r->z, r->z, fe->one, inf);
  fe_select(fe, r->t, r->t, fe->zero, inf);
}

static void
_edwards_to_mont(prime_field_t *fe, mge_t *r,
                 const xge_t *p, const fe_t c,
                 int invert, int isogeny) {
  /* [RFC7748] Section 4.1 & 4.2. */
  /* [MONT3] Page 6, Section 2.5. */
  /* [TWISTED] Theorem 3.2, Page 4, Section 3. */
  int zero = fe_is_zero(fe, p->x);
  int inf = zero & fe_equal(fe, p->y, p->z);
  int tor = zero & (inf ^ 1);
  fe_t uu, uz, vv, vz, two;

  if (isogeny && fe->bits == 448) {
    /* 4-isogeny maps for E(1,d)->M(2-4d,1):
     *
     *   u = y^2 / x^2
     *   v = (2 - x^2 - y^2) * y / x^3
     *
     * Undefined for x = 0.
     *
     * Exceptional Cases:
     *   - (0, 1) -> O
     *   - (0, -1) -> (0, 0)
     *
     * Unexceptional Cases:
     *   - (+-1, 0) -> (0, 0)
     */
    fe_sqr(fe, two, p->z);
    fe_add(fe, two, two, two);
    fe_sqr(fe, uu, p->y);
    fe_sqr(fe, uz, p->x);
    fe_sub(fe, vv, two, uz);
    fe_sub(fe, vv, vv, uu);
    fe_mul(fe, vv, vv, p->y);
    fe_mul(fe, vz, uz, p->x);
  } else if (invert) {
    /* Isomorphic maps for E(d,a)->M(A,B):
     *
     *   u = (y + 1) / (y - 1)
     *   v = +-sqrt((A - 2) / (B * a)) * u / x
     *
     * Undefined for x = 0 or y = 1.
     *
     * Exceptional Cases:
     *   - (0, 1) -> O
     *   - (0, -1) -> (0, 0)
     *
     * Unexceptional Cases:
     *   - (+-sqrt(1 / a), 0) -> (-1, +-sqrt((A - 2) / B))
     */
    fe_add(fe, uu, p->y, p->z);
    fe_sub(fe, uz, p->y, p->z);
    fe_mul(fe, vv, c, p->z);
    fe_mul(fe, vv, vv, uu);
    fe_mul(fe, vz, p->x, uz);
  } else {
    /* Isomorphic maps for E(a,d)->M(A,B):
     *
     *   u = (1 + y) / (1 - y)
     *   v = +-sqrt((A + 2) / (B * a)) * u / x
     *
     * Undefined for x = 0 or y = 1.
     *
     * Exceptional Cases:
     *   - (0, 1) -> O
     *   - (0, -1) -> (0, 0)
     *
     * Unexceptional Cases:
     *   - (+-sqrt(1 / a), 0) -> (1, +-sqrt((A + 2) / B))
     */
    fe_add(fe, uu, p->z, p->y);
    fe_sub(fe, uz, p->z, p->y);
    fe_mul(fe, vv, c, p->z);
    fe_mul(fe, vv, vv, uu);
    fe_mul(fe, vz, p->x, uz);
  }

  /* Completed point. */
  fe_mul(fe, r->x, uu, vz);
  fe_mul(fe, r->y, vv, uz);
  fe_mul(fe, uz, uz, vz);
  fe_invert(fe, uz, uz);
  fe_mul(fe, r->x, r->x, uz);
  fe_mul(fe, r->y, r->y, uz);

  /* Handle 2-torsion. */
  fe_select(fe, r->x, r->x, fe->zero, tor);
  fe_select(fe, r->y, r->y, fe->zero, tor);

  /* Handle infinity. */
  r->inf = inf;
}

/*
 * Fields
 */

/*
 * P192
 */

static const prime_def_t field_p192 = {
  .bits = 192,
  .words = P192_FIELD_WORDS,
  /* 2^192 - 2^64 - 1 (= 3 mod 4) */
  .p = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  },
  .add = fiat_p192_add,
  .sub = fiat_p192_sub,
  .opp = fiat_p192_opp,
  .mul = fiat_p192_carry_mul,
  .square = fiat_p192_carry_square,
  .from_montgomery = NULL,
  .nonzero = NULL,
  .selectznz = fiat_p192_selectznz,
  .to_bytes = fiat_p192_to_bytes,
  .from_bytes = fiat_p192_from_bytes,
  .carry = fiat_p192_carry,
  .invert = NULL,
  .sqrt = p192_fe_sqrt,
  .isqrt = NULL,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q192 = {
  .bits = 192,
  .n = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x99, 0xde, 0xf8, 0x36,
    0x14, 0x6b, 0xc9, 0xb1, 0xb4, 0xd2, 0x28, 0x31
  }
};

/*
 * P224
 */

static const prime_def_t field_p224 = {
  .bits = 224,
  .words = P224_FIELD_WORDS,
  /* 2^224 - 2^96 + 1 (no congruence) */
  .p = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01
  },
  .add = fiat_p224_add,
  .sub = fiat_p224_sub,
  .opp = fiat_p224_opp,
  .mul = fiat_p224_mul,
  .square = fiat_p224_square,
  .from_montgomery = fiat_p224_from_montgomery,
  .nonzero = fiat_p224_nonzero,
  .selectznz = fiat_p224_selectznz,
  .to_bytes = fiat_p224_to_bytes,
  .from_bytes = fiat_p224_from_bytes,
  .carry = NULL,
  .invert = p224_fe_invert,
  .sqrt = p224_fe_sqrt_var,
  .isqrt = NULL,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q224 = {
  .bits = 224,
  .n = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0xa2,
    0xe0, 0xb8, 0xf0, 0x3e, 0x13, 0xdd, 0x29, 0x45,
    0x5c, 0x5c, 0x2a, 0x3d
  }
};

/*
 * P256
 */

static const prime_def_t field_p256 = {
  .bits = 256,
  .words = P256_FIELD_WORDS,
  /* 2^256 - 2^224 + 2^192 + 2^96 - 1 (= 3 mod 4) */
  .p = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  },
  .add = fiat_p256_add,
  .sub = fiat_p256_sub,
  .opp = fiat_p256_opp,
  .mul = fiat_p256_mul,
  .square = fiat_p256_square,
  .from_montgomery = fiat_p256_from_montgomery,
  .nonzero = fiat_p256_nonzero,
  .selectznz = fiat_p256_selectznz,
  .to_bytes = fiat_p256_to_bytes,
  .from_bytes = fiat_p256_from_bytes,
  .carry = NULL,
  .invert = p256_fe_invert,
  .sqrt = p256_fe_sqrt,
  .isqrt = NULL,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q256 = {
  .bits = 256,
  .n = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e, 0x84,
    0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51
  }
};

/*
 * P384
 */

static const prime_def_t field_p384 = {
  .bits = 384,
  .words = P384_FIELD_WORDS,
  /* 2^384 - 2^128 - 2^96 + 2^32 - 1 (= 3 mod 4) */
  .p = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
  },
  .add = fiat_p384_add,
  .sub = fiat_p384_sub,
  .opp = fiat_p384_opp,
  .mul = fiat_p384_mul,
  .square = fiat_p384_square,
  .from_montgomery = fiat_p384_from_montgomery,
  .nonzero = fiat_p384_nonzero,
  .selectznz = fiat_p384_selectznz,
  .to_bytes = fiat_p384_to_bytes,
  .from_bytes = fiat_p384_from_bytes,
  .carry = NULL,
  .invert = p384_fe_invert,
  .sqrt = p384_fe_sqrt,
  .isqrt = NULL,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q384 = {
  .bits = 384,
  .n = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xc7, 0x63, 0x4d, 0x81, 0xf4, 0x37, 0x2d, 0xdf,
    0x58, 0x1a, 0x0d, 0xb2, 0x48, 0xb0, 0xa7, 0x7a,
    0xec, 0xec, 0x19, 0x6a, 0xcc, 0xc5, 0x29, 0x73
  }
};

/*
 * P521
 */

static const prime_def_t field_p521 = {
  .bits = 521,
  .words = P521_FIELD_WORDS,
  /* 2^521 - 1 (= 3 mod 4) */
  .p = {
    0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff
  },
  .add = fiat_p521_add,
  .sub = fiat_p521_sub,
  .opp = fiat_p521_opp,
  .mul = fiat_p521_carry_mul,
  .square = fiat_p521_carry_square,
  .from_montgomery = NULL,
  .nonzero = NULL,
  .selectznz = fiat_p521_selectznz,
  .to_bytes = fiat_p521_to_bytes,
  .from_bytes = fiat_p521_from_bytes,
  .carry = fiat_p521_carry,
  .invert = p521_fe_invert,
  .sqrt = p521_fe_sqrt,
  .isqrt = NULL,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q521 = {
  .bits = 521,
  .n = {
    0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfa, 0x51, 0x86, 0x87, 0x83, 0xbf, 0x2f,
    0x96, 0x6b, 0x7f, 0xcc, 0x01, 0x48, 0xf7, 0x09,
    0xa5, 0xd0, 0x3b, 0xb5, 0xc9, 0xb8, 0x89, 0x9c,
    0x47, 0xae, 0xbb, 0x6f, 0xb7, 0x1e, 0x91, 0x38,
    0x64, 0x09
  }
};

/*
 * P256K1
 */

static const prime_def_t field_p256k1 = {
  .bits = 256,
  .words = SECP256K1_FIELD_WORDS,
  /* 2^256 - 2^32 - 977 (= 3 mod 4) */
  .p = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfc, 0x2f
  },
  .add = fiat_secp256k1_add,
  .sub = fiat_secp256k1_sub,
  .opp = fiat_secp256k1_opp,
  .mul = fiat_secp256k1_mul,
  .square = fiat_secp256k1_square,
  .from_montgomery = fiat_secp256k1_from_montgomery,
  .nonzero = fiat_secp256k1_nonzero,
  .selectznz = fiat_secp256k1_selectznz,
  .to_bytes = fiat_secp256k1_to_bytes,
  .from_bytes = fiat_secp256k1_from_bytes,
  .carry = NULL,
  .invert = secp256k1_fe_invert,
  .sqrt = secp256k1_fe_sqrt,
  .isqrt = secp256k1_fe_isqrt,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q256k1 = {
  .bits = 256,
  .n = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b,
    0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41
  }
};

/*
 * P25519
 */

static const prime_def_t field_p25519 = {
  .bits = 255,
  .words = P25519_FIELD_WORDS,
  /* 2^255 - 19 (= 5 mod 8) */
  .p = {
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xed
  },
  .add = fiat_p25519_add,
  .sub = fiat_p25519_sub,
  .opp = fiat_p25519_opp,
  .mul = fiat_p25519_carry_mul,
  .square = fiat_p25519_carry_square,
  .from_montgomery = NULL,
  .nonzero = NULL,
  .selectznz = fiat_p25519_selectznz,
  .to_bytes = fiat_p25519_to_bytes,
  .from_bytes = fiat_p25519_from_bytes,
  .carry = fiat_p25519_carry,
  .invert = p25519_fe_invert,
  .sqrt = p25519_fe_sqrt,
  .isqrt = p25519_fe_isqrt,
  .scmul_121666 = fiat_p25519_carry_scmul_121666
};

static const scalar_def_t field_q25519 = {
  .bits = 253,
  .n = {
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x14, 0xde, 0xf9, 0xde, 0xa2, 0xf7, 0x9c, 0xd6,
    0x58, 0x12, 0x63, 0x1a, 0x5c, 0xf5, 0xd3, 0xed
  }
};

/*
 * P448
 */

static const prime_def_t field_p448 = {
  .bits = 448,
  .words = P448_FIELD_WORDS,
  /* 2^448 - 2^224 - 1 (= 3 mod 4) */
  .p = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  },
  .add = fiat_p448_add,
  .sub = fiat_p448_sub,
  .opp = fiat_p448_opp,
  .mul = fiat_p448_carry_mul,
  .square = fiat_p448_carry_square,
  .from_montgomery = NULL,
  .nonzero = NULL,
  .selectznz = fiat_p448_selectznz,
  .to_bytes = fiat_p448_to_bytes,
  .from_bytes = fiat_p448_from_bytes,
  .carry = fiat_p448_carry,
  .invert = p448_fe_invert,
  .sqrt = p448_fe_sqrt,
  .isqrt = p448_fe_isqrt,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q448 = {
  .bits = 446,
  .n = {
    0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x7c, 0xca, 0x23, 0xe9,
    0xc4, 0x4e, 0xdb, 0x49, 0xae, 0xd6, 0x36, 0x90,
    0x21, 0x6c, 0xc2, 0x72, 0x8d, 0xc5, 0x8f, 0x55,
    0x23, 0x78, 0xc2, 0x92, 0xab, 0x58, 0x44, 0xf3
  }
};

/*
 * P251
 */

static const prime_def_t field_p251 = {
  .bits = 251,
  .words = P251_FIELD_WORDS,
  /* 2^251 - 9 (= 3 mod 4) */
  .p = {
    0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7
  },
  .add = fiat_p251_add,
  .sub = fiat_p251_sub,
  .opp = fiat_p251_opp,
  .mul = fiat_p251_carry_mul,
  .square = fiat_p251_carry_square,
  .from_montgomery = NULL,
  .nonzero = NULL,
  .selectznz = fiat_p251_selectznz,
  .to_bytes = fiat_p251_to_bytes,
  .from_bytes = fiat_p251_from_bytes,
  .carry = fiat_p251_carry,
  .invert = NULL,
  .sqrt = NULL,
  .isqrt = NULL,
  .scmul_121666 = NULL
};

static const scalar_def_t field_q251 = {
  .bits = 249,
  .n = {
    0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xf7, 0x79, 0x65, 0xc4, 0xdf, 0xd3, 0x07, 0x34,
    0x89, 0x44, 0xd4, 0x5f, 0xd1, 0x66, 0xc9, 0x71
  }
};

/*
 * Short Weierstrass Curves
 */

static const wei_def_t curve_p192 = {
  .id = "P192",
  .hash = HASH_SHA256,
  .fe = &field_p192,
  .sc = &field_q192,
  /* -3 mod p */
  .a = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc
  },
  .b = {
    0x64, 0x21, 0x05, 0x19, 0xe5, 0x9c, 0x80, 0xe7,
    0x0f, 0xa7, 0xe9, 0xab, 0x72, 0x24, 0x30, 0x49,
    0xfe, 0xb8, 0xde, 0xec, 0xc1, 0x46, 0xb9, 0xb1
  },
  .h = 1,
  /* Icart */
  .z = -5,
  .x = {
    0x18, 0x8d, 0xa8, 0x0e, 0xb0, 0x30, 0x90, 0xf6,
    0x7c, 0xbf, 0x20, 0xeb, 0x43, 0xa1, 0x88, 0x00,
    0xf4, 0xff, 0x0a, 0xfd, 0x82, 0xff, 0x10, 0x12
  },
  .y = {
    0x07, 0x19, 0x2b, 0x95, 0xff, 0xc8, 0xda, 0x78,
    0x63, 0x10, 0x11, 0xed, 0x6b, 0x24, 0xcd, 0xd5,
    0x73, 0xf9, 0x77, 0xa1, 0x1e, 0x79, 0x48, 0x11
  },
  .endo = 0
};

static const wei_def_t curve_p224 = {
  .id = "P224",
  .hash = HASH_SHA256,
  .fe = &field_p224,
  .sc = &field_q224,
  /* -3 mod p */
  .a = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe
  },
  .b = {
    0xb4, 0x05, 0x0a, 0x85, 0x0c, 0x04, 0xb3, 0xab,
    0xf5, 0x41, 0x32, 0x56, 0x50, 0x44, 0xb0, 0xb7,
    0xd7, 0xbf, 0xd8, 0xba, 0x27, 0x0b, 0x39, 0x43,
    0x23, 0x55, 0xff, 0xb4
  },
  .h = 1,
  /* SSWU */
  .z = 31,
  .x = {
    0xb7, 0x0e, 0x0c, 0xbd, 0x6b, 0xb4, 0xbf, 0x7f,
    0x32, 0x13, 0x90, 0xb9, 0x4a, 0x03, 0xc1, 0xd3,
    0x56, 0xc2, 0x11, 0x22, 0x34, 0x32, 0x80, 0xd6,
    0x11, 0x5c, 0x1d, 0x21
  },
  .y = {
    0xbd, 0x37, 0x63, 0x88, 0xb5, 0xf7, 0x23, 0xfb,
    0x4c, 0x22, 0xdf, 0xe6, 0xcd, 0x43, 0x75, 0xa0,
    0x5a, 0x07, 0x47, 0x64, 0x44, 0xd5, 0x81, 0x99,
    0x85, 0x00, 0x7e, 0x34
  },
  .endo = 0
};

static const wei_def_t curve_p256 = {
  .id = "P256",
  .hash = HASH_SHA256,
  .fe = &field_p256,
  .sc = &field_q256,
  /* -3 mod p */
  .a = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc
  },
  .b = {
    0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a, 0x93, 0xe7,
    0xb3, 0xeb, 0xbd, 0x55, 0x76, 0x98, 0x86, 0xbc,
    0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53, 0xb0, 0xf6,
    0x3b, 0xce, 0x3c, 0x3e, 0x27, 0xd2, 0x60, 0x4b
  },
  .h = 1,
  /* SSWU */
  .z = -10,
  .x = {
    0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47,
    0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2,
    0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0,
    0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96
  },
  .y = {
    0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b,
    0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16,
    0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce,
    0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
  },
  .endo = 0
};

static const wei_def_t curve_p384 = {
  .id = "P384",
  .hash = HASH_SHA384,
  .fe = &field_p384,
  .sc = &field_q384,
  /* -3 mod p */
  .a = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfc
  },
  .b = {
    0xb3, 0x31, 0x2f, 0xa7, 0xe2, 0x3e, 0xe7, 0xe4,
    0x98, 0x8e, 0x05, 0x6b, 0xe3, 0xf8, 0x2d, 0x19,
    0x18, 0x1d, 0x9c, 0x6e, 0xfe, 0x81, 0x41, 0x12,
    0x03, 0x14, 0x08, 0x8f, 0x50, 0x13, 0x87, 0x5a,
    0xc6, 0x56, 0x39, 0x8d, 0x8a, 0x2e, 0xd1, 0x9d,
    0x2a, 0x85, 0xc8, 0xed, 0xd3, 0xec, 0x2a, 0xef
  },
  .h = 1,
  /* Icart */
  .z = -12,
  .x = {
    0xaa, 0x87, 0xca, 0x22, 0xbe, 0x8b, 0x05, 0x37,
    0x8e, 0xb1, 0xc7, 0x1e, 0xf3, 0x20, 0xad, 0x74,
    0x6e, 0x1d, 0x3b, 0x62, 0x8b, 0xa7, 0x9b, 0x98,
    0x59, 0xf7, 0x41, 0xe0, 0x82, 0x54, 0x2a, 0x38,
    0x55, 0x02, 0xf2, 0x5d, 0xbf, 0x55, 0x29, 0x6c,
    0x3a, 0x54, 0x5e, 0x38, 0x72, 0x76, 0x0a, 0xb7
  },
  .y = {
    0x36, 0x17, 0xde, 0x4a, 0x96, 0x26, 0x2c, 0x6f,
    0x5d, 0x9e, 0x98, 0xbf, 0x92, 0x92, 0xdc, 0x29,
    0xf8, 0xf4, 0x1d, 0xbd, 0x28, 0x9a, 0x14, 0x7c,
    0xe9, 0xda, 0x31, 0x13, 0xb5, 0xf0, 0xb8, 0xc0,
    0x0a, 0x60, 0xb1, 0xce, 0x1d, 0x7e, 0x81, 0x9d,
    0x7a, 0x43, 0x1d, 0x7c, 0x90, 0xea, 0x0e, 0x5f
  },
  .endo = 0
};

static const wei_def_t curve_p521 = {
  .id = "P521",
  .hash = HASH_SHA512,
  .fe = &field_p521,
  .sc = &field_q521,
  /* -3 mod p */
  .a = {
    0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfc
  },
  .b = {
    0x00, 0x51, 0x95, 0x3e, 0xb9, 0x61, 0x8e, 0x1c,
    0x9a, 0x1f, 0x92, 0x9a, 0x21, 0xa0, 0xb6, 0x85,
    0x40, 0xee, 0xa2, 0xda, 0x72, 0x5b, 0x99, 0xb3,
    0x15, 0xf3, 0xb8, 0xb4, 0x89, 0x91, 0x8e, 0xf1,
    0x09, 0xe1, 0x56, 0x19, 0x39, 0x51, 0xec, 0x7e,
    0x93, 0x7b, 0x16, 0x52, 0xc0, 0xbd, 0x3b, 0xb1,
    0xbf, 0x07, 0x35, 0x73, 0xdf, 0x88, 0x3d, 0x2c,
    0x34, 0xf1, 0xef, 0x45, 0x1f, 0xd4, 0x6b, 0x50,
    0x3f, 0x00
  },
  .h = 1,
  /* SSWU */
  .z = -4,
  .x = {
    0x00, 0xc6, 0x85, 0x8e, 0x06, 0xb7, 0x04, 0x04,
    0xe9, 0xcd, 0x9e, 0x3e, 0xcb, 0x66, 0x23, 0x95,
    0xb4, 0x42, 0x9c, 0x64, 0x81, 0x39, 0x05, 0x3f,
    0xb5, 0x21, 0xf8, 0x28, 0xaf, 0x60, 0x6b, 0x4d,
    0x3d, 0xba, 0xa1, 0x4b, 0x5e, 0x77, 0xef, 0xe7,
    0x59, 0x28, 0xfe, 0x1d, 0xc1, 0x27, 0xa2, 0xff,
    0xa8, 0xde, 0x33, 0x48, 0xb3, 0xc1, 0x85, 0x6a,
    0x42, 0x9b, 0xf9, 0x7e, 0x7e, 0x31, 0xc2, 0xe5,
    0xbd, 0x66
  },
  .y = {
    0x01, 0x18, 0x39, 0x29, 0x6a, 0x78, 0x9a, 0x3b,
    0xc0, 0x04, 0x5c, 0x8a, 0x5f, 0xb4, 0x2c, 0x7d,
    0x1b, 0xd9, 0x98, 0xf5, 0x44, 0x49, 0x57, 0x9b,
    0x44, 0x68, 0x17, 0xaf, 0xbd, 0x17, 0x27, 0x3e,
    0x66, 0x2c, 0x97, 0xee, 0x72, 0x99, 0x5e, 0xf4,
    0x26, 0x40, 0xc5, 0x50, 0xb9, 0x01, 0x3f, 0xad,
    0x07, 0x61, 0x35, 0x3c, 0x70, 0x86, 0xa2, 0x72,
    0xc2, 0x40, 0x88, 0xbe, 0x94, 0x76, 0x9f, 0xd1,
    0x66, 0x50
  },
  .endo = 0
};

static const wei_def_t curve_secp256k1 = {
  .id = "SECP256K1",
  .hash = HASH_SHA256,
  .fe = &field_p256k1,
  .sc = &field_q256k1,
  .a = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  },
  .b = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07
  },
  .h = 1,
  /* SVDW */
  .z = 1,
  /* sqrt(-3) */
  .c = {
    0x0a, 0x2d, 0x2b, 0xa9, 0x35, 0x07, 0xf1, 0xdf,
    0x23, 0x37, 0x70, 0xc2, 0xa7, 0x97, 0x96, 0x2c,
    0xc6, 0x1f, 0x6d, 0x15, 0xda, 0x14, 0xec, 0xd4,
    0x7d, 0x8d, 0x27, 0xae, 0x1c, 0xd5, 0xf8, 0x52
  },
  .x = {
    0x79, 0xbe, 0x66, 0x7e, 0xf9, 0xdc, 0xbb, 0xac,
    0x55, 0xa0, 0x62, 0x95, 0xce, 0x87, 0x0b, 0x07,
    0x02, 0x9b, 0xfc, 0xdb, 0x2d, 0xce, 0x28, 0xd9,
    0x59, 0xf2, 0x81, 0x5b, 0x16, 0xf8, 0x17, 0x98
  },
  .y = {
    0x48, 0x3a, 0xda, 0x77, 0x26, 0xa3, 0xc4, 0x65,
    0x5d, 0xa4, 0xfb, 0xfc, 0x0e, 0x11, 0x08, 0xa8,
    0xfd, 0x17, 0xb4, 0x48, 0xa6, 0x85, 0x54, 0x19,
    0x9c, 0x47, 0xd0, 0x8f, 0xfb, 0x10, 0xd4, 0xb8
  },
  .endo = 1,
  .beta = {
    0x7a, 0xe9, 0x6a, 0x2b, 0x65, 0x7c, 0x07, 0x10,
    0x6e, 0x64, 0x47, 0x9e, 0xac, 0x34, 0x34, 0xe9,
    0x9c, 0xf0, 0x49, 0x75, 0x12, 0xf5, 0x89, 0x95,
    0xc1, 0x39, 0x6c, 0x28, 0x71, 0x95, 0x01, 0xee
  },
  .lambda = {
    0xac, 0x9c, 0x52, 0xb3, 0x3f, 0xa3, 0xcf, 0x1f,
    0x5a, 0xd9, 0xe3, 0xfd, 0x77, 0xed, 0x9b, 0xa4,
    0xa8, 0x80, 0xb9, 0xfc, 0x8e, 0xc7, 0x39, 0xc2,
    0xe0, 0xcf, 0xc8, 0x10, 0xb5, 0x12, 0x83, 0xcf
  },
  .b1 = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe4, 0x43, 0x7e, 0xd6, 0x01, 0x0e, 0x88, 0x28,
    0x6f, 0x54, 0x7f, 0xa9, 0x0a, 0xbf, 0xe4, 0xc3
  },
  .b2 = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0x8a, 0x28, 0x0a, 0xc5, 0x07, 0x74, 0x34, 0x6d,
    0xd7, 0x65, 0xcd, 0xa8, 0x3d, 0xb1, 0x56, 0x2c
  },
  .g1 = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x86,
    0xd2, 0x21, 0xa7, 0xd4, 0x6b, 0xcd, 0xe8, 0x6c,
    0x90, 0xe4, 0x92, 0x84, 0xeb, 0x15, 0x3d, 0xab
  },
  .g2 = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe4, 0x43,
    0x7e, 0xd6, 0x01, 0x0e, 0x88, 0x28, 0x6f, 0x54,
    0x7f, 0xa9, 0x0a, 0xbf, 0xe4, 0xc4, 0x22, 0x12
  }
};

/*
 * Mont Curves
 */

static const mont_def_t curve_x25519 = {
  .id = "X25519",
  .fe = &field_p25519,
  .sc = &field_q25519,
  /* 486662 */
  .a = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6d, 0x06
  },
  .b = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
  },
  .h = 8,
  /* Elligator 2 */
  .z = 2,
  .invert = 0,
  /* sqrt(-486664) */
  .c = {
    /* See: https://github.com/cfrg/draft-irtf-cfrg-hash-to-curve/issues/206 */
    0x0f, 0x26, 0xed, 0xf4, 0x60, 0xa0, 0x06, 0xbb,
    0xd2, 0x7b, 0x08, 0xdc, 0x03, 0xfc, 0x4f, 0x7e,
    0xc5, 0xa1, 0xd3, 0xd1, 0x4b, 0x7d, 0x1a, 0x82,
    0xcc, 0x6e, 0x04, 0xaa, 0xff, 0x45, 0x7e, 0x06
  },
  .x = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09
  },
  .y = {
    /* See: https://www.rfc-editor.org/errata/eid4730 */
    0x5f, 0x51, 0xe6, 0x5e, 0x47, 0x5f, 0x79, 0x4b,
    0x1f, 0xe1, 0x22, 0xd3, 0x88, 0xb7, 0x2e, 0xb3,
    0x6d, 0xc2, 0xb2, 0x81, 0x92, 0x83, 0x9e, 0x4d,
    0xd6, 0x16, 0x3a, 0x5d, 0x81, 0x31, 0x2c, 0x14
  },
  .clamp = p25519_clamp
};

static const mont_def_t curve_x448 = {
  .id = "X448",
  .fe = &field_p448,
  .sc = &field_q448,
  /* 156326 */
  .a = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x62, 0xa6
  },
  .b = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
  },
  .h = 4,
  /* Elligator 2 */
  .z = -1,
  .invert = 1,
  /* IsoEd448 scaling factor. */
  .c = {
    0x45, 0xb2, 0xc5, 0xf7, 0xd6, 0x49, 0xee, 0xd0,
    0x77, 0xed, 0x1a, 0xe4, 0x5f, 0x44, 0xd5, 0x41,
    0x43, 0xe3, 0x4f, 0x71, 0x4b, 0x71, 0xaa, 0x96,
    0xc9, 0x45, 0xaf, 0x01, 0x2d, 0x18, 0x29, 0x75,
    0x07, 0x34, 0xcd, 0xe9, 0xfa, 0xdd, 0xbd, 0xa4,
    0xc0, 0x66, 0xf7, 0xed, 0x54, 0x41, 0x9c, 0xa5,
    0x2c, 0x85, 0xde, 0x1e, 0x8a, 0xae, 0x4e, 0x6c
  },
  .x = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05
  },
  .y = {
    0x7d, 0x23, 0x5d, 0x12, 0x95, 0xf5, 0xb1, 0xf6,
    0x6c, 0x98, 0xab, 0x6e, 0x58, 0x32, 0x6f, 0xce,
    0xcb, 0xae, 0x5d, 0x34, 0xf5, 0x55, 0x45, 0xd0,
    0x60, 0xf7, 0x5d, 0xc2, 0x8d, 0xf3, 0xf6, 0xed,
    0xb8, 0x02, 0x7e, 0x23, 0x46, 0x43, 0x0d, 0x21,
    0x13, 0x12, 0xc4, 0xb1, 0x50, 0x67, 0x7a, 0xf7,
    0x6f, 0xd7, 0x22, 0x3d, 0x45, 0x7b, 0x5b, 0x1a
  },
  .clamp = p448_clamp
};

/*
 * Edwards Curves
 */

static const edwards_def_t curve_ed25519 = {
  .id = "ED25519",
  .hash = HASH_SHA512,
  .context = 0,
  .prefix = "SigEd25519 no Ed25519 collisions",
  .fe = &field_p25519,
  .sc = &field_q25519,
  /* -1 mod p */
  .a = {
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xec
  },
  /* -121665 / 121666 mod p */
  .d = {
    0x52, 0x03, 0x6c, 0xee, 0x2b, 0x6f, 0xfe, 0x73,
    0x8c, 0xc7, 0x40, 0x79, 0x77, 0x79, 0xe8, 0x98,
    0x00, 0x70, 0x0a, 0x4d, 0x41, 0x41, 0xd8, 0xab,
    0x75, 0xeb, 0x4d, 0xca, 0x13, 0x59, 0x78, 0xa3
  },
  .h = 8,
  /* Elligator 2 */
  .z = 2,
  .invert = 0,
  /* sqrt(-486664) */
  .c = {
    /* See: https://github.com/cfrg/draft-irtf-cfrg-hash-to-curve/issues/206 */
    0x0f, 0x26, 0xed, 0xf4, 0x60, 0xa0, 0x06, 0xbb,
    0xd2, 0x7b, 0x08, 0xdc, 0x03, 0xfc, 0x4f, 0x7e,
    0xc5, 0xa1, 0xd3, 0xd1, 0x4b, 0x7d, 0x1a, 0x82,
    0xcc, 0x6e, 0x04, 0xaa, 0xff, 0x45, 0x7e, 0x06
  },
  .x = {
    0x21, 0x69, 0x36, 0xd3, 0xcd, 0x6e, 0x53, 0xfe,
    0xc0, 0xa4, 0xe2, 0x31, 0xfd, 0xd6, 0xdc, 0x5c,
    0x69, 0x2c, 0xc7, 0x60, 0x95, 0x25, 0xa7, 0xb2,
    0xc9, 0x56, 0x2d, 0x60, 0x8f, 0x25, 0xd5, 0x1a
  },
  /* 4/5 */
  .y = {
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x58
  },
  .clamp = p25519_clamp
};

static const edwards_def_t curve_ed448 = {
  .id = "ED448",
  .hash = HASH_SHAKE256,
  .context = 1,
  .prefix = "SigEd448",
  .fe = &field_p448,
  .sc = &field_q448,
  /* 1 mod p */
  .a = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
  },
  /* -39081 mod p */
  .d = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x67, 0x56
  },
  .h = 4,
  /* Elligator 2 */
  .z = -1,
  .invert = 1,
  /* Mont448 scaling factor. */
  .c = {
    0x41, 0x36, 0xd0, 0x2f, 0x92, 0x5d, 0x53, 0x0d,
    0x4b, 0x1d, 0x9e, 0x17, 0x83, 0x10, 0xf2, 0xcb,
    0xdd, 0x18, 0xa3, 0xe7, 0xc3, 0xa7, 0x67, 0xa8,
    0x48, 0xe6, 0xdb, 0x19, 0x8c, 0x3d, 0x06, 0x31,
    0x1e, 0x72, 0x5a, 0x0d, 0xb9, 0x91, 0xd0, 0xc6,
    0xc3, 0xd1, 0x12, 0x0f, 0x0e, 0xfa, 0x59, 0xf5,
    0x4b, 0xf3, 0x8e, 0x82, 0xb0, 0xe1, 0xe0, 0x28
  },
  .x = {
    0x4f, 0x19, 0x70, 0xc6, 0x6b, 0xed, 0x0d, 0xed,
    0x22, 0x1d, 0x15, 0xa6, 0x22, 0xbf, 0x36, 0xda,
    0x9e, 0x14, 0x65, 0x70, 0x47, 0x0f, 0x17, 0x67,
    0xea, 0x6d, 0xe3, 0x24, 0xa3, 0xd3, 0xa4, 0x64,
    0x12, 0xae, 0x1a, 0xf7, 0x2a, 0xb6, 0x65, 0x11,
    0x43, 0x3b, 0x80, 0xe1, 0x8b, 0x00, 0x93, 0x8e,
    0x26, 0x26, 0xa8, 0x2b, 0xc7, 0x0c, 0xc0, 0x5e
  },
  .y = {
    0x69, 0x3f, 0x46, 0x71, 0x6e, 0xb6, 0xbc, 0x24,
    0x88, 0x76, 0x20, 0x37, 0x56, 0xc9, 0xc7, 0x62,
    0x4b, 0xea, 0x73, 0x73, 0x6c, 0xa3, 0x98, 0x40,
    0x87, 0x78, 0x9c, 0x1e, 0x05, 0xa0, 0xc2, 0xd7,
    0x3a, 0xd3, 0xff, 0x1c, 0xe6, 0x7c, 0x39, 0xc4,
    0xfd, 0xbd, 0x13, 0x2c, 0x4e, 0xd7, 0xc8, 0xad,
    0x98, 0x08, 0x79, 0x5b, 0xf2, 0x30, 0xfa, 0x14
  },
  .clamp = p448_clamp
};

static const edwards_def_t curve_ed1174 = {
  .id = "ED1174",
  .hash = HASH_SHA512,
  .context = 1,
  .prefix = "SigEd1174",
  .fe = &field_p251,
  .sc = &field_q251,
  /* 1 mod p */
  .a = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
  },
  /* -1174 mod p */
  .d = {
    0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x61
  },
  .h = 4,
  /* Elligator 2 */
  .z = -1,
  .invert = 1,
  /* Should give us B=1. */
  .c = {
    0x00, 0x5a, 0x7a, 0x03, 0xfb, 0x02, 0xf7, 0x19,
    0x5e, 0x44, 0x1c, 0xd2, 0xe3, 0xf7, 0x08, 0xf9,
    0x6f, 0x8f, 0xfb, 0xe8, 0x35, 0x95, 0x48, 0xba,
    0x82, 0x76, 0xac, 0xe6, 0xbb, 0xe7, 0xdf, 0xd2
  },
  .x = {
    0x03, 0x7f, 0xbb, 0x0c, 0xea, 0x30, 0x8c, 0x47,
    0x93, 0x43, 0xae, 0xe7, 0xc0, 0x29, 0xa1, 0x90,
    0xc0, 0x21, 0xd9, 0x6a, 0x49, 0x2e, 0xcd, 0x65,
    0x16, 0x12, 0x3f, 0x27, 0xbc, 0xe2, 0x9e, 0xda
  },
  .y = {
    0x06, 0xb7, 0x2f, 0x82, 0xd4, 0x7f, 0xb7, 0xcc,
    0x66, 0x56, 0x84, 0x11, 0x69, 0x84, 0x0e, 0x0c,
    0x4f, 0xe2, 0xde, 0xe2, 0xaf, 0x3f, 0x97, 0x6b,
    0xa4, 0xcc, 0xb1, 0xbf, 0x9b, 0x46, 0x36, 0x0e
  },
  .clamp = p251_clamp
};

/*
 * Curve Registry
 */

static const wei_def_t *wei_curves[6] = {
  &curve_p192,
  &curve_p224,
  &curve_p256,
  &curve_p384,
  &curve_p521,
  &curve_secp256k1
};

static const mont_def_t *mont_curves[2] = {
  &curve_x25519,
  &curve_x448
};

static const edwards_def_t *edwards_curves[3] = {
  &curve_ed25519,
  &curve_ed448,
  &curve_ed1174
};

/*
 * ECDSA
 */

wei_t *
ecdsa_context_create(const char *id) {
  wei_t *ec = NULL;
  const wei_def_t *def = NULL;
  size_t i;

  if (id == NULL)
    return NULL;

  for (i = 0; i < ARRAY_SIZE(wei_curves); i++) {
    if (strcmp(wei_curves[i]->id, id) == 0) {
      def = wei_curves[i];
      break;
    }
  }

  if (def == NULL)
    return NULL;

  ec = malloc(sizeof(wei_t));

  if (ec == NULL)
    return NULL;

  wei_init(ec, def);

  return ec;
}

void
ecdsa_context_destroy(wei_t *ec) {
  if (ec != NULL)
    free(ec);
}

void
ecdsa_context_randomize(wei_t *ec, const unsigned char *entropy) {
  wei_randomize(ec, entropy);
}

wei_scratch_t *
ecdsa_scratch_create(wei_t *ec) {
  return malloc(sizeof(wei_scratch_t));
}

void
ecdsa_scratch_destroy(wei_t *ec, wei_scratch_t *scratch) {
  if (scratch != NULL)
    free(scratch);
}

size_t
ecdsa_scalar_size(wei_t *ec) {
  return ec->sc.size;
}

size_t
ecdsa_scalar_bits(wei_t *ec) {
  return ec->sc.bits;
}

size_t
ecdsa_field_size(wei_t *ec) {
  return ec->fe.size;
}

size_t
ecdsa_field_bits(wei_t *ec) {
  return ec->fe.bits;
}

size_t
ecdsa_privkey_size(wei_t *ec) {
  return ec->sc.size;
}

size_t
ecdsa_pubkey_size(wei_t *ec, int compact) {
  return compact ? 1 + ec->fe.size : 1 + ec->fe.size * 2;
}

size_t
ecdsa_sig_size(wei_t *ec) {
  return ec->sc.size * 2;
}

size_t
ecdsa_schnorr_size(wei_t *ec) {
  return ec->fe.size + ec->sc.size;
}

void
ecdsa_privkey_generate(wei_t *ec,
                       unsigned char *out,
                       const unsigned char *entropy) {
  scalar_field_t *sc = &ec->sc;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  for (;;) {
    drbg_generate(&rng, out, sc->size);

    if (bytes_zero(out, sc->size))
      continue;

    if (!bytes_lt(out, sc->raw, sc->size, sc->endian))
      continue;

    break;
  }

  cleanse(&rng, sizeof(rng));
}

int
ecdsa_privkey_verify(wei_t *ec, const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;
  int ret = 1;

  ret &= bytes_zero(priv, sc->size) ^ 1;
  ret &= bytes_lt(priv, sc->raw, sc->size, sc->endian);

  return ret;
}

int
ecdsa_privkey_export(wei_t *ec, unsigned char *out, const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;

  if (!ecdsa_privkey_verify(ec, priv))
    return 0;

  memcpy(out, priv, sc->size);

  return 1;
}

int
ecdsa_privkey_import(wei_t *ec,
                     unsigned char *out,
                     const unsigned char *bytes,
                     size_t len) {
  scalar_field_t *sc = &ec->sc;
  unsigned char key[MAX_SCALAR_SIZE];
  int r = 0;

  while (len > 0 && bytes[0] == 0x00) {
    len -= 1;
    bytes += 1;
  }

  if (len > sc->size)
    goto fail;

  memset(key, 0x00, sc->size - len);
  memcpy(key + sc->size - len, bytes, len);

  if (!ecdsa_privkey_verify(ec, key))
    goto fail;

  memcpy(out, key, sc->size);

  r = 1;
fail:
  cleanse(key, sizeof(key));
  return r;
}

int
ecdsa_privkey_tweak_add(wei_t *ec,
                        unsigned char *out,
                        const unsigned char *priv,
                        const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  sc_t a, t;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  if (!sc_import(sc, t, tweak))
    goto fail;

  sc_add(sc, a, a, t);

  if (sc_is_zero(sc, a))
    goto fail;

  sc_export(sc, out, a);

  ret = 1;
fail:
  sc_cleanse(sc, a);
  sc_cleanse(sc, t);
  return ret;
}

int
ecdsa_privkey_tweak_mul(wei_t *ec,
                        unsigned char *out,
                        const unsigned char *priv,
                        const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  sc_t a, t;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  if (!sc_import(sc, t, tweak))
    goto fail;

  sc_mul(sc, a, a, t);

  if (sc_is_zero(sc, a))
    goto fail;

  sc_export(sc, out, a);

  ret = 1;
fail:
  sc_cleanse(sc, a);
  sc_cleanse(sc, t);
  return ret;
}

int
ecdsa_privkey_reduce(wei_t *ec,
                     unsigned char *out,
                     const unsigned char *bytes,
                     size_t len) {
  scalar_field_t *sc = &ec->sc;
  unsigned char key[MAX_SCALAR_SIZE];
  sc_t a;
  int ret = 0;

  if (len > sc->size)
    len = sc->size;

  memset(key, 0x00, sc->size - len);
  memcpy(key + sc->size - len, bytes, len);

  sc_import_reduce(sc, a, key);

  if (sc_is_zero(sc, a))
    goto fail;

  sc_export(sc, out, a);

  ret = 1;
fail:
  cleanse(key, sizeof(key));
  sc_cleanse(sc, a);
  return ret;
}

int
ecdsa_privkey_negate(wei_t *ec, unsigned char *out, const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  sc_neg(sc, a, a);
  sc_export(sc, out, a);

  ret = 1;
fail:
  sc_cleanse(sc, a);
  return ret;
}

int
ecdsa_privkey_invert(wei_t *ec, unsigned char *out, const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  sc_invert(sc, a, a);
  sc_export(sc, out, a);

  ret = 1;
fail:
  sc_cleanse(sc, a);
  return ret;
}

int
ecdsa_pubkey_create(wei_t *ec,
                    unsigned char *pub,
                    size_t *pub_len,
                    const unsigned char *priv,
                    int compact) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  wge_t A;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  wei_mul_g(ec, &A, a);

  if (!wge_export(ec, pub, pub_len, &A, compact))
    goto fail;

  ret = 1;
fail:
  sc_cleanse(sc, a);
  wge_cleanse(ec, &A);
  return ret;
}

int
ecdsa_pubkey_convert(wei_t *ec,
                     unsigned char *out,
                     size_t *out_len,
                     const unsigned char *pub,
                     size_t pub_len,
                     int compact) {
  wge_t A;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  return wge_export(ec, out, out_len, &A, compact);
}

void
ecdsa_pubkey_from_uniform(wei_t *ec,
                          unsigned char *out,
                          size_t *out_len,
                          const unsigned char *bytes,
                          int compact) {
  wge_t A;

  wei_point_from_uniform(ec, &A, bytes);

  assert(wge_export(ec, out, out_len, &A, compact));
}

int
ecdsa_pubkey_to_uniform(wei_t *ec,
                        unsigned char *out,
                        const unsigned char *pub,
                        size_t pub_len,
                        unsigned int hint) {
  wge_t A;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  return wei_point_to_uniform(ec, out, &A, hint);
}

int
ecdsa_pubkey_from_hash(wei_t *ec,
                       unsigned char *out,
                       size_t *out_len,
                       const unsigned char *bytes,
                       int compact) {
  wge_t A;

  wei_point_from_hash(ec, &A, bytes);

  return wge_export(ec, out, out_len, &A, compact);
}

int
ecdsa_pubkey_to_hash(wei_t *ec,
                     unsigned char *out,
                     const unsigned char *pub,
                     size_t pub_len,
                     const unsigned char *entropy) {
  wge_t A;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  wei_point_to_hash(ec, out, &A, entropy);

  return 1;
}

int
ecdsa_pubkey_verify(wei_t *ec, const unsigned char *pub, size_t pub_len) {
  wge_t A;

  return wge_import(ec, &A, pub, pub_len);
}

int
ecdsa_pubkey_export(wei_t *ec,
                    unsigned char *x,
                    unsigned char *y,
                    const unsigned char *pub,
                    size_t pub_len) {
  prime_field_t *fe = &ec->fe;
  wge_t A;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  assert(!A.inf);

  fe_export(fe, x, A.x);
  fe_export(fe, y, A.y);

  return 1;
}

int
ecdsa_pubkey_import(wei_t *ec,
                    unsigned char *out,
                    size_t *out_len,
                    const unsigned char *x,
                    size_t x_len,
                    const unsigned char *y,
                    size_t y_len,
                    int sign,
                    int compact) {
  prime_field_t *fe = &ec->fe;
  unsigned char xp[MAX_FIELD_SIZE];
  unsigned char yp[MAX_FIELD_SIZE];
  int has_y = (y_len > 0);
  wge_t A;

  while (x_len > 0 && x[0] == 0x00) {
    x_len -= 1;
    x += 1;
  }

  while (y_len > 0 && y[0] == 0x00) {
    y_len -= 1;
    y += 1;
  }

  if (x_len > fe->size || y_len > fe->size)
    return 0;

  memset(xp, 0x00, fe->size - x_len);
  memcpy(xp + fe->size - x_len, x, x_len);

  memset(yp, 0x00, fe->size - y_len);
  memcpy(yp + fe->size - y_len, y, y_len);

  if (!fe_import(fe, A.x, xp))
    return 0;

  if (!fe_import(fe, A.y, yp))
    return 0;

  if (has_y) {
    wge_set_xy(ec, &A, A.x, A.y);

    if (!wge_validate(ec, &A))
      return 0;
  } else {
    if (!wge_set_x(ec, &A, A.x, sign))
      return 0;
  }

  return wge_export(ec, out, out_len, &A, compact);
}

int
ecdsa_pubkey_tweak_add(wei_t *ec,
                       unsigned char *out,
                       size_t *out_len,
                       const unsigned char *pub,
                       size_t pub_len,
                       const unsigned char *tweak,
                       int compact) {
  scalar_field_t *sc = &ec->sc;
  wge_t A, T;
  sc_t t;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  if (!sc_import(sc, t, tweak))
    return 0;

  wei_mul_g(ec, &T, t);
  wge_add(ec, &A, &A, &T);

  sc_cleanse(sc, t);

  return wge_export(ec, out, out_len, &A, compact);
}

int
ecdsa_pubkey_tweak_mul(wei_t *ec,
                       unsigned char *out,
                       size_t *out_len,
                       const unsigned char *pub,
                       size_t pub_len,
                       const unsigned char *tweak,
                       int compact) {
  scalar_field_t *sc = &ec->sc;
  wge_t A;
  sc_t t;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  if (!sc_import(sc, t, tweak))
    return 0;

  wei_mul(ec, &A, &A, t);

  sc_cleanse(sc, t);

  return wge_export(ec, out, out_len, &A, compact);
}

int
ecdsa_pubkey_combine(wei_t *ec,
                     unsigned char *out,
                     size_t *out_len,
                     const unsigned char **pubs,
                     size_t *pub_lens,
                     size_t len,
                     int compact) {
  wge_t A;
  jge_t P;
  size_t i;

  jge_zero(ec, &P);

  for (i = 0; i < len; i++) {
    if (!wge_import(ec, &A, pubs[i], pub_lens[i]))
      return 0;

    jge_mixed_add(ec, &P, &P, &A);
  }

  jge_to_wge(ec, &A, &P);

  return wge_export(ec, out, out_len, &A, compact);
}

int
ecdsa_pubkey_negate(wei_t *ec,
                    unsigned char *out,
                    size_t *out_len,
                    const unsigned char *pub,
                    size_t pub_len,
                    int compact) {
  wge_t A;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  wge_neg(ec, &A, &A);

  return wge_export(ec, out, out_len, &A, compact);
}

static void
ecdsa_encode_der(wei_t *ec,
                 unsigned char *out,
                 size_t *out_len,
                 const sc_t r,
                 const sc_t s) {
  scalar_field_t *sc = &ec->sc;
  unsigned char rp[MAX_SCALAR_SIZE];
  unsigned char sp[MAX_SCALAR_SIZE];
  size_t size = 0;
  size_t pos = 0;

  sc_export(sc, rp, r);
  sc_export(sc, sp, s);

  size += asn1_size_int(rp, sc->size);
  size += asn1_size_int(sp, sc->size);

  pos = asn1_write_seq(out, pos, size);
  pos = asn1_write_int(out, pos, rp, sc->size);
  pos = asn1_write_int(out, pos, sp, sc->size);

  *out_len = pos;
}

static int
ecdsa_decode_der(wei_t *ec,
                 sc_t r,
                 sc_t s,
                 const unsigned char *der,
                 size_t der_len,
                 int strict) {
  scalar_field_t *sc = &ec->sc;
  unsigned char rp[MAX_SCALAR_SIZE];
  unsigned char sp[MAX_SCALAR_SIZE];

  if (!asn1_read_seq(&der, &der_len, strict))
    return 0;

  if (!asn1_read_int(rp, sc->size, &der, &der_len, strict))
    return 0;

  if (!asn1_read_int(sp, sc->size, &der, &der_len, strict))
    return 0;

  if (strict && der_len != 0)
    return 0;

  if (!sc_import(sc, r, rp))
    return 0;

  if (!sc_import(sc, s, sp))
    return 0;

  return 1;
}

static int
ecdsa_reduce(wei_t *ec, sc_t r, const unsigned char *msg, size_t msg_len) {
  /* Byte array to integer conversion.
   *
   * [SEC1] Step 5, Page 45, Section 4.1.3.
   * [FIPS186] Page 25, Section B.2.
   *
   * The two sources above disagree on this.
   *
   * FIPS186 simply modulos the entire byte
   * array by the order, whereas SEC1 takes
   * the left-most ceil(log2(n)) bits modulo
   * the order (and maybe does other stuff).
   *
   * Instead of trying to decipher all of
   * this nonsense, we simply replicate the
   * OpenSSL behavior (which, in actuality,
   * is more similar to the SEC1 behavior).
   */
  scalar_field_t *sc = &ec->sc;
  unsigned char tmp[MAX_SCALAR_SIZE];
  int ret;

  /* Truncate. */
  if (msg_len > sc->size)
    msg_len = sc->size;

  /* Copy and pad. */
  memset(tmp, 0x00, sc->size - msg_len);
  memcpy(tmp + sc->size - msg_len, msg, msg_len);

  assert(sc->endian == 1);

  /* Shift by the remaining bits. */
  /* Note that the message length is not secret. */
  if (msg_len * 8 > sc->bits) {
    size_t shift = msg_len * 8 - sc->bits;
    unsigned char mask = (1 << shift) - 1;
    unsigned char cy = 0;
    size_t i;

    assert(shift > 0);
    assert(shift < 8);

    for (i = 0; i < sc->size; i++) {
      unsigned char ch = tmp[i];

      tmp[i] = (cy << (8 - shift)) | (ch >> shift);
      cy = ch & mask;
    }
  }

  ret = sc_import_weak(sc, r, tmp);

  cleanse(tmp, sizeof(tmp));

  return ret;
}

int
ecdsa_sig_export(wei_t *ec,
                 unsigned char *out,
                 size_t *out_len,
                 const unsigned char *sig) {
  scalar_field_t *sc = &ec->sc;
  sc_t r, s;

  if (!sc_import(sc, r, sig))
    return 0;

  if (!sc_import(sc, s, sig + sc->size))
    return 0;

  ecdsa_encode_der(ec, out, out_len, r, s);

  return 1;
}

int
ecdsa_sig_import(wei_t *ec,
                 unsigned char *out,
                 const unsigned char *der,
                 size_t der_len) {
  scalar_field_t *sc = &ec->sc;
  sc_t r, s;

  if (!ecdsa_decode_der(ec, r, s, der, der_len, 1))
    return 0;

  sc_export(sc, out, r);
  sc_export(sc, out + sc->size, s);

  return 1;
}

int
ecdsa_sig_import_lax(wei_t *ec,
                     unsigned char *out,
                     const unsigned char *der,
                     size_t der_len) {
  scalar_field_t *sc = &ec->sc;
  sc_t r, s;

  if (!ecdsa_decode_der(ec, r, s, der, der_len, 0))
    return 0;

  sc_export(sc, out, r);
  sc_export(sc, out + sc->size, s);

  return 1;
}

int
ecdsa_sig_normalize(wei_t *ec, unsigned char *out, const unsigned char *sig) {
  scalar_field_t *sc = &ec->sc;
  sc_t r, s;

  if (!sc_import(sc, r, sig))
    return 0;

  if (!sc_import(sc, s, sig + sc->size))
    return 0;

  if (sc_is_high_var(sc, s))
    sc_neg(sc, s, s);

  sc_export(sc, out, r);
  sc_export(sc, out + sc->size, s);

  return 1;
}

int
ecdsa_is_low_s(wei_t *ec, const unsigned char *sig) {
  scalar_field_t *sc = &ec->sc;
  sc_t r, s;

  if (!sc_import(sc, r, sig))
    return 0;

  if (!sc_import(sc, s, sig + sc->size))
    return 0;

  return !sc_is_high_var(sc, s);
}

int
ecdsa_sign(wei_t *ec,
           unsigned char *sig,
           unsigned int *param,
           const unsigned char *msg,
           size_t msg_len,
           const unsigned char *priv) {
  /* ECDSA Signing.
   *
   * [SEC1] Page 44, Section 4.1.3.
   * [GECC] Algorithm 4.29, Page 184, Section 4.4.1.
   * [RFC6979] Page 9, Section 2.4.
   * [RFC6979] Page 10, Section 3.2.
   *
   * Assumptions:
   *
   *   - Let `m` be an integer reduced from bytes.
   *   - Let `a` be a secret non-zero scalar.
   *   - Let `k` be a random non-zero scalar.
   *   - R != O, r != 0, s != 0.
   *
   * Computation:
   *
   *   k = random integer in [1,n-1]
   *   R = G * k
   *   r = x(R) mod n
   *   s = (r * a + m) / k mod n
   *   s = -s mod n, if s > n / 2
   *   S = (r, s)
   *
   * Note that `k` must remain secret,
   * otherwise an attacker can compute:
   *
   *   a = (s * k - m) / r mod n
   *
   * This means that if two signatures
   * share the same `r` value, an attacker
   * can compute:
   *
   *   k = (m1 - m2) / (+-s1 - +-s2) mod n
   *   a = (s1 * k - m1) / r mod n
   *
   * Assuming:
   *
   *   s1 = (r * a + m1) / k mod n
   *   s2 = (r * a + m2) / k mod n
   *
   * To mitigate this, `k` can be generated
   * deterministically using the HMAC-DRBG
   * construction described in [RFC6979].
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  drbg_t rng;
  sc_t a, m, k, r, s;
  wge_t R;
  unsigned char bytes[MAX_SCALAR_SIZE * 2];
  unsigned int sign, high;
  int ret = 0;

  if (param != NULL)
    *param = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  ecdsa_reduce(ec, m, msg, msg_len);

  sc_export(sc, bytes, a);
  sc_export(sc, bytes + sc->size, m);

  drbg_init(&rng, ec->hash, bytes, sc->size * 2);

  /* Constant-time (assuming 1 iteration). */
  for (;;) {
    drbg_generate(&rng, bytes, sc->size);

    if (!ecdsa_reduce(ec, k, bytes, sc->size))
      continue;

    if (sc_is_zero(sc, k))
      continue;

    wei_mul_g(ec, &R, k);

    if (wge_is_zero(ec, &R))
      continue;

    sign = fe_is_odd(fe, R.y);
    high = sc_set_fe(sc, fe, r, R.x) ^ 1;

    if (sc_is_zero(sc, r))
      continue;

    sc_invert(sc, k, k);
    sc_mul(sc, s, r, a);
    sc_add(sc, s, s, m);
    sc_mul(sc, s, s, k);

    sign ^= sc_minimize(sc, s, s);

    sc_export(sc, sig, r);
    sc_export(sc, sig + sc->size, s);

    if (param != NULL)
      *param = (high << 1) | sign;

    break;
  }

  ret = 1;
fail:
  cleanse(&rng, sizeof(rng));
  sc_cleanse(sc, a);
  sc_cleanse(sc, m);
  sc_cleanse(sc, k);
  sc_cleanse(sc, r);
  sc_cleanse(sc, s);
  wge_cleanse(ec, &R);
  cleanse(bytes, sizeof(bytes));
  return ret;
}

int
ecdsa_verify(wei_t *ec,
             const unsigned char *msg,
             size_t msg_len,
             const unsigned char *sig,
             const unsigned char *pub,
             size_t pub_len) {
  /* ECDSA Verification.
   *
   * [SEC1] Page 46, Section 4.1.4.
   * [GECC] Algorithm 4.30, Page 184, Section 4.4.1.
   *
   * Assumptions:
   *
   *   - Let `m` be an integer reduced from bytes.
   *   - Let `r` and `s` be signature elements.
   *   - Let `A` be a valid group element.
   *   - r != 0, r < n.
   *   - s != 0, s < n.
   *   - R != O.
   *
   * Computation:
   *
   *   u1 = m / s mod n
   *   u2 = r / s mod n
   *   R = G * u1 + A * u2
   *   r == x(R) mod n
   *
   * Note that the signer can verify their
   * own signatures more efficiently with:
   *
   *   R = G * ((u1 + u2 * a) mod n)
   *
   * Furthermore, we can avoid affinization
   * of `R` by scaling `r` by `z^2` and
   * repeatedly adding `n * z^2` to it up
   * to a certain threshold.
   */
  scalar_field_t *sc = &ec->sc;
  sc_t m, r, s, u1, u2;
  wge_t A;
  jge_t Rj;
#ifndef ECC_WITH_TRICK
  prime_field_t *fe = &ec->fe;
  wge_t Ra;
  sc_t re;
#endif

  ecdsa_reduce(ec, m, msg, msg_len);

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  if (!sc_import(sc, r, sig))
    return 0;

  if (!sc_import(sc, s, sig + sc->size))
    return 0;

  if (sc_is_zero(sc, r) || sc_is_zero(sc, s))
    return 0;

  if (sc_is_high(sc, s))
    return 0;

  sc_invert_var(sc, s, s);
  sc_mul(sc, u1, m, s);
  sc_mul(sc, u2, r, s);

  wei_jmul_double_var(ec, &Rj, u1, &A, u2);

#ifdef ECC_WITH_TRICK
  return jge_equal_r(ec, &Rj, r);
#else
  if (jge_is_zero(ec, &Rj))
    return 0;

  jge_to_wge_var(ec, &Ra, &Rj);
  sc_set_fe(sc, fe, re, Ra.x);

  return sc_equal(sc, r, re);
#endif
}

int
ecdsa_recover(wei_t *ec,
              unsigned char *pub,
              size_t *pub_len,
              const unsigned char *msg,
              size_t msg_len,
              const unsigned char *sig,
              unsigned int param,
              int compact) {
  /* ECDSA Public Key Recovery.
   *
   * [SEC1] Page 47, Section 4.1.6.
   *
   * Assumptions:
   *
   *   - Let `m` be an integer reduced from bytes.
   *   - Let `r` and `s` be signature elements.
   *   - Let `i` be an integer in [0,3].
   *   - x^3 + a * x + b is square in F(p).
   *   - If i > 1 then r < (p mod n).
   *   - r != 0, r < n.
   *   - s != 0, s < n.
   *   - A != O.
   *
   * Computation:
   *
   *   x = r + n, if i > 1
   *     = r, otherwise
   *   R' = (x, sqrt(x^3 + a * x + b))
   *   R = -R', if i mod 2 == 1
   *     = +R', otherwise
   *   s1 = m / r mod n
   *   s2 = s / r mod n
   *   A = R * s2 - G * s1
   *
   * Note that this implementation will have
   * trouble on curves where `p / n > 1`.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  unsigned int sign = param & 1;
  unsigned int high = param >> 1;
  sc_t m, r, s, s1, s2;
  fe_t x;
  wge_t R, A;

  ecdsa_reduce(ec, m, msg, msg_len);

  if (!sc_import(sc, r, sig))
    return 0;

  if (!sc_import(sc, s, sig + sc->size))
    return 0;

  if (sc_is_zero(sc, r) || sc_is_zero(sc, s))
    return 0;

  if (sc_is_high(sc, s))
    return 0;

  /* Assumes n < p. */
  assert(fe_set_sc(fe, sc, x, r));

  if (high) {
    if (sc_cmp_var(sc, r, ec->pmodn) >= 0)
      return 0;

    fe_add(fe, x, x, ec->red_n);
  }

  if (!wge_set_x(ec, &R, x, sign))
    return 0;

  sc_invert_var(sc, r, r);
  sc_mul(sc, s1, m, r);
  sc_mul(sc, s2, s, r);
  sc_neg(sc, s1, s1);

  wei_mul_double_var(ec, &A, s1, &R, s2);

  return wge_export(ec, pub, pub_len, &A, compact);
}

int
ecdsa_derive(wei_t *ec,
             unsigned char *secret,
             size_t *secret_len,
             const unsigned char *pub,
             const size_t pub_len,
             const unsigned char *priv,
             int compact) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  wge_t A, P;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  if (!wge_import(ec, &A, pub, pub_len))
    goto fail;

  wei_mul(ec, &P, &A, a);

  if (!wge_export(ec, secret, secret_len, &P, compact))
    goto fail;

  ret = 1;
fail:
  sc_cleanse(sc, a);
  wge_cleanse(ec, &A);
  wge_cleanse(ec, &P);
  return ret;
}

static void
ecdsa_schnorr_hash_am(wei_t *ec, sc_t k,
                      const unsigned char *scalar,
                      const unsigned char *msg) {
  scalar_field_t *sc = &ec->sc;
  unsigned char bytes[MAX_SCALAR_SIZE];
  size_t hash_size = hash_output_size(ec->hash);
  size_t off = 0;
  hash_t hash;

  assert(MAX_SCALAR_SIZE >= HASH_MAX_OUTPUT_SIZE);

  if (sc->size > hash_size) {
    off = sc->size - hash_size;
    memset(bytes, 0x00, off);
  }

  hash_init(&hash, ec->hash);
  hash_update(&hash, scalar, sc->size);
  hash_update(&hash, msg, 32);
  hash_final(&hash, bytes + off, sc->size);

  sc_import_reduce(sc, k, bytes);

  cleanse(bytes, sizeof(bytes));
  cleanse(&hash, sizeof(hash));
}

static void
ecdsa_schnorr_hash_ram(wei_t *ec, sc_t e,
                       const unsigned char *R,
                       const unsigned char *A,
                       const unsigned char *msg) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  unsigned char bytes[MAX_SCALAR_SIZE];
  size_t hash_size = hash_output_size(ec->hash);
  size_t off = 0;
  hash_t hash;

  assert(MAX_SCALAR_SIZE >= HASH_MAX_OUTPUT_SIZE);

  if (sc->size > hash_size) {
    off = sc->size - hash_size;
    memset(bytes, 0x00, off);
  }

  hash_init(&hash, ec->hash);
  hash_update(&hash, R, fe->size);
  hash_update(&hash, A, fe->size + 1);
  hash_update(&hash, msg, 32);
  hash_final(&hash, bytes + off, sc->size);

  sc_import_reduce(sc, e, bytes);

  cleanse(bytes, sizeof(bytes));
  cleanse(&hash, sizeof(hash));
}

int
ecdsa_schnorr_support(wei_t *ec) {
  /* Must satisfy p = 3 mod 4. */
  return (ec->fe.p[0] & 3) == 3;
}

int
ecdsa_schnorr_sign(wei_t *ec,
                   unsigned char *sig,
                   const unsigned char *msg,
                   const unsigned char *priv) {
  /* Schnorr Signing.
   *
   * [SCHNORR] "Signing".
   * [CASH] "Recommended practices for secure signature generation".
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a 32-byte array.
   *   - Let `a` be a secret non-zero scalar.
   *   - k != 0.
   *
   * Computation:
   *
   *   A = G * a
   *   k = H(a, m) mod n
   *   R = G * k
   *   k = -k mod n, if y(R) is not square
   *   r = x(R)
   *   e = H(r, A, m) mod n
   *   s = (k + e * a) mod n
   *   S = (r, s)
   *
   * Note that `k` must remain secret,
   * otherwise an attacker can compute:
   *
   *   a = (s - k) / e mod n
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  unsigned char *Rraw = sig;
  unsigned char *sraw = sig + fe->size;
  unsigned char Araw[MAX_FIELD_SIZE + 1];
  sc_t a, k, e, s;
  wge_t A, R;
  int ret = 0;

  /* Must satisfy p = 3 mod 4. */
  if ((fe->p[0] & 3) != 3)
    return 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  wei_mul_g(ec, &A, a);

  ecdsa_schnorr_hash_am(ec, k, priv, msg);

  if (sc_is_zero(sc, k))
    goto fail;

  wei_mul_g(ec, &R, k);

  sc_neg_cond(sc, k, k, wge_is_square(ec, &R) ^ 1);

  wge_export_x(ec, Rraw, &R);
  wge_export(ec, Araw, NULL, &A, 1);

  ecdsa_schnorr_hash_ram(ec, e, Rraw, Araw, msg);

  sc_mul(sc, s, e, a);
  sc_add(sc, s, s, k);

  sc_export(sc, sraw, s);

  ret = 1;
fail:
  cleanse(Araw, sizeof(Araw));
  sc_cleanse(sc, a);
  sc_cleanse(sc, k);
  sc_cleanse(sc, e);
  sc_cleanse(sc, s);
  wge_cleanse(ec, &A);
  wge_cleanse(ec, &R);
  return ret;
}

int
ecdsa_schnorr_verify(wei_t *ec,
                     const unsigned char *msg,
                     const unsigned char *sig,
                     const unsigned char *pub,
                     size_t pub_len) {
  /* Schnorr Verification.
   *
   * [SCHNORR] "Verification".
   * [CASH] "Signature verification algorithm".
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a 32-byte array.
   *   - Let `r` and `s` be signature elements.
   *   - Let `A` be a valid group element.
   *   - r^3 + a * r + b is square in F(p).
   *   - r < p, s < n.
   *   - R != O.
   *
   * Computation:
   *
   *   R = (r, sqrt(r^3 + a * r + b))
   *   e = H(r, A, m) mod n
   *   R == G * s - A * e
   *
   * We can skip a square root with:
   *
   *   e = H(r, A, m) mod n
   *   R = G * s - A * e
   *   y(R) is square
   *   x(R) == r
   *
   * We can also avoid affinization by
   * replacing the two assertions with:
   *
   *   (y(R) * z(R) mod p) is square
   *   x(R) == r * z(R)^2 mod p
   *
   * Furthermore, squareness can be calculated
   * with a variable time Jacobi symbol algorithm.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  const unsigned char *Rraw = sig;
  const unsigned char *sraw = sig + fe->size;
  unsigned char Araw[MAX_FIELD_SIZE + 1];
  fe_t r;
  sc_t s, e;
  wge_t A;
  jge_t R;

  /* Must satisfy p = 3 mod 4. */
  if ((fe->p[0] & 3) != 3)
    return 0;

  if (!fe_import(fe, r, Rraw))
    return 0;

  if (!sc_import(sc, s, sraw))
    return 0;

  if (!wge_import(ec, &A, pub, pub_len))
    return 0;

  wge_export(ec, Araw, NULL, &A, 1);

  ecdsa_schnorr_hash_ram(ec, e, Rraw, Araw, msg);

  sc_neg(sc, e, e);

  wei_jmul_double_var(ec, &R, s, &A, e);

  if (!jge_is_square_var(ec, &R))
    return 0;

  if (!jge_equal_x(ec, &R, r))
    return 0;

  return 1;
}

int
ecdsa_schnorr_verify_batch(wei_t *ec,
                           const unsigned char **msgs,
                           const unsigned char **sigs,
                           const unsigned char **pubs,
                           size_t *pub_lens,
                           size_t len,
                           wei_scratch_t *scratch) {
  /* Schnorr Batch Verification.
   *
   * [SCHNORR] "Batch Verification".
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a 32-byte array.
   *   - Let `r` and `s` be signature elements.
   *   - Let `A` be a valid group element.
   *   - Let `i` be the batch item index.
   *   - r^3 + a * r + b is square in F(p).
   *   - r < p, s < n.
   *   - a1 = 1 mod n.
   *
   * Computation:
   *
   *   Ri = (ri, sqrt(ri^3 + a * ri + b))
   *   ei = H(ri, Ai, mi) mod n
   *   ai = random integer in [1,n-1]
   *   lhs = si * ai + ... mod n
   *   rhs = Ri * ai + Ai * (ei * ai mod n) + ...
   *   G * -lhs + rhs == O
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  wge_t *points = scratch->points;
  sc_t *coeffs = scratch->coeffs;
  unsigned char Araw[MAX_FIELD_SIZE + 1];
  drbg_t rng;
  wge_t R, A;
  jge_t r;
  sc_t sum, s, e, a;
  size_t j = 0;
  size_t i;

  /* Must satisfy p = 3 mod 4. */
  if ((fe->p[0] & 3) != 3)
    return 0;

  /* Seed RNG. */
  {
    unsigned char bytes[64];
    hash_t hash;

    hash_init(&hash, HASH_SHA512);

    for (i = 0; i < len; i++) {
      const unsigned char *msg = msgs[i];
      const unsigned char *sig = sigs[i];
      const unsigned char *pub = pubs[i];
      size_t pub_len = pub_lens[i];

      /* Quick key reserialization. */
      if (pub_len == fe->size + 1) {
        memcpy(Araw, pub, pub_len);
      } else if (pub_len == fe->size * 2 + 1) {
        Araw[0] = 0x02 | (pub[pub_len - 1] & 1);
        memcpy(Araw + 1, pub + 1, fe->size);
      } else {
        memset(Araw, 0x00, fe->size + 1);
      }

      hash_update(&hash, msg, 32);
      hash_update(&hash, sig, fe->size + sc->size);
      hash_update(&hash, Araw, fe->size + 1);
    }

    hash_final(&hash, bytes, 64);

    drbg_init(&rng, HASH_SHA256, bytes, 64);
  }

  /* Intialize sum. */
  sc_zero(sc, sum);

  /* Verify signatures. */
  for (i = 0; i < len; i++) {
    const unsigned char *msg = msgs[i];
    const unsigned char *sig = sigs[i];
    const unsigned char *pub = pubs[i];
    size_t pub_len = pub_lens[i];
    const unsigned char *Rraw = sig;
    const unsigned char *sraw = sig + fe->size;

    if (!wge_import_x(ec, &R, Rraw))
      return 0;

    if (!wge_import(ec, &A, pub, pub_len))
      return 0;

    if (!sc_import(sc, s, sraw))
      return 0;

    wge_export(ec, Araw, NULL, &A, 1);

    ecdsa_schnorr_hash_ram(ec, e, Rraw, Araw, msg);

    if (j == 0) {
      sc_zero(sc, a);
      a[0] = 1;
    } else {
      sc_random(sc, a, &rng);
    }

    sc_mul(sc, e, e, a);
    sc_mul(sc, s, s, a);
    sc_add(sc, sum, sum, s);

    wge_set(ec, &points[j + 0], &R);
    wge_set(ec, &points[j + 1], &A);

    sc_set(sc, coeffs[j + 0], a);
    sc_set(sc, coeffs[j + 1], e);

    j += 2;

    if (j == 64) {
      sc_neg(sc, sum, sum);

      wei_jmul_multi_var(ec, &r, sum, points, coeffs, j, scratch);

      if (!jge_is_zero(ec, &r))
        return 0;

      sc_zero(sc, sum);

      j = 0;
    }
  }

  if (j > 0) {
    sc_neg(sc, sum, sum);

    wei_jmul_multi_var(ec, &r, sum, points, coeffs, j, scratch);

    if (!jge_is_zero(ec, &r))
      return 0;
  }

  return 1;
}

/*
 * Schnorr
 */

wei_t *
schnorr_context_create(const char *id) {
  wei_t *ec = ecdsa_context_create(id);

  if (ec == NULL)
    return NULL;

  /* Must be congruent to 3 mod 4. */
  if ((ec->fe.p[0] & 3) != 3) {
    ecdsa_context_destroy(ec);
    return 0;
  }

  return ec;
}

void
schnorr_context_destroy(wei_t *ec) {
  ecdsa_context_destroy(ec);
}

void
schnorr_context_randomize(wei_t *ec, const unsigned char *entropy) {
  ecdsa_context_randomize(ec, entropy);
}

wei_scratch_t *
schnorr_scratch_create(wei_t *ec) {
  return ecdsa_scratch_create(ec);
}

void
schnorr_scratch_destroy(wei_t *ec, wei_scratch_t *scratch) {
  ecdsa_scratch_destroy(ec, scratch);
}

size_t
schnorr_scalar_size(wei_t *ec) {
  return ec->sc.size;
}

size_t
schnorr_scalar_bits(wei_t *ec) {
  return ec->sc.bits;
}

size_t
schnorr_field_size(wei_t *ec) {
  return ec->fe.size;
}

size_t
schnorr_field_bits(wei_t *ec) {
  return ec->fe.bits;
}

size_t
schnorr_privkey_size(wei_t *ec) {
  return ec->sc.size;
}

size_t
schnorr_pubkey_size(wei_t *ec) {
  return ec->fe.size;
}

size_t
schnorr_sig_size(wei_t *ec) {
  return ec->fe.size + ec->sc.size;
}

void
schnorr_privkey_generate(wei_t *ec,
                         unsigned char *out,
                         const unsigned char *entropy) {
  ecdsa_privkey_generate(ec, out, entropy);
}

int
schnorr_privkey_verify(wei_t *ec, const unsigned char *priv) {
  return ecdsa_privkey_verify(ec, priv);
}

int
schnorr_privkey_export(wei_t *ec,
                       unsigned char *out,
                       const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  jge_t A;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  wei_jmul_g(ec, &A, a);

  if (!jge_is_square_var(ec, &A))
    sc_neg(sc, a, a);

  sc_export(sc, out, a);

  ret = 1;
fail:
  sc_cleanse(sc, a);
  jge_cleanse(ec, &A);
  return ret;
}

int
schnorr_privkey_import(wei_t *ec,
                       unsigned char *out,
                       const unsigned char *bytes,
                       size_t len) {
  return ecdsa_privkey_import(ec, out, bytes, len);
}

int
schnorr_privkey_tweak_add(wei_t *ec,
                          unsigned char *out,
                          const unsigned char *priv,
                          const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  sc_t a, t;
  jge_t A;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  if (!sc_import(sc, t, tweak))
    goto fail;

  wei_jmul_g(ec, &A, a);

  if (!jge_is_square_var(ec, &A))
    sc_neg(sc, a, a);

  sc_add(sc, a, a, t);

  if (sc_is_zero(sc, a))
    goto fail;

  sc_export(sc, out, a);

  ret = 1;
fail:
  sc_cleanse(sc, a);
  sc_cleanse(sc, t);
  jge_cleanse(ec, &A);
  return ret;
}

int
schnorr_privkey_tweak_mul(wei_t *ec,
                          unsigned char *out,
                          const unsigned char *priv,
                          const unsigned char *tweak) {
  return ecdsa_privkey_tweak_mul(ec, out, priv, tweak);
}

int
schnorr_privkey_reduce(wei_t *ec,
                       unsigned char *out,
                       const unsigned char *bytes,
                       size_t len) {
  return ecdsa_privkey_reduce(ec, out, bytes, len);
}

int
schnorr_privkey_invert(wei_t *ec,
                       unsigned char *out,
                       const unsigned char *priv) {
  return ecdsa_privkey_invert(ec, out, priv);
}

int
schnorr_pubkey_create(wei_t *ec,
                      unsigned char *pub,
                      const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  wge_t A;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  wei_mul_g(ec, &A, a);

  if (!wge_export_x(ec, pub, &A))
    goto fail;

  ret = 1;
fail:
  sc_cleanse(sc, a);
  wge_cleanse(ec, &A);
  return ret;
}

void
schnorr_pubkey_from_uniform(wei_t *ec,
                            unsigned char *out,
                            const unsigned char *bytes) {
  wge_t A;

  wei_point_from_uniform(ec, &A, bytes);

  assert(wge_export_x(ec, out, &A));
}

int
schnorr_pubkey_to_uniform(wei_t *ec,
                          unsigned char *out,
                          const unsigned char *pub,
                          unsigned int hint) {
  wge_t A;

  if (!wge_import_x(ec, &A, pub))
    return 0;

  return wei_point_to_uniform(ec, out, &A, hint);
}

int
schnorr_pubkey_from_hash(wei_t *ec,
                         unsigned char *out,
                         const unsigned char *bytes) {
  wge_t A;

  wei_point_from_hash(ec, &A, bytes);

  return wge_export_x(ec, out, &A);
}

int
schnorr_pubkey_to_hash(wei_t *ec,
                       unsigned char *out,
                       const unsigned char *pub,
                       const unsigned char *entropy) {
  wge_t A;

  if (!wge_import_x(ec, &A, pub))
    return 0;

  wei_point_to_hash(ec, out, &A, entropy);

  return 1;
}

int
schnorr_pubkey_verify(wei_t *ec, unsigned char *out, const unsigned char *pub) {
  wge_t A;

  return wge_import_x(ec, &A, pub);
}

int
schnorr_pubkey_export(wei_t *ec,
                      unsigned char *x,
                      unsigned char *y,
                      const unsigned char *pub) {
  prime_field_t *fe = &ec->fe;
  wge_t A;

  if (!wge_import_x(ec, &A, pub))
    return 0;

  assert(!A.inf);

  fe_export(fe, x, A.x);
  fe_export(fe, y, A.y);

  return 1;
}

int
schnorr_pubkey_import(wei_t *ec,
                      unsigned char *out,
                      const unsigned char *x,
                      size_t x_len) {
  prime_field_t *fe = &ec->fe;
  unsigned char xp[MAX_FIELD_SIZE];
  wge_t A;

  while (x_len > 0 && x[0] == 0x00) {
    x_len -= 1;
    x += 1;
  }

  if (x_len > fe->size)
    return 0;

  memset(xp, 0x00, fe->size - x_len);
  memcpy(xp + fe->size - x_len, x, x_len);

  if (!wge_import_x(ec, &A, xp))
    return 0;

  return wge_export_x(ec, out, &A);
}

int
schnorr_pubkey_tweak_add(wei_t *ec,
                         unsigned char *out,
                         const unsigned char *pub,
                         const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  wge_t A, T;
  sc_t t;

  if (!wge_import_x(ec, &A, pub))
    return 0;

  if (!sc_import(sc, t, tweak))
    return 0;

  wei_mul_g(ec, &T, t);
  wge_add(ec, &A, &A, &T);

  sc_cleanse(sc, t);

  return wge_export_x(ec, out, &A);
}

int
schnorr_pubkey_tweak_mul(wei_t *ec,
                         unsigned char *out,
                         const unsigned char *pub,
                         const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  wge_t A;
  sc_t t;

  if (!wge_import_x(ec, &A, pub))
    return 0;

  if (!sc_import(sc, t, tweak))
    return 0;

  wei_mul(ec, &A, &A, t);

  sc_cleanse(sc, t);

  return wge_export_x(ec, out, &A);
}

int
schnorr_pubkey_combine(wei_t *ec,
                       unsigned char *out,
                       const unsigned char **pubs,
                       size_t len) {
  wge_t A;
  jge_t P;
  size_t i;

  jge_zero(ec, &P);

  for (i = 0; i < len; i++) {
    if (!wge_import_x(ec, &A, pubs[i]))
      return 0;

    jge_mixed_add(ec, &P, &P, &A);
  }

  jge_to_wge(ec, &A, &P);

  return wge_export_x(ec, out, &A);
}

static void
schnorr_hash_init(hash_t *hash, int type, const char *tag) {
  size_t size = hash_output_size(type);
  unsigned char bytes[HASH_MAX_OUTPUT_SIZE * 2];

  hash_init(hash, type);
  hash_update(hash, tag, strlen(tag));
  hash_final(hash, bytes, size);
  memcpy(bytes + size, bytes, size);

  hash_init(hash, type);
  hash_update(hash, bytes, size * 2);
}

static void
schnorr_hash_am(wei_t *ec, sc_t k,
                const unsigned char *scalar,
                const unsigned char *msg) {
  scalar_field_t *sc = &ec->sc;
  unsigned char bytes[MAX_SCALAR_SIZE];
  size_t hash_size = hash_output_size(ec->hash);
  size_t off = 0;
  hash_t hash;

  assert(MAX_SCALAR_SIZE >= HASH_MAX_OUTPUT_SIZE);

  if (sc->size > hash_size) {
    off = sc->size - hash_size;
    memset(bytes, 0x00, off);
  }

  schnorr_hash_init(&hash, ec->hash, "BIPSchnorrDerive");

  hash_update(&hash, scalar, sc->size);
  hash_update(&hash, msg, 32);
  hash_final(&hash, bytes + off, sc->size);

  sc_import_reduce(sc, k, bytes);

  cleanse(bytes, sizeof(bytes));
  cleanse(&hash, sizeof(hash));
}

static void
schnorr_hash_ram(wei_t *ec, sc_t e,
                 const unsigned char *R,
                 const unsigned char *A,
                 const unsigned char *msg) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  unsigned char bytes[MAX_SCALAR_SIZE];
  size_t hash_size = hash_output_size(ec->hash);
  size_t off = 0;
  hash_t hash;

  assert(MAX_SCALAR_SIZE >= HASH_MAX_OUTPUT_SIZE);

  if (sc->size > hash_size) {
    off = sc->size - hash_size;
    memset(bytes, 0x00, off);
  }

  schnorr_hash_init(&hash, ec->hash, "BIPSchnorr");

  hash_update(&hash, R, fe->size);
  hash_update(&hash, A, fe->size);
  hash_update(&hash, msg, 32);
  hash_final(&hash, bytes + off, sc->size);

  sc_import_reduce(sc, e, bytes);

  cleanse(bytes, sizeof(bytes));
  cleanse(&hash, sizeof(hash));
}

int
schnorr_sign(wei_t *ec,
             unsigned char *sig,
             const unsigned char *msg,
             const unsigned char *priv) {
  /* Schnorr Signing.
   *
   * [SCHNORR] "Default Signing".
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a 32-byte array.
   *   - Let `a` be a secret non-zero scalar.
   *   - k != 0.
   *
   * Computation:
   *
   *   A = G * a
   *   a = -a mod n, if y(A) is not square
   *   k = H("BIPSchnorrDerive", a, m) mod n
   *   R = G * k
   *   k = -k mod n, if y(R) is not square
   *   r = x(R)
   *   x = x(A)
   *   e = H("BIPSchnorr", r, x, m) mod n
   *   s = (k + e * a) mod n
   *   S = (r, s)
   *
   * Note that `k` must remain secret,
   * otherwise an attacker can compute:
   *
   *   a = (s - k) / e mod n
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  unsigned char *Rraw = sig;
  unsigned char *sraw = sig + fe->size;
  unsigned char araw[MAX_SCALAR_SIZE];
  unsigned char Araw[MAX_FIELD_SIZE];
  sc_t a, k, e, s;
  wge_t A, R;
  int ret = 0;

  /* Must satisfy p = 3 mod 4. */
  assert((fe->p[0] & 3) == 3);

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  wei_mul_g(ec, &A, a);

  sc_neg_cond(sc, a, a, wge_is_square(ec, &A) ^ 1);
  sc_export(sc, araw, a);

  schnorr_hash_am(ec, k, araw, msg);

  if (sc_is_zero(sc, k))
    goto fail;

  wei_mul_g(ec, &R, k);

  sc_neg_cond(sc, k, k, wge_is_square(ec, &R) ^ 1);

  wge_export_x(ec, Rraw, &R);
  wge_export_x(ec, Araw, &A);

  schnorr_hash_ram(ec, e, Rraw, Araw, msg);

  sc_mul(sc, s, e, a);
  sc_add(sc, s, s, k);

  sc_export(sc, sraw, s);

  ret = 1;
fail:
  cleanse(araw, sizeof(araw));
  cleanse(Araw, sizeof(Araw));
  sc_cleanse(sc, a);
  sc_cleanse(sc, k);
  sc_cleanse(sc, e);
  sc_cleanse(sc, s);
  wge_cleanse(ec, &A);
  wge_cleanse(ec, &R);
  return ret;
}

int
schnorr_verify(wei_t *ec,
               const unsigned char *msg,
               const unsigned char *sig,
               const unsigned char *pub) {
  /* Schnorr Verification.
   *
   * [SCHNORR] "Verification".
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a 32-byte array.
   *   - Let `r` and `s` be signature elements.
   *   - Let `x` be a field element.
   *   - r^3 + a * r + b is square in F(p).
   *   - x^3 + a * x + b is square in F(p).
   *   - r < p, s < n, x < p.
   *   - R != O.
   *
   * Computation:
   *
   *   R = (r, sqrt(r^3 + a * r + b))
   *   A = (x, sqrt(x^3 + a * x + b))
   *   e = H("BIPSchnorr", r, x, m) mod n
   *   R == G * s - A * e
   *
   * We can skip a square root with:
   *
   *   A = (x, sqrt(x^3 + a * x + b))
   *   e = H("BIPSchnorr", r, x, m) mod n
   *   R = G * s - A * e
   *   y(R) is square
   *   x(R) == r
   *
   * We can also avoid affinization by
   * replacing the two assertions with:
   *
   *   (y(R) * z(R) mod p) is square
   *   x(R) == r * z(R)^2 mod p
   *
   * Furthermore, squareness can be calculated
   * with a variable time Jacobi symbol algorithm.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  const unsigned char *Rraw = sig;
  const unsigned char *sraw = sig + fe->size;
  fe_t r;
  sc_t s, e;
  wge_t A;
  jge_t R;

  /* Must satisfy p = 3 mod 4. */
  assert((fe->p[0] & 3) == 3);

  if (!fe_import(fe, r, Rraw))
    return 0;

  if (!sc_import(sc, s, sraw))
    return 0;

  if (!wge_import_x(ec, &A, pub))
    return 0;

  schnorr_hash_ram(ec, e, Rraw, pub, msg);

  sc_neg(sc, e, e);

  wei_jmul_double_var(ec, &R, s, &A, e);

  if (!jge_is_square_var(ec, &R))
    return 0;

  if (!jge_equal_x(ec, &R, r))
    return 0;

  return 1;
}

int
schnorr_verify_batch(wei_t *ec,
                     const unsigned char **msgs,
                     const unsigned char **sigs,
                     const unsigned char **pubs,
                     size_t len,
                     wei_scratch_t *scratch) {
  /* Schnorr Batch Verification.
   *
   * [SCHNORR] "Batch Verification".
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a 32-byte array.
   *   - Let `r` and `s` be signature elements.
   *   - Let `x` be a field element.
   *   - Let `i` be the batch item index.
   *   - r^3 + a * r + b is square in F(p).
   *   - x^3 + a * x + b is square in F(p).
   *   - r < p, s < n, x < p.
   *   - a1 = 1 mod n.
   *
   * Computation:
   *
   *   Ri = (ri, sqrt(ri^3 + a * ri + b))
   *   Ai = (xi, sqrt(xi^3 + a * xi + b))
   *   ei = H("BIPSchnorr", ri, xi, mi) mod n
   *   ai = random integer in [1,n-1]
   *   lhs = si * ai + ... mod n
   *   rhs = Ri * ai + Ai * (ei * ai mod n) + ...
   *   G * -lhs + rhs == O
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  wge_t *points = scratch->points;
  sc_t *coeffs = scratch->coeffs;
  drbg_t rng;
  wge_t R, A;
  jge_t r;
  sc_t sum, s, e, a;
  size_t j = 0;
  size_t i;

  /* Must satisfy p = 3 mod 4. */
  assert((fe->p[0] & 3) == 3);

  /* Seed RNG. */
  {
    unsigned char bytes[64];
    hash_t hash;

    hash_init(&hash, HASH_SHA512);

    for (i = 0; i < len; i++) {
      const unsigned char *msg = msgs[i];
      const unsigned char *sig = sigs[i];
      const unsigned char *pub = pubs[i];

      hash_update(&hash, msg, 32);
      hash_update(&hash, sig, fe->size + sc->size);
      hash_update(&hash, pub, fe->size);
    }

    hash_final(&hash, bytes, 64);

    drbg_init(&rng, HASH_SHA256, bytes, 64);
  }

  /* Intialize sum. */
  sc_zero(sc, sum);

  /* Verify signatures. */
  for (i = 0; i < len; i++) {
    const unsigned char *msg = msgs[i];
    const unsigned char *sig = sigs[i];
    const unsigned char *pub = pubs[i];
    const unsigned char *Rraw = sig;
    const unsigned char *sraw = sig + fe->size;

    if (!wge_import_x(ec, &R, Rraw))
      return 0;

    if (!wge_import_x(ec, &A, pub))
      return 0;

    if (!sc_import(sc, s, sraw))
      return 0;

    schnorr_hash_ram(ec, e, Rraw, pub, msg);

    if (j == 0) {
      sc_zero(sc, a);
      a[0] = 1;
    } else {
      sc_random(sc, a, &rng);
    }

    sc_mul(sc, e, e, a);
    sc_mul(sc, s, s, a);
    sc_add(sc, sum, sum, s);

    wge_set(ec, &points[j + 0], &R);
    wge_set(ec, &points[j + 1], &A);

    sc_set(sc, coeffs[j + 0], a);
    sc_set(sc, coeffs[j + 1], e);

    j += 2;

    if (j == 64) {
      sc_neg(sc, sum, sum);

      wei_jmul_multi_var(ec, &r, sum, points, coeffs, j, scratch);

      if (!jge_is_zero(ec, &r))
        return 0;

      sc_zero(sc, sum);

      j = 0;
    }
  }

  if (j > 0) {
    sc_neg(sc, sum, sum);

    wei_jmul_multi_var(ec, &r, sum, points, coeffs, j, scratch);

    if (!jge_is_zero(ec, &r))
      return 0;
  }

  return 1;
}

int
schnorr_derive(wei_t *ec,
               unsigned char *secret,
               const unsigned char *pub,
               const unsigned char *priv) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  wge_t A, P;
  int ret = 0;

  if (!sc_import(sc, a, priv))
    goto fail;

  if (sc_is_zero(sc, a))
    goto fail;

  if (!wge_import_x(ec, &A, pub))
    goto fail;

  wei_mul(ec, &P, &A, a);

  if (!wge_export_x(ec, secret, &P))
    goto fail;

  ret = 1;
fail:
  sc_cleanse(sc, a);
  wge_cleanse(ec, &A);
  wge_cleanse(ec, &P);
  return ret;
}

/*
 * ECDH
 */

mont_t *
ecdh_context_create(const char *id) {
  mont_t *ec = NULL;
  const mont_def_t *def = NULL;
  size_t i;

  if (id == NULL)
    return NULL;

  for (i = 0; i < ARRAY_SIZE(mont_curves); i++) {
    if (strcmp(mont_curves[i]->id, id) == 0) {
      def = mont_curves[i];
      break;
    }
  }

  if (def == NULL)
    return NULL;

  ec = malloc(sizeof(mont_t));

  if (ec == NULL)
    return NULL;

  mont_init(ec, def);

  return ec;
}

void
ecdh_context_destroy(mont_t *ec) {
  if (ec != NULL)
    free(ec);
}

size_t
ecdh_scalar_size(mont_t *ec) {
  return ec->sc.size;
}

size_t
ecdh_scalar_bits(mont_t *ec) {
  return ec->sc.bits;
}

size_t
ecdh_field_size(mont_t *ec) {
  return ec->fe.size;
}

size_t
ecdh_field_bits(mont_t *ec) {
  return ec->fe.bits;
}

size_t
ecdh_privkey_size(mont_t *ec) {
  return ec->sc.size;
}

size_t
ecdh_pubkey_size(mont_t *ec) {
  return ec->fe.size;
}

void
ecdh_privkey_generate(mont_t *ec,
                      unsigned char *out,
                      const unsigned char *entropy) {
  scalar_field_t *sc = &ec->sc;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  drbg_generate(&rng, out, sc->size);

  mont_clamp(ec, out, out);

  cleanse(&rng, sizeof(rng));
}

int
ecdh_privkey_verify(mont_t *ec, const unsigned char *priv) {
  return 1;
}

int
ecdh_privkey_export(mont_t *ec, unsigned char *out, const unsigned char *priv) {
  memcpy(out, priv, ec->sc.size);
  return 1;
}

int
ecdh_privkey_import(mont_t *ec,
                    unsigned char *out,
                    const unsigned char *bytes,
                    size_t len) {
  scalar_field_t *sc = &ec->sc;
  unsigned char key[MAX_SCALAR_SIZE];

  while (len > 0 && bytes[len - 1] == 0x00)
    len -= 1;

  if (len > sc->size)
    return 0;

  memcpy(key, bytes, len);
  memset(key + len, 0x00, sc->size - len);
  memcpy(out, key, sc->size);

  cleanse(key, sizeof(key));

  return 1;
}

void
ecdh_pubkey_create(mont_t *ec, unsigned char *pub, const unsigned char *priv) {
  unsigned char clamped[MAX_SCALAR_SIZE];
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  pge_t A;

  mont_clamp(ec, clamped, priv);

  sc_import(sc, a, clamped);

  mont_mul_g(ec, &A, a);

  assert(pge_export(ec, pub, &A));

  cleanse(clamped, sizeof(clamped));
  sc_cleanse(sc, a);
  pge_cleanse(ec, &A);
}

int
ecdh_pubkey_convert(mont_t *ec,
                    unsigned char *out,
                    const unsigned char *pub,
                    int sign) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  mge_t A;
  pge_t P;
  sc_t k;
  xge_t e;

  /* Compensate for the 4-isogeny. */
  if (fe->bits == 448) {
    pge_import(ec, &P, pub);

    if (!pge_validate(ec, &P))
      return 0;

    pge_mulh(ec, &P, &P);

    sc_set_word(sc, k, 16);
    sc_invert_var(sc, k, k);

    mont_mul(ec, &P, &P, k);

    assert(pge_to_mge(ec, &A, &P, -1));
  } else {
    if (!mge_import(ec, &A, pub, -1))
      return 0;
  }

  /* Convert to Edwards. */
  mge_to_xge(ec, &e, &A);

  /* Affinize. */
  assert(fe_invert(fe, e.z, e.z));
  fe_mul(fe, e.x, e.x, e.z);
  fe_mul(fe, e.y, e.y, e.z);

  /* Set sign and export. */
  if (sign != -1)
    fe_set_odd(fe, e.x, e.x, sign);

  fe_export(fe, out, e.y);

  /* Quirk: we need an extra byte (p448). */
  if ((fe->bits & 7) == 0)
    out[fe->size] = fe_is_odd(fe, e.x) << 7;
  else
    out[fe->size - 1] |= fe_is_odd(fe, e.x) << 7;

  return 1;
}

void
ecdh_pubkey_from_uniform(mont_t *ec,
                         unsigned char *out,
                         const unsigned char *bytes) {
  mge_t A;

  mont_point_from_uniform(ec, &A, bytes);

  assert(mge_export(ec, out, &A));
}

int
ecdh_pubkey_to_uniform(mont_t *ec,
                       unsigned char *out,
                       const unsigned char *pub,
                       unsigned int hint) {
  mge_t A;

  if (!mge_import(ec, &A, pub, -1))
    return 0;

  return mont_point_to_uniform(ec, out, &A, hint);
}

int
ecdh_pubkey_from_hash(mont_t *ec,
                      unsigned char *out,
                      const unsigned char *bytes,
                      int pake) {
  mge_t A;
  pge_t P;

  mont_point_from_hash(ec, &A, bytes);
  mge_to_pge(ec, &P, &A);

  if (pake)
    pge_mulh(ec, &P, &P);

  return pge_export(ec, out, &P);
}

int
ecdh_pubkey_to_hash(mont_t *ec,
                    unsigned char *out,
                    const unsigned char *pub,
                    const unsigned char *entropy) {
  mge_t A;

  if (!mge_import(ec, &A, pub, -1))
    return 0;

  mont_point_to_hash(ec, out, &A, entropy);

  return 1;
}

int
ecdh_pubkey_verify(mont_t *ec, const unsigned char *pub) {
  pge_t A;

  pge_import(ec, &A, pub);

  return pge_validate(ec, &A);
}

int
ecdh_pubkey_export(mont_t *ec,
                   unsigned char *x,
                   unsigned char *y,
                   const unsigned char *pub,
                   int sign) {
  prime_field_t *fe = &ec->fe;
  mge_t A;

  if (!mge_import(ec, &A, pub, sign))
    return 0;

  assert(!A.inf);

  fe_export(fe, x, A.x);
  fe_export(fe, y, A.y);

  return 1;
}

int
ecdh_pubkey_import(mont_t *ec,
                   unsigned char *out,
                   const unsigned char *x,
                   size_t x_len) {
  prime_field_t *fe = &ec->fe;
  unsigned char xp[MAX_FIELD_SIZE];
  pge_t A;

  while (x_len > 0 && x[x_len - 1] == 0x00)
    x_len -= 1;

  if (x_len > fe->size)
    return 0;

  memcpy(xp, x, x_len);
  memset(xp + x_len, 0x00, fe->size - x_len);

  pge_import(ec, &A, xp);

  if (!pge_validate(ec, &A))
    return 0;

  return pge_export(ec, out, &A);
}

int
ecdh_pubkey_is_small(mont_t *ec, const unsigned char *pub) {
  pge_t A;

  pge_import(ec, &A, pub);

  if (!pge_validate(ec, &A))
    return 0;

  return pge_is_small(ec, &A);
}

int
ecdh_pubkey_has_torsion(mont_t *ec, const unsigned char *pub) {
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  pge_t A;
  int zero;

  pge_import(ec, &A, pub);

  if (!pge_validate(ec, &A))
    return 0;

  zero = fe_is_zero(fe, A.x);

  mont_mul(ec, &A, &A, sc->n);

  return (pge_is_zero(ec, &A) ^ 1) | zero;
}

int
ecdh_derive(mont_t *ec,
            unsigned char *secret,
            const unsigned char *pub,
            const unsigned char *priv) {
  unsigned char clamped[MAX_SCALAR_SIZE];
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  pge_t A, P;
  int ret = 0;

  mont_clamp(ec, clamped, priv);

  sc_import(sc, a, clamped);

  pge_import(ec, &A, pub);

  mont_mul(ec, &P, &A, a);

  ret = pge_export(ec, secret, &P);

  cleanse(clamped, sizeof(clamped));
  sc_cleanse(sc, a);
  pge_cleanse(ec, &A);
  pge_cleanse(ec, &P);

  return ret;
}

/*
 * EdDSA
 */

edwards_t *
eddsa_context_create(const char *id) {
  edwards_t *ec = NULL;
  const edwards_def_t *def = NULL;
  size_t i;

  if (id == NULL)
    return NULL;

  for (i = 0; i < ARRAY_SIZE(edwards_curves); i++) {
    if (strcmp(edwards_curves[i]->id, id) == 0) {
      def = edwards_curves[i];
      break;
    }
  }

  if (def == NULL)
    return NULL;

  ec = malloc(sizeof(edwards_t));

  if (ec == NULL)
    return NULL;

  edwards_init(ec, def);

  return ec;
}

void
eddsa_context_destroy(edwards_t *ec) {
  if (ec != NULL)
    free(ec);
}

void
eddsa_context_randomize(edwards_t *ec, const unsigned char *entropy) {
  edwards_randomize(ec, entropy);
}

edwards_scratch_t *
eddsa_scratch_create(edwards_t *ec) {
  return malloc(sizeof(edwards_scratch_t));
}

void
eddsa_scratch_destroy(edwards_t *ec, edwards_scratch_t *scratch) {
  if (scratch != NULL)
    free(scratch);
}

size_t
eddsa_scalar_size(edwards_t *ec) {
  return ec->sc.size;
}

size_t
eddsa_scalar_bits(edwards_t *ec) {
  return ec->sc.bits;
}

size_t
eddsa_field_size(edwards_t *ec) {
  return ec->fe.size;
}

size_t
eddsa_field_bits(edwards_t *ec) {
  return ec->fe.bits;
}

size_t
eddsa_privkey_size(edwards_t *ec) {
  return ec->fe.adj_size;
}

size_t
eddsa_pubkey_size(edwards_t *ec) {
  return ec->fe.adj_size;
}

size_t
eddsa_sig_size(edwards_t *ec) {
  return ec->fe.adj_size * 2;
}

static void
eddsa_privkey_hash(edwards_t *ec,
                   unsigned char *out,
                   const unsigned char *priv) {
  prime_field_t *fe = &ec->fe;
  hash_t hash;

  hash_init(&hash, ec->hash);
  hash_update(&hash, priv, fe->adj_size);
  hash_final(&hash, out, fe->adj_size * 2);

  edwards_clamp(ec, out, out);
}

void
eddsa_privkey_generate(edwards_t *ec,
                       unsigned char *out,
                       const unsigned char *entropy) {
  prime_field_t *fe = &ec->fe;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  drbg_generate(&rng, out, fe->adj_size);

  cleanse(&rng, sizeof(rng));
}

void
eddsa_scalar_generate(edwards_t *ec,
                      unsigned char *out,
                      const unsigned char *entropy) {
  scalar_field_t *sc = &ec->sc;
  drbg_t rng;

  drbg_init(&rng, HASH_SHA256, entropy, 32);

  drbg_generate(&rng, out, sc->size);

  edwards_clamp(ec, out, out);

  cleanse(&rng, sizeof(rng));
}

void
eddsa_privkey_expand(edwards_t *ec,
                     unsigned char *scalar,
                     unsigned char *prefix,
                     const unsigned char *priv) {
  unsigned char bytes[(MAX_FIELD_SIZE + 1) * 2];
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;

  eddsa_privkey_hash(ec, bytes, priv);

  memcpy(scalar, bytes, sc->size);
  memcpy(prefix, bytes + fe->adj_size, fe->adj_size);

  cleanse(bytes, sizeof(bytes));
}

void
eddsa_privkey_convert(edwards_t *ec,
                      unsigned char *scalar,
                      const unsigned char *priv) {
  unsigned char bytes[(MAX_FIELD_SIZE + 1) * 2];
  scalar_field_t *sc = &ec->sc;

  eddsa_privkey_hash(ec, bytes, priv);

  memcpy(scalar, bytes, sc->size);

  cleanse(bytes, sizeof(bytes));
}

int
eddsa_privkey_verify(edwards_t *ec, const unsigned char *priv) {
  return 1;
}

int
eddsa_privkey_export(edwards_t *ec,
                     unsigned char *out,
                     const unsigned char *priv) {
  memcpy(out, priv, ec->fe.adj_size);
  return 1;
}

int
eddsa_privkey_import(edwards_t *ec,
                     unsigned char *out,
                     const unsigned char *bytes,
                     size_t len) {
  if (len != ec->fe.adj_size)
    return 0;

  memcpy(out, bytes, ec->fe.adj_size);

  return 1;
}

int
eddsa_scalar_verify(edwards_t *ec, const unsigned char *scalar) {
  return 1;
}

int
eddsa_scalar_is_zero(edwards_t *ec, const unsigned char *scalar) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  int ret;

  sc_import_reduce(sc, a, scalar);

  ret = sc_is_zero(sc, a);

  sc_cleanse(sc, a);

  return ret;
}

void
eddsa_scalar_clamp(edwards_t *ec,
                   unsigned char *out,
                   const unsigned char *scalar) {
  edwards_clamp(ec, out, scalar);
}

void
eddsa_scalar_tweak_add(edwards_t *ec,
                       unsigned char *out,
                       const unsigned char *scalar,
                       const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  sc_t a, t;

  sc_import_reduce(sc, a, scalar);
  sc_import_reduce(sc, t, tweak);
  sc_add(sc, a, a, t);
  sc_export(sc, out, a);
  sc_cleanse(sc, a);
}

void
eddsa_scalar_tweak_mul(edwards_t *ec,
                       unsigned char *out,
                       const unsigned char *scalar,
                       const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  sc_t a, t;

  sc_import_reduce(sc, a, scalar);
  sc_import_reduce(sc, t, tweak);
  sc_mul(sc, a, a, t);
  sc_export(sc, out, a);
  sc_cleanse(sc, a);
}

void
eddsa_scalar_reduce(edwards_t *ec,
                    unsigned char *out,
                    const unsigned char *bytes,
                    size_t len) {
  scalar_field_t *sc = &ec->sc;
  unsigned char scalar[MAX_SCALAR_SIZE];
  sc_t a;

  if (len > sc->size)
    len = sc->size;

  memcpy(scalar, bytes, len);
  memset(scalar + len, 0x00, sc->size - len);

  sc_import_reduce(sc, a, scalar);
  sc_export(sc, out, a);
  sc_cleanse(sc, a);

  cleanse(scalar, sizeof(scalar));
}

void
eddsa_scalar_negate(edwards_t *ec,
                    unsigned char *out,
                    const unsigned char *scalar) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;

  sc_import_reduce(sc, a, scalar);
  sc_neg(sc, a, a);
  sc_export(sc, out, a);
  sc_cleanse(sc, a);
}

void
eddsa_scalar_invert(edwards_t *ec,
                    unsigned char *out,
                    const unsigned char *scalar) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;

  sc_import_reduce(sc, a, scalar);
  sc_invert(sc, a, a);
  sc_export(sc, out, a);
  sc_cleanse(sc, a);
}

void
eddsa_pubkey_from_scalar(edwards_t *ec,
                         unsigned char *pub,
                         const unsigned char *scalar) {
  scalar_field_t *sc = &ec->sc;
  sc_t a;
  xge_t A;

  sc_import_reduce(sc, a, scalar);

  edwards_mul_g(ec, &A, a);

  xge_export(ec, pub, &A);

  sc_cleanse(sc, a);
  xge_cleanse(ec, &A);
}

void
eddsa_pubkey_create(edwards_t *ec,
                    unsigned char *pub,
                    const unsigned char *priv) {
  unsigned char scalar[MAX_SCALAR_SIZE];

  eddsa_privkey_convert(ec, scalar, priv);
  eddsa_pubkey_from_scalar(ec, pub, scalar);

  cleanse(scalar, sizeof(scalar));
}

int
eddsa_pubkey_convert(edwards_t *ec,
                     unsigned char *out,
                     const unsigned char *pub) {
  prime_field_t *fe = &ec->fe;
  xge_t A;
  mge_t p;

  if (!xge_import(ec, &A, pub))
    return 0;

  xge_to_mge(ec, &p, &A);

  if (p.inf)
    return 0;

  fe_export(fe, out, p.x);

  return 1;
}

void
eddsa_pubkey_from_uniform(edwards_t *ec,
                          unsigned char *out,
                          const unsigned char *bytes) {
  xge_t A;

  edwards_point_from_uniform(ec, &A, bytes);

  xge_export(ec, out, &A);
}

int
eddsa_pubkey_to_uniform(edwards_t *ec,
                        unsigned char *out,
                        const unsigned char *pub,
                        unsigned int hint) {
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  return edwards_point_to_uniform(ec, out, &A, hint);
}

void
eddsa_pubkey_from_hash(edwards_t *ec,
                       unsigned char *out,
                       const unsigned char *bytes,
                       int pake) {
  xge_t A;

  edwards_point_from_hash(ec, &A, bytes);

  if (pake)
    xge_mulh(ec, &A, &A);

  xge_export(ec, out, &A);
}

int
eddsa_pubkey_to_hash(edwards_t *ec,
                     unsigned char *out,
                     const unsigned char *pub,
                     const unsigned char *entropy) {
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  edwards_point_to_hash(ec, out, &A, entropy);

  return 1;
}

int
eddsa_pubkey_verify(edwards_t *ec, const unsigned char *pub) {
  xge_t A;
  return xge_import(ec, &A, pub);
}

int
eddsa_pubkey_export(edwards_t *ec,
                    unsigned char *x,
                    unsigned char *y,
                    const unsigned char *pub) {
  prime_field_t *fe = &ec->fe;
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  fe_export(fe, x, A.x);
  fe_export(fe, y, A.y);

  return 1;
}

int
eddsa_pubkey_import(edwards_t *ec,
                    unsigned char *out,
                    const unsigned char *x,
                    size_t x_len,
                    const unsigned char *y,
                    size_t y_len,
                    int sign) {
  prime_field_t *fe = &ec->fe;
  unsigned char xp[MAX_FIELD_SIZE];
  unsigned char yp[MAX_FIELD_SIZE];
  int has_x = (x_len > 0);
  int has_y = (y_len > 0);
  xge_t A;

  while (x_len > 0 && x[x_len - 1] == 0x00)
    x_len -= 1;

  while (y_len > 0 && y[y_len - 1] == 0x00)
    y_len -= 1;

  if (x_len > fe->size || y_len > fe->size)
    return 0;

  memcpy(xp, x, x_len);
  memset(xp + x_len, 0x00, fe->size - x_len);

  memcpy(yp, y, y_len);
  memset(yp + y_len, 0x00, fe->size - y_len);

  if (!fe_import(fe, A.x, xp))
    return 0;

  if (!fe_import(fe, A.y, yp))
    return 0;

  if (has_x && has_y) {
    xge_set_xy(ec, &A, A.x, A.y);

    if (!xge_validate(ec, &A))
      return 0;
  } else if (has_x) {
    if (!xge_set_x(ec, &A, A.x, sign))
      return 0;
  } else if (has_y) {
    if (!xge_set_y(ec, &A, A.y, sign))
      return 0;
  }

  xge_export(ec, out, &A);

  return 1;
}

int
eddsa_pubkey_is_infinity(edwards_t *ec, const unsigned char *pub) {
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  return xge_is_zero(ec, &A);
}

int
eddsa_pubkey_is_small(edwards_t *ec, const unsigned char *pub) {
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  return xge_is_small(ec, &A);
}

int
eddsa_pubkey_has_torsion(edwards_t *ec, const unsigned char *pub) {
  scalar_field_t *sc = &ec->sc;
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  edwards_mul(ec, &A, &A, sc->n);

  return xge_is_zero(ec, &A) ^ 1;
}

int
eddsa_pubkey_tweak_add(edwards_t *ec,
                       unsigned char *out,
                       const unsigned char *pub,
                       const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  xge_t A, T;
  sc_t t;

  if (!xge_import(ec, &A, pub))
    return 0;

  sc_import_reduce(sc, t, tweak);

  edwards_mul_g(ec, &T, t);

  xge_add(ec, &A, &A, &T);
  xge_export(ec, out, &A);

  sc_cleanse(sc, t);

  return 1;
}

int
eddsa_pubkey_tweak_mul(edwards_t *ec,
                       unsigned char *out,
                       const unsigned char *pub,
                       const unsigned char *tweak) {
  scalar_field_t *sc = &ec->sc;
  xge_t A;
  sc_t t;

  if (!xge_import(ec, &A, pub))
    return 0;

  sc_import_reduce(sc, t, tweak);

  edwards_mul(ec, &A, &A, t);

  xge_export(ec, out, &A);

  sc_cleanse(sc, t);

  return 1;
}

int
eddsa_pubkey_combine(edwards_t *ec,
                     unsigned char *out,
                     const unsigned char **pubs,
                     size_t len) {
  xge_t P, A;
  size_t i;

  xge_zero(ec, &P);

  for (i = 0; i < len; i++) {
    if (!xge_import(ec, &A, pubs[i]))
      return 0;

    xge_add(ec, &P, &P, &A);
  }

  xge_export(ec, out, &P);

  return 1;
}

int
eddsa_pubkey_negate(edwards_t *ec,
                    unsigned char *out,
                    const unsigned char *pub) {
  xge_t A;

  if (!xge_import(ec, &A, pub))
    return 0;

  xge_neg(ec, &A, &A);
  xge_export(ec, out, &A);

  return 1;
}

static void
eddsa_hash_init(edwards_t *ec,
                hash_t *hash,
                int ph,
                const unsigned char *ctx,
                size_t ctx_len) {
  hash_init(hash, ec->hash);

  if (ctx_len > 255)
    ctx_len = 255;

  if (ec->context || ph != -1 || ctx_len > 0) {
    uint8_t prehash = (ph > 0);
    uint8_t length = ctx_len;

    if (ec->prefix != NULL)
      hash_update(hash, ec->prefix, strlen(ec->prefix));

    hash_update(hash, &prehash, sizeof(prehash));
    hash_update(hash, &length, sizeof(length));
    hash_update(hash, ctx, ctx_len);
  }
}

static void
eddsa_hash_final(edwards_t *ec, sc_t r, hash_t *hash) {
  unsigned char bytes[(MAX_FIELD_SIZE + 1) * 2];
  mp_limb_t k[MAX_REDUCE_LIMBS];
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;

  hash_final(hash, bytes, fe->adj_size * 2);

  mpn_import(k, ARRAY_SIZE(k), bytes, fe->adj_size * 2, sc->endian);

  sc_reduce(sc, r, k);

  cleanse(bytes, sizeof(bytes));
  mpn_cleanse(k, ARRAY_SIZE(k));
}

static void
eddsa_hash_am(edwards_t *ec,
              sc_t r,
              int ph,
              const unsigned char *ctx,
              size_t ctx_len,
              const unsigned char *prefix,
              const unsigned char *msg,
              size_t msg_len) {
  prime_field_t *fe = &ec->fe;
  hash_t hash;

  eddsa_hash_init(ec, &hash, ph, ctx, ctx_len);

  hash_update(&hash, prefix, fe->adj_size);
  hash_update(&hash, msg, msg_len);

  eddsa_hash_final(ec, r, &hash);
}

static void
eddsa_hash_ram(edwards_t *ec,
               sc_t r,
               int ph,
               const unsigned char *ctx,
               size_t ctx_len,
               const unsigned char *R,
               const unsigned char *A,
               const unsigned char *msg,
               size_t msg_len) {
  prime_field_t *fe = &ec->fe;
  hash_t hash;

  eddsa_hash_init(ec, &hash, ph, ctx, ctx_len);

  hash_update(&hash, R, fe->adj_size);
  hash_update(&hash, A, fe->adj_size);
  hash_update(&hash, msg, msg_len);

  eddsa_hash_final(ec, r, &hash);
}

void
eddsa_sign_with_scalar(edwards_t *ec,
                       unsigned char *sig,
                       const unsigned char *msg,
                       size_t msg_len,
                       const unsigned char *scalar,
                       const unsigned char *prefix,
                       int ph,
                       const unsigned char *ctx,
                       size_t ctx_len) {
  /* EdDSA Signing.
   *
   * [EDDSA] Page 12, Section 4.
   * [RFC8032] Page 8, Section 3.3.
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a byte array of arbitrary size.
   *   - Let `a` be a secret scalar.
   *   - Let `w` be a secret byte array.
   *
   * Computation:
   *
   *   k = H(w, m) mod n
   *   R = G * k
   *   A = G * a
   *   e = H(R, A, m) mod n
   *   s = (k + e * a) mod n
   *   S = (R, s)
   *
   * Note that `k` must remain secret,
   * otherwise an attacker can compute:
   *
   *   a = (s - k) / e mod n
   *
   * The same is true of `w` as `k`
   * can be re-derived as `H(w, m)`.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  unsigned char *Rraw = sig;
  unsigned char *sraw = sig + fe->adj_size;
  unsigned char Araw[MAX_FIELD_SIZE + 1];
  sc_t k, a, e, s;
  xge_t R, A;

  eddsa_hash_am(ec, k, ph, ctx, ctx_len, prefix, msg, msg_len);

  edwards_mul_g(ec, &R, k);
  xge_export(ec, Rraw, &R);

  sc_import_reduce(sc, a, scalar);

  edwards_mul_g(ec, &A, a);
  xge_export(ec, Araw, &A);

  eddsa_hash_ram(ec, e, ph, ctx, ctx_len, Rraw, Araw, msg, msg_len);

  sc_mul(sc, s, e, a);
  sc_add(sc, s, s, k);
  sc_export(sc, sraw, s);

  if ((fe->bits & 7) == 0)
    sraw[fe->size] = 0;

  cleanse(Araw, sizeof(Araw));
  sc_cleanse(sc, k);
  sc_cleanse(sc, a);
  sc_cleanse(sc, e);
  sc_cleanse(sc, s);
  xge_cleanse(ec, &R);
  xge_cleanse(ec, &A);
}

void
eddsa_sign(edwards_t *ec,
           unsigned char *sig,
           const unsigned char *msg,
           size_t msg_len,
           const unsigned char *priv,
           int ph,
           const unsigned char *ctx,
           size_t ctx_len) {
  unsigned char scalar[MAX_SCALAR_SIZE];
  unsigned char prefix[MAX_FIELD_SIZE + 1];

  eddsa_privkey_expand(ec, scalar, prefix, priv);

  eddsa_sign_with_scalar(ec, sig, msg, msg_len,
                         scalar, prefix,
                         ph, ctx, ctx_len);

  cleanse(scalar, sizeof(scalar));
  cleanse(prefix, sizeof(prefix));
}

void
eddsa_sign_tweak_add(edwards_t *ec,
                     unsigned char *sig,
                     const unsigned char *msg,
                     size_t msg_len,
                     const unsigned char *priv,
                     const unsigned char *tweak,
                     int ph,
                     const unsigned char *ctx,
                     size_t ctx_len) {
  unsigned char scalar[MAX_SCALAR_SIZE];
  unsigned char prefix[MAX_FIELD_SIZE + 1];
  hash_t hash;

  eddsa_privkey_expand(ec, scalar, prefix, priv);
  eddsa_scalar_tweak_add(ec, scalar, scalar, tweak);

  assert(MAX_FIELD_SIZE + 1 >= HASH_MAX_OUTPUT_SIZE);

  hash_init(&hash, ec->hash);
  hash_update(&hash, prefix, ec->fe.adj_size);
  hash_update(&hash, tweak, ec->sc.size);
  hash_final(&hash, prefix, ec->fe.adj_size);

  eddsa_sign_with_scalar(ec, sig, msg, msg_len,
                         scalar, prefix,
                         ph, ctx, ctx_len);

  cleanse(scalar, sizeof(scalar));
  cleanse(prefix, sizeof(prefix));
}

void
eddsa_sign_tweak_mul(edwards_t *ec,
                     unsigned char *sig,
                     const unsigned char *msg,
                     size_t msg_len,
                     const unsigned char *priv,
                     const unsigned char *tweak,
                     int ph,
                     const unsigned char *ctx,
                     size_t ctx_len) {
  unsigned char scalar[MAX_SCALAR_SIZE];
  unsigned char prefix[MAX_FIELD_SIZE + 1];
  hash_t hash;

  eddsa_privkey_expand(ec, scalar, prefix, priv);
  eddsa_scalar_tweak_mul(ec, scalar, scalar, tweak);

  assert(MAX_FIELD_SIZE + 1 >= HASH_MAX_OUTPUT_SIZE);

  hash_init(&hash, ec->hash);
  hash_update(&hash, prefix, ec->fe.adj_size);
  hash_update(&hash, tweak, ec->sc.size);
  hash_final(&hash, prefix, ec->fe.adj_size);

  eddsa_sign_with_scalar(ec, sig, msg, msg_len,
                         scalar, prefix,
                         ph, ctx, ctx_len);

  cleanse(scalar, sizeof(scalar));
  cleanse(prefix, sizeof(prefix));
}

int
eddsa_verify(edwards_t *ec,
             const unsigned char *msg,
             size_t msg_len,
             const unsigned char *sig,
             const unsigned char *pub,
             int ph,
             const unsigned char *ctx,
             size_t ctx_len) {
  /* EdDSA Verification.
   *
   * [EDDSA] Page 15, Section 5.
   * [RFC8032] Page 8, Section 3.4.
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a byte array of arbitrary size.
   *   - Let `R` and `s` be signature elements.
   *   - Let `A` be a valid group element.
   *   - s < n.
   *
   * Computation:
   *
   *   e = H(R, A, m) mod n
   *   G * s == R + A * e
   *
   * Alternatively, we can compute:
   *
   *   R == G * s - A * e
   *
   * This allows us to make use of a
   * multi-exponentiation algorithm.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  const unsigned char *Rraw = sig;
  const unsigned char *sraw = sig + fe->adj_size;
  xge_t R, A, Re;
  sc_t s, e;

  if (!xge_import(ec, &R, Rraw))
    return 0;

  if (!xge_import(ec, &A, pub))
    return 0;

  if (!sc_import(sc, s, sraw))
    return 0;

  if ((fe->bits & 7) == 0) {
    if (sraw[fe->size] != 0)
      return 0;
  }

  eddsa_hash_ram(ec, e, ph, ctx, ctx_len, Rraw, pub, msg, msg_len);

  xge_neg(ec, &A, &A);

  edwards_mul_double_var(ec, &Re, s, &A, e);

  return xge_equal(ec, &R, &Re);
}

int
eddsa_verify_single(edwards_t *ec,
                    const unsigned char *msg,
                    size_t msg_len,
                    const unsigned char *sig,
                    const unsigned char *pub,
                    int ph,
                    const unsigned char *ctx,
                    size_t ctx_len) {
  /* EdDSA Verification (with cofactor multiplication).
   *
   * [EDDSA] Page 15, Section 5.
   * [RFC8032] Page 8, Section 3.4.
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `m` be a byte array of arbitrary size.
   *   - Let `R` and `s` be signature elements.
   *   - Let `A` be a valid group element.
   *   - s < n.
   *
   * Computation:
   *
   *   e = H(R, A, m) mod n
   *   (G * s) * h == (R + A * e) * h
   *
   * Alternatively, we can compute:
   *
   *   R * h == G * (s * h) - (A * h) * e
   *
   * This allows us to make use of a
   * multi-exponentiation algorithm.
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  const unsigned char *Rraw = sig;
  const unsigned char *sraw = sig + fe->adj_size;
  xge_t R, A, Re;
  sc_t s, e;

  if (!xge_import(ec, &R, Rraw))
    return 0;

  if (!xge_import(ec, &A, pub))
    return 0;

  if (!sc_import(sc, s, sraw))
    return 0;

  if ((fe->bits & 7) == 0) {
    if (sraw[fe->size] != 0)
      return 0;
  }

  eddsa_hash_ram(ec, e, ph, ctx, ctx_len, Rraw, pub, msg, msg_len);

  sc_mul_word(sc, s, s, ec->h);
  xge_mulh(ec, &A, &A);
  xge_mulh(ec, &R, &R);

  xge_neg(ec, &A, &A);

  edwards_mul_double_var(ec, &Re, s, &A, e);

  return xge_equal(ec, &R, &Re);
}

int
eddsa_verify_batch(edwards_t *ec,
                   const unsigned char **msgs,
                   size_t *msg_lens,
                   const unsigned char **sigs,
                   const unsigned char **pubs,
                   size_t len,
                   int ph,
                   const unsigned char *ctx,
                   size_t ctx_len,
                   edwards_scratch_t *scratch) {
  /* EdDSA Batch Verification.
   *
   * [EDDSA] Page 16, Section 5.
   *
   * Assumptions:
   *
   *   - Let `H` be a cryptographic hash function.
   *   - Let `R` and `s` be signature elements.
   *   - Let `A` be a valid group element.
   *   - Let `i` be the batch item index.
   *   - s < n.
   *   - a1 = 1 mod n.
   *
   * Computation:
   *
   *   ei = H(Ri, Ai, mi) mod n
   *   ai = random integer in [1,n-1]
   *   lhs = (si * ai + ...) * h mod n
   *   rhs = (Ri * h) * ai + (Ai * h) * (ei * ai mod n) + ...
   *   G * -lhs + rhs == O
   */
  prime_field_t *fe = &ec->fe;
  scalar_field_t *sc = &ec->sc;
  xge_t *points = scratch->points;
  sc_t *coeffs = scratch->coeffs;
  drbg_t rng;
  xge_t R, A;
  sc_t sum, s, e, a;
  size_t j = 0;
  size_t i;

  /* Seed RNG. */
  {
    unsigned char bytes[64];
    hash_t outer, inner;

    hash_init(&outer, HASH_SHA512);

    for (i = 0; i < len; i++) {
      const unsigned char *msg = msgs[i];
      size_t msg_len = msg_lens[i];
      const unsigned char *sig = sigs[i];
      const unsigned char *pub = pubs[i];

      hash_init(&inner, HASH_SHA256);
      hash_update(&inner, msg, msg_len);
      hash_final(&inner, bytes, 32);

      hash_update(&outer, bytes, 32);
      hash_update(&outer, sig, fe->adj_size * 2);
      hash_update(&outer, pub, fe->adj_size);
    }

    hash_final(&outer, bytes, 64);

    drbg_init(&rng, HASH_SHA256, bytes, 64);
  }

  /* Intialize sum. */
  sc_zero(sc, sum);

  /* Verify signatures. */
  for (i = 0; i < len; i++) {
    const unsigned char *msg = msgs[i];
    size_t msg_len = msg_lens[i];
    const unsigned char *sig = sigs[i];
    const unsigned char *pub = pubs[i];
    const unsigned char *Rraw = sig;
    const unsigned char *sraw = sig + fe->adj_size;

    if (!xge_import(ec, &R, Rraw))
      return 0;

    if (!xge_import(ec, &A, pub))
      return 0;

    if (!sc_import(sc, s, sraw))
      return 0;

    if ((fe->bits & 7) == 0) {
      if (sraw[fe->size] != 0)
        return 0;
    }

    eddsa_hash_ram(ec, e, ph, ctx, ctx_len, Rraw, pub, msg, msg_len);

    if (j == 0) {
      sc_zero(sc, a);
      a[0] = 1;
    } else {
      sc_random(sc, a, &rng);
    }

    sc_mul(sc, e, e, a);
    sc_mul(sc, s, s, a);
    sc_add(sc, sum, sum, s);

    xge_mulh(ec, &R, &R);
    xge_mulh(ec, &A, &A);

    xge_set(ec, &points[j + 0], &R);
    xge_set(ec, &points[j + 1], &A);

    sc_set(sc, coeffs[j + 0], a);
    sc_set(sc, coeffs[j + 1], e);

    j += 2;

    if (j == 64) {
      sc_mul_word(sc, sum, sum, ec->h);
      sc_neg(sc, sum, sum);

      edwards_mul_multi_var(ec, &R, sum, points, coeffs, j, scratch);

      if (!xge_is_zero(ec, &R))
        return 0;

      sc_zero(sc, sum);

      j = 0;
    }
  }

  if (j > 0) {
    sc_mul_word(sc, sum, sum, ec->h);
    sc_neg(sc, sum, sum);

    edwards_mul_multi_var(ec, &R, sum, points, coeffs, j, scratch);

    if (!xge_is_zero(ec, &R))
      return 0;
  }

  return 1;
}

int
eddsa_derive_with_scalar(edwards_t *ec,
                         unsigned char *secret,
                         const unsigned char *pub,
                         const unsigned char *scalar) {
  scalar_field_t *sc = &ec->sc;
  unsigned char clamped[MAX_SCALAR_SIZE];
  sc_t a;
  xge_t A, P;
  int ret = 0;

  edwards_clamp(ec, clamped, scalar);

  sc_import(sc, a, clamped);

  if (!xge_import(ec, &A, pub))
    goto fail;

  edwards_mul(ec, &P, &A, a);

  if (xge_is_zero(ec, &P))
    goto fail;

  xge_export(ec, secret, &P);

  ret = 1;
fail:
  cleanse(clamped, sizeof(clamped));
  sc_cleanse(sc, a);
  xge_cleanse(ec, &A);
  xge_cleanse(ec, &P);
  return ret;
}

int
eddsa_derive(edwards_t *ec,
             unsigned char *secret,
             const unsigned char *pub,
             const unsigned char *priv) {
  unsigned char scalar[MAX_SCALAR_SIZE];
  int ret;

  eddsa_privkey_convert(ec, scalar, priv);

  ret = eddsa_derive_with_scalar(ec, secret, pub, scalar);

  cleanse(scalar, sizeof(scalar));

  return ret;
}
