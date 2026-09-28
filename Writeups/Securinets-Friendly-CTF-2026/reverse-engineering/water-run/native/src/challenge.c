#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "challenge.h"
#include "gamekey.h"
#include "gamevault.h"
#include "sha256.h"

struct cg_ctx {
    int score;
    int bottles;
#ifdef CG_VELOCITY_MODE
    unsigned int game_id;
    unsigned int phase;
    float vertical;
    float peak;
#else
    float lift;
    unsigned int game_id;
    unsigned int phase;
#endif
    int nonce_set;
    char nonce[CG_NONCE_MAX + 1];
};

static void load_key(unsigned char out[CG_KEY_LEN])
{
    size_t i;
    for (i = 0; i < CG_KEY_LEN; i++)
        out[i] = (unsigned char)(CG_KEY_STORE[i] ^ CG_KEY_MASK[i % CG_KEY_MASK_LEN]);
}

static int nonce_char_ok(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-';
}

cg_ctx *cg_create(unsigned int game_id)
{
    cg_ctx *ctx = (cg_ctx *)calloc(1, sizeof(cg_ctx));
    if (!ctx)
        return NULL;
    ctx->game_id = game_id;
    ctx->bottles = CG_BOTTLES_MAX;
#ifndef CG_VELOCITY_MODE
    ctx->lift = 9.25347f;
#endif
    return ctx;
}

void cg_destroy(cg_ctx *ctx)
{
    if (!ctx)
        return;
    memset(ctx, 0, sizeof(*ctx));
    free(ctx);
}

int cg_set_nonce(cg_ctx *ctx, const char *nonce)
{
    size_t len, i;

    if (!ctx || !nonce)
        return CG_ERR_ARG;

    len = strlen(nonce);
    if (len == 0 || len > CG_NONCE_MAX)
        return CG_ERR_NONCE;

    for (i = 0; i < len; i++) {
        if (!nonce_char_ok(nonce[i]))
            return CG_ERR_NONCE;
    }

    memset(ctx->nonce, 0, sizeof(ctx->nonce));
    memcpy(ctx->nonce, nonce, len);
    ctx->nonce_set = 1;
    return CG_OK;
}

int cg_has_nonce(const cg_ctx *ctx)
{
    return (ctx && ctx->nonce_set) ? 1 : 0;
}

void cg_add_score(cg_ctx *ctx, int delta)
{
    long long next;

    if (!ctx)
        return;

    next = (long long)ctx->score + delta;
    if (next > CG_SCORE_CLAMP)
        next = CG_SCORE_CLAMP;
    if (next < 0)
        next = 0;
    ctx->score = (int)next;
}

int cg_get_score(const cg_ctx *ctx)
{
    return ctx ? ctx->score : 0;
}

void cg_take_hit(cg_ctx *ctx)
{
    if (!ctx || ctx->bottles <= 0)
        return;
    ctx->bottles--;
}

void cg_collect_bottle(cg_ctx *ctx)
{
    if (!ctx)
        return;
    if (ctx->bottles >= CG_BOTTLES_MAX)
        cg_add_score(ctx, CG_BOTTLE_BANK);
    else
        ctx->bottles++;
}

int cg_get_bottles(const cg_ctx *ctx)
{
    return ctx ? ctx->bottles : 0;
}

int cg_is_caught(const cg_ctx *ctx)
{
    return (ctx && ctx->bottles <= 0) ? 1 : 0;
}

void cg_reset_run(cg_ctx *ctx)
{
    if (!ctx)
        return;
    ctx->score = 0;
    ctx->bottles = CG_BOTTLES_MAX;
#ifdef CG_VELOCITY_MODE
    ctx->vertical = 0.0f;
    ctx->peak = 0.0f;
#else
    ctx->lift = 9.25347f;
#endif
    ctx->phase = 0;
}

float cg_get_lift(const cg_ctx *ctx)
{
#ifdef CG_VELOCITY_MODE
    cg_ctx *live = (cg_ctx *)ctx;
    if (live && live->vertical > live->peak)
        live->peak = live->vertical;
    return live ? live->vertical : 0.0f;
#else
    return ctx ? ctx->lift : 9.25347f;
#endif
}

int cg_motion_mode(void)
{
#ifdef CG_VELOCITY_MODE
    return 1;
#else
    return 0;
#endif
}

void cg_motion(cg_ctx *ctx, int step)
{
#ifdef CG_VELOCITY_MODE
    if (!ctx)
        return;
    if (ctx->vertical > ctx->peak)
        ctx->peak = ctx->vertical;
    if (step < 0)
        ctx->vertical = (float)(37u ^ 44u);
    else if (step == 0)
        ctx->vertical = 0.0f;
    else
        ctx->vertical -= 24.0f * ((float)step / 1000000.0f);
#else
    (void)ctx;
    (void)step;
#endif
}

int cg_phase_a(cg_ctx *ctx)
{
    if (!ctx || ctx->score < CG_GATE_SCORE)
        return CG_ERR_STATE;
#ifdef CG_VELOCITY_MODE
    if (ctx->vertical > ctx->peak)
        ctx->peak = ctx->vertical;
    if (ctx->peak < 12.5f)
        return CG_ERR_STATE;
#else
    if (ctx->lift < 12.5f)
        return CG_ERR_STATE;
#endif
    ctx->phase = 0x91d6a42bu ^ (unsigned int)CG_GATE_SCORE;
    return CG_OK;
}

int cg_phase_b(cg_ctx *ctx)
{
    unsigned int want;
    if (!ctx)
        return CG_ERR_ARG;
    want = 0x91d6a42bu ^ (unsigned int)CG_GATE_SCORE;
    if (ctx->phase != want)
        return CG_ERR_STATE;
    ctx->phase = (want << 7) | (want >> 25);
    ctx->phase ^= 0x6f23c18du;
    return CG_OK;
}

int cg_receipt(const cg_ctx *ctx, char *out, size_t out_len)
{
    unsigned char master[CG_KEY_LEN];
    unsigned char key[CG_SHA256_DIGEST];
    unsigned char stream[CG_SHA256_DIGEST];
    unsigned char cipher[CG_VAULT_LEN];
    unsigned char plain[CG_VAULT_LEN];
    unsigned char check[CG_SHA256_DIGEST];
    unsigned char material[24];
    unsigned char tagged[CG_VAULT_LEN + 10];
    unsigned int want, counter;
    size_t i, at;
    int bad = 0;

    if (!ctx || !out || out_len <= CG_VAULT_LEN)
        return CG_ERR_ARG;
    want = 0x91d6a42bu ^ (unsigned int)CG_GATE_SCORE;
    want = ((want << 7) | (want >> 25)) ^ 0x6f23c18du;
    if (ctx->phase != want || ctx->score < CG_GATE_SCORE)
        return CG_ERR_STATE;

    load_key(master);
    memcpy(material, "vault|", 6);
    memcpy(material + 6, CG_VAULT_S, 16);
    material[22] = 0;
    material[23] = 0;
    cg_hmac_sha256(master, sizeof(master), material, 22, key);

    for (i = 0; i < CG_VAULT_LEN; i++)
        cipher[i] = CG_VAULT_C[CG_VAULT_P[i]];

    memcpy(tagged, cipher, CG_VAULT_LEN);
    memcpy(tagged + CG_VAULT_LEN, "|8000|A7C3", 10);
    cg_hmac_sha256(key, sizeof(key), tagged, sizeof(tagged), check);
    for (i = 0; i < 16; i++)
        bad |= (int)(check[i] ^ CG_VAULT_T[i]);

    at = 0;
    for (counter = 0; at < CG_VAULT_LEN; counter++) {
        memcpy(material, CG_VAULT_S, 16);
        material[16] = (unsigned char)counter;
        material[17] = (unsigned char)(counter >> 8);
        material[18] = (unsigned char)(counter >> 16);
        material[19] = (unsigned char)(counter >> 24);
        cg_hmac_sha256(key, sizeof(key), material, 20, stream);
        for (i = 0; i < sizeof(stream) && at < CG_VAULT_LEN; i++, at++)
            plain[at] = (unsigned char)(cipher[at] ^ stream[i]);
    }
    if (bad != 0) {
        memset(master, 0, sizeof(master));
        memset(key, 0, sizeof(key));
        memset(stream, 0, sizeof(stream));
        memset(cipher, 0, sizeof(cipher));
        memset(plain, 0, sizeof(plain));
        memset(check, 0, sizeof(check));
        return CG_ERR_STATE;
    }
    memcpy(out, plain, CG_VAULT_LEN);
    out[CG_VAULT_LEN] = 0;
    memset(master, 0, sizeof(master));
    memset(key, 0, sizeof(key));
    memset(stream, 0, sizeof(stream));
    memset(cipher, 0, sizeof(cipher));
    memset(plain, 0, sizeof(plain));
    memset(check, 0, sizeof(check));
    memset(material, 0, sizeof(material));
    memset(tagged, 0, sizeof(tagged));
    return CG_OK;
}

int cg_token(const cg_ctx *ctx, char *out, size_t out_len)
{
    unsigned char key[CG_KEY_LEN];
    unsigned char mac[CG_SHA256_DIGEST];
    char msg[CG_NONCE_MAX + 64];
    static const char *digits = "0123456789ABCDEF";
    int n, group, i, w = 0;

    if (!ctx || !out || out_len < CG_TOKEN_LEN)
        return CG_ERR_ARG;
    if (!ctx->nonce_set)
        return CG_ERR_NONCE;
    if (ctx->score < CG_TOKEN_MIN)
        return CG_ERR_STATE;

    n = snprintf(msg, sizeof(msg), "cgv1|%u|%s|cleared", ctx->game_id, ctx->nonce);
    if (n <= 0 || (size_t)n >= sizeof(msg))
        return CG_ERR_ARG;

    load_key(key);
    if (cg_hmac_sha256(key, sizeof(key), (const unsigned char *)msg, (size_t)n, mac) != 0) {
        memset(key, 0, sizeof(key));
        return CG_ERR_ARG;
    }
    memset(key, 0, sizeof(key));

    for (group = 0; group < 5; group++) {
        if (group)
            out[w++] = '-';
        for (i = 0; i < 2; i++) {
            unsigned char b = mac[group * 2 + i];
            out[w++] = digits[b >> 4];
            out[w++] = digits[b & 15];
        }
    }
    out[w] = 0;

    memset(mac, 0, sizeof(mac));
    memset(msg, 0, sizeof(msg));
    return CG_OK;
}

#ifdef CG_DESIGNER
void cg_designer_force_score(cg_ctx *ctx, int score)
{
    if (!ctx)
        return;
    ctx->score = score;
}

void cg_designer_print_plan(const cg_ctx *ctx)
{
    char tok[CG_TOKEN_LEN];
    cg_ctx probe;

    if (!ctx)
        return;

    probe = *ctx;
    probe.score = CG_TOKEN_MIN;

    printf("game id        %u\n", ctx->game_id);
    printf("nonce          %s\n", ctx->nonce_set ? ctx->nonce : "(unset)");
    printf("score          %d\n", ctx->score);
    printf("bottles        %d\n", ctx->bottles);
    printf("clamp          %d\n", CG_SCORE_CLAMP);
    printf("token needs    %d\n", CG_TOKEN_MIN);
    if (cg_token(&probe, tok, sizeof(tok)) == CG_OK)
        printf("token at min   %s\n", tok);
    else
        printf("token at min   (needs a nonce)\n");
}
#endif
