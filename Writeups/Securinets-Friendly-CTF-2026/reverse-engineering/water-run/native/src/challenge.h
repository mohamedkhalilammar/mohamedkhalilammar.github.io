#ifndef CG_CHALLENGE_H
#define CG_CHALLENGE_H

#include <stddef.h>

#if defined(CG_EMBEDDED)
#define CG_API
#elif defined(_WIN32)
#define CG_API __declspec(dllexport)
#else
#define CG_API __attribute__((visibility("default")))
#endif

#define CG_SCORE_CLAMP 99999
#define CG_TOKEN_MIN (CG_SCORE_CLAMP + 1)
#define CG_NONCE_MAX 32
#define CG_TOKEN_LEN 25
#define CG_BOTTLE_BANK 100
#define CG_BOTTLES_MAX 6
#define CG_GATE_SCORE 8000
#define CG_REWARD_MAX 96

#define CG_OK 0
#define CG_ERR_ARG (-1)
#define CG_ERR_NONCE (-2)
#define CG_ERR_STATE (-3)

typedef struct cg_ctx cg_ctx;

#ifdef __cplusplus
extern "C" {
#endif

CG_API cg_ctx *cg_create(unsigned int game_id);
CG_API void cg_destroy(cg_ctx *ctx);

CG_API int cg_set_nonce(cg_ctx *ctx, const char *nonce);
CG_API int cg_has_nonce(const cg_ctx *ctx);

CG_API void cg_add_score(cg_ctx *ctx, int delta);
CG_API int cg_get_score(const cg_ctx *ctx);

CG_API void cg_take_hit(cg_ctx *ctx);
CG_API void cg_collect_bottle(cg_ctx *ctx);
CG_API int cg_get_bottles(const cg_ctx *ctx);
CG_API int cg_is_caught(const cg_ctx *ctx);

CG_API void cg_reset_run(cg_ctx *ctx);

CG_API float cg_get_lift(const cg_ctx *ctx);
CG_API int cg_motion_mode(void);
CG_API void cg_motion(cg_ctx *ctx, int step);
CG_API int cg_phase_a(cg_ctx *ctx);
CG_API int cg_phase_b(cg_ctx *ctx);
CG_API int cg_receipt(const cg_ctx *ctx, char *out, size_t out_len);

CG_API int cg_token(const cg_ctx *ctx, char *out, size_t out_len);

#ifdef CG_DESIGNER
CG_API void cg_designer_force_score(cg_ctx *ctx, int score);
CG_API void cg_designer_print_plan(const cg_ctx *ctx);
#endif

#ifdef __cplusplus
}
#endif

#endif
