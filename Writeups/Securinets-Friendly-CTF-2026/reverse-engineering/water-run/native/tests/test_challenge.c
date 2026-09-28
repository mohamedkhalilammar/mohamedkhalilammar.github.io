#include <stdio.h>
#include <string.h>

#include "challenge.h"
#include "sha256.h"

static int failures;
static int checks;

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) {
        failures++;
        printf("  FAIL  %s\n", what);
    }
}

static const char *hex(const unsigned char *b, size_t n)
{
    static char buf[129];
    static const char *d = "0123456789abcdef";
    size_t i;
    for (i = 0; i < n && i < 64; i++) {
        buf[i * 2] = d[b[i] >> 4];
        buf[i * 2 + 1] = d[b[i] & 15];
    }
    buf[i * 2] = 0;
    return buf;
}

static void test_sha256_vectors(void)
{
    unsigned char out[32];

    cg_sha256((const unsigned char *)"", 0, out);
    ok(strcmp(hex(out, 32),
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == 0,
       "sha256 of empty string");

    cg_sha256((const unsigned char *)"abc", 3, out);
    ok(strcmp(hex(out, 32),
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0,
       "sha256 of abc");

    cg_sha256((const unsigned char *)
              "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56, out);
    ok(strcmp(hex(out, 32),
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1") == 0,
       "sha256 of the 56-byte vector (spans a block boundary)");
}

static void test_hmac_vectors(void)
{
    unsigned char key[20], out[32];
    memset(key, 0x0b, sizeof(key));

    cg_hmac_sha256(key, sizeof(key), (const unsigned char *)"Hi There", 8, out);
    ok(strcmp(hex(out, 32),
              "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7") == 0,
       "rfc4231 hmac case 1");

    cg_hmac_sha256((const unsigned char *)"Jefe", 4,
                   (const unsigned char *)"what do ya want for nothing?", 28, out);
    ok(strcmp(hex(out, 32),
              "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843") == 0,
       "rfc4231 hmac case 2");
}

static void test_score_clamp(void)
{
    cg_ctx *c = cg_create(1);

    ok(cg_get_score(c) == 0, "a fresh run scores zero");

    cg_add_score(c, 50);
    cg_add_score(c, 25);
    ok(cg_get_score(c) == 75, "score accumulates");

    cg_add_score(c, -10);
    ok(cg_get_score(c) == 65, "a negative delta subtracts");

    cg_add_score(c, -1000);
    ok(cg_get_score(c) == 0, "score never goes below zero");

    cg_add_score(c, CG_SCORE_CLAMP + 10000);
    ok(cg_get_score(c) == CG_SCORE_CLAMP, "score clamps at the ceiling");

    cg_add_score(c, 1);
    ok(cg_get_score(c) == CG_SCORE_CLAMP, "playing on cannot pass the ceiling");

    cg_destroy(c);
}

static void test_bottles(void)
{
    cg_ctx *c = cg_create(1);
    int i;

    ok(cg_get_bottles(c) == CG_BOTTLES_MAX, "a run starts with a full six-pack");
    ok(cg_is_caught(c) == 0, "a full pack is not caught");

    cg_take_hit(c);
    ok(cg_get_bottles(c) == CG_BOTTLES_MAX - 1, "a hit costs one bottle");

    cg_collect_bottle(c);
    ok(cg_get_bottles(c) == CG_BOTTLES_MAX, "a collected bottle refills the pack");

    cg_collect_bottle(c);
    ok(cg_get_bottles(c) == CG_BOTTLES_MAX, "the pack never holds more than six");
    ok(cg_get_score(c) > 0, "a bottle collected on a full pack banks as score");

    for (i = 0; i < CG_BOTTLES_MAX; i++)
        cg_take_hit(c);
    ok(cg_get_bottles(c) == 0, "bottles bottom out at zero");
    ok(cg_is_caught(c) == 1, "an empty pack means caught");

    cg_take_hit(c);
    ok(cg_get_bottles(c) == 0, "hits after being caught do not underflow");

    cg_destroy(c);
}

static void test_reset_run(void)
{
    cg_ctx *c = cg_create(1);
    char a[CG_TOKEN_LEN], b[CG_TOKEN_LEN];

    cg_set_nonce(c, "AAAA-BBBB-CCCC");
    cg_add_score(c, 500);
    cg_take_hit(c);

    cg_reset_run(c);
    ok(cg_get_score(c) == 0, "reset clears the score");
    ok(cg_get_bottles(c) == CG_BOTTLES_MAX, "reset refills the pack");
    ok(cg_has_nonce(c) == 1, "reset keeps the nonce");

    ok(cg_set_nonce(c, "AAAA-BBBB-CCCC") == CG_OK, "the nonce can be re-entered");
    cg_add_score(c, 500);
    (void)a;
    (void)b;
    cg_destroy(c);
}

static void test_nonce_validation(void)
{
    cg_ctx *c = cg_create(1);
    char longone[CG_NONCE_MAX + 8];

    ok(cg_has_nonce(c) == 0, "a fresh context has no nonce");
    ok(cg_set_nonce(c, "3KQ7-9WTM-P2XA") == CG_OK, "a well-formed nonce is accepted");
    ok(cg_has_nonce(c) == 1, "the nonce registers");

    ok(cg_set_nonce(c, NULL) == CG_ERR_ARG, "a null nonce is rejected");
    ok(cg_set_nonce(c, "") == CG_ERR_NONCE, "an empty nonce is rejected");
    ok(cg_set_nonce(c, "lower-case") == CG_ERR_NONCE, "lowercase is rejected");
    ok(cg_set_nonce(c, "HAS SPACE") == CG_ERR_NONCE, "a space is rejected");
    ok(cg_set_nonce(c, "SEMI;COLON") == CG_ERR_NONCE, "punctuation is rejected");

    memset(longone, 'A', sizeof(longone) - 1);
    longone[sizeof(longone) - 1] = 0;
    ok(cg_set_nonce(c, longone) == CG_ERR_NONCE, "an over-long nonce is rejected");

    ok(cg_has_nonce(c) == 1, "a rejected nonce leaves the accepted one in place");

    cg_destroy(c);
}

static void test_token_gate(void)
{
    cg_ctx *c = cg_create(1);
    char tok[CG_TOKEN_LEN];

    cg_add_score(c, CG_SCORE_CLAMP);
    ok(cg_token(c, tok, sizeof(tok)) == CG_ERR_NONCE, "no nonce, no token");

    cg_set_nonce(c, "3KQ7-9WTM-P2XA");
    ok(cg_token(c, tok, sizeof(tok)) == CG_ERR_STATE,
       "a clamped-out score is still below the gate");

    ok(cg_token(c, tok, 4) == CG_ERR_ARG, "a short buffer is refused");
    ok(cg_token(c, NULL, sizeof(tok)) == CG_ERR_ARG, "a null buffer is refused");

    cg_destroy(c);
}

static void test_token_shape_and_binding(void)
{
    cg_ctx *a = cg_create(1);
    cg_ctx *b = cg_create(1);
    cg_ctx *g = cg_create(2);
    char ta[CG_TOKEN_LEN], tb[CG_TOKEN_LEN], tg[CG_TOKEN_LEN];
    size_t i;
    int dashes = 0;

    cg_set_nonce(a, "3KQ7-9WTM-P2XA");
    cg_set_nonce(b, "3KQ7-9WTM-P2XA");
    cg_set_nonce(g, "3KQ7-9WTM-P2XA");

    cg_designer_force_score(a, CG_TOKEN_MIN);
    cg_designer_force_score(b, CG_TOKEN_MIN);
    cg_designer_force_score(g, CG_TOKEN_MIN);

    ok(cg_token(a, ta, sizeof(ta)) == CG_OK, "a state above the clamp yields a token");
    ok(strlen(ta) == CG_TOKEN_LEN - 1, "the token is the documented length");
    ok(strstr(ta, "Securinets") == NULL, "the token is not the flag");

    for (i = 0; i < strlen(ta); i++) {
        if (ta[i] == '-')
            dashes++;
        else
            ok((ta[i] >= 'A' && ta[i] <= 'Z') || (ta[i] >= '0' && ta[i] <= '9'),
               "the token is uppercase alphanumeric with dashes");
    }
    ok(dashes == 4, "the token is grouped into five blocks");

    cg_token(b, tb, sizeof(tb));
    ok(strcmp(ta, tb) == 0, "the same nonce, game and score give the same token");

    cg_token(g, tg, sizeof(tg));
    ok(strcmp(ta, tg) != 0, "the token is bound to the game id");

    cg_set_nonce(b, "9ZZZ-9WTM-P2XA");
    cg_token(b, tb, sizeof(tb));
    ok(strcmp(ta, tb) != 0, "the token is bound to the nonce");

    cg_designer_force_score(b, CG_TOKEN_MIN + 50000);
    cg_set_nonce(b, "3KQ7-9WTM-P2XA");
    cg_token(b, tb, sizeof(tb));
    ok(strcmp(ta, tb) == 0,
       "the token does not vary with the score above the clamp, so the service verifies in O(1)");

    cg_destroy(a);
    cg_destroy(b);
    cg_destroy(g);
}

static void test_written_score_is_honoured(void)
{
    cg_ctx *c = cg_create(1);
    char tok[CG_TOKEN_LEN];
    int *scan = (int *)c;

    cg_set_nonce(c, "3KQ7-9WTM-P2XA");
    cg_add_score(c, 1234);

    ok(*scan == 1234, "the score is the first int in the struct, findable by an exact scan");

    *scan = CG_TOKEN_MIN + 500;
    ok(cg_get_score(c) == CG_TOKEN_MIN + 500, "a value written into memory is read back");
    ok(cg_token(c, tok, sizeof(tok)) == CG_OK, "a written score passes the gate");

    cg_destroy(c);
}

static void test_offline_gate(void)
{
    cg_ctx *c = cg_create(1);
    char out[CG_REWARD_MAX];
    int *score = (int *)c;
#ifdef CG_VELOCITY_MODE
    float *lift = (float *)((unsigned char *)c + sizeof(int) * 4);
#else
    float *lift = (float *)((unsigned char *)c + sizeof(int) * 2);
#endif

    *score = CG_GATE_SCORE;
    ok(cg_phase_a(c) == CG_ERR_STATE, "ordinary jump cannot arm the final passage");
    *lift = 16.0f;
    ok(cg_phase_a(c) == CG_OK, "a modified jump arms the final passage");
    *score += 37;
    ok(cg_receipt(c, out, sizeof(out)) == CG_ERR_STATE,
       "crossing without collecting does not open the payload");
    ok(cg_phase_b(c) == CG_OK, "the post-wall artifact completes the sequence");
    ok(cg_receipt(c, out, sizeof(out)) == CG_OK, "the completed sequence opens the payload");
    ok(strncmp(out, "Securinets{", 11) == 0, "the recovered payload has the flag shape");
    cg_destroy(c);
}

int main(void)
{
    printf("challenge library\n");
    test_sha256_vectors();
    test_hmac_vectors();
    test_score_clamp();
    test_bottles();
    test_reset_run();
    test_nonce_validation();
    test_token_gate();
    test_token_shape_and_binding();
    test_written_score_is_honoured();
    test_offline_gate();
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
