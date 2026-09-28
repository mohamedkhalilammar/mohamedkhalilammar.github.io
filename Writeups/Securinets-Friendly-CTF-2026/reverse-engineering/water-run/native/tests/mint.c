#include <stdio.h>
#include <stdlib.h>

#include "challenge.h"

int main(int argc, char **argv)
{
    cg_ctx *ctx;
    char tok[CG_TOKEN_LEN];
    int rc;

    if (argc != 3) {
        fprintf(stderr, "usage: mint <game-id> <nonce>\n");
        return 2;
    }

    ctx = cg_create((unsigned int)strtoul(argv[1], NULL, 10));
    if (!ctx)
        return 2;

    if (cg_set_nonce(ctx, argv[2]) != CG_OK) {
        fprintf(stderr, "mint: bad nonce\n");
        cg_destroy(ctx);
        return 2;
    }

    cg_designer_force_score(ctx, CG_TOKEN_MIN);
    rc = cg_token(ctx, tok, sizeof(tok));
    if (rc == CG_OK)
        printf("%s\n", tok);
    else
        fprintf(stderr, "mint: no token (%d)\n", rc);

    cg_destroy(ctx);
    return rc == CG_OK ? 0 : 1;
}
