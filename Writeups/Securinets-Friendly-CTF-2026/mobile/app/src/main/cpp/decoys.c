#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#ifdef SX_DESIGNER
#include <stdlib.h>
#endif

#define SX_BUF 192
#define SX_PAY 128
#define SX_SCR (SX_BUF - SX_PAY)

typedef struct {
    uint8_t  raw[SX_BUF];
    uint8_t  fwd[256];
    uint8_t  rev[256];
    uint8_t  gin[256];
    uint32_t lane[16];
    uint32_t sched[32];
    uint32_t hash;
    uint32_t sum;
    uint32_t poly;
    uint32_t cpoly;
    uint32_t step;
    uint32_t vec[4];
    int      rlo;
    int      rln;
    int      wlo;
    int      wln;
    int      slo;
    int      sln;
    int      size;
    uint8_t  name[64];
    int      nlen;
    uint32_t shash;
    uint32_t ssum;
    uint32_t slane[16];
    uint8_t  sscr[SX_SCR];
} sx_state;

static uint32_t sx_rl(uint32_t v, int r) { return (v << r) | (v >> (32 - r)); }
static uint32_t sx_rr(uint32_t v, int r) { return (v >> r) | (v << (32 - r)); }
static uint8_t sx_bl(uint8_t v, int r) { return (uint8_t)((v << r) | (v >> (8 - r))); }
static uint8_t sx_br(uint8_t v, int r) { return (uint8_t)((v >> r) | (v << (8 - r))); }

static uint32_t sx_step(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static uint8_t sx_gf(sx_state *c, uint8_t a, uint8_t b) __attribute__((used, noinline));
static uint32_t sx_crc(sx_state *c, uint32_t crc, uint8_t x) __attribute__((used, noinline));

static uint8_t sx_gf(sx_state *c, uint8_t a, uint8_t b) {
    uint8_t p = 0;
    int i;
    for (i = 0; i < 8; i++) {
        if (b & 1u) p = (uint8_t)(p ^ a);
        uint8_t hi = (uint8_t)(a & 0x80u);
        a = (uint8_t)(a << 1);
        if (hi) a = (uint8_t)(a ^ (uint8_t)c->poly);
        b = (uint8_t)(b >> 1);
    }
    return p;
}

static uint32_t sx_crc(sx_state *c, uint32_t crc, uint8_t x) {
    int i;
    crc ^= (uint32_t)x << 24;
    for (i = 0; i < 8; i++) {
        crc = (crc & 0x80000000u) ? ((crc << 1) ^ c->cpoly) : (crc << 1);
    }
    return crc;
}

__attribute__((visibility("hidden")))
void sx_entry(uint8_t *out, const uint8_t *in, int len, uint32_t nonce, int mode);

static void relay_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t chain_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int link_label(sx_state *c) __attribute__((used, noinline));
static void sift_count(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sync_window(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fill_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t clamp_band(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int mix_state(sx_state *c) __attribute__((used, noinline));
static uint32_t coal_level(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int move_lease(sx_state *c) __attribute__((used, noinline));
static uint32_t tally_arena(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void latch_token(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t push_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pack_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int latch_entry(sx_state *c) __attribute__((used, noinline));
static uint8_t mark_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int fold_pairing(sx_state *c) __attribute__((used, noinline));
static int drain_page(sx_state *c) __attribute__((used, noinline));
static uint8_t chain_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int trace_marker(sx_state *c) __attribute__((used, noinline));
static int poll_digest(sx_state *c) __attribute__((used, noinline));
static void purge_entry(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t swap_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t reset_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int reap_row(sx_state *c) __attribute__((used, noinline));
static uint32_t poll_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int poll_range(sx_state *c) __attribute__((used, noinline));
static uint8_t flush_index(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t yield_offset(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void flush_path(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void rotate_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t close_token(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t shift_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t blend_slot(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t tap_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void poll_list(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t hold_bucket(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t pack_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t grow_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int fill_layer(sx_state *c) __attribute__((used, noinline));
static int tap_frame(sx_state *c) __attribute__((used, noinline));
static int drain_head(sx_state *c) __attribute__((used, noinline));
static void fill_port(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t pick_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pin_segment(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void poll_table(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t fold_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t pack_seat(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t reset_rate(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t latch_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t pair_group(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void join_unit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void coal_cursor(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void relay_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t seek_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void latch_group(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void grow_unit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t mix_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void settle_line(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int grow_run(sx_state *c) __attribute__((used, noinline));
static uint32_t flush_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void place_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void peek_view(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void parse_field(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t probe_field(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int cache_seat(sx_state *c) __attribute__((used, noinline));
static int move_lease_66(sx_state *c) __attribute__((used, noinline));
static uint32_t slice_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int trim_stack(sx_state *c) __attribute__((used, noinline));
static void poll_value(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sift_cell(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t trim_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t pair_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tune_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void scan_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t clamp_stream(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void push_line(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void blend_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void map_view(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t flush_record(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t link_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t parse_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int trim_run(sx_state *c) __attribute__((used, noinline));
static int probe_delta(sx_state *c) __attribute__((used, noinline));
static int merge_table(sx_state *c) __attribute__((used, noinline));
static void sort_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t poll_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t purge_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int trim_band(sx_state *c) __attribute__((used, noinline));
static uint8_t trim_group(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int rotate_pool(sx_state *c) __attribute__((used, noinline));
static void sift_item(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t close_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t purge_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t patch_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t place_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void scan_item(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int tap_token(sx_state *c) __attribute__((used, noinline));
static void relay_rate(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t push_range(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pick_cursor(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t fold_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void sift_token(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int sift_pool(sx_state *c) __attribute__((used, noinline));
static int shift_level(sx_state *c) __attribute__((used, noinline));
static void tune_port(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t shift_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void close_offset(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void relay_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int prime_region(sx_state *c) __attribute__((used, noinline));
static void hold_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int swap_band(sx_state *c) __attribute__((used, noinline));
static void merge_pool(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t drain_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void slice_stream(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t hold_window(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void coal_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void pack_frame(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void store_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void sync_window_119(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pick_item(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void fill_count(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t load_row(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t mark_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t relay_run_124(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t prime_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void step_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int latch_cell(sx_state *c) __attribute__((used, noinline));
static void sort_item(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t scan_queue(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int place_group(sx_state *c) __attribute__((used, noinline));
static uint32_t align_head(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t pick_page(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t scan_bucket(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t tune_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void resize_scope(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void tune_line(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t sync_seat(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t mix_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t slice_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t sort_bound(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t blend_head(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fold_mask(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void parse_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t blend_port(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void step_level(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t chain_unit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t scan_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t hold_tail(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t mark_delta(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void align_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t drain_head_151(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fill_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int pick_index(sx_state *c) __attribute__((used, noinline));
static int stage_scope(sx_state *c) __attribute__((used, noinline));
static uint32_t trim_level(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t merge_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int load_window(sx_state *c) __attribute__((used, noinline));
static uint8_t place_slot(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t merge_range(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int scan_path(sx_state *c) __attribute__((used, noinline));
static int slice_page(sx_state *c) __attribute__((used, noinline));
static uint8_t blend_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void link_index(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void tally_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void load_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void parse_run(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void seek_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int fill_layer_168(sx_state *c) __attribute__((used, noinline));
static void move_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t hold_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t pin_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t split_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t prime_mask(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_pool(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t yield_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t push_segment(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void sort_gap(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t move_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void push_stream(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t join_band(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t settle_frame(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t load_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int step_field(sx_state *c) __attribute__((used, noinline));
static int load_group(sx_state *c) __attribute__((used, noinline));
static int purge_line(sx_state *c) __attribute__((used, noinline));
static int sync_index(sx_state *c) __attribute__((used, noinline));
static int patch_table(sx_state *c) __attribute__((used, noinline));
static uint32_t load_run(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t merge_row(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void sort_limit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void fold_cell(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t sync_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void reset_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t resize_bucket(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t parse_gap(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int seek_seat(sx_state *c) __attribute__((used, noinline));
static uint32_t poll_queue(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int join_frame(sx_state *c) __attribute__((used, noinline));
static uint32_t push_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t chain_digest_201(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int merge_digest(sx_state *c) __attribute__((used, noinline));
static void settle_queue(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void resize_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sort_band(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void scan_window(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int flush_queue(sx_state *c) __attribute__((used, noinline));
static uint32_t fold_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t shift_mask(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_stream(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void align_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t settle_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tally_entry(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void wrap_tail(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t merge_page(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t trim_value(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void reap_range(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void sort_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void peek_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void mark_node(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void pair_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t merge_stream(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t close_line(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t resize_label(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void reap_level(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int load_node(sx_state *c) __attribute__((used, noinline));
static uint32_t hold_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t step_line(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t resize_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int link_unit(sx_state *c) __attribute__((used, noinline));
static uint32_t coal_tail(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void blend_range(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t shift_group_235(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void align_table(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void stage_arena(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void patch_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void fold_line(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t resize_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void store_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t defer_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void parse_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int relay_arena(sx_state *c) __attribute__((used, noinline));
static void parse_token(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int tune_rate(sx_state *c) __attribute__((used, noinline));
static void prime_part(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void probe_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void move_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int relay_range(sx_state *c) __attribute__((used, noinline));
static uint32_t reset_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t split_entry(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t mix_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void mark_page(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int place_path(sx_state *c) __attribute__((used, noinline));
static void tune_bucket(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t step_scope(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int drain_cell(sx_state *c) __attribute__((used, noinline));
static uint32_t wrap_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pair_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t emit_run(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t link_track(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pick_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int peek_mask(sx_state *c) __attribute__((used, noinline));
static uint32_t rotate_queue(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pair_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t merge_state(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void join_band_268(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t queue_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t place_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t fold_layer(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void align_cursor(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t place_field(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_store(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t cache_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int sync_range(sx_state *c) __attribute__((used, noinline));
static void merge_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t seek_tail(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int clamp_cell(sx_state *c) __attribute__((used, noinline));
static void tally_rate(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t parse_level(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t merge_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t pair_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t trim_key(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int pair_band(sx_state *c) __attribute__((used, noinline));
static int patch_table_287(sx_state *c) __attribute__((used, noinline));
static void purge_digest(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void coal_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t peek_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void relay_level(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void wrap_state(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t pack_pairing(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int tune_gap(sx_state *c) __attribute__((used, noinline));
static void tally_field(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t fold_window(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t place_offset(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_key_298(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t reset_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void coal_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t sift_stream(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t cache_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int merge_pairing(sx_state *c) __attribute__((used, noinline));
static uint8_t close_port(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t defer_table(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t cache_segment(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void yield_tail(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t slice_frame(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void store_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t cache_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void load_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t yield_segment(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t swap_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fetch_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int coal_head(sx_state *c) __attribute__((used, noinline));
static void align_entry(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t merge_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t poll_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t purge_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t parse_run_320(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void place_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void probe_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int rotate_region(sx_state *c) __attribute__((used, noinline));
static uint32_t prime_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void yield_window(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int tap_port(sx_state *c) __attribute__((used, noinline));
static uint32_t mark_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pair_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t mark_part(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t queue_run(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int pick_head(sx_state *c) __attribute__((used, noinline));
static void trim_delta(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t chain_run(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void mix_batch(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void prime_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t fetch_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t tally_arena_337(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int sift_region(sx_state *c) __attribute__((used, noinline));
static int move_store(sx_state *c) __attribute__((used, noinline));
static uint8_t latch_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void purge_pool(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int peek_bucket(sx_state *c) __attribute__((used, noinline));
static void clamp_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t queue_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t hold_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t join_segment(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void peek_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void trace_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void clamp_offset(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int wrap_bound(sx_state *c) __attribute__((used, noinline));
static uint32_t link_count(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int join_segment_353(sx_state *c) __attribute__((used, noinline));
static void drain_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t reset_page(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void grow_store(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void merge_state_357(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t relay_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t mark_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void load_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t trace_port(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void peek_page(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int parse_head(sx_state *c) __attribute__((used, noinline));
static void step_digest(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sync_segment(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t drain_table(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t flush_level(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t link_queue(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int latch_page(sx_state *c) __attribute__((used, noinline));
static void drain_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t clamp_frame(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t trim_item(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tally_tuple(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t mark_offset(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int settle_row(sx_state *c) __attribute__((used, noinline));
static uint8_t swap_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int probe_segment(sx_state *c) __attribute__((used, noinline));
static uint32_t push_group_378(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_band(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t close_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t close_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void close_delta(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int scan_arena(sx_state *c) __attribute__((used, noinline));
static void mark_rate(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t cache_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_line(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t move_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t hold_stack(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t shift_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int slice_pool(sx_state *c) __attribute__((used, noinline));
static uint8_t pack_cursor(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t map_band(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int load_item(sx_state *c) __attribute__((used, noinline));
static void mark_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pick_row(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t mark_unit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tune_cell(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t store_entry(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void fetch_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int link_group(sx_state *c) __attribute__((used, noinline));
static void patch_delta(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t defer_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t cache_chunk(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void sort_line(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fill_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sort_rate(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t queue_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void parse_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void relay_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t peek_pairing(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t queue_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int hold_item(sx_state *c) __attribute__((used, noinline));
static void trace_tail(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_rate(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t chain_record(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t flush_rate(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void emit_run_417(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int align_seat(sx_state *c) __attribute__((used, noinline));
static void sync_lease_419(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void push_gap(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void reap_cursor(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int blend_token_422(sx_state *c) __attribute__((used, noinline));
static void relay_range_423(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pair_gap(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t trim_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t swap_seat(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t split_cursor(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int shift_layer(sx_state *c) __attribute__((used, noinline));
static uint8_t blend_offset(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t parse_pool(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tune_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t latch_view(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t poll_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void prime_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t shift_record(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t chain_record_438(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t poll_frame(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fold_unit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t chain_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void reset_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t drain_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t shift_scope(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t clamp_tail(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t pick_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t sift_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_limit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void chain_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int prime_level(sx_state *c) __attribute__((used, noinline));
static void cache_band(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t emit_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trace_token(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t cache_segment_454(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t reset_delta_455(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int coal_node(sx_state *c) __attribute__((used, noinline));
static void flush_list(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int trace_layer(sx_state *c) __attribute__((used, noinline));
static uint32_t push_line_459(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void flush_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t reset_region(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void store_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tune_limit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t slice_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t drain_delta(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t mark_slot(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t settle_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void patch_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void load_page(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t step_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t reap_field(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t settle_row_472(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fold_path(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t load_row_474(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void latch_delta(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void blend_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t trim_pool_477(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void store_list(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void link_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void cache_stream(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t fill_stream(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void fold_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t flush_unit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void sort_key(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void map_index(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void split_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void chain_lease(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void join_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void swap_table(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t mark_offset_490(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void flush_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t defer_page(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void swap_track(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t split_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void trim_list(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pin_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void wrap_segment(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t close_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int fold_layer_499(sx_state *c) __attribute__((used, noinline));
static uint8_t swap_entry(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void step_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t reap_count(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void move_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int flush_item(sx_state *c) __attribute__((used, noinline));
static void latch_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void split_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t flush_path_507(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t prime_pool(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tune_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void settle_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t sift_record(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pin_list(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void close_span(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t pin_frame(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fill_frame(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int link_run(sx_state *c) __attribute__((used, noinline));
static void push_page(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void purge_tuple(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void defer_field(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t clamp_page(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t cache_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t resize_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void parse_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t poll_view(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t resize_span(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t wrap_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void resize_field(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t fold_part(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void swap_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void clamp_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void queue_offset(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sync_part(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pin_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void flush_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void place_tail(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t sift_entry_536(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int mark_cell(sx_state *c) __attribute__((used, noinline));
static void sort_pairing(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t map_pool(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int trim_queue(sx_state *c) __attribute__((used, noinline));
static uint32_t coal_unit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void trace_level(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void align_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t fold_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void load_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t reap_count_546(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t queue_layer(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int latch_row(sx_state *c) __attribute__((used, noinline));
static uint32_t trace_mask(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int cache_label(sx_state *c) __attribute__((used, noinline));
static int step_index(sx_state *c) __attribute__((used, noinline));
static void trim_lease(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void hold_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t grow_row(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int place_region(sx_state *c) __attribute__((used, noinline));
static uint8_t patch_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void slice_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void grow_queue(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sort_arena(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void mix_node_561(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int link_table(sx_state *c) __attribute__((used, noinline));
static void fold_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void peek_item(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void stage_key(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void mark_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int load_span_567(sx_state *c) __attribute__((used, noinline));
static uint32_t relay_digest_568(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t stage_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t load_list(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t fold_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void slice_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int yield_span(sx_state *c) __attribute__((used, noinline));
static void chain_count(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void chain_slot(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void cache_entry(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void wrap_store(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t push_stream_578(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t coal_tuple(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void reset_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t fill_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t relay_view(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t pick_level(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tally_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void chain_row(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t split_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void yield_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t fill_tail_588(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t sort_part(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void poll_unit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void chain_gap(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t swap_band_592(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int scan_row(sx_state *c) __attribute__((used, noinline));
static void coal_page(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fill_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void cache_batch(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int seek_rate(sx_state *c) __attribute__((used, noinline));
static int align_scope_598(sx_state *c) __attribute__((used, noinline));
static uint32_t clamp_scope(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void chain_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int align_level(sx_state *c) __attribute__((used, noinline));
static uint32_t push_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void sync_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t coal_digest_604(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t purge_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void close_region_606(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void split_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t blend_layer(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sync_state(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void sync_port(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int fold_tail(sx_state *c) __attribute__((used, noinline));
static uint32_t poll_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void merge_stream_613(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t wrap_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pin_level(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int queue_count(sx_state *c) __attribute__((used, noinline));
static int tune_seat(sx_state *c) __attribute__((used, noinline));
static void prime_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pick_band(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void peek_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t relay_region(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tally_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int merge_label(sx_state *c) __attribute__((used, noinline));
static void swap_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void tap_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void seek_record(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t cache_frame(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int step_value(sx_state *c) __attribute__((used, noinline));
static uint32_t emit_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int pack_marker(sx_state *c) __attribute__((used, noinline));
static uint32_t push_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void rotate_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void pick_level_633(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void sift_span(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t sort_tuple(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t defer_token_636(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t clamp_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t join_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void reap_group(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void hold_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t sift_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int rotate_gap(sx_state *c) __attribute__((used, noinline));
static void cache_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t clamp_marker(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tap_slot(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void probe_tuple(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fetch_lease(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void coal_table(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void yield_limit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t merge_delta(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void blend_bucket(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void flush_count(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void swap_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int swap_segment(sx_state *c) __attribute__((used, noinline));
static void step_marker(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t prime_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void chain_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t peek_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t emit_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int align_slot(sx_state *c) __attribute__((used, noinline));
static void align_record_661(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t join_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void trim_offset(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int fill_index(sx_state *c) __attribute__((used, noinline));
static int fill_cell(sx_state *c) __attribute__((used, noinline));
static uint32_t mark_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int trace_store(sx_state *c) __attribute__((used, noinline));
static uint32_t sort_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int pick_field(sx_state *c) __attribute__((used, noinline));
static void reap_cursor_670(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t split_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t align_group(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int slice_count(sx_state *c) __attribute__((used, noinline));
static uint8_t pair_store(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pair_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t poll_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t trace_segment(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t store_lease(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void cache_state(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t align_store(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int parse_item(sx_state *c) __attribute__((used, noinline));
static uint8_t peek_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t drain_part(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t latch_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t place_rate(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tap_page(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t trim_slot(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t join_band_688(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int emit_cursor(sx_state *c) __attribute__((used, noinline));
static int tally_label(sx_state *c) __attribute__((used, noinline));
static void grow_group(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t scan_item_692(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t sync_ring(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pick_mask(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int peek_store_695(sx_state *c) __attribute__((used, noinline));
static uint32_t reset_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void move_cell(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void scan_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t fold_seat_699(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t grow_row_700(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int queue_lease(sx_state *c) __attribute__((used, noinline));
static uint32_t tap_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void slice_stream_703(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int defer_window(sx_state *c) __attribute__((used, noinline));
static uint32_t slice_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t queue_band(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int swap_key(sx_state *c) __attribute__((used, noinline));
static int hold_line(sx_state *c) __attribute__((used, noinline));
static int sift_bucket(sx_state *c) __attribute__((used, noinline));
static void fold_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int clamp_tail_711(sx_state *c) __attribute__((used, noinline));
static void close_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t step_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t move_port(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t swap_scope(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t cache_run(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t settle_value(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t join_count(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t grow_unit_719(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void fill_scope(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t scan_key_721(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int clamp_head(sx_state *c) __attribute__((used, noinline));
static int queue_port(sx_state *c) __attribute__((used, noinline));
static void fetch_node(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void blend_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void clamp_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t yield_stream(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t split_track(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t stage_label(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void rotate_count(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int store_tail(sx_state *c) __attribute__((used, noinline));
static void push_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void store_queue(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int reset_batch(sx_state *c) __attribute__((used, noinline));
static int parse_gap_735(sx_state *c) __attribute__((used, noinline));
static uint32_t probe_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t close_group(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void drain_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void hold_track(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t grow_offset(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t pack_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t latch_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t close_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t sort_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void latch_tail(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t defer_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t split_record(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void flush_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t queue_path(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void trace_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void load_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t defer_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t chain_band(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t peek_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void defer_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t step_scope_756(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t poll_line(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void prime_stack_758(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t join_region(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t grow_segment(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void cache_node(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t latch_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t step_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_record_764(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t drain_cell_765(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t relay_arena_766(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void rotate_run_767(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));

static uint32_t step_field_768(uint32_t x) __attribute__((used, noinline));
static int mark_stream(sx_state *c) __attribute__((used, noinline));
static uint32_t emit_unit(uint32_t prime, uint32_t h, uint8_t x) __attribute__((used, noinline));
static uint32_t fold_cursor(sx_state *c, uint32_t nonce, int size) __attribute__((used, noinline));
static void drain_track_772(sx_state *c, uint32_t master) __attribute__((used, noinline));
static void hold_level(sx_state *c, uint32_t seed) __attribute__((used, noinline));
static void defer_chunk_774(sx_state *c) __attribute__((used, noinline));
static void drain_row_775(sx_state *c) __attribute__((used, noinline));
static void fold_tail_776(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void defer_bound(sx_state *c, uint32_t *out) __attribute__((used, noinline));
static void pin_view(sx_state *c, uint32_t master, int rnd, int lo, int ln, uint32_t *out) __attribute__((used, noinline));
static void pack_key(sx_state *c, const uint32_t *kk, uint32_t *out) __attribute__((used, noinline));
static void chain_list(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void flush_band(sx_state *c, const uint32_t *w, uint32_t *v) __attribute__((used, noinline));
static void join_head(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void emit_path(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void slice_track(uint32_t *s, int a, int b, int d, int e) __attribute__((used, noinline));
static void pin_node(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void pair_tuple(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void reap_stream(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void move_layer(sx_state *c, int idx, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void patch_stack(sx_state *c, uint32_t master, int rnd, int alo, int aln, int tlo, int tln, int s0, int s1, int fwd) __attribute__((used, noinline));


static void relay_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += seek_key(c, c->rlo, c->rln);
    pick_cursor(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 13756u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    flush_path(c, t0, t1);
    t0 ^= fold_group(c, t1);
    c->lane[6] ^= sx_rl(c->lane[3], 18);
    fill_port(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2b) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56323u) % (uint32_t)c->rln)] << 16;
    t2 += close_token(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x15) << 16;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 15);
    t2 += (uint32_t)trim_stack(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x90ab21a5u;
    sift_count(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20364u) % (uint32_t)c->rln)] << 0;
    t2 += (uint32_t)link_label(c);
    t2 += chain_digest(c, c->slo, c->sln);
    t0 ^= poll_index(c, t1);
    t1 ^= (uint32_t)drain_marker(c, (uint8_t)(t0 >> 16), t2);
    c->sched[1] = c->hash ^ sx_rl(c->lane[7], 2);
    pack_frame(c, t0, t1);
    t2 += (uint32_t)cache_seat(c);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t chain_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xb26e0c1bu) ^ sx_rr(c->hash, 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 54992u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[3] ^= sx_rl(c->lane[13], 12);
    poll_list(c, &c->lane[7], 1);
    latch_token(c, &c->lane[7], 3);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23361u) % (uint32_t)c->rln)] << 8;
    rotate_run(c, t0, t1);
    t2 += coal_level(c, c->slo, c->sln);
    t1 ^= (uint32_t)shift_cell(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= purge_slot(c, t1);
    t1 ^= (uint32_t)mark_scope(c, (uint8_t)(t0 >> 16), t2);
    c->lane[10] += c->lane[14]; c->lane[7] ^= c->lane[10]; c->lane[7] = sx_rl(c->lane[7], 12);
    peek_view(c, &c->lane[3], 4);
    t2 += (uint32_t)latch_cell(c);
    c->sum += t1;
    return t0 + t2;
}

static int link_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 59909u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t1 ^= (uint32_t)blend_slot(c, (uint8_t)(t0 >> 0), t2);
    t2 += pick_item(c, c->rlo, c->rln);
    t2 += (uint32_t)mix_state(c);
    c->sched[25] = c->hash ^ sx_rl(c->lane[3], 16);
    t2 += (uint32_t)fold_pairing(c);
    purge_entry(c, &c->lane[1], 2);
    c->sched[7] = c->hash ^ sx_rl(c->lane[10], 28);
    sync_window(c, &c->lane[10], 4);
    c->hash = (c->hash * 0x7cf46037u) ^ sx_rr(c->hash, 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 += (uint32_t)reap_row(c);
    t1 ^= (uint32_t)clamp_band(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= tally_arena(c, t1);
    t2 += (uint32_t)swap_band(c);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sift_count(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[25] = c->hash ^ sx_rl(c->lane[2], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcbf962e5u;
    t0 ^= fill_tail(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 ^= push_group(c, t1);
    coal_cursor(c, t0, t1);
    c->lane[1] += c->lane[0] ^ 0x9a4f4691u;
    t2 += (uint32_t)move_lease(c);
    c->lane[14] += c->lane[4] ^ 0x2b678cfdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14813u) % (uint32_t)c->rln)] << 24;
    t0 ^= pack_rate(c, t1);
    t2 += (uint32_t)poll_range(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 += (uint32_t)poll_digest(c);
    t1 ^= (uint32_t)trim_group(c, (uint8_t)(t0 >> 16), t2);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void sync_window(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x49) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sched[6] = c->hash ^ sx_rl(c->lane[6], 29);
    t0 ^= reset_limit(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe9) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4b) << 16;
    t2 += (uint32_t)drain_page(c);
    t2 += (uint32_t)reap_row(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x59) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xdf) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fill_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x8a95c4d9u) ^ sx_rr(c->hash, 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8b7586ebu;
    c->hash ^= c->lane[15] + 0x6da36195u;
    t2 += (uint32_t)poll_range(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xce) << 8;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x03) << 8;
    c->raw[c->slo + (int)((t0 + 60379u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t clamp_band(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[31] = c->hash ^ sx_rl(c->lane[2], 17);
    t1 ^= (uint32_t)pair_group(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 45922u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xdd59c005u;
    t2 += (uint32_t)merge_table(c);
    t0 ^= parse_tuple(c, t1);
    t2 += (uint32_t)trace_marker(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40591u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[2] + 0xecc25c79u;
    t1 ^= (uint32_t)chain_state(c, (uint8_t)(t0 >> 8), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int mix_state(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->raw[c->slo + (int)((t0 + 40707u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6647u) % (uint32_t)c->rln)] << 0;
    t0 ^= relay_run_124(c, t1);
    c->lane[6] ^= sx_rl(c->lane[1], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3027u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)flush_index(c, (uint8_t)(t0 >> 16), t2);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 31);
    t2 = (t2 ^ c->sum) * 0x8b174727u;
    settle_line(c, &c->lane[4], 1);
    c->lane[4] += c->lane[2]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 15);
    t2 += (uint32_t)fold_pairing(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    poll_table(c, &c->lane[3], 3);
    c->sched[10] = c->hash ^ sx_rl(c->lane[2], 15);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t coal_level(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[24] = c->hash ^ sx_rl(c->lane[6], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] += c->lane[7]; c->lane[11] ^= c->lane[10]; c->lane[11] = sx_rl(c->lane[11], 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t1 ^= (uint32_t)swap_label(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1297u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)fill_layer(c);
    purge_entry(c, &c->lane[10], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11075u) % (uint32_t)c->rln)] << 0;
    t2 += (uint32_t)latch_entry(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63081u) % (uint32_t)c->rln)] << 24;
    c->lane[1] ^= sx_rl(c->lane[1], 12);
    c->lane[5] += c->lane[1] ^ 0x730491cfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7c) << 0;
    c->raw[c->slo + (int)((t0 + 15492u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum += t1;
    return t0 + t2;
}

static int move_lease(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->lane[14] += c->lane[2] ^ 0x0b9a1a28u;
    c->lane[8] ^= sx_rl(c->lane[9], 11);
    t1 ^= (uint32_t)swap_label(c, (uint8_t)(t0 >> 16), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51443u) % (uint32_t)c->rln)] << 0;
    slice_stream(c, &c->lane[10], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x47) << 8;
    c->hash = (c->hash * 0xf4f69bd1u) ^ sx_rr(c->hash, 28);
    c->lane[12] += c->lane[12] ^ 0x2d934032u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32566u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)shift_cell(c, (uint8_t)(t0 >> 16), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t tally_arena(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] += c->lane[6] ^ 0x83837ceau;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 15);
    t0 ^= poll_index(c, t1);
    c->lane[7] += c->lane[10]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 28);
    c->lane[12] += c->lane[9] ^ 0x03112af1u;
    c->lane[11] ^= sx_rl(c->lane[14], 24);
    t1 ^= (uint32_t)blend_slot(c, (uint8_t)(t0 >> 16), t2);
    rotate_run(c, t0, t1);
    c->sched[18] = c->hash ^ sx_rl(c->lane[8], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47029u) % (uint32_t)c->rln)] << 24;
    t2 += close_token(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0xa2c348afu;
    c->sched[3] = c->hash ^ sx_rl(c->lane[8], 14);
    t2 += yield_offset(c, c->rlo, c->rln);
    c->lane[8] += c->lane[11] ^ 0x32c3616fu;
    c->lane[1] ^= sx_rl(c->lane[13], 30);
    t1 ^= (uint32_t)mark_scope(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void latch_token(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[10], 23);
    t1 ^= (uint32_t)flush_index(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] ^= sx_rl(c->lane[7], 6);
    pin_segment(c, &c->lane[5], 3);
    c->lane[4] += c->lane[13]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 5);
    c->sched[27] = c->hash ^ sx_rl(c->lane[10], 18);
    t2 = (t2 ^ c->sum) * 0x6a20e14du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x34) << 0;
    c->sched[18] = c->hash ^ sx_rl(c->lane[3], 18);
    c->sched[12] = c->hash ^ sx_rl(c->lane[9], 9);
    c->lane[7] += c->lane[7] ^ 0xfeb72038u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t push_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    flush_path(c, t0, t1);
    c->hash = (c->hash * 0x312cef0bu) ^ sx_rr(c->hash, 24);
    t0 ^= reset_limit(c, t1);
    c->lane[8] += c->lane[2]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 27);
    c->sched[10] = c->hash ^ sx_rl(c->lane[13], 29);
    place_row(c, t0, t1);
    t2 += (uint32_t)latch_entry(c);
    c->lane[8] += c->lane[14] ^ 0x2ba3f8e2u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pack_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= hold_window(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x67d3e1c5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6972c04du;
    c->sched[1] = c->hash ^ sx_rl(c->lane[15], 6);
    c->hash = (c->hash * 0x05da0f95u) ^ sx_rr(c->hash, 18);
    t2 = (t2 ^ c->sum) * 0x8536b6f9u;
    t2 += (uint32_t)poll_digest(c);
    c->sched[2] = c->hash ^ sx_rl(c->lane[12], 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int latch_entry(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    t1 ^= (uint32_t)reset_rate(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 34514u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[3] + 0xee57e24fu;
    t2 += (uint32_t)trim_stack(c);
    c->sched[16] = c->hash ^ sx_rl(c->lane[7], 19);
    t2 += (uint32_t)cache_seat(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t mark_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc8) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd4) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[1] += c->lane[14]; c->lane[6] ^= c->lane[1]; c->lane[6] = sx_rl(c->lane[6], 30);
    c->lane[7] += c->lane[13]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 17);
    t2 = (t2 ^ c->sum) * 0x7d9df173u;
    c->hash ^= c->lane[2] + 0xd2c70c29u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x16c80c8du;
    coal_cursor(c, t0, t1);
    c->hash = (c->hash * 0x19c69499u) ^ sx_rr(c->hash, 11);
    peek_view(c, &c->lane[2], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int fold_pairing(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->lane[5] += c->lane[3] ^ 0x54df08adu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa5e97377u;
    c->hash ^= c->lane[0] + 0x80259406u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += seek_key(c, c->slo, c->sln);
    t1 ^= (uint32_t)hold_bucket(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17170u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x16f1845bu;
    c->sched[4] = c->hash ^ sx_rl(c->lane[4], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x366b2dd3u;
    t1 ^= (uint32_t)latch_label(c, (uint8_t)(t0 >> 16), t2);
    join_unit(c, &c->lane[7], 3);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int drain_page(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    poll_list(c, &c->lane[8], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39621u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0xd3de95c9u) ^ sx_rr(c->hash, 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59378u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 31495u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += (uint32_t)rotate_pool(c);
    c->sched[9] = c->hash ^ sx_rl(c->lane[10], 27);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t chain_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x8f) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x206f0cb1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3606u) % (uint32_t)c->rln)] << 24;
    c->lane[11] += c->lane[5] ^ 0x987ea0a3u;
    t2 += (uint32_t)grow_run(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1e35aaf3u;
    t2 += (uint32_t)fill_layer(c);
    relay_rate(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 += (uint32_t)tap_frame(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int trace_marker(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->raw[c->slo + (int)((t0 + 53264u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 ^= probe_field(c, t1);
    c->sched[6] = c->hash ^ sx_rl(c->lane[3], 13);
    c->raw[c->slo + (int)((t0 + 37016u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    store_seat(c, &c->lane[1], 2);
    c->sched[10] = c->hash ^ sx_rl(c->lane[11], 16);
    t1 ^= (uint32_t)tap_head(c, (uint8_t)(t0 >> 16), t2);
    place_row(c, t0, t1);
    push_line(c, &c->lane[1], 4);
    c->raw[c->slo + (int)((t0 + 3704u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int poll_digest(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->hash ^= c->lane[3] + 0xc15c5903u;
    c->hash = (c->hash * 0x5011814du) ^ sx_rr(c->hash, 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x58) << 16;
    poll_table(c, &c->lane[8], 2);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8b) << 16;
    c->sched[26] = c->hash ^ sx_rl(c->lane[14], 9);
    t2 += mix_gap(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc9) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb20d997du;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void purge_entry(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[3]; c->lane[12] ^= c->lane[1]; c->lane[12] = sx_rl(c->lane[12], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32030u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[4] ^= sx_rl(c->lane[13], 19);
    c->sched[26] = c->hash ^ sx_rl(c->lane[2], 29);
    c->hash = (c->hash * 0xaaf3f877u) ^ sx_rr(c->hash, 4);
    c->hash = (c->hash * 0x7e8cc717u) ^ sx_rr(c->hash, 12);
    parse_field(c, t0, t1);
    c->lane[13] += c->lane[15]; c->lane[7] ^= c->lane[13]; c->lane[7] = sx_rl(c->lane[7], 15);
    settle_line(c, &c->lane[6], 3);
    c->lane[2] += c->lane[6]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 11);
    c->hash = (c->hash * 0x2672c561u) ^ sx_rr(c->hash, 17);
    t2 += flush_span(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48866u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t swap_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    fill_port(c, t0, t1);
    c->hash = (c->hash * 0x9a903cd3u) ^ sx_rr(c->hash, 1);
    c->raw[c->slo + (int)((t0 + 41381u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[14] += c->lane[6]; c->lane[13] ^= c->lane[14]; c->lane[13] = sx_rl(c->lane[13], 14);
    c->hash ^= c->lane[6] + 0x101156c1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb9eda971u;
    c->lane[0] += c->lane[7] ^ 0x7fbe18fbu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t reset_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] += c->lane[6]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 18);
    t2 = (t2 ^ c->sum) * 0xf180fdd5u;
    c->hash ^= c->lane[14] + 0x7882a116u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 2);
    c->lane[14] ^= sx_rl(c->lane[1], 6);
    c->hash = (c->hash * 0xe7032e57u) ^ sx_rr(c->hash, 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[4] + 0xc35857a0u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int reap_row(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t1 ^= (uint32_t)hold_bucket(c, (uint8_t)(t0 >> 8), t2);
    c->lane[3] += c->lane[2] ^ 0x9b37fcc1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[5] = c->hash ^ sx_rl(c->lane[13], 24);
    c->hash = (c->hash * 0x88c5bb79u) ^ sx_rr(c->hash, 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[26] = c->hash ^ sx_rl(c->lane[12], 26);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t poll_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= slice_digest(c, t1);
    c->raw[c->slo + (int)((t0 + 32148u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x83ab9687u) ^ sx_rr(c->hash, 7);
    c->lane[13] += c->lane[0] ^ 0x40612229u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x074a6449u;
    c->lane[12] ^= sx_rl(c->lane[15], 2);
    c->lane[14] += c->lane[4]; c->lane[13] ^= c->lane[14]; c->lane[13] = sx_rl(c->lane[13], 29);
    latch_group(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int poll_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->sched[17] = c->hash ^ sx_rl(c->lane[1], 29);
    t2 = (t2 ^ c->sum) * 0x3e91efc5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t2 += mix_gap(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x24f66739u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x1a) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39823u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= c->lane[6] + 0xbe403392u;
    relay_run(c, t0, t1);
    c->hash = (c->hash * 0x434699c5u) ^ sx_rr(c->hash, 16);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t flush_index(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xd8595f07u) ^ sx_rr(c->hash, 2);
    c->hash ^= c->lane[3] + 0xe4ed4570u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31078u) % (uint32_t)c->rln)] << 24;
    tune_port(c, t0, t1);
    c->lane[1] ^= sx_rl(c->lane[0], 17);
    c->raw[c->slo + (int)((t0 + 15152u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)pair_group(c, (uint8_t)(t0 >> 16), t2);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 10);
    t2 = (t2 ^ c->sum) * 0x60517145u;
    t1 ^= (uint32_t)grow_level(c, (uint8_t)(t0 >> 0), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t yield_offset(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[6] + 0xae4be711u;
    c->lane[3] += c->lane[9] ^ 0x86158a14u;
    c->hash ^= c->lane[0] + 0xe6ad5d32u;
    c->lane[3] += c->lane[0]; c->lane[11] ^= c->lane[3]; c->lane[11] = sx_rl(c->lane[11], 28);
    c->hash ^= c->lane[15] + 0xf9ca7071u;
    t1 ^= (uint32_t)pack_table(c, (uint8_t)(t0 >> 16), t2);
    c->lane[10] += c->lane[9]; c->lane[15] ^= c->lane[10]; c->lane[15] = sx_rl(c->lane[15], 4);
    c->sum += t1;
    return t0 + t2;
}

static void flush_path(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x89761d31u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58114u) % (uint32_t)c->rln)] << 8;
    t2 += pack_seat(c, c->rlo, c->rln);
    t2 += (uint32_t)drain_head(c);
    c->lane[4] += c->lane[8] ^ 0x99dfebc0u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47164u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45038u) % (uint32_t)c->rln)] << 16;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void rotate_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa6) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x10) << 0;
    t1 ^= (uint32_t)reset_rate(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[7] + 0x7d89ced2u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x79) << 0;
    c->lane[10] += c->lane[13] ^ 0x5a6bc182u;
    t2 += (uint32_t)move_lease_66(c);
    c->lane[10] ^= sx_rl(c->lane[15], 13);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t close_token(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    pin_segment(c, &c->lane[4], 1);
    c->raw[c->slo + (int)((t0 + 22884u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 35737u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[1] ^ 0xe88b5261u;
    t1 ^= (uint32_t)pick_line(c, (uint8_t)(t0 >> 0), t2);
    c->lane[9] += c->lane[11]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 23);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t shift_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe23d2f11u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x381f46efu;
    t1 ^= (uint32_t)fold_token(c, (uint8_t)(t0 >> 0), t2);
    c->lane[5] ^= sx_rl(c->lane[3], 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x40) << 0;
    c->hash = (c->hash * 0x9cecffb1u) ^ sx_rr(c->hash, 18);
    t2 = (t2 ^ c->sum) * 0x0ed71793u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t blend_slot(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[10] = c->hash ^ sx_rl(c->lane[10], 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x6ffe399fu) ^ sx_rr(c->hash, 14);
    c->lane[1] += c->lane[9] ^ 0x7daf93fbu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x78b470a3u;
    grow_unit(c, &c->lane[2], 1);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 18);
    c->lane[3] += c->lane[8] ^ 0x91263a42u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sched[8] = c->hash ^ sx_rl(c->lane[12], 30);
    c->hash = (c->hash * 0x98f16ecbu) ^ sx_rr(c->hash, 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t tap_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[9]; c->lane[7] ^= c->lane[10]; c->lane[7] = sx_rl(c->lane[7], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[13] ^= sx_rl(c->lane[11], 30);
    c->sched[30] = c->hash ^ sx_rl(c->lane[3], 4);
    c->lane[11] += c->lane[5]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 7);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void poll_list(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t1 ^= (uint32_t)link_marker(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12543u) % (uint32_t)c->rln)] << 8;
    push_line(c, &c->lane[7], 2);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[0] ^= sx_rl(c->lane[2], 5);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t hold_bucket(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= hold_window(c, t1);
    c->raw[c->slo + (int)((t0 + 37695u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    map_view(c, &c->lane[6], 3);
    t2 = (t2 ^ c->sum) * 0x7dfa5b7bu;
    c->hash ^= c->lane[10] + 0xa373dbfbu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[6] = c->hash ^ sx_rl(c->lane[3], 16);
    slice_stream(c, &c->lane[3], 3);
    c->raw[c->slo + (int)((t0 + 20432u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[25] = c->hash ^ sx_rl(c->lane[4], 5);
    t2 = (t2 ^ c->sum) * 0x86b190b9u;
    c->lane[0] += c->lane[10]; c->lane[11] ^= c->lane[0]; c->lane[11] = sx_rl(c->lane[11], 14);
    t2 += (uint32_t)merge_table(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[8] + 0x62e7324bu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t pack_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x74c8e171u;
    t2 = (t2 ^ c->sum) * 0xa67cb6a5u;
    t2 = (t2 ^ c->sum) * 0x7aed2f63u;
    relay_layer(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57897u) % (uint32_t)c->rln)] << 24;
    t1 ^= (uint32_t)load_row(c, (uint8_t)(t0 >> 8), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t grow_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= push_range(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x27) << 8;
    t2 = (t2 ^ c->sum) * 0x5392582bu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 15);
    c->sched[23] = c->hash ^ sx_rl(c->lane[6], 27);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int fill_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    t1 ^= (uint32_t)drain_marker(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56340u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41912u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 18);
    c->hash = (c->hash * 0xc907f021u) ^ sx_rr(c->hash, 6);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int tap_frame(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->sched[0] = c->hash ^ sx_rl(c->lane[5], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x34e0615bu;
    c->lane[10] ^= sx_rl(c->lane[0], 14);
    t0 ^= shift_group(c, t1);
    c->lane[5] ^= sx_rl(c->lane[4], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    close_offset(c, &c->lane[0], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa5dee403u;
    c->hash = (c->hash * 0xdcd69c03u) ^ sx_rr(c->hash, 13);
    c->raw[c->slo + (int)((t0 + 32887u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int drain_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 23);
    c->hash = (c->hash * 0x3590803fu) ^ sx_rr(c->hash, 17);
    tune_store(c, &c->lane[9], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb06cd14bu;
    t2 += trim_index(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xeb427f05u;
    c->hash = (c->hash * 0x532fdcffu) ^ sx_rr(c->hash, 2);
    c->lane[0] += c->lane[12]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 10);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fill_port(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61158u) % (uint32_t)c->rln)] << 0;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x1b) << 0;
    c->hash ^= c->lane[5] + 0x935aae1du;
    blend_row(c, t0, t1);
    c->hash ^= c->lane[11] + 0xca212eafu;
    store_seat(c, &c->lane[2], 1);
    c->raw[c->slo + (int)((t0 + 1322u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x77b7c1c9u;
    c->raw[c->slo + (int)((t0 + 17012u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[3] = c->hash ^ sx_rl(c->lane[11], 31);
    c->lane[14] += c->lane[0] ^ 0xecaa93f5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x02d8a29fu;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t pick_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[2], 28);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 25);
    poll_value(c, t0, t1);
    t2 += (uint32_t)sift_pool(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61698u) % (uint32_t)c->rln)] << 0;
    c->lane[0] += c->lane[6] ^ 0x23264142u;
    c->hash = (c->hash * 0xa40122efu) ^ sx_rr(c->hash, 11);
    c->sched[24] = c->hash ^ sx_rl(c->lane[10], 8);
    t1 ^= (uint32_t)trim_group(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x83e44a61u;
    c->lane[15] ^= sx_rl(c->lane[13], 12);
    t1 ^= (uint32_t)place_arena(c, (uint8_t)(t0 >> 16), t2);
    t2 += (uint32_t)trim_band(c);
    c->lane[0] += c->lane[5]; c->lane[7] ^= c->lane[0]; c->lane[7] = sx_rl(c->lane[7], 26);
    c->hash = (c->hash * 0x12de4b09u) ^ sx_rr(c->hash, 21);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void pin_segment(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 35463u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 26145u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    scan_store(c, &c->lane[1], 2);
    merge_pool(c, t0, t1);
    c->lane[1] += c->lane[10] ^ 0x9031a2f3u;
    c->lane[3] ^= sx_rl(c->lane[7], 15);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void poll_table(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)tap_token(c);
    t0 ^= push_range(c, t1);
    c->lane[7] += c->lane[11]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 25);
    sort_block(c, &c->lane[2], 2);
    c->hash ^= c->lane[4] + 0x790d9662u;
    c->raw[c->slo + (int)((t0 + 58327u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)probe_delta(c);
    c->hash = (c->hash * 0x2d4d965bu) ^ sx_rr(c->hash, 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t fold_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] += c->lane[8] ^ 0x98740ea4u;
    c->lane[1] += c->lane[2]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 31);
    c->lane[1] ^= sx_rl(c->lane[12], 19);
    t0 ^= patch_tuple(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xe64fc895u;
    c->hash = (c->hash * 0x70ee9435u) ^ sx_rr(c->hash, 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[12] = c->hash ^ sx_rl(c->lane[2], 18);
    t2 = (t2 ^ c->sum) * 0x9609f345u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[5], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40832u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t pack_seat(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40480u) % (uint32_t)c->rln)] << 0;
    t0 ^= purge_slot(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xdf) << 8;
    c->lane[6] ^= sx_rl(c->lane[14], 30);
    step_store(c, &c->lane[2], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x6a) << 8;
    c->lane[14] += c->lane[12]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 27);
    c->sched[31] = c->hash ^ sx_rl(c->lane[8], 3);
    t2 = (t2 ^ c->sum) * 0x74d8d40fu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t reset_rate(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    coal_digest(c, &c->lane[5], 2);
    merge_pool(c, t0, t1);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 25);
    t2 = (t2 ^ c->sum) * 0xdb1a9d67u;
    c->hash ^= c->lane[14] + 0x156d428eu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    fill_count(c, t0, t1);
    c->lane[13] ^= sx_rl(c->lane[11], 26);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t latch_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15217u) % (uint32_t)c->rln)] << 16;
    c->sched[14] = c->hash ^ sx_rl(c->lane[13], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6ee3b679u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa6) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t pair_group(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x03) << 0;
    c->lane[14] ^= sx_rl(c->lane[1], 31);
    c->hash = (c->hash * 0x5548dc11u) ^ sx_rr(c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] += c->lane[15]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 26);
    c->raw[c->slo + (int)((t0 + 3809u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= patch_tuple(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void join_unit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 11);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 10);
    c->lane[6] += c->lane[10] ^ 0x80bb143cu;
    c->hash = (c->hash * 0x9b1d367du) ^ sx_rr(c->hash, 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12933u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[5] + 0x92bee7dbu;
    scan_store(c, &c->lane[10], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51501u) % (uint32_t)c->rln)] << 16;
    pick_cursor(c, t0, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void coal_cursor(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xaf9e6cf5u) ^ sx_rr(c->hash, 23);
    c->hash ^= c->lane[4] + 0x9e673804u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[9] ^= sx_rl(c->lane[5], 9);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 7);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void relay_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += prime_stack(c, c->rlo, c->rln);
    c->lane[0] += c->lane[7] ^ 0x71af6cc4u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 24);
    c->lane[3] += c->lane[15]; c->lane[2] ^= c->lane[3]; c->lane[2] = sx_rl(c->lane[2], 29);
    c->lane[10] += c->lane[5]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 30);
    c->hash ^= c->lane[14] + 0x54821db4u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37799u) % (uint32_t)c->rln)] << 8;
    c->lane[2] ^= sx_rl(c->lane[0], 5);
    t2 += (uint32_t)rotate_pool(c);
    c->hash ^= c->lane[6] + 0xc4bc0829u;
    t1 ^= (uint32_t)flush_record(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 50036u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t seek_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[2] += c->lane[6]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50817u) % (uint32_t)c->rln)] << 8;
    c->lane[3] += c->lane[9] ^ 0xf243c1a6u;
    c->sum += t1;
    return t0 + t2;
}

static void latch_group(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x60) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe6) << 0;
    t1 ^= (uint32_t)flush_record(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= fold_group(c, t1);
    c->lane[12] += c->lane[13] ^ 0xb0109060u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 1);
    t0 ^= parse_tuple(c, t1);
    c->sched[10] = c->hash ^ sx_rl(c->lane[4], 30);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void grow_unit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= relay_run_124(c, t1);
    c->raw[c->slo + (int)((t0 + 63810u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 10);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    sync_window_119(c, &c->lane[3], 3);
    c->lane[15] ^= sx_rl(c->lane[10], 18);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    scan_item(c, &c->lane[9], 2);
    t2 += (uint32_t)probe_delta(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[12] ^= sx_rl(c->lane[1], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += (uint32_t)shift_level(c);
    c->lane[1] += c->lane[3] ^ 0xd4dead65u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t mix_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9c6f3bf1u;
    t0 ^= pair_block(c, t1);
    c->hash = (c->hash * 0xb3259177u) ^ sx_rr(c->hash, 22);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 2);
    c->lane[0] += c->lane[9]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 10);
    c->lane[5] ^= sx_rl(c->lane[3], 18);
    c->lane[6] ^= sx_rl(c->lane[1], 4);
    c->sum += t1;
    return t0 + t2;
}

static void settle_line(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa992a1b3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x66) << 8;
    sift_token(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x128b27e9u;
    t2 += pick_item(c, c->rlo, c->rln);
    relay_layer(c, t0, t1);
    t2 += (uint32_t)latch_cell(c);
    t2 = (t2 ^ c->sum) * 0x61d50671u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0f) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34635u) % (uint32_t)c->rln)] << 16;
    c->sched[20] = c->hash ^ sx_rl(c->lane[8], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x431b72b5u) ^ sx_rr(c->hash, 30);
    c->lane[1] ^= sx_rl(c->lane[5], 5);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int grow_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbf) << 16;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 11);
    c->raw[c->slo + (int)((t0 + 17039u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += (uint32_t)trim_run(c);
    c->raw[c->slo + (int)((t0 + 48827u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[12] ^= sx_rl(c->lane[3], 29);
    t2 += trim_index(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24839u) % (uint32_t)c->rln)] << 8;
    c->lane[1] += c->lane[12] ^ 0x13837a1du;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 4);
    tune_port(c, t0, t1);
    c->hash = (c->hash * 0x04e86e75u) ^ sx_rr(c->hash, 16);
    t1 ^= (uint32_t)poll_head(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[3] ^= sx_rl(c->lane[1], 29);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t flush_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[8] += c->lane[2]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47265u) % (uint32_t)c->rln)] << 24;
    c->lane[8] ^= sx_rl(c->lane[2], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1d) << 0;
    c->lane[10] ^= sx_rl(c->lane[6], 26);
    sift_item(c, &c->lane[3], 3);
    c->hash = (c->hash * 0xa1f5ff81u) ^ sx_rr(c->hash, 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 751u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x4bfdcb59u) ^ sx_rr(c->hash, 3);
    c->sum += t1;
    return t0 + t2;
}

static void place_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] += c->lane[2]; c->lane[0] ^= c->lane[13]; c->lane[0] = sx_rl(c->lane[0], 20);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 10);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 27);
    t0 ^= close_region(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb00c8047u;
    c->lane[12] += c->lane[10]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 29);
    c->lane[4] += c->lane[8] ^ 0xee0dd9d0u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[27] = c->hash ^ sx_rl(c->lane[15], 21);
    c->lane[2] += c->lane[10]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 1);
    c->lane[12] ^= sx_rl(c->lane[12], 15);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void peek_view(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfcf8040du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33374u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x18) << 0;
    c->lane[6] ^= sx_rl(c->lane[13], 13);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 19);
    sift_cell(c, &c->lane[4], 4);
    t2 = (t2 ^ c->sum) * 0xe3b1c23du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[4] + 0x5dda5f5cu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void parse_field(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[7] += c->lane[12]; c->lane[2] ^= c->lane[7]; c->lane[2] = sx_rl(c->lane[2], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb9b94705u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[2], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26692u) % (uint32_t)c->rln)] << 8;
    relay_rate(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59617u) % (uint32_t)c->rln)] << 8;
    c->lane[10] += c->lane[4] ^ 0x6acdf740u;
    t0 ^= clamp_stream(c, t1);
    c->lane[4] += c->lane[1] ^ 0x6a91fbf4u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t probe_field(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 3147u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 26);
    t0 ^= purge_ring(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35728u) % (uint32_t)c->rln)] << 16;
    c->lane[1] += c->lane[0] ^ 0x18a1c9efu;
    t2 = (t2 ^ c->sum) * 0xb6dcdebdu;
    hold_rate(c, &c->lane[1], 1);
    c->lane[12] += c->lane[2]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int cache_seat(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->sched[10] = c->hash ^ sx_rl(c->lane[2], 1);
    c->sched[2] = c->hash ^ sx_rl(c->lane[2], 13);
    t2 = (t2 ^ c->sum) * 0x3bcf03dfu;
    c->lane[4] += c->lane[8]; c->lane[2] ^= c->lane[4]; c->lane[2] = sx_rl(c->lane[2], 23);
    c->hash ^= c->lane[15] + 0x97883fbbu;
    c->sched[12] = c->hash ^ sx_rl(c->lane[9], 29);
    t2 = (t2 ^ c->sum) * 0xf70b54d3u;
    t2 += (uint32_t)swap_band(c);
    pack_frame(c, t0, t1);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 25);
    c->hash = (c->hash * 0xe900291fu) ^ sx_rr(c->hash, 25);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int move_lease_66(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[15] ^= sx_rl(c->lane[14], 4);
    c->hash ^= c->lane[9] + 0xa34808bfu;
    c->raw[c->slo + (int)((t0 + 61955u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= mark_track(c, t1);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 28);
    c->sched[21] = c->hash ^ sx_rl(c->lane[10], 20);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 5);
    c->lane[11] += c->lane[2]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 6);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t slice_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x49f04705u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] += c->lane[3]; c->lane[9] ^= c->lane[12]; c->lane[9] = sx_rl(c->lane[9], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 19688u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 1118u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[2] ^= sx_rl(c->lane[5], 28);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int trim_stack(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->lane[12] += c->lane[9] ^ 0x6f996fddu;
    c->lane[15] += c->lane[10]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47962u) % (uint32_t)c->rln)] << 16;
    t2 += (uint32_t)prime_region(c);
    c->hash = (c->hash * 0xabbdaef9u) ^ sx_rr(c->hash, 29);
    c->hash ^= c->lane[12] + 0x8eb5e0e8u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 10);
    c->lane[6] += c->lane[14]; c->lane[12] ^= c->lane[6]; c->lane[12] = sx_rl(c->lane[12], 24);
    c->hash ^= c->lane[13] + 0x76d2f6cau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1998u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x5fa469afu;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void poll_value(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x48547831u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa7a16077u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52288u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x9a1298e7u) ^ sx_rr(c->hash, 27);
    c->sched[5] = c->hash ^ sx_rl(c->lane[11], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static void sift_cell(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x30) << 0;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[15] = c->hash ^ sx_rl(c->lane[1], 30);
    c->lane[0] += c->lane[8]; c->lane[11] ^= c->lane[0]; c->lane[11] = sx_rl(c->lane[11], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t trim_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xe469a7b7u) ^ sx_rr(c->hash, 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[13] = c->hash ^ sx_rl(c->lane[11], 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[12] += c->lane[3] ^ 0x16790991u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t pair_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6113u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x5d2bc44bu;
    t2 = (t2 ^ c->sum) * 0x0ed0d785u;
    c->hash = (c->hash * 0xacbbb597u) ^ sx_rr(c->hash, 30);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x165e681fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x822b9befu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tune_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[9] = c->hash ^ sx_rl(c->lane[15], 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x5b241c93u) ^ sx_rr(c->hash, 29);
    c->raw[c->slo + (int)((t0 + 10341u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 48297u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5d837ad5u;
    t2 = (t2 ^ c->sum) * 0x2c0d583du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56940u) % (uint32_t)c->rln)] << 0;
    c->lane[10] ^= sx_rl(c->lane[2], 2);
    c->hash = (c->hash * 0xe651d9afu) ^ sx_rr(c->hash, 17);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void scan_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[13] += c->lane[9]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 4);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 5);
    c->raw[c->slo + (int)((t0 + 26738u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[3] + 0x4ab54637u;
    c->sched[29] = c->hash ^ sx_rl(c->lane[0], 2);
    t2 = (t2 ^ c->sum) * 0x49ca5b99u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t clamp_stream(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 36697u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void push_line(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[14] + 0x78a94ac3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[10] + 0x639e677au;
    c->lane[15] ^= sx_rl(c->lane[13], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x719e7bf9u;
    c->lane[15] += c->lane[2] ^ 0x927710d9u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void blend_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27532u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5ef2fd59u;
    c->lane[6] += c->lane[15]; c->lane[9] ^= c->lane[6]; c->lane[9] = sx_rl(c->lane[9], 19);
    c->sched[28] = c->hash ^ sx_rl(c->lane[14], 27);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 23);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void map_view(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xde61d04du) ^ sx_rr(c->hash, 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64277u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x22e72f49u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30821u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16983u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x3d16129bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x67382075u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t flush_record(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x4b9ba0ebu) ^ sx_rr(c->hash, 13);
    c->raw[c->slo + (int)((t0 + 16565u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= close_line(c, t1);
    t2 = (t2 ^ c->sum) * 0x5f890be3u;
    c->hash = (c->hash * 0x24bea1a3u) ^ sx_rr(c->hash, 2);
    c->sched[19] = c->hash ^ sx_rl(c->lane[5], 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x07eaa2fau;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t link_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[6] + 0x0c20eb4bu;
    t2 = (t2 ^ c->sum) * 0x3b43d923u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 29);
    c->lane[4] += c->lane[8] ^ 0x8111ac57u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t parse_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30772u) % (uint32_t)c->rln)] << 0;
    c->lane[5] += c->lane[1]; c->lane[15] ^= c->lane[5]; c->lane[15] = sx_rl(c->lane[15], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= c->lane[0] + 0x92d16316u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int trim_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->lane[2] += c->lane[8]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 16);
    t2 = (t2 ^ c->sum) * 0xcbaa74ddu;
    c->hash ^= c->lane[15] + 0x2f28e2e9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x813d2e8fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash = (c->hash * 0x422336c9u) ^ sx_rr(c->hash, 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9b) << 8;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int probe_delta(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    t2 = (t2 ^ c->sum) * 0x6302bff1u;
    c->hash ^= c->lane[1] + 0xc91e3b73u;
    t2 = (t2 ^ c->sum) * 0x5b193385u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 5760u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x3d9f2c5bu) ^ sx_rr(c->hash, 19);
    c->sched[5] = c->hash ^ sx_rl(c->lane[0], 17);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 31);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int merge_table(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4134u) % (uint32_t)c->rln)] << 16;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 18);
    c->hash = (c->hash * 0xaef897c1u) ^ sx_rr(c->hash, 25);
    c->sched[12] = c->hash ^ sx_rl(c->lane[13], 9);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sort_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[13] += c->lane[7] ^ 0x09007f32u;
    c->lane[13] += c->lane[2]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65243u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41172u) % (uint32_t)c->rln)] << 0;
    c->lane[6] ^= sx_rl(c->lane[15], 26);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t poll_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x4f4272cbu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xfab1d42du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x46deb59fu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 2);
    c->raw[c->slo + (int)((t0 + 60462u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13829u) % (uint32_t)c->rln)] << 8;
    c->lane[10] += c->lane[11]; c->lane[13] ^= c->lane[10]; c->lane[13] = sx_rl(c->lane[13], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x39) << 0;
    c->lane[9] += c->lane[15] ^ 0xec9ad81eu;
    c->hash ^= c->lane[8] + 0x50207df1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t purge_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 42874u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[15] ^= sx_rl(c->lane[15], 13);
    c->lane[4] ^= sx_rl(c->lane[15], 5);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int trim_band(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->lane[11] ^= sx_rl(c->lane[11], 16);
    c->sched[22] = c->hash ^ sx_rl(c->lane[14], 1);
    c->hash ^= c->lane[12] + 0xa35cbb3fu;
    c->lane[4] += c->lane[7]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3709u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[6] + 0x4bfcee94u;
    c->hash ^= c->lane[12] + 0x1feaccacu;
    c->raw[c->slo + (int)((t0 + 48294u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[10] ^= sx_rl(c->lane[5], 12);
    c->lane[1] += c->lane[13] ^ 0xd95fa075u;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t trim_group(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[10] + 0x1a5d717fu;
    c->sched[8] = c->hash ^ sx_rl(c->lane[12], 26);
    c->raw[c->slo + (int)((t0 + 28222u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x5535fbc9u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int rotate_pool(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->sched[31] = c->hash ^ sx_rl(c->lane[6], 16);
    c->lane[1] += c->lane[13]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x89) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sched[25] = c->hash ^ sx_rl(c->lane[1], 5);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sift_item(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xe383d8a7u) ^ sx_rr(c->hash, 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x19) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x55e27379u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb8) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[6] += c->lane[11]; c->lane[9] ^= c->lane[6]; c->lane[9] = sx_rl(c->lane[9], 29);
    c->lane[8] += c->lane[9]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 31);
    c->lane[0] += c->lane[15] ^ 0x5e4387f4u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t close_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xf6b0473bu) ^ sx_rr(c->hash, 27);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x49daf8a3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51457u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xd99e62d7u;
    c->lane[5] ^= sx_rl(c->lane[3], 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48294u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[12] + 0xd967b728u;
    c->lane[11] += c->lane[13] ^ 0x815e0fc8u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t purge_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x9c3883b3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa0) << 0;
    c->hash = (c->hash * 0x0dd8c97du) ^ sx_rr(c->hash, 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t patch_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x2e75d08du) ^ sx_rr(c->hash, 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x02) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xea62d577u;
    c->hash = (c->hash * 0x49b8b5a1u) ^ sx_rr(c->hash, 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32110u) % (uint32_t)c->rln)] << 24;
    c->lane[4] ^= sx_rl(c->lane[1], 25);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t place_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52887u) % (uint32_t)c->rln)] << 24;
    c->lane[3] += c->lane[9]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xeff3eaadu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60247u) % (uint32_t)c->rln)] << 8;
    c->lane[3] += c->lane[11] ^ 0xa8bd1272u;
    c->raw[c->slo + (int)((t0 + 60806u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0xe136b5e5u;
    c->lane[11] += c->lane[0]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 23);
    c->lane[2] += c->lane[10]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void scan_item(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0xbc991d2du;
    c->sched[29] = c->hash ^ sx_rl(c->lane[8], 10);
    t2 = (t2 ^ c->sum) * 0x3ce35e63u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x162cce0fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1337573du;
    c->hash ^= c->lane[10] + 0xa305cf12u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int tap_token(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[12] ^= sx_rl(c->lane[14], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 61130u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0x88417b49u;
    c->raw[c->slo + (int)((t0 + 3684u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] ^= sx_rl(c->lane[15], 6);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void relay_rate(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5d94243du;
    c->lane[13] += c->lane[7]; c->lane[14] ^= c->lane[13]; c->lane[14] = sx_rl(c->lane[14], 21);
    t2 = (t2 ^ c->sum) * 0xa1da7c53u;
    c->lane[12] ^= sx_rl(c->lane[15], 1);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t push_range(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] += c->lane[3] ^ 0x3e4750a1u;
    c->sched[16] = c->hash ^ sx_rl(c->lane[9], 14);
    c->raw[c->slo + (int)((t0 + 2042u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0xbb35c2f7u) ^ sx_rr(c->hash, 19);
    c->sched[14] = c->hash ^ sx_rl(c->lane[13], 5);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pick_cursor(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[27] = c->hash ^ sx_rl(c->lane[14], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16713u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[0] + 0x205212d9u;
    c->lane[11] += c->lane[15] ^ 0x45f13680u;
    t2 = (t2 ^ c->sum) * 0x6b3ef829u;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t fold_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 16);
    c->raw[c->slo + (int)((t0 + 64749u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x6ca55537u;
    t2 = (t2 ^ c->sum) * 0x82689bf1u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 ^= resize_label(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9d00556du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void sift_token(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x27) << 8;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1d9460adu;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[16] = c->hash ^ sx_rl(c->lane[1], 17);
    c->hash = (c->hash * 0x52d01443u) ^ sx_rr(c->hash, 16);
    c->hash = (c->hash * 0x14ce67a5u) ^ sx_rr(c->hash, 24);
    c->raw[c->slo + (int)((t0 + 27081u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static int sift_pool(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= c->lane[5] + 0xca5cdf45u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13379u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xff2b4a1bu;
    c->lane[7] ^= sx_rl(c->lane[4], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb11dfb2bu;
    c->hash ^= c->lane[13] + 0x837a5fcfu;
    c->lane[11] ^= sx_rl(c->lane[7], 8);
    c->lane[7] += c->lane[2]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 20);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int shift_level(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->sched[31] = c->hash ^ sx_rl(c->lane[15], 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbd) << 16;
    c->raw[c->slo + (int)((t0 + 21663u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[4] = c->hash ^ sx_rl(c->lane[10], 15);
    c->hash ^= c->lane[4] + 0x39307fbcu;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tune_port(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x26ad37e7u) ^ sx_rr(c->hash, 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x83) << 16;
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 3);
    c->lane[1] += c->lane[13]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 10);
    c->raw[c->slo + (int)((t0 + 62075u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17187u) % (uint32_t)c->rln)] << 24;
    c->lane[2] += c->lane[3]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 21);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t shift_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[13] += c->lane[1]; c->lane[15] ^= c->lane[13]; c->lane[15] = sx_rl(c->lane[15], 22);
    c->sched[2] = c->hash ^ sx_rl(c->lane[4], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11336u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void close_offset(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 22);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 17);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void relay_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7cdbc65bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7c) << 16;
    c->hash ^= c->lane[4] + 0x38bb0ffcu;
    c->lane[15] ^= sx_rl(c->lane[11], 19);
    c->sched[7] = c->hash ^ sx_rl(c->lane[3], 3);
    c->raw[c->slo + (int)((t0 + 16024u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3bf61dd5u;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static int prime_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[14] += c->lane[4]; c->lane[15] ^= c->lane[14]; c->lane[15] = sx_rl(c->lane[15], 25);
    c->hash ^= c->lane[9] + 0x174a2451u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x77) << 8;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 15);
    c->sched[20] = c->hash ^ sx_rl(c->lane[6], 18);
    c->lane[1] += c->lane[1] ^ 0x433bd2fbu;
    c->sched[29] = c->hash ^ sx_rl(c->lane[10], 15);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 29);
    c->hash = (c->hash * 0x7d1063c3u) ^ sx_rr(c->hash, 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void hold_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] ^= sx_rl(c->lane[13], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11164u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x4e2c7783u;
    c->hash ^= c->lane[0] + 0x42cdfefau;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= c->lane[2] + 0x782b2262u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int swap_band(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->lane[6] += c->lane[10]; c->lane[11] ^= c->lane[6]; c->lane[11] = sx_rl(c->lane[11], 14);
    c->hash ^= c->lane[10] + 0x6660dbf5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash = (c->hash * 0xe821661bu) ^ sx_rr(c->hash, 27);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void merge_pool(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] ^= sx_rl(c->lane[2], 9);
    c->lane[12] += c->lane[2]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14119u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x15aec64bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4a) << 16;
    c->lane[3] += c->lane[11] ^ 0xbe48099du;
    c->raw[c->slo + (int)((t0 + 63485u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t drain_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[23] = c->hash ^ sx_rl(c->lane[3], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[6] ^= sx_rl(c->lane[14], 3);
    c->raw[c->slo + (int)((t0 + 24602u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[12] ^= sx_rl(c->lane[10], 21);
    c->lane[15] ^= sx_rl(c->lane[15], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[6] + 0x90f42ae8u;
    c->hash = (c->hash * 0x9ba26d3fu) ^ sx_rr(c->hash, 24);
    c->lane[8] ^= sx_rl(c->lane[9], 5);
    c->hash ^= c->lane[12] + 0x2494e317u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void slice_stream(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[4] + 0x829ed1ffu;
    c->lane[3] += c->lane[15] ^ 0xc8d009ecu;
    c->sched[0] = c->hash ^ sx_rl(c->lane[15], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t hold_window(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[0] ^ 0x3366d052u;
    c->lane[7] ^= sx_rl(c->lane[9], 6);
    c->lane[13] += c->lane[11] ^ 0xa765fe74u;
    c->lane[14] += c->lane[7] ^ 0x183658bcu;
    c->lane[1] ^= sx_rl(c->lane[8], 24);
    c->lane[7] += c->lane[10]; c->lane[14] ^= c->lane[7]; c->lane[14] = sx_rl(c->lane[14], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void coal_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[13] += c->lane[13] ^ 0x2dd5d0cdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26789u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 36519u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 5);
    c->lane[13] ^= sx_rl(c->lane[0], 15);
    c->lane[10] += c->lane[1]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 7);
    c->sched[18] = c->hash ^ sx_rl(c->lane[7], 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[1] ^= sx_rl(c->lane[15], 3);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void pack_frame(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[14] + 0xde6979a6u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 18);
    c->raw[c->slo + (int)((t0 + 34470u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[1] ^= sx_rl(c->lane[9], 17);
    c->lane[8] += c->lane[2]; c->lane[14] ^= c->lane[8]; c->lane[14] = sx_rl(c->lane[14], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 440u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x78) << 16;
    c->lane[7] += c->lane[2]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 20);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void store_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9505fe87u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x73) << 8;
    c->lane[13] += c->lane[2]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3b) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void sync_window_119(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 29517u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[3] + 0x6b700b6bu;
    c->raw[c->slo + (int)((t0 + 27011u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 12282u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 18);
    c->lane[5] += c->lane[2]; c->lane[0] ^= c->lane[5]; c->lane[0] = sx_rl(c->lane[0], 30);
    t2 = (t2 ^ c->sum) * 0xccdb471du;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pick_item(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe5) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[2] += c->lane[0]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 9);
    c->hash ^= c->lane[7] + 0xae996e12u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf74fd217u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 51721u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[2] + 0x682bf288u;
    c->sum += t1;
    return t0 + t2;
}

static void fill_count(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[27] = c->hash ^ sx_rl(c->lane[2], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6617u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 6769u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] += c->lane[2] ^ 0x6e87aa8eu;
    c->hash ^= c->lane[2] + 0xf20f6273u;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t load_row(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x41) << 8;
    c->sched[3] = c->hash ^ sx_rl(c->lane[3], 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55197u) % (uint32_t)c->rln)] << 0;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb0) << 16;
    c->lane[13] += c->lane[4] ^ 0xe12482d7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t mark_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x93ab3dc5u) ^ sx_rr(c->hash, 23);
    c->sched[22] = c->hash ^ sx_rl(c->lane[10], 27);
    c->lane[13] += c->lane[4] ^ 0xf17dc3a6u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 27);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t relay_run_124(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[4] + 0x66660e0cu;
    c->lane[8] += c->lane[3]; c->lane[13] ^= c->lane[8]; c->lane[13] = sx_rl(c->lane[13], 25);
    c->sched[29] = c->hash ^ sx_rl(c->lane[9], 23);
    c->lane[11] += c->lane[2]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 9);
    c->raw[c->slo + (int)((t0 + 9762u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x6b77a2a5u;
    t2 = (t2 ^ c->sum) * 0x8f42e65bu;
    c->sched[29] = c->hash ^ sx_rl(c->lane[13], 20);
    c->hash = (c->hash * 0x5e23e26du) ^ sx_rr(c->hash, 30);
    t2 = (t2 ^ c->sum) * 0xb9dfbd2bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t prime_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x40) << 16;
    c->lane[10] += c->lane[8]; c->lane[14] ^= c->lane[10]; c->lane[14] = sx_rl(c->lane[14], 20);
    c->hash ^= c->lane[0] + 0x561eea64u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[3] += c->lane[7] ^ 0x4b51eb8eu;
    c->lane[3] ^= sx_rl(c->lane[9], 15);
    c->hash = (c->hash * 0x1ae66ef3u) ^ sx_rr(c->hash, 30);
    c->lane[1] ^= sx_rl(c->lane[15], 18);
    c->sum += t1;
    return t0 + t2;
}

static void step_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25460u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48476u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[11] + 0xb3f78938u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf4) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int latch_cell(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[13] += c->lane[12]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x403c5419u;
    t2 = (t2 ^ c->sum) * 0xaa86091du;
    c->hash ^= c->lane[10] + 0xa170e156u;
    c->hash = (c->hash * 0x932a7409u) ^ sx_rr(c->hash, 23);
    t2 = (t2 ^ c->sum) * 0x2f2a69afu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd3) << 16;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sort_item(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[30] = c->hash ^ sx_rl(c->lane[9], 30);
    t1 ^= (uint32_t)blend_port(c, (uint8_t)(t0 >> 8), t2);
    c->lane[15] += c->lane[9]; c->lane[3] ^= c->lane[15]; c->lane[3] = sx_rl(c->lane[3], 16);
    t2 += scan_queue(c, c->slo, c->sln);
    t2 += scan_key(c, c->rlo, c->rln);
    t2 += (uint32_t)load_window(c);
    t2 += resize_bucket(c, c->slo, c->sln);
    t2 += align_head(c, c->rlo, c->rln);
    parse_digest(c, &c->lane[5], 1);
    t1 ^= (uint32_t)split_digest(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)place_group(c);
    t2 += (uint32_t)stage_scope(c);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32339u) % (uint32_t)c->rln)] << 24;
    c->lane[11] += c->lane[3] ^ 0x6f03adc2u;
    c->hash = (c->hash * 0x77006be9u) ^ sx_rr(c->hash, 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63739u) % (uint32_t)c->rln)] << 8;
    t0 ^= close_line(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x36b274dbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27643u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55688u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)scan_path(c);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t scan_queue(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)pick_page(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)sync_lease(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[5] + 0x9c4ead51u;
    t1 ^= (uint32_t)place_slot(c, (uint8_t)(t0 >> 16), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22442u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)sort_bound(c, (uint8_t)(t0 >> 8), t2);
    sort_band(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x48) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54885u) % (uint32_t)c->rln)] << 0;
    t2 += shift_group_235(c, c->slo, c->sln);
    tally_frame(c, &c->lane[4], 1);
    t2 += seek_field(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1289u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0xfddbc42bu) ^ sx_rr(c->hash, 23);
    t0 ^= blend_head(c, t1);
    c->lane[1] += c->lane[0]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 31);
    c->lane[7] += c->lane[15] ^ 0x2f11aca3u;
    c->sum += t1;
    return t0 + t2;
}

static int place_group(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 ^= poll_queue(c, t1);
    t1 ^= (uint32_t)scan_bucket(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5a) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfe83fbc3u;
    c->hash = (c->hash * 0x5c0e8ae9u) ^ sx_rr(c->hash, 3);
    t2 += sync_seat(c, c->slo, c->sln);
    t1 ^= (uint32_t)chain_unit(c, (uint8_t)(t0 >> 0), t2);
    c->sched[17] = c->hash ^ sx_rl(c->lane[14], 9);
    sort_gap(c, &c->lane[10], 1);
    c->lane[0] += c->lane[13]; c->lane[9] ^= c->lane[0]; c->lane[9] = sx_rl(c->lane[9], 28);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 4);
    t0 ^= push_segment(c, t1);
    c->lane[15] += c->lane[14]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb9630489u;
    align_scope(c, &c->lane[10], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x536e5bb5u;
    peek_rate(c, &c->lane[10], 4);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t align_head(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] += c->lane[14] ^ 0x64a07b64u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 4);
    t0 ^= mix_node(c, t1);
    t0 ^= hold_tuple(c, t1);
    t1 ^= (uint32_t)tune_token(c, (uint8_t)(t0 >> 8), t2);
    t2 += yield_port(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x05ce16c1u) ^ sx_rr(c->hash, 4);
    link_index(c, t0, t1);
    tune_line(c, &c->lane[9], 3);
    resize_scope(c, t0, t1);
    t0 ^= slice_block(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x6c) << 16;
    move_window(c, t0, t1);
    t0 ^= shift_mask(c, t1);
    t2 += (uint32_t)place_path(c);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t pick_page(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 2);
    t2 += (uint32_t)slice_page(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7f65c97du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17368u) % (uint32_t)c->rln)] << 8;
    c->lane[1] += c->lane[13] ^ 0x7c61a503u;
    c->lane[10] += c->lane[3]; c->lane[2] ^= c->lane[10]; c->lane[2] = sx_rl(c->lane[2], 28);
    fold_cell(c, &c->lane[6], 2);
    c->lane[2] ^= sx_rl(c->lane[6], 6);
    c->hash ^= c->lane[14] + 0x159f99b8u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t scan_bucket(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[11] + 0xa56ea31cu;
    c->lane[4] += c->lane[0] ^ 0xef4bac8bu;
    t2 += scan_key(c, c->rlo, c->rln);
    t1 ^= (uint32_t)load_span(c, (uint8_t)(t0 >> 8), t2);
    t2 += merge_range(c, c->slo, c->sln);
    t1 ^= (uint32_t)mark_delta(c, (uint8_t)(t0 >> 8), t2);
    c->lane[10] += c->lane[5]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 25);
    t2 += (uint32_t)load_window(c);
    t2 += merge_bound(c, c->slo, c->sln);
    wrap_tail(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28998u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t tune_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xaf7d9559u) ^ sx_rr(c->hash, 3);
    c->lane[10] ^= sx_rl(c->lane[15], 22);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += trim_level(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc22aa72du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x26) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7b506091u;
    t2 += (uint32_t)pick_index(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[14] += c->lane[13]; c->lane[4] ^= c->lane[14]; c->lane[4] = sx_rl(c->lane[4], 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31588u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x24095b11u) ^ sx_rr(c->hash, 23);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void resize_scope(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    step_level(c, t0, t1);
    t1 ^= (uint32_t)drain_head_151(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xbc) << 16;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7af3e5c9u;
    t2 = (t2 ^ c->sum) * 0x8ccc8fe3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6569u) % (uint32_t)c->rln)] << 24;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61477u) % (uint32_t)c->rln)] << 0;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void tune_line(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x95) << 8;
    t2 += (uint32_t)scan_path(c);
    c->lane[13] ^= sx_rl(c->lane[9], 8);
    c->lane[11] ^= sx_rl(c->lane[1], 27);
    c->hash ^= c->lane[3] + 0x28502386u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t sync_seat(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9283u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 30932u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4b) << 16;
    c->hash ^= c->lane[1] + 0x9aaaadc0u;
    t2 += defer_index(c, c->slo, c->sln);
    t2 += trim_batch(c, c->rlo, c->rln);
    parse_digest(c, &c->lane[7], 2);
    reap_range(c, &c->lane[6], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x74d4a7b9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[11] = c->hash ^ sx_rl(c->lane[5], 19);
    t2 += (uint32_t)flush_queue(c);
    t2 = (t2 ^ c->sum) * 0xf7cdea57u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15154u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 6655u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t mix_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)tune_rate(c);
    t2 += fill_state(c, c->slo, c->sln);
    t1 ^= (uint32_t)place_slot(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    align_digest(c, &c->lane[8], 4);
    c->hash = (c->hash * 0x89c9d1e9u) ^ sx_rr(c->hash, 16);
    t1 ^= (uint32_t)chain_unit(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += (uint32_t)load_group(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42284u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)stage_scope(c);
    t2 += hold_tail(c, c->slo, c->sln);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t slice_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2d67993fu;
    c->lane[7] += c->lane[1] ^ 0x26434d7cu;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[12] += c->lane[3]; c->lane[7] ^= c->lane[12]; c->lane[7] = sx_rl(c->lane[7], 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash = (c->hash * 0xbb14f36du) ^ sx_rr(c->hash, 9);
    c->lane[11] += c->lane[9]; c->lane[4] ^= c->lane[11]; c->lane[4] = sx_rl(c->lane[4], 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t sort_bound(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x884ee0f5u;
    c->hash = (c->hash * 0x846f47ebu) ^ sx_rr(c->hash, 5);
    c->lane[14] ^= sx_rl(c->lane[1], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x70301bc9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x91) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5829fbadu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x48) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8f6f36fbu;
    c->hash ^= c->lane[0] + 0x3f7a78aeu;
    c->lane[15] += c->lane[9] ^ 0xc75a0efeu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x222488bfu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t blend_head(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += fill_state(c, c->rlo, c->rln);
    t2 += (uint32_t)pick_index(c);
    c->raw[c->slo + (int)((t0 + 47518u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0xd11ffdfdu) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd0) << 0;
    scan_window(c, &c->lane[7], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)blend_port(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= resize_label(c, t1);
    t0 ^= fold_mask(c, t1);
    t2 = (t2 ^ c->sum) * 0x34630e75u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fold_mask(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 5);
    c->lane[5] ^= sx_rl(c->lane[4], 25);
    t2 += (uint32_t)sync_index(c);
    t1 ^= (uint32_t)prime_mask(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    reset_entry(c, t0, t1);
    c->lane[7] ^= sx_rl(c->lane[10], 7);
    c->lane[15] += c->lane[10]; c->lane[12] ^= c->lane[15]; c->lane[12] = sx_rl(c->lane[12], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57372u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void parse_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] += c->lane[6] ^ 0x97ccd3acu;
    c->hash = (c->hash * 0xebe039c1u) ^ sx_rr(c->hash, 1);
    c->raw[c->slo + (int)((t0 + 16681u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 58581u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 31300u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xba) << 16;
    t2 = (t2 ^ c->sum) * 0x703756d1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x9ee2ef4bu;
    c->lane[6] += c->lane[4] ^ 0x31293e6du;
    t2 += (uint32_t)patch_table(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t blend_port(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[16] = c->hash ^ sx_rl(c->lane[7], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4c) << 8;
    c->sched[2] = c->hash ^ sx_rl(c->lane[4], 10);
    tally_frame(c, &c->lane[6], 3);
    c->hash ^= c->lane[1] + 0x9cc62198u;
    c->lane[1] += c->lane[0] ^ 0x17b63c48u;
    t0 ^= move_node(c, t1);
    parse_run(c, &c->lane[8], 4);
    c->raw[c->slo + (int)((t0 + 15184u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[19] = c->hash ^ sx_rl(c->lane[1], 11);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void step_level(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    parse_run(c, &c->lane[3], 4);
    c->sched[2] = c->hash ^ sx_rl(c->lane[11], 5);
    c->hash ^= c->lane[3] + 0xcee6aad3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x78) << 0;
    t2 += (uint32_t)step_field(c);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 3);
    t2 += fold_key(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0fb31447u;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint8_t chain_unit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc4) << 8;
    c->lane[8] += c->lane[7]; c->lane[13] ^= c->lane[8]; c->lane[13] = sx_rl(c->lane[13], 28);
    c->hash = (c->hash * 0x0a426253u) ^ sx_rr(c->hash, 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb1) << 16;
    c->lane[10] += c->lane[3]; c->lane[13] ^= c->lane[10]; c->lane[13] = sx_rl(c->lane[13], 14);
    c->raw[c->slo + (int)((t0 + 20767u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[15] ^ 0x7d734ffcu;
    t0 ^= push_segment(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t scan_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += yield_port(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x7c0f4341u;
    t2 = (t2 ^ c->sum) * 0xbae853c3u;
    t2 += (uint32_t)fill_layer_168(c);
    t2 += (uint32_t)load_group(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd6) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2e8dd4d5u;
    c->lane[3] += c->lane[9] ^ 0xec96432au;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static uint32_t hold_tail(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[8] = c->hash ^ sx_rl(c->lane[2], 23);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 6);
    t2 += (uint32_t)link_unit(c);
    t2 += mix_label(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xad70c6bbu;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 27);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t mark_delta(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[17] = c->hash ^ sx_rl(c->lane[10], 14);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 20);
    c->raw[c->slo + (int)((t0 + 27544u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[6] += c->lane[4] ^ 0x4e888de6u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd1) << 16;
    c->lane[5] += c->lane[1]; c->lane[11] ^= c->lane[5]; c->lane[11] = sx_rl(c->lane[11], 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55424u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += trim_batch(c, c->slo, c->sln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void align_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    sort_limit(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 42621u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39670u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[6] + 0xd7ce1066u;
    c->hash = (c->hash * 0x660c36b7u) ^ sx_rr(c->hash, 19);
    c->lane[10] += c->lane[1]; c->lane[9] ^= c->lane[10]; c->lane[9] = sx_rl(c->lane[9], 19);
    t1 ^= (uint32_t)split_digest(c, (uint8_t)(t0 >> 8), t2);
    c->sched[26] = c->hash ^ sx_rl(c->lane[8], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x19) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51801u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t drain_head_151(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 35192u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    load_chunk(c, &c->lane[11], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xda8190d5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15492u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbecba6fbu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t fill_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf7) << 8;
    fold_cell(c, &c->lane[4], 2);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 26);
    c->hash = (c->hash * 0x0e696e17u) ^ sx_rr(c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[4] ^= sx_rl(c->lane[5], 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xff5f4b1fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[0] + 0xb75ca386u;
    move_window(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43532u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static int pick_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18908u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0xe3477821u;
    c->lane[15] += c->lane[11]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 9);
    t2 += trim_pool(c, c->slo, c->sln);
    c->hash = (c->hash * 0x9a55fa23u) ^ sx_rr(c->hash, 23);
    c->lane[13] ^= sx_rl(c->lane[8], 20);
    c->hash = (c->hash * 0x2bcfe35fu) ^ sx_rr(c->hash, 27);
    sort_limit(c, t0, t1);
    c->sched[25] = c->hash ^ sx_rl(c->lane[5], 6);
    c->lane[4] += c->lane[14]; c->lane[3] ^= c->lane[4]; c->lane[3] = sx_rl(c->lane[3], 11);
    c->lane[0] += c->lane[12] ^ 0xc2e443afu;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int stage_scope(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t2 += (uint32_t)purge_line(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30041u) % (uint32_t)c->rln)] << 0;
    t2 += (uint32_t)join_frame(c);
    t0 ^= hold_tuple(c, t1);
    c->lane[13] += c->lane[12]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 12);
    c->raw[c->slo + (int)((t0 + 58132u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 40423u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    prime_part(c, &c->lane[1], 3);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t trim_level(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20782u) % (uint32_t)c->rln)] << 24;
    t1 ^= (uint32_t)pin_item(c, (uint8_t)(t0 >> 16), t2);
    c->lane[15] += c->lane[3]; c->lane[6] ^= c->lane[15]; c->lane[6] = sx_rl(c->lane[6], 27);
    c->raw[c->slo + (int)((t0 + 32963u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 44038u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[2] += c->lane[5]; c->lane[13] ^= c->lane[2]; c->lane[13] = sx_rl(c->lane[13], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t1 ^= (uint32_t)sync_lease(c, (uint8_t)(t0 >> 16), t2);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t merge_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[7], 19);
    c->lane[13] += c->lane[14] ^ 0xab3b05a8u;
    sort_gap(c, &c->lane[5], 2);
    c->lane[11] += c->lane[14] ^ 0xd8b45e3au;
    c->hash = (c->hash * 0x8f88b043u) ^ sx_rr(c->hash, 25);
    push_stream(c, &c->lane[3], 3);
    c->lane[9] += c->lane[11] ^ 0x8f84ded1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x97) << 16;
    c->sum += t1;
    return t0 + t2;
}

static int load_window(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->lane[12] ^= sx_rl(c->lane[8], 21);
    t1 ^= (uint32_t)merge_row(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= c->lane[12] + 0x52abc593u;
    t1 ^= (uint32_t)load_span(c, (uint8_t)(t0 >> 8), t2);
    seek_batch(c, &c->lane[0], 1);
    c->hash ^= c->lane[13] + 0x35a02e6du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[9] += c->lane[15]; c->lane[13] ^= c->lane[9]; c->lane[13] = sx_rl(c->lane[13], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa4) << 0;
    t2 = (t2 ^ c->sum) * 0xe7219a4bu;
    c->sched[28] = c->hash ^ sx_rl(c->lane[14], 10);
    t2 += (uint32_t)fill_layer_168(c);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t place_slot(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59774u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0xb5d087ffu;
    t2 += resize_bucket(c, c->rlo, c->rln);
    c->hash ^= c->lane[8] + 0x599a61bdu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0b5dfe1fu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t merge_range(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x28705fc5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x375411e7u;
    c->sched[17] = c->hash ^ sx_rl(c->lane[15], 1);
    t2 = (t2 ^ c->sum) * 0x2a1c12cfu;
    c->lane[9] += c->lane[1]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe97f79e1u;
    t1 ^= (uint32_t)join_band(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[9] + 0x1d92229du;
    c->sum += t1;
    return t0 + t2;
}

static int scan_path(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    settle_queue(c, t0, t1);
    t0 ^= parse_gap(c, t1);
    c->lane[1] += c->lane[3] ^ 0x2aa5c7b8u;
    t1 ^= (uint32_t)blend_token(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0xf8ce34c7u) ^ sx_rr(c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    link_index(c, t0, t1);
    c->hash = (c->hash * 0xd51f09e3u) ^ sx_rr(c->hash, 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int slice_page(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    t0 ^= settle_frame(c, t1);
    c->lane[5] += c->lane[1]; c->lane[6] ^= c->lane[5]; c->lane[6] = sx_rl(c->lane[6], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19939u) % (uint32_t)c->rln)] << 8;
    c->lane[3] += c->lane[11]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x80687e3du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22936u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[5] + 0x5d0c4eebu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 24);
    t0 ^= load_run(c, t1);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t blend_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4253e95bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[2] += c->lane[2] ^ 0x1950a6afu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5f7dd14fu;
    c->sched[5] = c->hash ^ sx_rl(c->lane[0], 5);
    t0 ^= shift_mask(c, t1);
    c->hash ^= c->lane[0] + 0x9f1f56e8u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x24) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    parse_token(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void link_index(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[14] = c->hash ^ sx_rl(c->lane[5], 5);
    reap_level(c, t0, t1);
    align_table(c, &c->lane[11], 1);
    c->hash ^= c->lane[12] + 0x7c726b30u;
    c->hash ^= c->lane[0] + 0x7731dd73u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    resize_entry(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void tally_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 41427u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc6) << 8;
    c->sched[25] = c->hash ^ sx_rl(c->lane[11], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t1 ^= (uint32_t)split_entry(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x51) << 8;
    c->lane[8] += c->lane[4] ^ 0xb86a7c73u;
    t2 += coal_tail(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xad75c107u;
    c->lane[1] += c->lane[4]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 21);
    c->lane[10] += c->lane[0] ^ 0x830f81ecu;
    t0 ^= step_line(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void load_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += mix_label(c, c->rlo, c->rln);
    c->hash ^= c->lane[4] + 0x551a4ce5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[0] += c->lane[6] ^ 0x4d00c035u;
    c->lane[4] += c->lane[0] ^ 0xc7c33897u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa9) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1cc6a035u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc1c85523u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void parse_run(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4a) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x24abbb01u;
    wrap_tail(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t1 ^= (uint32_t)resize_table(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x8a8a770du;
    c->lane[14] += c->lane[13] ^ 0x3a178528u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void seek_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[21] = c->hash ^ sx_rl(c->lane[1], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    settle_queue(c, t0, t1);
    c->sched[7] = c->hash ^ sx_rl(c->lane[1], 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[14] += c->lane[7] ^ 0x0574b463u;
    c->lane[15] += c->lane[12]; c->lane[11] ^= c->lane[15]; c->lane[11] = sx_rl(c->lane[11], 15);
    t2 += (uint32_t)join_frame(c);
    c->raw[c->slo + (int)((t0 + 65469u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7965u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int fill_layer_168(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->lane[3] ^= sx_rl(c->lane[3], 10);
    c->lane[3] += c->lane[7]; c->lane[15] ^= c->lane[3]; c->lane[15] = sx_rl(c->lane[15], 10);
    c->lane[10] += c->lane[7] ^ 0xb8b30dd3u;
    c->lane[3] += c->lane[13]; c->lane[5] ^= c->lane[3]; c->lane[5] = sx_rl(c->lane[5], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += seek_field(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 6);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 21);
    t0 ^= settle_group(c, t1);
    c->lane[14] ^= sx_rl(c->lane[5], 16);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void move_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x714d9433u;
    c->hash = (c->hash * 0xa9277809u) ^ sx_rr(c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 10437u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x0fe7ab61u) ^ sx_rr(c->hash, 3);
    t2 += rotate_stream(c, c->rlo, c->rln);
    t0 ^= close_line(c, t1);
    c->hash ^= c->lane[5] + 0xd75cd4e9u;
    prime_part(c, &c->lane[3], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 264u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[3] ^= sx_rl(c->lane[10], 17);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t hold_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x48c7f615u;
    c->hash = (c->hash * 0xeb53e43fu) ^ sx_rr(c->hash, 28);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x27) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21621u) % (uint32_t)c->rln)] << 24;
    t1 ^= (uint32_t)merge_stream(c, (uint8_t)(t0 >> 16), t2);
    c->lane[15] += c->lane[6] ^ 0x7ced1853u;
    c->lane[13] += c->lane[7] ^ 0xaf93c050u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t trim_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x40a836c3u) ^ sx_rr(c->hash, 13);
    parse_ring(c, t0, t1);
    c->lane[5] += c->lane[13] ^ 0x18847aebu;
    stage_arena(c, &c->lane[2], 4);
    t2 = (t2 ^ c->sum) * 0x7c68eacdu;
    c->sched[14] = c->hash ^ sx_rl(c->lane[7], 9);
    c->sched[16] = c->hash ^ sx_rl(c->lane[7], 17);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t pin_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[1] += c->lane[1] ^ 0x73a2fe6au;
    c->hash = (c->hash * 0x25d889dfu) ^ sx_rr(c->hash, 4);
    t2 += (uint32_t)tune_rate(c);
    c->lane[2] ^= sx_rl(c->lane[4], 23);
    c->hash = (c->hash * 0xd5c14aa1u) ^ sx_rr(c->hash, 6);
    c->sched[6] = c->hash ^ sx_rl(c->lane[1], 17);
    c->lane[3] += c->lane[5]; c->lane[4] ^= c->lane[3]; c->lane[4] = sx_rl(c->lane[4], 27);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t split_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xec) << 0;
    c->raw[c->slo + (int)((t0 + 26762u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[1] + 0x6be99b01u;
    c->hash ^= c->lane[0] + 0xbc7d594eu;
    c->raw[c->slo + (int)((t0 + 44859u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[6] + 0x68d34cacu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t prime_mask(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xea380443u) ^ sx_rr(c->hash, 29);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 14);
    t2 = (t2 ^ c->sum) * 0x00d8eddfu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb1c8ab21u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t trim_pool(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 48879u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0xc2a17645u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfc8da36bu;
    c->hash = (c->hash * 0xdd06de33u) ^ sx_rr(c->hash, 28);
    t2 += (uint32_t)relay_range(c);
    t2 += (uint32_t)relay_arena(c);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t yield_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= c->lane[12] + 0xf4df6b8bu;
    c->lane[9] += c->lane[1] ^ 0xb603debeu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 3101u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x362aea75u) ^ sx_rr(c->hash, 28);
    c->raw[c->slo + (int)((t0 + 13615u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t push_segment(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 38374u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] ^= sx_rl(c->lane[4], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[3] += c->lane[1]; c->lane[7] ^= c->lane[3]; c->lane[7] = sx_rl(c->lane[7], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8582u) % (uint32_t)c->rln)] << 16;
    c->lane[8] ^= sx_rl(c->lane[10], 12);
    c->lane[12] += c->lane[0]; c->lane[7] ^= c->lane[12]; c->lane[7] = sx_rl(c->lane[7], 19);
    scan_window(c, &c->lane[1], 4);
    c->lane[3] += c->lane[13] ^ 0x49d34950u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void sort_gap(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 9);
    t2 = (t2 ^ c->sum) * 0x3bc000bdu;
    t2 = (t2 ^ c->sum) * 0x36f54b59u;
    c->lane[2] ^= sx_rl(c->lane[2], 26);
    c->raw[c->slo + (int)((t0 + 57110u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[4] + 0xe2902722u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8f) << 16;
    c->lane[11] ^= sx_rl(c->lane[9], 4);
    t0 ^= merge_page(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25255u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xee) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t move_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[0] += c->lane[3] ^ 0x74322fd8u;
    c->raw[c->slo + (int)((t0 + 19034u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x4004ff2du) ^ sx_rr(c->hash, 27);
    c->lane[14] += c->lane[7]; c->lane[9] ^= c->lane[14]; c->lane[9] = sx_rl(c->lane[9], 23);
    c->hash ^= c->lane[0] + 0xf19a6b3au;
    c->lane[6] += c->lane[8]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 27);
    c->lane[11] += c->lane[8]; c->lane[1] ^= c->lane[11]; c->lane[1] = sx_rl(c->lane[1], 5);
    t0 ^= reset_tail(c, t1);
    t2 += fold_key(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8a22f1c5u;
    c->hash = (c->hash * 0x2f5ac83fu) ^ sx_rr(c->hash, 5);
    move_delta(c, &c->lane[8], 4);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void push_stream(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    probe_frame(c, &c->lane[10], 2);
    c->lane[12] ^= sx_rl(c->lane[14], 6);
    c->lane[0] ^= sx_rl(c->lane[14], 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2f5c115fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc52d7789u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8e2192bfu;
    t1 ^= (uint32_t)resize_level(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)load_node(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t join_band(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1be5766bu;
    c->lane[14] ^= sx_rl(c->lane[0], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe2) << 0;
    t2 = (t2 ^ c->sum) * 0xd837539du;
    sort_band(c, t0, t1);
    store_pairing(c, &c->lane[10], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1d2bf977u;
    c->lane[5] ^= sx_rl(c->lane[14], 22);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t settle_frame(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)flush_queue(c);
    t2 = (t2 ^ c->sum) * 0xf73dfb63u;
    c->lane[10] += c->lane[15]; c->lane[14] ^= c->lane[10]; c->lane[14] = sx_rl(c->lane[14], 21);
    c->sched[10] = c->hash ^ sx_rl(c->lane[2], 12);
    c->lane[12] += c->lane[6]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 17);
    reap_level(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t load_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[12] = c->hash ^ sx_rl(c->lane[11], 5);
    c->lane[4] += c->lane[0]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 28);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 8);
    c->lane[14] ^= sx_rl(c->lane[9], 5);
    c->raw[c->slo + (int)((t0 + 31113u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[2] ^ 0x0cf7c823u;
    c->lane[8] += c->lane[1] ^ 0x7e35fb2fu;
    t2 += (uint32_t)seek_seat(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0xaf4051bfu;
    t0 ^= settle_group(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int step_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5992u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xbc) << 16;
    c->raw[c->slo + (int)((t0 + 52869u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x63) << 16;
    peek_rate(c, &c->lane[10], 2);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int load_group(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8a) << 8;
    t2 += push_list(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa2) << 8;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    c->sched[6] = c->hash ^ sx_rl(c->lane[9], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 28);
    c->hash ^= c->lane[3] + 0x9b9c85fcu;
    blend_range(c, &c->lane[1], 2);
    c->lane[7] ^= sx_rl(c->lane[7], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    tally_entry(c, &c->lane[5], 1);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int purge_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x54) << 8;
    c->raw[c->slo + (int)((t0 + 61095u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[6] ^ 0xc48f9977u;
    c->lane[14] += c->lane[13]; c->lane[15] ^= c->lane[14]; c->lane[15] = sx_rl(c->lane[15], 22);
    c->raw[c->slo + (int)((t0 + 44277u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 9145u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x7d103d55u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe041d955u;
    t2 += chain_digest_201(c, c->rlo, c->rln);
    t2 += defer_index(c, c->rlo, c->rln);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 12);
    c->raw[c->slo + (int)((t0 + 44514u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int sync_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xac36d1d7u;
    c->sched[13] = c->hash ^ sx_rl(c->lane[5], 10);
    c->raw[c->slo + (int)((t0 + 13827u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    mark_node(c, &c->lane[9], 2);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int patch_table(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x04fae093u;
    c->lane[13] ^= sx_rl(c->lane[6], 31);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 14);
    c->sched[18] = c->hash ^ sx_rl(c->lane[13], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe9) << 0;
    t2 = (t2 ^ c->sum) * 0x40919a09u;
    c->hash = (c->hash * 0xc4987af5u) ^ sx_rr(c->hash, 24);
    c->lane[4] ^= sx_rl(c->lane[3], 3);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t load_run(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 1);
    fold_line(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcd) << 0;
    c->lane[3] += c->lane[14] ^ 0x619ecd3fu;
    t2 = (t2 ^ c->sum) * 0x29039605u;
    c->hash ^= c->lane[1] + 0x88c16b42u;
    c->lane[7] += c->lane[13]; c->lane[6] ^= c->lane[7]; c->lane[6] = sx_rl(c->lane[6], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[14] + 0x901ba2ddu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] ^= sx_rl(c->lane[9], 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t merge_row(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 ^= resize_label(c, t1);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe9) << 16;
    c->sched[29] = c->hash ^ sx_rl(c->lane[9], 8);
    c->sched[20] = c->hash ^ sx_rl(c->lane[8], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void sort_limit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 11960u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x748eed93u;
    t2 = (t2 ^ c->sum) * 0xaea3115du;
    c->lane[6] ^= sx_rl(c->lane[2], 28);
    t2 += shift_group_235(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 14);
    patch_track(c, &c->lane[3], 3);
    t0 ^= poll_queue(c, t1);
    c->hash ^= c->lane[15] + 0x5a0caa65u;
    c->hash = (c->hash * 0x6ea3a255u) ^ sx_rr(c->hash, 27);
    c->lane[2] += c->lane[9]; c->lane[15] ^= c->lane[2]; c->lane[15] = sx_rl(c->lane[15], 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void fold_cell(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x0cbbf6e3u;
    c->lane[7] += c->lane[15] ^ 0x5d768bd1u;
    mark_page(c, &c->lane[0], 3);
    c->lane[2] += c->lane[9]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43335u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x14) << 16;
    c->hash = (c->hash * 0x6a0e4f65u) ^ sx_rr(c->hash, 10);
    c->lane[2] += c->lane[7] ^ 0x887be06eu;
    t2 = (t2 ^ c->sum) * 0x4794b7c9u;
    align_scope(c, &c->lane[3], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0f890649u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t sync_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4ed1c14du;
    c->hash = (c->hash * 0x47ebe70fu) ^ sx_rr(c->hash, 3);
    c->lane[13] += c->lane[4]; c->lane[11] ^= c->lane[13]; c->lane[11] = sx_rl(c->lane[11], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] += c->lane[0]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 21);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void reset_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)trim_value(c, (uint8_t)(t0 >> 16), t2);
    pair_head(c, t0, t1);
    c->sched[26] = c->hash ^ sx_rl(c->lane[1], 26);
    c->raw[c->slo + (int)((t0 + 55916u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)link_unit(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    sort_chunk(c, &c->lane[9], 3);
    c->raw[c->slo + (int)((t0 + 7658u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t resize_bucket(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xfee882dfu) ^ sx_rr(c->hash, 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x39) << 0;
    c->lane[15] += c->lane[7]; c->lane[12] ^= c->lane[15]; c->lane[12] = sx_rl(c->lane[12], 26);
    c->lane[9] += c->lane[9] ^ 0x1261091fu;
    c->lane[10] += c->lane[11]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 17);
    c->lane[13] += c->lane[12]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 31);
    c->lane[15] ^= sx_rl(c->lane[2], 1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 28);
    c->hash ^= c->lane[4] + 0xd3bf1bf6u;
    mark_page(c, &c->lane[0], 4);
    c->lane[1] += c->lane[12] ^ 0x4c400f8eu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t parse_gap(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= reap_ring(c, t1);
    c->lane[13] += c->lane[14] ^ 0x149563b3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x35) << 16;
    reap_range(c, &c->lane[4], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe7f22bd3u;
    t2 += hold_label(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t2 += (uint32_t)merge_digest(c);
    t2 += (uint32_t)place_path(c);
    c->raw[c->slo + (int)((t0 + 18336u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1746u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int seek_seat(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x14dace69u;
    c->lane[7] ^= sx_rl(c->lane[0], 2);
    c->lane[12] += c->lane[5]; c->lane[11] ^= c->lane[12]; c->lane[11] = sx_rl(c->lane[11], 28);
    c->hash ^= c->lane[10] + 0x2b56cddbu;
    c->hash ^= c->lane[4] + 0x03834379u;
    t2 = (t2 ^ c->sum) * 0xcdf69109u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19412u) % (uint32_t)c->rln)] << 8;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t poll_queue(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x2515f5b9u) ^ sx_rr(c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 15);
    c->lane[11] += c->lane[3]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 9);
    t2 = (t2 ^ c->sum) * 0x14b7947fu;
    c->lane[6] += c->lane[4]; c->lane[14] ^= c->lane[6]; c->lane[14] = sx_rl(c->lane[14], 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x27b3c7f3u;
    c->lane[6] += c->lane[11]; c->lane[14] ^= c->lane[6]; c->lane[14] = sx_rl(c->lane[14], 17);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 24);
    c->sched[7] = c->hash ^ sx_rl(c->lane[15], 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int join_frame(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 31);
    c->hash = (c->hash * 0x6c49f44du) ^ sx_rr(c->hash, 22);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[13] += c->lane[0]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 29);
    c->lane[7] ^= sx_rl(c->lane[2], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x26) << 8;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t push_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x0af25cadu) ^ sx_rr(c->hash, 16);
    c->sched[3] = c->hash ^ sx_rl(c->lane[7], 6);
    c->lane[1] ^= sx_rl(c->lane[4], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[11] += c->lane[4] ^ 0xe3966b88u;
    c->sched[3] = c->hash ^ sx_rl(c->lane[8], 16);
    c->hash = (c->hash * 0x4ef581ffu) ^ sx_rr(c->hash, 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xa7efcaedu;
    c->hash ^= c->lane[5] + 0x4a9289dcu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t chain_digest_201(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 46330u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[2] += c->lane[7] ^ 0x92e268feu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 9);
    c->sum += t1;
    return t0 + t2;
}

static int merge_digest(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 64239u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16214u) % (uint32_t)c->rln)] << 24;
    c->lane[0] ^= sx_rl(c->lane[13], 5);
    c->hash = (c->hash * 0x34d0ea81u) ^ sx_rr(c->hash, 21);
    c->hash = (c->hash * 0x55d78935u) ^ sx_rr(c->hash, 30);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 28);
    c->hash = (c->hash * 0x3a7e2a2du) ^ sx_rr(c->hash, 30);
    c->lane[2] += c->lane[7]; c->lane[15] ^= c->lane[2]; c->lane[15] = sx_rl(c->lane[15], 27);
    c->hash ^= c->lane[2] + 0x1b05145fu;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void settle_queue(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0xe8583e1du) ^ sx_rr(c->hash, 19);
    c->lane[2] += c->lane[9] ^ 0xf7411b5eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xdf) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4811u) % (uint32_t)c->rln)] << 16;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static void resize_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 21754u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 3553u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 41246u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x1767eb59u;
    c->lane[6] += c->lane[2] ^ 0x7d1ab803u;
    c->lane[4] ^= sx_rl(c->lane[0], 17);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 7);
    t1 ^= (uint32_t)latch_arena(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xfe) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x02) << 16;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void sort_band(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x9049859fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x2e) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3c) << 8;
    c->raw[c->slo + (int)((t0 + 34424u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[6] = c->hash ^ sx_rl(c->lane[12], 18);
    c->hash ^= c->lane[15] + 0x07ac9dc5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x37ccfdbdu;
    c->hash = (c->hash * 0xa19a8f4du) ^ sx_rr(c->hash, 26);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void scan_window(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30048u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 26602u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] += c->lane[6]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 29);
    t2 = (t2 ^ c->sum) * 0x6dfd44e1u;
    t2 = (t2 ^ c->sum) * 0x61f22195u;
    t2 = (t2 ^ c->sum) * 0x7de35353u;
    c->hash ^= c->lane[7] + 0xd6d9b86au;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int flush_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[2] += c->lane[9]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa7889735u;
    c->hash ^= c->lane[10] + 0x5983d7afu;
    c->sched[30] = c->hash ^ sx_rl(c->lane[4], 6);
    c->raw[c->slo + (int)((t0 + 7945u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[5] += c->lane[11] ^ 0xd96bf714u;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t fold_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1858d5c1u;
    c->hash ^= c->lane[7] + 0x7e9e501du;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x885d6011u;
    c->lane[13] += c->lane[14] ^ 0x875b1adau;
    t2 = (t2 ^ c->sum) * 0x1d6fed25u;
    t2 = (t2 ^ c->sum) * 0x8b9739efu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2319u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 30719u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t shift_mask(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xbc70e12bu) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x07) << 8;
    c->hash ^= c->lane[9] + 0x7d418049u;
    c->lane[6] += c->lane[14]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 18);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t rotate_stream(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xfc013801u) ^ sx_rr(c->hash, 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xee) << 8;
    c->lane[15] += c->lane[14] ^ 0x911512eeu;
    c->raw[c->slo + (int)((t0 + 45938u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += trim_item(c, c->slo, c->sln);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 30);
    t2 = (t2 ^ c->sum) * 0x6c8bdb25u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void align_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] += c->lane[0] ^ 0xfaf386c4u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 9);
    c->lane[13] += c->lane[9]; c->lane[11] ^= c->lane[13]; c->lane[11] = sx_rl(c->lane[11], 17);
    c->lane[7] ^= sx_rl(c->lane[7], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[3] ^= sx_rl(c->lane[3], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa926abb3u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t settle_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x68a0faf5u;
    c->hash = (c->hash * 0x70be8f19u) ^ sx_rr(c->hash, 23);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 2);
    t2 = (t2 ^ c->sum) * 0x30356e55u;
    c->raw[c->slo + (int)((t0 + 52408u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[14] += c->lane[8]; c->lane[3] ^= c->lane[14]; c->lane[3] = sx_rl(c->lane[3], 8);
    c->lane[6] ^= sx_rl(c->lane[6], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x32) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tally_entry(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18715u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[20] = c->hash ^ sx_rl(c->lane[6], 27);
    c->raw[c->slo + (int)((t0 + 50761u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sched[30] = c->hash ^ sx_rl(c->lane[1], 2);
    c->hash = (c->hash * 0x7e742c31u) ^ sx_rr(c->hash, 1);
    c->hash = (c->hash * 0x0410ecb3u) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x35) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void wrap_tail(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27913u) % (uint32_t)c->rln)] << 24;
    c->lane[4] += c->lane[12]; c->lane[14] ^= c->lane[4]; c->lane[14] = sx_rl(c->lane[14], 21);
    c->sched[19] = c->hash ^ sx_rl(c->lane[5], 26);
    c->lane[15] += c->lane[8] ^ 0x629bf50fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xdf72f3e3u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[6], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x578c6315u;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t merge_page(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[31] = c->hash ^ sx_rl(c->lane[10], 25);
    c->hash = (c->hash * 0xccb998cdu) ^ sx_rr(c->hash, 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8fef1bf3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2c) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53393u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0xba64c4c5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x05781981u) ^ sx_rr(c->hash, 6);
    c->sched[2] = c->hash ^ sx_rl(c->lane[6], 12);
    c->lane[7] ^= sx_rl(c->lane[6], 27);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t seek_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x41279a09u;
    t2 = (t2 ^ c->sum) * 0xa8e3bb49u;
    c->hash ^= c->lane[0] + 0x0732daa5u;
    c->lane[9] += c->lane[7] ^ 0x43c410feu;
    c->hash ^= c->lane[10] + 0xa71e14cau;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16117u) % (uint32_t)c->rln)] << 24;
    c->sched[1] = c->hash ^ sx_rl(c->lane[15], 26);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t trim_value(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xaa046589u) ^ sx_rr(c->hash, 5);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 22);
    c->hash ^= c->lane[9] + 0x89aa6701u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sched[17] = c->hash ^ sx_rl(c->lane[11], 15);
    t2 = (t2 ^ c->sum) * 0x1993a10du;
    c->lane[15] ^= sx_rl(c->lane[7], 27);
    c->lane[4] ^= sx_rl(c->lane[2], 28);
    c->lane[6] += c->lane[7] ^ 0x4edd2c47u;
    c->raw[c->slo + (int)((t0 + 5761u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void reap_range(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[3] + 0xfb9cd0e1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[15] += c->lane[4]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 6);
    c->lane[7] += c->lane[5] ^ 0x5613bea8u;
    c->lane[10] += c->lane[13] ^ 0x5bac2446u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb276115fu;
    c->raw[c->slo + (int)((t0 + 44688u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 8);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 31);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void sort_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43185u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40431u) % (uint32_t)c->rln)] << 16;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void peek_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x8e5aafb7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8fda0cadu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)mark_label(c, (uint8_t)(t0 >> 8), t2);
    c->sched[20] = c->hash ^ sx_rl(c->lane[9], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x06) << 16;
    c->raw[c->slo + (int)((t0 + 28799u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void mark_node(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[8], 1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sched[17] = c->hash ^ sx_rl(c->lane[10], 26);
    c->raw[c->slo + (int)((t0 + 10516u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x792b894bu;
    c->lane[1] ^= sx_rl(c->lane[6], 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void pair_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 4);
    c->raw[c->slo + (int)((t0 + 23896u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] += c->lane[4] ^ 0xd4ac3f7fu;
    c->lane[14] += c->lane[7]; c->lane[15] ^= c->lane[14]; c->lane[15] = sx_rl(c->lane[15], 3);
    t2 = (t2 ^ c->sum) * 0x37879017u;
    c->lane[5] += c->lane[12]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x791902ebu;
    c->hash = (c->hash * 0xc942933du) ^ sx_rr(c->hash, 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 41309u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint8_t merge_stream(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33852u) % (uint32_t)c->rln)] << 16;
    c->lane[9] ^= sx_rl(c->lane[15], 6);
    c->lane[12] += c->lane[15] ^ 0xefd810f1u;
    c->hash ^= c->lane[12] + 0x05debcf1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t close_line(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 15004u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[13] + 0xb83a00fcu;
    c->lane[0] += c->lane[12] ^ 0xe65705e4u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa6) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 45322u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[23] = c->hash ^ sx_rl(c->lane[9], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1782710du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t resize_label(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd0) << 16;
    c->lane[13] += c->lane[3] ^ 0xb6b438c2u;
    t0 ^= drain_table(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60214u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xaa557dd5u;
    c->hash ^= c->lane[7] + 0xf4e22829u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void reap_level(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 12);
    c->raw[c->slo + (int)((t0 + 29162u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[0] ^= sx_rl(c->lane[15], 1);
    t1 ^= (uint32_t)swap_state(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[12] + 0xfa71adb0u;
    c->hash ^= c->lane[0] + 0xef645d18u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 10157u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x425fc911u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb1f877e7u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static int load_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[9] += c->lane[8]; c->lane[1] ^= c->lane[9]; c->lane[1] = sx_rl(c->lane[1], 25);
    c->hash ^= c->lane[5] + 0x424c7d7du;
    c->raw[c->slo + (int)((t0 + 31221u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x56) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbed5e273u;
    c->lane[9] ^= sx_rl(c->lane[2], 10);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t hold_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 53006u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x7f799601u) ^ sx_rr(c->hash, 9);
    c->raw[c->slo + (int)((t0 + 50298u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[8] = c->hash ^ sx_rl(c->lane[2], 13);
    c->raw[c->slo + (int)((t0 + 34672u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[6]; c->lane[5] ^= c->lane[11]; c->lane[5] = sx_rl(c->lane[5], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x31a5756du) ^ sx_rr(c->hash, 6);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t step_line(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[25] = c->hash ^ sx_rl(c->lane[8], 12);
    c->raw[c->slo + (int)((t0 + 21279u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x694cd2e5u) ^ sx_rr(c->hash, 23);
    c->lane[9] ^= sx_rl(c->lane[14], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25411u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2811u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6498cc6bu;
    c->sched[8] = c->hash ^ sx_rl(c->lane[9], 29);
    c->lane[15] ^= sx_rl(c->lane[15], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0xedf754b9u) ^ sx_rr(c->hash, 5);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t reap_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] ^= sx_rl(c->lane[6], 17);
    c->lane[5] += c->lane[1] ^ 0xdc3b4ab0u;
    t2 = (t2 ^ c->sum) * 0x47f53b79u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59375u) % (uint32_t)c->rln)] << 16;
    c->lane[9] += c->lane[7] ^ 0x33f52ec3u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t resize_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[14] ^ 0xc4723dd5u;
    c->lane[11] += c->lane[8]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 27);
    c->lane[2] += c->lane[8] ^ 0xb1e61601u;
    c->hash = (c->hash * 0xb081c219u) ^ sx_rr(c->hash, 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int link_unit(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= c->lane[8] + 0x5c472bd7u;
    c->lane[14] += c->lane[2]; c->lane[4] ^= c->lane[14]; c->lane[4] = sx_rl(c->lane[4], 14);
    c->hash = (c->hash * 0xf8d65953u) ^ sx_rr(c->hash, 7);
    c->raw[c->slo + (int)((t0 + 26344u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 12);
    c->sched[17] = c->hash ^ sx_rl(c->lane[2], 16);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t coal_tail(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[4]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x78217d8fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[10] = c->hash ^ sx_rl(c->lane[8], 20);
    c->sum += t1;
    return t0 + t2;
}

static void blend_range(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] += c->lane[13]; c->lane[12] ^= c->lane[7]; c->lane[12] = sx_rl(c->lane[12], 19);
    c->sched[19] = c->hash ^ sx_rl(c->lane[12], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0xbda00199u) ^ sx_rr(c->hash, 23);
    t2 = (t2 ^ c->sum) * 0xe769c795u;
    c->raw[c->slo + (int)((t0 + 61034u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25673u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 11264u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t shift_group_235(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x8aa10a67u) ^ sx_rr(c->hash, 25);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20995u) % (uint32_t)c->rln)] << 0;
    c->lane[12] += c->lane[1]; c->lane[9] ^= c->lane[12]; c->lane[9] = sx_rl(c->lane[9], 16);
    c->sum += t1;
    return t0 + t2;
}

static void align_table(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd8) << 8;
    c->hash ^= c->lane[6] + 0xe66b21c7u;
    c->lane[0] += c->lane[10]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 27);
    t2 = (t2 ^ c->sum) * 0xb7df5657u;
    c->hash = (c->hash * 0x715879e1u) ^ sx_rr(c->hash, 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void stage_arena(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xe3b2ddbdu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[14] += c->lane[12]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 2);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x110e7643u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void patch_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xad) << 0;
    t2 = (t2 ^ c->sum) * 0xccc874c3u;
    c->lane[2] += c->lane[12]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 28);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 48600u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x65) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void fold_line(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60092u) % (uint32_t)c->rln)] << 24;
    c->lane[5] += c->lane[14]; c->lane[13] ^= c->lane[5]; c->lane[13] = sx_rl(c->lane[13], 9);
    c->lane[12] ^= sx_rl(c->lane[12], 2);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb5) << 16;
    c->sched[28] = c->hash ^ sx_rl(c->lane[10], 31);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t resize_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4a) << 8;
    c->sched[28] = c->hash ^ sx_rl(c->lane[5], 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[13] ^= sx_rl(c->lane[7], 21);
    c->lane[1] += c->lane[2]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 22);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    c->lane[6] += c->lane[3]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 26);
    t2 = (t2 ^ c->sum) * 0x371421c9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49003u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbcfc7951u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void store_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[15] += c->lane[9] ^ 0x91e7f533u;
    c->lane[7] += c->lane[14] ^ 0x80530108u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t defer_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4290u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23279u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5b2ee8bfu;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb9543451u;
    c->hash ^= c->lane[3] + 0x25f0ef7au;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] += c->lane[9] ^ 0x51a974fau;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc8c8b885u;
    c->sum += t1;
    return t0 + t2;
}

static void parse_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x13) << 8;
    c->lane[13] ^= sx_rl(c->lane[10], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46482u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31684u) % (uint32_t)c->rln)] << 24;
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static int relay_arena(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 10);
    c->lane[13] ^= sx_rl(c->lane[4], 10);
    c->hash ^= c->lane[11] + 0x461cab6au;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xb82056ddu) ^ sx_rr(c->hash, 4);
    c->hash = (c->hash * 0xe80115b9u) ^ sx_rr(c->hash, 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 61829u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] += c->lane[3]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[9] ^= sx_rl(c->lane[0], 12);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void parse_token(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x4ee20b05u) ^ sx_rr(c->hash, 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe7) << 8;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xdbea03bbu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static int tune_rate(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf0d9c0c5u;
    c->sched[6] = c->hash ^ sx_rl(c->lane[13], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7cd926b7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void prime_part(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[24] = c->hash ^ sx_rl(c->lane[12], 11);
    c->raw[c->slo + (int)((t0 + 48520u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 33472u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] ^= sx_rl(c->lane[1], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void probe_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 27140u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[5] += c->lane[6] ^ 0x24d3034eu;
    c->raw[c->slo + (int)((t0 + 2204u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4901u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56204u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb6) << 8;
    c->lane[3] += c->lane[5]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39121u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 37038u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[9] += c->lane[10]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 13);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void move_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t1 ^= (uint32_t)mark_label(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x07) << 8;
    c->sched[8] = c->hash ^ sx_rl(c->lane[12], 9);
    c->raw[c->slo + (int)((t0 + 63853u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 39072u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x815f0c6du) ^ sx_rr(c->hash, 20);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int relay_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sched[29] = c->hash ^ sx_rl(c->lane[15], 22);
    c->lane[6] += c->lane[11]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 2);
    c->lane[15] ^= sx_rl(c->lane[3], 31);
    c->lane[10] ^= sx_rl(c->lane[2], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10708u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[2] + 0x172a74a0u;
    c->raw[c->slo + (int)((t0 + 5651u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 64610u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[5] ^= sx_rl(c->lane[3], 28);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t reset_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x57) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x6f) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t split_entry(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 11455u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x4276002fu) ^ sx_rr(c->hash, 2);
    c->lane[4] += c->lane[6] ^ 0x7c1290dfu;
    c->lane[14] += c->lane[14] ^ 0x118b37c9u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t mix_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[4] + 0xe931a35bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[1] += c->lane[14]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 12);
    c->hash = (c->hash * 0x3e54d253u) ^ sx_rr(c->hash, 8);
    c->sum += t1;
    return t0 + t2;
}

static void mark_page(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[14]; c->lane[7] ^= c->lane[1]; c->lane[7] = sx_rl(c->lane[7], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x29) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x571d6d2bu;
    c->hash ^= c->lane[4] + 0x5246a83du;
    c->lane[1] += c->lane[9]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 20);
    c->lane[6] ^= sx_rl(c->lane[7], 8);
    c->hash ^= c->lane[8] + 0x89cb2215u;
    c->hash = (c->hash * 0xbf816dcdu) ^ sx_rr(c->hash, 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int place_path(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t2 = (t2 ^ c->sum) * 0x99263195u;
    c->hash = (c->hash * 0x75908111u) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x37ed90e5u;
    c->lane[8] ^= sx_rl(c->lane[13], 21);
    c->raw[c->slo + (int)((t0 + 49418u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47340u) % (uint32_t)c->rln)] << 24;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tune_bucket(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)pack_pairing(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)shift_range(c, (uint8_t)(t0 >> 16), t2);
    t2 += step_scope(c, c->rlo, c->rln);
    t0 ^= wrap_region(c, t1);
    t1 ^= (uint32_t)shift_store(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[12] + 0x1dbe76eau;
    merge_band(c, t0, t1);
    t1 ^= (uint32_t)close_port(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)link_queue(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= peek_lease(c, t1);
    t2 += seek_tail(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x39161967u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17524u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)sift_stream(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= yield_segment(c, t1);
    drain_track(c, &c->lane[2], 4);
    t2 = (t2 ^ c->sum) * 0x78a5f9dbu;
    t2 = (t2 ^ c->sum) * 0x25510237u;
    t2 += (uint32_t)sift_region(c);
    t2 += flush_level(c, c->slo, c->sln);
    t2 += (uint32_t)drain_cell(c);
    c->hash ^= c->lane[7] + 0xfd077a7fu;
    t2 = (t2 ^ c->sum) * 0xb6c2cc11u;
    c->lane[10] += c->lane[6]; c->lane[12] ^= c->lane[10]; c->lane[12] = sx_rl(c->lane[12], 21);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t step_scope(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[1], 23);
    c->lane[5] ^= sx_rl(c->lane[11], 22);
    pair_cursor(c, &c->lane[0], 3);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 24);
    t1 ^= (uint32_t)link_track(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)peek_mask(c);
    pair_queue(c, &c->lane[7], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4ec614efu;
    t0 ^= cache_ring(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash = (c->hash * 0xdd49f851u) ^ sx_rr(c->hash, 17);
    c->lane[9] += c->lane[6] ^ 0x6959371bu;
    c->hash ^= c->lane[13] + 0xf30d0470u;
    c->sched[17] = c->hash ^ sx_rl(c->lane[6], 16);
    t1 ^= (uint32_t)merge_cell(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 30);
    t0 ^= parse_level(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum += t1;
    return t0 + t2;
}

static int drain_cell(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t2 += (uint32_t)latch_page(c);
    c->lane[13] += c->lane[10] ^ 0xee7cac7du;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 18);
    join_band_268(c, &c->lane[8], 1);
    load_layer(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd3) << 16;
    t2 += queue_track(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 4121u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    step_digest(c, t0, t1);
    c->lane[11] += c->lane[12]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 2);
    t0 ^= rotate_queue(c, t1);
    t2 += pair_track(c, c->rlo, c->rln);
    t2 += prime_key(c, c->rlo, c->rln);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t wrap_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9844u) % (uint32_t)c->rln)] << 8;
    t0 ^= merge_state(c, t1);
    c->hash = (c->hash * 0x24cc6d9bu) ^ sx_rr(c->hash, 17);
    c->hash = (c->hash * 0xdaa3942bu) ^ sx_rr(c->hash, 13);
    t2 += reset_page(c, c->rlo, c->rln);
    t1 ^= (uint32_t)fold_window(c, (uint8_t)(t0 >> 16), t2);
    c->raw[c->slo + (int)((t0 + 26390u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[6] += c->lane[10]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 12);
    tally_field(c, t0, t1);
    c->lane[11] += c->lane[5]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 4);
    t1 ^= (uint32_t)emit_run(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x97) << 8;
    t1 ^= (uint32_t)cache_table(c, (uint8_t)(t0 >> 0), t2);
    pick_run(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xa53222f7u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pair_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    coal_region(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[3] + 0xfcd2513fu;
    c->hash ^= c->lane[3] + 0x2a08aafcu;
    t2 = (t2 ^ c->sum) * 0xca83e2b1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= c->lane[14] + 0xc963bde0u;
    c->lane[14] ^= sx_rl(c->lane[3], 12);
    t2 += seek_tail(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26992u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t emit_run(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x3e) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8420u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0xf64c75b5u;
    t2 += (uint32_t)pick_head(c);
    t2 = (t2 ^ c->sum) * 0xef831f3fu;
    t1 ^= (uint32_t)shift_store(c, (uint8_t)(t0 >> 8), t2);
    purge_digest(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t link_track(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[21] = c->hash ^ sx_rl(c->lane[11], 1);
    t0 ^= place_layer(c, t1);
    t0 ^= mark_part(c, t1);
    c->lane[12] ^= sx_rl(c->lane[8], 2);
    c->sched[31] = c->hash ^ sx_rl(c->lane[3], 11);
    c->sched[29] = c->hash ^ sx_rl(c->lane[12], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 += reset_part(c, c->rlo, c->rln);
    t2 += (uint32_t)sync_range(c);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 5);
    t0 ^= place_field(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void pick_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 12);
    t1 ^= (uint32_t)shift_range(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x97) << 16;
    t1 ^= (uint32_t)fold_layer(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= parse_level(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 += pair_index(c, c->slo, c->sln);
    c->lane[1] ^= sx_rl(c->lane[5], 11);
    c->lane[14] += c->lane[9]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 19);
    grow_store(c, t0, t1);
    yield_window(c, &c->lane[9], 3);
    t2 += (uint32_t)sync_range(c);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static int peek_mask(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash = (c->hash * 0x30133befu) ^ sx_rr(c->hash, 15);
    t2 += (uint32_t)merge_pairing(c);
    c->hash = (c->hash * 0x632c3867u) ^ sx_rr(c->hash, 2);
    t2 += (uint32_t)clamp_cell(c);
    c->lane[15] += c->lane[10]; c->lane[11] ^= c->lane[15]; c->lane[11] = sx_rl(c->lane[11], 24);
    t2 += pair_index(c, c->slo, c->sln);
    c->hash = (c->hash * 0x19568cfbu) ^ sx_rr(c->hash, 27);
    t1 ^= (uint32_t)trace_port(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= cache_ring(c, t1);
    t2 += (uint32_t)pair_band(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x25) << 0;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t rotate_queue(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    merge_path(c, &c->lane[2], 2);
    t1 ^= (uint32_t)trim_key(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50419u) % (uint32_t)c->rln)] << 24;
    c->lane[4] += c->lane[8] ^ 0x71b36e14u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 29);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pair_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    tally_rate(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    align_cursor(c, t0, t1);
    c->lane[11] ^= sx_rl(c->lane[14], 29);
    c->lane[2] += c->lane[10]; c->lane[0] ^= c->lane[2]; c->lane[0] = sx_rl(c->lane[0], 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55906u) % (uint32_t)c->rln)] << 0;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 21);
    c->lane[0] += c->lane[11] ^ 0xe9e0eebfu;
    c->lane[9] += c->lane[13]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 31);
    t2 += (uint32_t)patch_table_287(c);
    t2 = (t2 ^ c->sum) * 0x791e581du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47739u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t merge_state(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t1 ^= (uint32_t)cache_gap(c, (uint8_t)(t0 >> 0), t2);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 28);
    place_head(c, t0, t1);
    c->hash ^= c->lane[10] + 0xf7a14528u;
    c->sched[16] = c->hash ^ sx_rl(c->lane[7], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x96) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void join_band_268(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[13] = c->hash ^ sx_rl(c->lane[0], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2329234bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37958u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 39072u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x36380f99u) ^ sx_rr(c->hash, 6);
    c->hash ^= c->lane[3] + 0x6d21058fu;
    peek_part(c, t0, t1);
    c->sched[12] = c->hash ^ sx_rl(c->lane[15], 16);
    c->lane[7] ^= sx_rl(c->lane[8], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += merge_index(c, c->slo, c->sln);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t queue_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4914u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd021cef9u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb250eeafu;
    c->lane[3] += c->lane[14] ^ 0x983633f0u;
    yield_tail(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 9633u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x84) << 0;
    merge_path(c, &c->lane[9], 3);
    c->lane[0] ^= sx_rl(c->lane[15], 8);
    c->raw[c->slo + (int)((t0 + 17648u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46874u) % (uint32_t)c->rln)] << 24;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t place_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 60849u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[5] + 0xeb03edb1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[2] + 0xbd2e3956u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xbd) << 16;
    c->hash = (c->hash * 0xd64cea13u) ^ sx_rr(c->hash, 1);
    c->sched[17] = c->hash ^ sx_rl(c->lane[4], 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t fold_layer(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 63416u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    place_head(c, t0, t1);
    c->hash ^= c->lane[11] + 0xc574b69eu;
    c->lane[15] ^= sx_rl(c->lane[4], 2);
    c->lane[14] += c->lane[15]; c->lane[11] ^= c->lane[14]; c->lane[11] = sx_rl(c->lane[11], 21);
    t2 += (uint32_t)rotate_region(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += poll_field(c, c->slo, c->sln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void align_cursor(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[26] = c->hash ^ sx_rl(c->lane[6], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x82) << 8;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 8);
    t0 ^= swap_region(c, t1);
    c->raw[c->slo + (int)((t0 + 28165u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t place_field(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6d3af515u;
    tally_field(c, t0, t1);
    c->lane[0] += c->lane[9]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 3);
    t2 = (t2 ^ c->sum) * 0xa6359cf1u;
    c->lane[3] += c->lane[2]; c->lane[9] ^= c->lane[3]; c->lane[9] = sx_rl(c->lane[9], 12);
    t0 ^= seek_key_298(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xc1) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t shift_store(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[11] ^ 0x38c53d3fu;
    c->lane[8] += c->lane[15]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1de9c995u;
    c->lane[9] += c->lane[7] ^ 0xc3158e0au;
    t1 ^= (uint32_t)merge_cell(c, (uint8_t)(t0 >> 16), t2);
    c->lane[9] += c->lane[1]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 30);
    c->lane[10] += c->lane[3] ^ 0x8653bdcdu;
    t0 ^= defer_table(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 20);
    t1 ^= (uint32_t)pack_pairing(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x1a3d51e9u) ^ sx_rr(c->hash, 7);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 6);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t cache_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42869u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x2f) << 16;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 28);
    c->lane[11] += c->lane[0] ^ 0xf84085c5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29218u) % (uint32_t)c->rln)] << 8;
    t2 += prime_key(c, c->rlo, c->rln);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 14);
    c->hash = (c->hash * 0xa5265b01u) ^ sx_rr(c->hash, 11);
    yield_tail(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t shift_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 26);
    relay_level(c, &c->lane[5], 1);
    t1 ^= (uint32_t)cache_segment(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= parse_run_320(c, t1);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 5);
    store_frame(c, &c->lane[10], 2);
    t0 ^= seek_key_298(c, t1);
    c->raw[c->slo + (int)((t0 + 58348u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[6] ^= sx_rl(c->lane[15], 28);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int sync_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[6] ^= sx_rl(c->lane[13], 9);
    t1 ^= (uint32_t)fold_window(c, (uint8_t)(t0 >> 8), t2);
    coal_gap(c, t0, t1);
    c->lane[2] += c->lane[9]; c->lane[11] ^= c->lane[2]; c->lane[11] = sx_rl(c->lane[11], 21);
    c->lane[0] += c->lane[7] ^ 0x30e1fe20u;
    t2 = (t2 ^ c->sum) * 0xb5b5c2b3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45585u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 3369u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8e) << 16;
    t2 = (t2 ^ c->sum) * 0x8539dc09u;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void merge_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x78e28637u;
    c->lane[10] += c->lane[11]; c->lane[1] ^= c->lane[10]; c->lane[1] = sx_rl(c->lane[1], 9);
    c->sched[21] = c->hash ^ sx_rl(c->lane[7], 11);
    c->hash = (c->hash * 0xdaf68b29u) ^ sx_rr(c->hash, 20);
    t0 ^= defer_table(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x1f) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x3f) << 16;
    c->lane[5] ^= sx_rl(c->lane[3], 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t seek_tail(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)cache_table(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 46408u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x20414d2du;
    c->sum += t1;
    return t0 + t2;
}

static int clamp_cell(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[9] = c->hash ^ sx_rl(c->lane[0], 11);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 28);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 12);
    c->hash ^= c->lane[8] + 0xd613447cu;
    c->hash ^= c->lane[10] + 0x9cb50f05u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x29d59387u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39976u) % (uint32_t)c->rln)] << 8;
    t2 += fetch_port(c, c->slo, c->sln);
    c->hash ^= c->lane[0] + 0xdc85782au;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tally_rate(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[5], 25);
    c->hash ^= c->lane[10] + 0x9fb81befu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[1] ^= sx_rl(c->lane[4], 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)cache_gap(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x40a3a771u;
    c->hash = (c->hash * 0xfa2b1f31u) ^ sx_rr(c->hash, 10);
    c->sched[11] = c->hash ^ sx_rl(c->lane[7], 20);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint32_t parse_level(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[1] ^= sx_rl(c->lane[4], 5);
    c->lane[12] += c->lane[3]; c->lane[13] ^= c->lane[12]; c->lane[13] = sx_rl(c->lane[13], 2);
    c->hash = (c->hash * 0x5eecaf55u) ^ sx_rr(c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[10] = c->hash ^ sx_rl(c->lane[13], 21);
    t2 = (t2 ^ c->sum) * 0x726719afu;
    c->hash = (c->hash * 0x4f2b6c71u) ^ sx_rr(c->hash, 20);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 21);
    c->hash = (c->hash * 0x8777a7d5u) ^ sx_rr(c->hash, 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t merge_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)close_port(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= peek_lease(c, t1);
    c->hash ^= c->lane[0] + 0xb06b17bbu;
    load_layer(c, t0, t1);
    t0 ^= slice_frame(c, t1);
    c->lane[4] += c->lane[14]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xdb) << 8;
    t2 = (t2 ^ c->sum) * 0xd45b5731u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[0], 3);
    load_queue(c, &c->lane[11], 1);
    c->sched[29] = c->hash ^ sx_rl(c->lane[15], 1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t pair_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += reset_part(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 39284u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    coal_gap(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[15] += c->lane[4] ^ 0x42c0d612u;
    probe_store(c, &c->lane[4], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf4ebeffbu;
    c->hash ^= c->lane[8] + 0x35fa1cebu;
    t2 = (t2 ^ c->sum) * 0xbc44774bu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t trim_key(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    relay_level(c, &c->lane[9], 4);
    c->lane[1] ^= sx_rl(c->lane[7], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd615a50du;
    c->lane[4] += c->lane[6] ^ 0xe25e6322u;
    c->lane[8] += c->lane[10]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 27);
    t2 = (t2 ^ c->sum) * 0x02f2c543u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe8) << 8;
    t2 = (t2 ^ c->sum) * 0xb86d576fu;
    c->raw[c->slo + (int)((t0 + 5371u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] ^= sx_rl(c->lane[1], 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc3b819b9u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int pair_band(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->lane[5] += c->lane[11]; c->lane[8] ^= c->lane[5]; c->lane[8] = sx_rl(c->lane[8], 25);
    c->sched[26] = c->hash ^ sx_rl(c->lane[5], 4);
    t2 += (uint32_t)tune_gap(c);
    c->lane[2] ^= sx_rl(c->lane[15], 19);
    c->sched[4] = c->hash ^ sx_rl(c->lane[12], 8);
    wrap_state(c, t0, t1);
    c->lane[9] ^= sx_rl(c->lane[1], 28);
    t2 += (uint32_t)coal_head(c);
    t2 += (uint32_t)peek_bucket(c);
    c->raw[c->slo + (int)((t0 + 52750u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 7229u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int patch_table_287(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 46068u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[3] ^= sx_rl(c->lane[14], 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    align_entry(c, &c->lane[7], 1);
    c->hash ^= c->lane[2] + 0xf3b55d19u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 ^= place_offset(c, t1);
    c->hash = (c->hash * 0xf51f9971u) ^ sx_rr(c->hash, 23);
    c->hash ^= c->lane[14] + 0xf50e0787u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[9], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void purge_digest(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x29) << 16;
    c->lane[11] ^= sx_rl(c->lane[9], 23);
    c->raw[c->slo + (int)((t0 + 55248u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 5);
    c->lane[14] += c->lane[0] ^ 0xc99ee7e8u;
    c->hash ^= c->lane[4] + 0x26484a24u;
    c->hash ^= c->lane[4] + 0x7a9ffd8cu;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void coal_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] += c->lane[6] ^ 0xcb33ee18u;
    t1 ^= (uint32_t)sift_stream(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= yield_segment(c, t1);
    c->sched[18] = c->hash ^ sx_rl(c->lane[11], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7a) << 16;
    c->lane[15] += c->lane[14] ^ 0x7713642du;
    t0 ^= queue_run(c, t1);
    t2 += (uint32_t)merge_pairing(c);
    c->lane[3] ^= sx_rl(c->lane[5], 13);
    t2 += purge_field(c, c->slo, c->sln);
    c->lane[9] += c->lane[0]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8054u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[14] ^= sx_rl(c->lane[6], 22);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x70) << 16;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t peek_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x73d1ca1bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xaa) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x09618f7bu;
    purge_pool(c, t0, t1);
    c->lane[5] += c->lane[3]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8d9dec1bu;
    t0 ^= push_group_378(c, t1);
    c->hash = (c->hash * 0x5c1c5645u) ^ sx_rr(c->hash, 1);
    c->lane[1] += c->lane[2]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 15);
    c->hash ^= c->lane[10] + 0x9f923712u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 26);
    c->lane[7] += c->lane[8] ^ 0x979ff28bu;
    c->lane[13] += c->lane[4] ^ 0x1e8961feu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void relay_level(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16707u) % (uint32_t)c->rln)] << 8;
    t2 += reset_page(c, c->slo, c->sln);
    c->hash = (c->hash * 0x109a7fcfu) ^ sx_rr(c->hash, 25);
    clamp_record(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 7026u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[5] += c->lane[1] ^ 0x69490729u;
    c->hash = (c->hash * 0x4f936ab3u) ^ sx_rr(c->hash, 10);
    c->hash ^= c->lane[10] + 0xd11345e3u;
    c->lane[7] += c->lane[4] ^ 0xe253d1f8u;
    t2 += tally_tuple(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void wrap_state(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += mark_port(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xaa8206abu;
    c->sched[10] = c->hash ^ sx_rl(c->lane[6], 9);
    c->hash ^= c->lane[3] + 0xa76e12ddu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21319u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] += c->lane[7]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 22);
    t1 ^= (uint32_t)join_segment(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sched[13] = c->hash ^ sx_rl(c->lane[10], 2);
    c->lane[2] += c->lane[0]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x35) << 8;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint8_t pack_pairing(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xe570b5d5u;
    c->lane[11] += c->lane[2]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 14);
    c->sched[30] = c->hash ^ sx_rl(c->lane[3], 12);
    c->lane[9] ^= sx_rl(c->lane[9], 2);
    c->hash ^= c->lane[3] + 0x95a44aabu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf6) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x86848fddu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int tune_gap(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[13] += c->lane[15]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 29);
    t1 ^= (uint32_t)join_segment(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xa96ad8d5u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbcb1122du;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tally_field(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xf2b051cdu;
    c->hash = (c->hash * 0x0769750fu) ^ sx_rr(c->hash, 3);
    c->lane[5] ^= sx_rl(c->lane[13], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 40132u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 7);
    c->lane[3] += c->lane[13] ^ 0x2e406151u;
    c->lane[15] += c->lane[3] ^ 0x4909836bu;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint8_t fold_window(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[13] + 0x6856731bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb45c2fbdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64338u) % (uint32_t)c->rln)] << 0;
    close_delta(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t place_offset(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    trim_delta(c, t0, t1);
    c->sched[11] = c->hash ^ sx_rl(c->lane[3], 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t1 ^= (uint32_t)hold_state(c, (uint8_t)(t0 >> 0), t2);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 21);
    c->hash ^= c->lane[13] + 0x590bf670u;
    c->lane[14] ^= sx_rl(c->lane[4], 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t seek_key_298(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe967a145u;
    mix_batch(c, t0, t1);
    t2 += close_table(c, c->rlo, c->rln);
    c->hash = (c->hash * 0xdb73c0c5u) ^ sx_rr(c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59201u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37945u) % (uint32_t)c->rln)] << 16;
    c->lane[11] ^= sx_rl(c->lane[4], 2);
    c->lane[3] += c->lane[6]; c->lane[15] ^= c->lane[3]; c->lane[15] = sx_rl(c->lane[15], 18);
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 13);
    trace_label(c, &c->lane[9], 4);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18116u) % (uint32_t)c->rln)] << 0;
    c->lane[10] += c->lane[1]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t reset_part(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] += c->lane[7]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 29);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[12] += c->lane[2] ^ 0x6f3c6ca3u;
    c->lane[9] += c->lane[7] ^ 0x1c7eeeb5u;
    c->hash = (c->hash * 0x117c1375u) ^ sx_rr(c->hash, 14);
    c->hash ^= c->lane[15] + 0xb2104f97u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x27b9b909u;
    peek_part(c, t0, t1);
    c->lane[9] += c->lane[0]; c->lane[8] ^= c->lane[9]; c->lane[8] = sx_rl(c->lane[8], 12);
    c->sum += t1;
    return t0 + t2;
}

static void coal_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += clamp_frame(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd0) << 0;
    t1 ^= (uint32_t)latch_arena(c, (uint8_t)(t0 >> 0), t2);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 26);
    drain_track(c, &c->lane[5], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xc72cc891u;
    t2 += (uint32_t)sift_region(c);
    c->sched[11] = c->hash ^ sx_rl(c->lane[6], 18);
    t0 ^= link_count(c, t1);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint8_t sift_stream(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= mark_part(c, t1);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 6);
    t2 += (uint32_t)latch_page(c);
    t2 += (uint32_t)tap_port(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27936u) % (uint32_t)c->rln)] << 8;
    c->lane[1] ^= sx_rl(c->lane[2], 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56531u) % (uint32_t)c->rln)] << 24;
    t0 ^= queue_run(c, t1);
    peek_page(c, &c->lane[0], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t cache_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x96dd3db7u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    c->lane[7] += c->lane[15]; c->lane[8] ^= c->lane[7]; c->lane[8] = sx_rl(c->lane[8], 1);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 11);
    t1 ^= (uint32_t)trace_port(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)probe_segment(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6db81ee9u;
    c->hash = (c->hash * 0x27575e23u) ^ sx_rr(c->hash, 19);
    c->raw[c->slo + (int)((t0 + 32705u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    grow_store(c, t0, t1);
    c->lane[7] ^= sx_rl(c->lane[11], 26);
    t2 = (t2 ^ c->sum) * 0x1a590e37u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int merge_pairing(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x5476831du) ^ sx_rr(c->hash, 12);
    c->raw[c->slo + (int)((t0 + 38060u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8b) << 16;
    t2 += close_gap(c, c->rlo, c->rln);
    t2 += trim_item(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[25] = c->hash ^ sx_rl(c->lane[11], 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6e) << 16;
    t2 += tally_arena_337(c, c->rlo, c->rln);
    t2 += queue_label(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash = (c->hash * 0x3387c8f7u) ^ sx_rr(c->hash, 21);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t close_port(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[5] ^= sx_rl(c->lane[11], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x83) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x95) << 8;
    t2 = (t2 ^ c->sum) * 0xa36dd81du;
    c->hash = (c->hash * 0x07f0a86fu) ^ sx_rr(c->hash, 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33366u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t defer_table(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[5] += c->lane[2] ^ 0x248282b1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xc590f081u;
    c->hash = (c->hash * 0x73640349u) ^ sx_rr(c->hash, 22);
    c->lane[14] += c->lane[15]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 12);
    t2 = (t2 ^ c->sum) * 0xf0dcc4e3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc22dedd3u;
    c->raw[c->slo + (int)((t0 + 55644u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t cache_segment(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc3) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[2] = c->hash ^ sx_rl(c->lane[2], 1);
    c->raw[c->slo + (int)((t0 + 28476u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t1 ^= (uint32_t)swap_state(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= c->lane[0] + 0xe3afde46u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28540u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void yield_tail(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[5] + 0x64580503u;
    c->lane[4] += c->lane[10] ^ 0x63524428u;
    t1 ^= (uint32_t)mark_label(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= c->lane[4] + 0xb0828c22u;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t slice_frame(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12816u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[7] ^= sx_rl(c->lane[1], 31);
    c->lane[11] += c->lane[4]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 26);
    t2 = (t2 ^ c->sum) * 0xa8176b2du;
    c->lane[3] += c->lane[10] ^ 0x7f8a848au;
    c->sched[8] = c->hash ^ sx_rl(c->lane[5], 18);
    t2 += (uint32_t)join_segment_353(c);
    merge_band(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 24450u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[0] + 0x3ea146b6u;
    c->hash = (c->hash * 0x0f2bf36du) ^ sx_rr(c->hash, 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void store_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)peek_bucket(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x994d5dcbu;
    c->lane[2] += c->lane[2] ^ 0xe6a55334u;
    c->lane[1] ^= sx_rl(c->lane[2], 3);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 20);
    c->raw[c->slo + (int)((t0 + 50814u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x3f867d25u) ^ sx_rr(c->hash, 2);
    t1 ^= (uint32_t)chain_run(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42138u) % (uint32_t)c->rln)] << 24;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t cache_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x52185d21u;
    t2 += (uint32_t)wrap_bound(c);
    c->hash ^= c->lane[11] + 0xa1d0dfafu;
    c->hash ^= c->lane[9] + 0x4f587715u;
    c->hash = (c->hash * 0x6644d2e5u) ^ sx_rr(c->hash, 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x16) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[15] + 0x032952e0u;
    t0 ^= drain_table(c, t1);
    c->lane[2] += c->lane[11]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 20);
    c->hash = (c->hash * 0x0aef80f9u) ^ sx_rr(c->hash, 13);
    c->lane[14] ^= sx_rl(c->lane[0], 15);
    t2 = (t2 ^ c->sum) * 0xc2ab7fa1u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void load_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 14545u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] += c->lane[11] ^ 0x62cb8f13u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[2], 20);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 8);
    peek_page(c, &c->lane[0], 3);
    drain_block(c, &c->lane[5], 2);
    t2 += (uint32_t)join_segment_353(c);
    t2 += (uint32_t)scan_arena(c);
    t2 += reap_part(c, c->slo, c->sln);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t yield_segment(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 25121u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5442u) % (uint32_t)c->rln)] << 24;
    c->lane[1] += c->lane[12]; c->lane[14] ^= c->lane[1]; c->lane[14] = sx_rl(c->lane[14], 24);
    load_queue(c, &c->lane[3], 4);
    t2 += flush_level(c, c->slo, c->sln);
    prime_slot(c, &c->lane[6], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61520u) % (uint32_t)c->rln)] << 8;
    c->lane[10] += c->lane[3]; c->lane[1] ^= c->lane[10]; c->lane[1] = sx_rl(c->lane[1], 9);
    c->sched[6] = c->hash ^ sx_rl(c->lane[12], 17);
    c->sched[3] = c->hash ^ sx_rl(c->lane[0], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t swap_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)pick_head(c);
    c->lane[7] += c->lane[6] ^ 0xddc0f453u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7cbafc03u;
    c->hash ^= c->lane[14] + 0x643cffd1u;
    c->hash = (c->hash * 0xeea64ba5u) ^ sx_rr(c->hash, 10);
    c->lane[14] += c->lane[0]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 17);
    c->hash ^= c->lane[13] + 0xf5973925u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash = (c->hash * 0x441c30b9u) ^ sx_rr(c->hash, 31);
    c->hash ^= c->lane[1] + 0x9c7d52a0u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fetch_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[4] = c->hash ^ sx_rl(c->lane[12], 28);
    c->hash ^= c->lane[12] + 0x46e79c7fu;
    t2 += (uint32_t)probe_segment(c);
    clamp_offset(c, t0, t1);
    c->hash ^= c->lane[3] + 0x0039bcfdu;
    c->raw[c->slo + (int)((t0 + 34368u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa5) << 0;
    c->lane[4] ^= sx_rl(c->lane[7], 9);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15430u) % (uint32_t)c->rln)] << 8;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 24);
    c->sum += t1;
    return t0 + t2;
}

static int coal_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->lane[15] ^= sx_rl(c->lane[2], 22);
    c->raw[c->slo + (int)((t0 + 49117u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    sync_segment(c, t0, t1);
    c->hash = (c->hash * 0x92b4b97fu) ^ sx_rr(c->hash, 18);
    t2 += relay_digest(c, c->rlo, c->rln);
    c->hash = (c->hash * 0xa3cbbe91u) ^ sx_rr(c->hash, 15);
    c->lane[0] ^= sx_rl(c->lane[12], 25);
    t2 += (uint32_t)wrap_bound(c);
    t1 ^= (uint32_t)fetch_head(c, (uint8_t)(t0 >> 8), t2);
    c->sched[27] = c->hash ^ sx_rl(c->lane[4], 23);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void align_entry(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62223u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)mark_offset(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x4823ad49u) ^ sx_rr(c->hash, 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x4b0daa6bu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    c->lane[7] ^= sx_rl(c->lane[7], 16);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 9);
    c->sched[19] = c->hash ^ sx_rl(c->lane[1], 10);
    c->hash ^= c->lane[14] + 0x533ed52fu;
    t2 += (uint32_t)parse_head(c);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 21);
    yield_window(c, &c->lane[0], 2);
    c->raw[c->slo + (int)((t0 + 40708u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t merge_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    step_digest(c, t0, t1);
    c->hash ^= c->lane[15] + 0x2affa57eu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 23);
    t2 += tally_tuple(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 38170u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8b) << 8;
    c->hash = (c->hash * 0x0b8f0385u) ^ sx_rr(c->hash, 2);
    merge_state_357(c, t0, t1);
    t2 += close_table(c, c->slo, c->sln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t poll_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[8] + 0x53bbb8dcu;
    t1 ^= (uint32_t)link_queue(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x107cfefdu;
    t2 = (t2 ^ c->sum) * 0x6be224e3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60707u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0xe28f43fdu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t purge_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa42885f3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32718u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[19] = c->hash ^ sx_rl(c->lane[3], 28);
    c->hash ^= c->lane[2] + 0x20495b9au;
    c->hash ^= c->lane[12] + 0xeb91cd61u;
    c->lane[8] += c->lane[7]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 4);
    c->raw[c->slo + (int)((t0 + 45841u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[7] = c->hash ^ sx_rl(c->lane[0], 26);
    c->hash = (c->hash * 0xa60328d7u) ^ sx_rr(c->hash, 15);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t parse_run_320(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24658u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[1] + 0x43d32fdfu;
    c->lane[8] += c->lane[10] ^ 0xecc73886u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void place_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x33) << 0;
    c->hash ^= c->lane[14] + 0x54da49cfu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31967u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0x26493061u) ^ sx_rr(c->hash, 4);
    pair_queue(c, &c->lane[4], 1);
    c->lane[10] ^= sx_rl(c->lane[10], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void probe_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x878f039du) ^ sx_rr(c->hash, 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd0) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x258d3225u;
    c->raw[c->slo + (int)((t0 + 33034u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8dae4063u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 7);
    c->raw[c->slo + (int)((t0 + 7988u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0xa37fb11fu) ^ sx_rr(c->hash, 17);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int rotate_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb6478201u;
    t2 += (uint32_t)settle_row(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[11] += c->lane[8]; c->lane[13] ^= c->lane[11]; c->lane[13] = sx_rl(c->lane[13], 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xb24aa945u) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd4) << 0;
    t2 += (uint32_t)move_store(c);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t prime_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[1] + 0xc90f90fdu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 20);
    t1 ^= (uint32_t)mark_offset(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x60fb299du;
    c->lane[15] += c->lane[11]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 24);
    c->raw[c->slo + (int)((t0 + 58308u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x38886daeu;
    c->raw[c->slo + (int)((t0 + 50439u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 2);
    c->sum += t1;
    return t0 + t2;
}

static void yield_window(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x558f809fu;
    c->lane[14] += c->lane[10] ^ 0xacb88961u;
    c->hash = (c->hash * 0x9124c4a7u) ^ sx_rr(c->hash, 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x12) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58048u) % (uint32_t)c->rln)] << 0;
    c->lane[12] += c->lane[9]; c->lane[4] ^= c->lane[12]; c->lane[4] = sx_rl(c->lane[4], 5);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 26);
    c->lane[11] += c->lane[5] ^ 0xe01986e8u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xfd35545bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8496821bu;
    t2 = (t2 ^ c->sum) * 0x1c917841u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int tap_port(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xfb8a4c91u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1e1725a3u;
    c->lane[13] ^= sx_rl(c->lane[0], 18);
    c->sched[9] = c->hash ^ sx_rl(c->lane[5], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash = (c->hash * 0x134b4f51u) ^ sx_rr(c->hash, 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x1996455fu;
    c->raw[c->slo + (int)((t0 + 52353u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mark_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28168u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4909u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 7388u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50116u) % (uint32_t)c->rln)] << 0;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 24);
    c->lane[7] ^= sx_rl(c->lane[3], 30);
    c->sum += t1;
    return t0 + t2;
}

static void pair_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] += c->lane[14]; c->lane[8] ^= c->lane[13]; c->lane[8] = sx_rl(c->lane[8], 1);
    c->lane[4] += c->lane[12] ^ 0x4e1d307bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 28);
    c->lane[7] ^= sx_rl(c->lane[14], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38962u) % (uint32_t)c->rln)] << 16;
    c->lane[3] ^= sx_rl(c->lane[3], 10);
    c->sched[11] = c->hash ^ sx_rl(c->lane[9], 26);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t mark_part(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[10] += c->lane[3]; c->lane[15] ^= c->lane[10]; c->lane[15] = sx_rl(c->lane[15], 24);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 6);
    c->lane[10] += c->lane[10] ^ 0x28c03189u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39650u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x998d731bu;
    c->raw[c->slo + (int)((t0 + 31609u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[21] = c->hash ^ sx_rl(c->lane[2], 14);
    t2 = (t2 ^ c->sum) * 0xa52964efu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x037549e1u;
    t2 = (t2 ^ c->sum) * 0x040c784fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t queue_run(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45745u) % (uint32_t)c->rln)] << 24;
    c->lane[9] ^= sx_rl(c->lane[12], 19);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5540u) % (uint32_t)c->rln)] << 24;
    c->lane[5] += c->lane[6]; c->lane[12] ^= c->lane[5]; c->lane[12] = sx_rl(c->lane[12], 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 51646u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int pick_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcdd305efu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 18);
    c->lane[11] ^= sx_rl(c->lane[11], 6);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 2);
    c->lane[4] += c->lane[3]; c->lane[13] ^= c->lane[4]; c->lane[13] = sx_rl(c->lane[13], 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64164u) % (uint32_t)c->rln)] << 0;
    c->lane[12] += c->lane[4]; c->lane[3] ^= c->lane[12]; c->lane[3] = sx_rl(c->lane[3], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28991u) % (uint32_t)c->rln)] << 8;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void trim_delta(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x52d0cc87u) ^ sx_rr(c->hash, 31);
    t2 = (t2 ^ c->sum) * 0x147babc1u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 17);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xdf) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x15) << 16;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint8_t chain_run(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[7] ^ 0xbb710684u;
    c->lane[2] += c->lane[12] ^ 0x0f49fad1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x133d1de5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0xd783ed5bu) ^ sx_rr(c->hash, 11);
    c->hash = (c->hash * 0x9f9e19e7u) ^ sx_rr(c->hash, 20);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void mix_batch(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x589fcd69u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xff) << 16;
    c->hash = (c->hash * 0xaecd75f3u) ^ sx_rr(c->hash, 17);
    t2 = (t2 ^ c->sum) * 0x1283210fu;
    c->lane[11] += c->lane[3]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8599a239u;
    c->sched[19] = c->hash ^ sx_rl(c->lane[0], 19);
    t2 = (t2 ^ c->sum) * 0x16ac8441u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void prime_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[9] += c->lane[0]; c->lane[10] ^= c->lane[9]; c->lane[10] = sx_rl(c->lane[10], 12);
    t2 = (t2 ^ c->sum) * 0xc60b9c7fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50046u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0xe462fa45u;
    c->lane[7] ^= sx_rl(c->lane[9], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash = (c->hash * 0xb59d5e2du) ^ sx_rr(c->hash, 23);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 18);
    c->hash = (c->hash * 0x0831cfa5u) ^ sx_rr(c->hash, 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7293u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[10] + 0xc9c4d12cu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t fetch_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] ^= sx_rl(c->lane[1], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash = (c->hash * 0xf4e06db5u) ^ sx_rr(c->hash, 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbae409fdu;
    c->hash = (c->hash * 0xc0ee3457u) ^ sx_rr(c->hash, 31);
    c->sched[17] = c->hash ^ sx_rl(c->lane[14], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5198d06bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x65) << 0;
    t2 = (t2 ^ c->sum) * 0x57fc1865u;
    c->raw[c->slo + (int)((t0 + 26357u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t tally_arena_337(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xf28c89e1u;
    c->hash = (c->hash * 0x8e146e9bu) ^ sx_rr(c->hash, 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[0] ^= sx_rl(c->lane[0], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x84) << 8;
    c->lane[15] += c->lane[10] ^ 0x06542c3bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static int sift_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= c->lane[8] + 0xe8d51a5au;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[10] += c->lane[0] ^ 0x89c6065eu;
    c->lane[13] += c->lane[5] ^ 0x2e32d984u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x48) << 8;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int move_store(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash = (c->hash * 0xd6feec9fu) ^ sx_rr(c->hash, 12);
    c->lane[6] += c->lane[15]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 25);
    t2 = (t2 ^ c->sum) * 0x3ebf0421u;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7d156285u;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t latch_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] += c->lane[5]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 29);
    c->hash ^= c->lane[12] + 0x816859a0u;
    c->hash ^= c->lane[9] + 0x1151a919u;
    c->lane[0] += c->lane[12]; c->lane[7] ^= c->lane[0]; c->lane[7] = sx_rl(c->lane[7], 24);
    c->hash = (c->hash * 0xb2bb9839u) ^ sx_rr(c->hash, 24);
    c->hash = (c->hash * 0x9a8d55ddu) ^ sx_rr(c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[7] += c->lane[14]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38986u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void purge_pool(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x49cbe885u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfe3be5c5u;
    c->sched[16] = c->hash ^ sx_rl(c->lane[0], 1);
    t2 = (t2 ^ c->sum) * 0xd4ab3587u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x55042ee5u) ^ sx_rr(c->hash, 24);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 27);
    c->lane[12] ^= sx_rl(c->lane[10], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbbfbfef5u;
    c->sched[1] = c->hash ^ sx_rl(c->lane[2], 19);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static int peek_bucket(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 21);
    c->raw[c->slo + (int)((t0 + 17005u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[9] + 0x07ca35b0u;
    c->hash = (c->hash * 0xd1170333u) ^ sx_rr(c->hash, 27);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void clamp_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 9);
    t2 = (t2 ^ c->sum) * 0x4ada8ed1u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 4);
    c->hash = (c->hash * 0xc9c42237u) ^ sx_rr(c->hash, 10);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t queue_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x7a305d17u;
    c->lane[13] += c->lane[14]; c->lane[8] ^= c->lane[13]; c->lane[8] = sx_rl(c->lane[8], 18);
    c->raw[c->slo + (int)((t0 + 23245u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 42969u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t hold_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xaa) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33451u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x93f5a90bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb6) << 0;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[4] += c->lane[6] ^ 0x0291fa2au;
    c->hash ^= c->lane[10] + 0x8038d062u;
    c->lane[0] ^= sx_rl(c->lane[2], 28);
    c->lane[6] ^= sx_rl(c->lane[7], 9);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t reap_part(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[23] = c->hash ^ sx_rl(c->lane[5], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10585u) % (uint32_t)c->rln)] << 8;
    c->lane[3] += c->lane[13] ^ 0x1321301bu;
    c->hash ^= c->lane[3] + 0x2b8848b2u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t join_segment(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52952u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5b) << 0;
    c->sched[13] = c->hash ^ sx_rl(c->lane[15], 24);
    c->sched[2] = c->hash ^ sx_rl(c->lane[1], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[2] += c->lane[12]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6ac5aaedu;
    t2 = (t2 ^ c->sum) * 0x29e1f80du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x738950fdu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void peek_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[6], 20);
    c->hash = (c->hash * 0x9203548fu) ^ sx_rr(c->hash, 13);
    c->hash = (c->hash * 0x9dbcb7e5u) ^ sx_rr(c->hash, 17);
    c->hash ^= c->lane[10] + 0xc8b7f08bu;
    c->raw[c->slo + (int)((t0 + 40149u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[8] + 0xcb59969fu;
    c->hash ^= c->lane[2] + 0xe43f10f3u;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static void trace_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 12);
    c->lane[5] += c->lane[7] ^ 0x69e473f9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16679u) % (uint32_t)c->rln)] << 8;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x96) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa6) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0x4e7210c1u;
    c->lane[11] += c->lane[12] ^ 0x6ea2eeecu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x409f2a37u;
    c->lane[1] ^= sx_rl(c->lane[15], 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void clamp_offset(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5f) << 16;
    t2 = (t2 ^ c->sum) * 0xd2b98623u;
    c->raw[c->slo + (int)((t0 + 3781u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 6);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static int wrap_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd870d6a9u;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x61efe239u;
    c->lane[14] ^= sx_rl(c->lane[7], 5);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 31);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t link_count(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x9feb683bu;
    c->hash = (c->hash * 0x33622899u) ^ sx_rr(c->hash, 24);
    c->lane[8] += c->lane[9]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 2);
    c->lane[1] ^= sx_rl(c->lane[10], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x26a6733du;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42690u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53080u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int join_segment_353(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->hash = (c->hash * 0x8e3e3fe7u) ^ sx_rr(c->hash, 11);
    c->lane[9] += c->lane[7]; c->lane[14] ^= c->lane[9]; c->lane[14] = sx_rl(c->lane[14], 7);
    c->sched[29] = c->hash ^ sx_rl(c->lane[0], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[1] += c->lane[9]; c->lane[7] ^= c->lane[1]; c->lane[7] = sx_rl(c->lane[7], 9);
    c->hash ^= c->lane[11] + 0xd51ebcf6u;
    c->raw[c->slo + (int)((t0 + 48751u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= c->lane[4] + 0xfff037a7u;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void drain_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 13);
    c->raw[c->slo + (int)((t0 + 63485u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 48630u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[14] ^= sx_rl(c->lane[7], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 18);
    c->sched[2] = c->hash ^ sx_rl(c->lane[12], 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x47) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t reset_page(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] ^= sx_rl(c->lane[8], 4);
    c->raw[c->slo + (int)((t0 + 6072u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[15] += c->lane[10] ^ 0x6f8d92a6u;
    c->lane[14] ^= sx_rl(c->lane[1], 19);
    c->lane[2] += c->lane[10]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x66) << 16;
    t2 = (t2 ^ c->sum) * 0xdc610637u;
    t2 = (t2 ^ c->sum) * 0xe6d16fcdu;
    c->sum += t1;
    return t0 + t2;
}

static void grow_store(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 21);
    c->hash ^= c->lane[12] + 0xabeaa70eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 14416u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[0] ^= sx_rl(c->lane[10], 22);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static void merge_state_357(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63865u) % (uint32_t)c->rln)] << 8;
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd3708627u;
    c->hash = (c->hash * 0x69a5fbcdu) ^ sx_rr(c->hash, 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[10] ^= sx_rl(c->lane[12], 5);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t relay_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62020u) % (uint32_t)c->rln)] << 16;
    c->sched[8] = c->hash ^ sx_rl(c->lane[13], 12);
    c->lane[4] += c->lane[13] ^ 0xe8a3e1f8u;
    c->hash = (c->hash * 0x8d6208cdu) ^ sx_rr(c->hash, 16);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t mark_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf4f1b4cdu;
    c->hash = (c->hash * 0x66c6ba1du) ^ sx_rr(c->hash, 27);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 13);
    c->hash ^= c->lane[2] + 0xfb165323u;
    c->lane[4] += c->lane[5] ^ 0xfbc68a35u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13050u) % (uint32_t)c->rln)] << 24;
    c->lane[4] += c->lane[4] ^ 0x873037d0u;
    c->hash = (c->hash * 0x2ca30f35u) ^ sx_rr(c->hash, 23);
    c->hash = (c->hash * 0x690d056fu) ^ sx_rr(c->hash, 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x48) << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void load_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[0], 24);
    c->hash = (c->hash * 0x3b6d2257u) ^ sx_rr(c->hash, 1);
    t2 = (t2 ^ c->sum) * 0x450a776bu;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 26);
    c->raw[c->slo + (int)((t0 + 10637u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16035u) % (uint32_t)c->rln)] << 8;
    c->lane[11] += c->lane[2] ^ 0x6af7a0a3u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 22);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t trace_port(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 23);
    c->raw[c->slo + (int)((t0 + 11296u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[4] += c->lane[11] ^ 0xd3790974u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void peek_page(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] += c->lane[4]; c->lane[2] ^= c->lane[0]; c->lane[2] = sx_rl(c->lane[2], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26302u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[13], 21);
    c->raw[c->slo + (int)((t0 + 26697u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int parse_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->sched[8] = c->hash ^ sx_rl(c->lane[12], 27);
    c->lane[4] ^= sx_rl(c->lane[1], 25);
    c->lane[9] += c->lane[3]; c->lane[7] ^= c->lane[9]; c->lane[7] = sx_rl(c->lane[7], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x83c52935u;
    c->sched[12] = c->hash ^ sx_rl(c->lane[12], 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xaff881fdu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xfd83eecbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60187u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x68cf5f5du) ^ sx_rr(c->hash, 8);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void step_digest(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8828u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[15] + 0x24fee535u;
    c->lane[11] += c->lane[12] ^ 0xc4ce670bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x23ec1991u;
    c->raw[c->slo + (int)((t0 + 62415u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[10] += c->lane[9]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 10);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void sync_segment(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[1] += c->lane[15]; c->lane[14] ^= c->lane[1]; c->lane[14] = sx_rl(c->lane[14], 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15021u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x70be6fd3u;
    c->hash = (c->hash * 0xd3a386b9u) ^ sx_rr(c->hash, 13);
    c->lane[5] += c->lane[5] ^ 0xf8e3ae96u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x08cbf9d7u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] += c->lane[4]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 18);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t drain_table(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe4dbc0cfu;
    c->lane[6] ^= sx_rl(c->lane[12], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51807u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17151u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[2] + 0x392a02f3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[6] + 0x54e1a4d3u;
    c->lane[15] += c->lane[9]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t flush_level(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8b5c7d97u;
    c->hash ^= c->lane[8] + 0xac352382u;
    t2 = (t2 ^ c->sum) * 0xd590c3ebu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa42e8379u;
    c->raw[c->slo + (int)((t0 + 12698u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t link_queue(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[6] += c->lane[7] ^ 0xad32c6e1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8070a1d1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash = (c->hash * 0x9fbff305u) ^ sx_rr(c->hash, 19);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 28);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int latch_page(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->lane[2] += c->lane[13]; c->lane[15] ^= c->lane[2]; c->lane[15] = sx_rl(c->lane[15], 19);
    c->raw[c->slo + (int)((t0 + 3720u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[27] = c->hash ^ sx_rl(c->lane[11], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x64) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[7] += c->lane[11]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30989u) % (uint32_t)c->rln)] << 0;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void drain_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[12] + 0x74319a62u;
    c->lane[12] += c->lane[1] ^ 0xf810d6e1u;
    c->lane[8] ^= sx_rl(c->lane[12], 1);
    c->lane[2] ^= sx_rl(c->lane[8], 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48948u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[7] + 0x4832b8a5u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[4], 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xce43dc5fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t clamp_frame(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[15] += c->lane[8]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 30);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x672c5435u;
    c->lane[15] += c->lane[6] ^ 0xbf6be19du;
    c->lane[1] += c->lane[4]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 15);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 9);
    c->lane[0] += c->lane[6]; c->lane[9] ^= c->lane[0]; c->lane[9] = sx_rl(c->lane[9], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x05) << 8;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t trim_item(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xfb606473u) ^ sx_rr(c->hash, 17);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 22);
    c->raw[c->slo + (int)((t0 + 3128u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xbf) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash = (c->hash * 0xe0c4e0f9u) ^ sx_rr(c->hash, 22);
    c->lane[10] += c->lane[0] ^ 0xd27af77au;
    c->sched[27] = c->hash ^ sx_rl(c->lane[15], 19);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tally_tuple(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xb3537483u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 134u) % (uint32_t)c->rln)] << 16;
    c->lane[13] += c->lane[15] ^ 0xd45bafc1u;
    c->hash ^= c->lane[3] + 0xeffbab0bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xd304f0adu) ^ sx_rr(c->hash, 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t mark_offset(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] ^= sx_rl(c->lane[9], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe44cac6bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 12);
    c->lane[6] ^= sx_rl(c->lane[7], 4);
    c->lane[9] ^= sx_rl(c->lane[0], 13);
    t2 = (t2 ^ c->sum) * 0xf6abfbedu;
    c->raw[c->slo + (int)((t0 + 6691u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[1] = c->hash ^ sx_rl(c->lane[5], 21);
    c->raw[c->slo + (int)((t0 + 64464u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44478u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int settle_row(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf9) << 0;
    c->hash = (c->hash * 0x23f3c7bdu) ^ sx_rr(c->hash, 20);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x16e81f51u;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t swap_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[3] + 0x4ffe21e5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11397u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27974u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int probe_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[14] ^= sx_rl(c->lane[13], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7b) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4f) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe29484b7u;
    c->lane[4] += c->lane[5]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 11);
    c->hash = (c->hash * 0x0fc87df7u) ^ sx_rr(c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[9] ^= sx_rl(c->lane[12], 2);
    t2 = (t2 ^ c->sum) * 0x13811c49u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t push_group_378(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11368u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[1] += c->lane[5]; c->lane[10] ^= c->lane[1]; c->lane[10] = sx_rl(c->lane[10], 1);
    c->raw[c->slo + (int)((t0 + 51881u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[25] = c->hash ^ sx_rl(c->lane[10], 11);
    c->lane[7] ^= sx_rl(c->lane[0], 16);
    c->hash ^= c->lane[2] + 0x31da6d3du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 3442u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_band(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48832u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 53913u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x37628749u;
    c->lane[4] ^= sx_rl(c->lane[10], 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[8] += c->lane[15] ^ 0xaf4b4594u;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t close_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[10] = c->hash ^ sx_rl(c->lane[7], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd0) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] ^= sx_rl(c->lane[1], 12);
    c->lane[0] ^= sx_rl(c->lane[8], 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63322u) % (uint32_t)c->rln)] << 0;
    c->lane[7] += c->lane[13]; c->lane[12] ^= c->lane[7]; c->lane[12] = sx_rl(c->lane[12], 23);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xde) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 23);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t close_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] += c->lane[14]; c->lane[12] ^= c->lane[9]; c->lane[12] = sx_rl(c->lane[12], 16);
    c->sched[28] = c->hash ^ sx_rl(c->lane[0], 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x8e811025u;
    c->sum += t1;
    return t0 + t2;
}

static void close_delta(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42793u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38493u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= c->lane[9] + 0x6015922cu;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static int scan_arena(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa8a8b37du;
    c->hash = (c->hash * 0x144ddf13u) ^ sx_rr(c->hash, 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x39) << 0;
    c->sched[10] = c->hash ^ sx_rl(c->lane[1], 27);
    c->sched[21] = c->hash ^ sx_rl(c->lane[6], 29);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mark_rate(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    join_delta(c, &c->lane[11], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x82) << 0;
    t1 ^= (uint32_t)defer_token(c, (uint8_t)(t0 >> 16), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 30);
    c->hash = (c->hash * 0x16f66625u) ^ sx_rr(c->hash, 6);
    t2 += (uint32_t)hold_item(c);
    t1 ^= (uint32_t)drain_digest(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe8) << 0;
    chain_head(c, &c->lane[8], 1);
    c->hash ^= c->lane[6] + 0x43d53032u;
    cache_band(c, &c->lane[7], 1);
    t2 += patch_rate(c, c->rlo, c->rln);
    c->lane[0] += c->lane[7]; c->lane[10] ^= c->lane[0]; c->lane[10] = sx_rl(c->lane[10], 6);
    t1 ^= (uint32_t)move_item(c, (uint8_t)(t0 >> 8), t2);
    t2 += cache_port(c, c->slo, c->sln);
    tap_line(c, t0, t1);
    c->hash = (c->hash * 0xa583f7b3u) ^ sx_rr(c->hash, 26);
    t0 ^= sift_entry(c, t1);
    trace_tail(c, &c->lane[11], 1);
    c->hash ^= c->lane[1] + 0x63535c14u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t cache_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdf85f747u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60979u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0xc46d42adu) ^ sx_rr(c->hash, 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= hold_stack(c, t1);
    t2 += shift_value(c, c->rlo, c->rln);
    swap_table(c, t0, t1);
    t1 ^= (uint32_t)pack_cursor(c, (uint8_t)(t0 >> 8), t2);
    tune_cell(c, t0, t1);
    c->lane[11] ^= sx_rl(c->lane[7], 18);
    t0 ^= map_band(c, t1);
    t2 = (t2 ^ c->sum) * 0xa536bd2fu;
    t2 += (uint32_t)slice_pool(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    store_list(c, &c->lane[11], 3);
    t2 = (t2 ^ c->sum) * 0x8828a79bu;
    t2 += pick_bound(c, c->slo, c->sln);
    c->sum += t1;
    return t0 + t2;
}

static void tap_line(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += step_port(c, c->slo, c->sln);
    c->hash = (c->hash * 0x40660adfu) ^ sx_rr(c->hash, 19);
    t2 = (t2 ^ c->sum) * 0xd622ad0du;
    step_pairing(c, &c->lane[3], 3);
    c->raw[c->slo + (int)((t0 + 50039u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    mark_cursor(c, &c->lane[7], 4);
    t0 ^= reap_field(c, t1);
    flush_region(c, t0, t1);
    t2 += pick_row(c, c->rlo, c->rln);
    t1 ^= (uint32_t)load_row_474(c, (uint8_t)(t0 >> 16), t2);
    chain_lease(c, &c->lane[0], 2);
    t2 = (t2 ^ c->sum) * 0xf64e867bu;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint8_t move_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[13] + 0xd1eb31cbu;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 17);
    sort_rate(c, t0, t1);
    t0 ^= mark_unit(c, t1);
    c->raw[c->slo + (int)((t0 + 37843u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[0] += c->lane[6] ^ 0x880c4970u;
    load_page(c, t0, t1);
    t2 += (uint32_t)load_item(c);
    t1 ^= (uint32_t)chain_record_438(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[13] = c->hash ^ sx_rl(c->lane[7], 25);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t hold_stack(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[8] += c->lane[3] ^ 0x79d8a308u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x10e8b7f9u;
    t2 = (t2 ^ c->sum) * 0x7ae927dfu;
    c->lane[11] += c->lane[4] ^ 0x3f1d2352u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x49) << 8;
    c->sched[22] = c->hash ^ sx_rl(c->lane[13], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa8) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14967u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)defer_token(c, (uint8_t)(t0 >> 8), t2);
    flush_list(c, &c->lane[0], 1);
    t2 += (uint32_t)hold_item(c);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t shift_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x656767a5u;
    c->hash ^= c->lane[4] + 0x47961b1du;
    c->lane[9] += c->lane[4]; c->lane[14] ^= c->lane[9]; c->lane[14] = sx_rl(c->lane[14], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb54ba79fu;
    c->raw[c->slo + (int)((t0 + 52270u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += fill_list(c, c->slo, c->sln);
    t0 ^= trim_region(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x1cae02afu;
    sort_rate(c, t0, t1);
    t2 += latch_view(c, c->slo, c->sln);
    c->hash = (c->hash * 0x16fe42f7u) ^ sx_rr(c->hash, 24);
    c->hash = (c->hash * 0xb14fa679u) ^ sx_rr(c->hash, 6);
    c->lane[5] ^= sx_rl(c->lane[14], 25);
    c->lane[5] += c->lane[12]; c->lane[11] ^= c->lane[5]; c->lane[11] = sx_rl(c->lane[11], 3);
    c->lane[13] += c->lane[3]; c->lane[6] ^= c->lane[13]; c->lane[6] = sx_rl(c->lane[6], 14);
    t0 ^= cache_chunk(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static int slice_pool(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[15] += c->lane[8]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 1);
    t2 += fill_list(c, c->rlo, c->rln);
    c->lane[6] += c->lane[6] ^ 0x2c58d126u;
    parse_lease(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x4d7ad179u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[8], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13163u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[10] + 0xd37704bbu;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t pack_cursor(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[16] = c->hash ^ sx_rl(c->lane[8], 28);
    c->lane[8] += c->lane[13] ^ 0x7d2f2480u;
    c->lane[6] ^= sx_rl(c->lane[14], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[15] += c->lane[11] ^ 0xa788bd04u;
    t0 ^= queue_ring(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x08) << 16;
    t1 ^= (uint32_t)flush_rate(c, (uint8_t)(t0 >> 0), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t map_band(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xcd6f8309u;
    c->hash ^= c->lane[4] + 0xc85b9962u;
    sort_line(c, &c->lane[4], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x25) << 0;
    t2 = (t2 ^ c->sum) * 0x50535c0fu;
    trace_tail(c, &c->lane[2], 3);
    t1 ^= (uint32_t)parse_pool(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0b) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int load_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    t2 += patch_rate(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[0] + 0xce278525u;
    c->lane[4] ^= sx_rl(c->lane[8], 5);
    t1 ^= (uint32_t)clamp_tail(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x37662215u;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mark_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] += c->lane[3] ^ 0xab4ec8f1u;
    c->hash = (c->hash * 0xc9a8c75du) ^ sx_rr(c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[10] + 0x936eb216u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42493u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[1] + 0xff9a9a17u;
    patch_delta(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pick_row(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 17);
    emit_run_417(c, t0, t1);
    c->lane[4] += c->lane[14]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 21);
    c->lane[10] += c->lane[11]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7c) << 8;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t mark_unit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += queue_part(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x703a64abu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t1 ^= (uint32_t)store_entry(c, (uint8_t)(t0 >> 8), t2);
    fetch_range(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 23085u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 17);
    t2 += (uint32_t)link_group(c);
    c->sched[0] = c->hash ^ sx_rl(c->lane[14], 6);
    t1 ^= (uint32_t)peek_pairing(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 40568u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[16] = c->hash ^ sx_rl(c->lane[10], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x6b) << 0;
    c->lane[7] += c->lane[5]; c->lane[3] ^= c->lane[7]; c->lane[3] = sx_rl(c->lane[3], 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tune_cell(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] += c->lane[10] ^ 0xb24d071bu;
    c->lane[14] ^= sx_rl(c->lane[11], 12);
    c->raw[c->slo + (int)((t0 + 50279u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[13]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 27);
    relay_seat(c, &c->lane[8], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51135u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)flush_rate(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= poll_tuple(c, t1);
    t0 ^= chain_record(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x08e69081u;
    sort_key(c, &c->lane[0], 4);
    latch_delta(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    parse_lease(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t store_entry(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 23);
    cache_stream(c, t0, t1);
    relay_range_423(c, &c->lane[2], 1);
    t0 ^= swap_seat(c, t1);
    c->sched[20] = c->hash ^ sx_rl(c->lane[2], 7);
    t1 ^= (uint32_t)drain_digest(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38628u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void fetch_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[15] = c->hash ^ sx_rl(c->lane[2], 21);
    c->sched[11] = c->hash ^ sx_rl(c->lane[7], 22);
    t2 = (t2 ^ c->sum) * 0x1772bf97u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x0fedd879u;
    t2 = (t2 ^ c->sum) * 0x041efe45u;
    reset_delta(c, &c->lane[11], 1);
    c->lane[4] += c->lane[14]; c->lane[3] ^= c->lane[4]; c->lane[3] = sx_rl(c->lane[3], 22);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static int link_group(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    fold_seat(c, &c->lane[4], 4);
    t2 += pick_bound(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    push_gap(c, &c->lane[7], 4);
    c->hash ^= c->lane[5] + 0xb9c356d0u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash = (c->hash * 0xec1bce81u) ^ sx_rr(c->hash, 30);
    sync_lease_419(c, &c->lane[1], 2);
    c->raw[c->slo + (int)((t0 + 6288u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void patch_delta(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash = (c->hash * 0xbaa436adu) ^ sx_rr(c->hash, 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26168u) % (uint32_t)c->rln)] << 24;
    c->sched[13] = c->hash ^ sx_rl(c->lane[1], 31);
    c->hash ^= c->lane[2] + 0xa282ac53u;
    t2 = (t2 ^ c->sum) * 0x756b1325u;
    settle_bound(c, &c->lane[8], 2);
    c->lane[15] ^= sx_rl(c->lane[4], 19);
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 25);
    c->hash = (c->hash * 0x802492e9u) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb2f12ee1u;
    c->hash = (c->hash * 0xe5930367u) ^ sx_rr(c->hash, 24);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t defer_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[4] ^= sx_rl(c->lane[15], 2);
    cache_band(c, &c->lane[5], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbe) << 8;
    c->lane[15] ^= sx_rl(c->lane[8], 17);
    c->hash = (c->hash * 0x1a97625du) ^ sx_rr(c->hash, 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    reap_cursor(c, t0, t1);
    t0 ^= split_cursor(c, t1);
    c->hash ^= c->lane[3] + 0x276f064au;
    t2 = (t2 ^ c->sum) * 0xb4d9cbe5u;
    c->lane[10] ^= sx_rl(c->lane[14], 27);
    c->lane[4] += c->lane[5]; c->lane[6] ^= c->lane[4]; c->lane[6] = sx_rl(c->lane[6], 8);
    c->hash ^= c->lane[0] + 0x5736e9d4u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t cache_chunk(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[10] += c->lane[1]; c->lane[8] ^= c->lane[10]; c->lane[8] = sx_rl(c->lane[8], 21);
    c->lane[8] += c->lane[3]; c->lane[4] ^= c->lane[8]; c->lane[4] = sx_rl(c->lane[4], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x197d7c23u;
    t0 ^= fold_unit(c, t1);
    c->lane[8] ^= sx_rl(c->lane[15], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58856u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)shift_record(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)blend_token_422(c);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void sort_line(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += latch_view(c, c->slo, c->sln);
    t2 += (uint32_t)coal_node(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb4) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd8369aa3u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fill_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x208e153fu;
    c->lane[12] += c->lane[7] ^ 0x9e61b260u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += seek_cell(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41015u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)parse_pool(c, (uint8_t)(t0 >> 16), t2);
    c->sum += t1;
    return t0 + t2;
}

static void sort_rate(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4e240921u;
    c->sched[19] = c->hash ^ sx_rl(c->lane[13], 27);
    c->lane[2] += c->lane[11]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 10);
    c->hash = (c->hash * 0xc1a26825u) ^ sx_rr(c->hash, 12);
    c->hash = (c->hash * 0xd8b5af31u) ^ sx_rr(c->hash, 17);
    reap_cursor(c, t0, t1);
    t0 ^= swap_seat(c, t1);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t queue_part(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 29726u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[8] = c->hash ^ sx_rl(c->lane[8], 9);
    c->lane[13] += c->lane[14]; c->lane[8] ^= c->lane[13]; c->lane[8] = sx_rl(c->lane[8], 9);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= c->lane[14] + 0xb2fae465u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31760u) % (uint32_t)c->rln)] << 8;
    c->lane[12] += c->lane[11] ^ 0x44d65111u;
    c->sum += t1;
    return t0 + t2;
}

static void parse_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[9] ^= sx_rl(c->lane[5], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50709u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x2b) << 0;
    c->lane[6] += c->lane[11]; c->lane[0] ^= c->lane[6]; c->lane[0] = sx_rl(c->lane[0], 11);
    t2 = (t2 ^ c->sum) * 0x90c3d187u;
    t1 ^= (uint32_t)emit_digest(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc9) << 0;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 17);
    c->hash ^= c->lane[12] + 0xbf6cbc34u;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static void relay_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    c->lane[0] ^= sx_rl(c->lane[6], 15);
    c->raw[c->slo + (int)((t0 + 63910u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x15) << 0;
    c->hash ^= c->lane[9] + 0x104b83c2u;
    c->lane[2] += c->lane[9] ^ 0x7efe777cu;
    t1 ^= (uint32_t)flush_unit(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe83de04du;
    t2 = (t2 ^ c->sum) * 0x28d23295u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50725u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 52855u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    push_gap(c, &c->lane[10], 4);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 14);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t peek_pairing(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x5d32df6bu) ^ sx_rr(c->hash, 15);
    prime_frame(c, &c->lane[6], 4);
    t1 ^= (uint32_t)clamp_tail(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)align_seat(c);
    c->raw[c->slo + (int)((t0 + 39403u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[8] += c->lane[4]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 18);
    c->hash = (c->hash * 0x6c15ae91u) ^ sx_rr(c->hash, 27);
    t0 ^= chain_store(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t queue_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xb78499e9u) ^ sx_rr(c->hash, 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x69) << 16;
    c->hash = (c->hash * 0x0e03ae79u) ^ sx_rr(c->hash, 23);
    c->hash = (c->hash * 0xda1de5cbu) ^ sx_rr(c->hash, 28);
    c->hash = (c->hash * 0xd3e4d711u) ^ sx_rr(c->hash, 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[10] + 0x307dd592u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4e9b0175u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x28b81121u;
    t0 ^= fold_unit(c, t1);
    c->sched[12] = c->hash ^ sx_rl(c->lane[15], 23);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int hold_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->raw[c->slo + (int)((t0 + 5498u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd6) << 16;
    c->raw[c->slo + (int)((t0 + 56733u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 3);
    t2 += rotate_limit(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 54405u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void trace_tail(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x34929f6du;
    t0 ^= trim_region(c, t1);
    c->lane[4] += c->lane[13]; c->lane[2] ^= c->lane[4]; c->lane[2] = sx_rl(c->lane[2], 4);
    c->lane[4] ^= sx_rl(c->lane[3], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += shift_scope(c, c->slo, c->sln);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xca) << 8;
    t1 ^= (uint32_t)poll_frame(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa69a8dd9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x300347a1u;
    c->lane[13] += c->lane[10] ^ 0xa4fa7866u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_rate(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] += c->lane[13]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 28);
    c->sched[3] = c->hash ^ sx_rl(c->lane[10], 8);
    t2 += seek_cell(c, c->rlo, c->rln);
    t0 ^= sift_entry(c, t1);
    t0 ^= pair_gap(c, t1);
    t2 = (t2 ^ c->sum) * 0x88b01bffu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64359u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[14] ^= sx_rl(c->lane[3], 12);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t chain_record(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] += c->lane[4] ^ 0xc571e337u;
    c->hash ^= c->lane[9] + 0x61c799f0u;
    t2 += (uint32_t)shift_layer(c);
    t0 ^= poll_tuple(c, t1);
    t2 += rotate_delta(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25142u) % (uint32_t)c->rln)] << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t flush_rate(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[3] += c->lane[10] ^ 0x349a4e73u;
    c->lane[10] += c->lane[5]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 13);
    t2 = (t2 ^ c->sum) * 0x453babe3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += (uint32_t)trace_layer(c);
    t1 ^= (uint32_t)chain_record_438(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x075b8d6du) ^ sx_rr(c->hash, 22);
    tune_view(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void emit_run_417(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[7] + 0x75a24261u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xdf) << 16;
    t1 ^= (uint32_t)blend_offset(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)prime_level(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash = (c->hash * 0x65712cbbu) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5c) << 16;
    c->hash = (c->hash * 0x220fbbe3u) ^ sx_rr(c->hash, 23);
    c->hash = (c->hash * 0xca7284d5u) ^ sx_rr(c->hash, 29);
    chain_head(c, &c->lane[0], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58012u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static int align_seat(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[15] += c->lane[10] ^ 0x9ac44111u;
    c->lane[15] += c->lane[8] ^ 0x5f076faeu;
    c->raw[c->slo + (int)((t0 + 61344u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sync_lease_419(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[2] += c->lane[0] ^ 0x8aee7f10u;
    c->lane[5] += c->lane[0] ^ 0x7fa1389bu;
    c->sched[16] = c->hash ^ sx_rl(c->lane[2], 5);
    c->lane[12] += c->lane[7]; c->lane[5] ^= c->lane[12]; c->lane[5] = sx_rl(c->lane[5], 20);
    c->lane[10] ^= sx_rl(c->lane[3], 28);
    t2 += step_port(c, c->rlo, c->rln);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 12);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 23);
    c->hash ^= c->lane[0] + 0xd37e286au;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe2f10069u;
    c->hash = (c->hash * 0x6ce2ded9u) ^ sx_rr(c->hash, 3);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void push_gap(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa7448683u;
    c->lane[2] += c->lane[13]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2f) << 16;
    c->sched[9] = c->hash ^ sx_rl(c->lane[10], 12);
    t2 += pin_arena(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[1] = c->hash ^ sx_rl(c->lane[11], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void reap_cursor(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6366ea45u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61056u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[9] + 0x7438f528u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)swap_entry(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xac) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46960u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x7c2750d7u) ^ sx_rr(c->hash, 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4c) << 8;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int blend_token_422(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->lane[12] += c->lane[5] ^ 0x4c288b65u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x31c32173u;
    c->hash ^= c->lane[11] + 0x9d6e83c2u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 12);
    t0 ^= cache_segment_454(c, t1);
    c->raw[c->slo + (int)((t0 + 21836u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[6] ^= sx_rl(c->lane[7], 25);
    c->lane[11] += c->lane[15] ^ 0xa4680334u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15022u) % (uint32_t)c->rln)] << 0;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void relay_range_423(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x6c2ee64du) ^ sx_rr(c->hash, 7);
    c->raw[c->slo + (int)((t0 + 20353u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0x70b614bfu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5372u) % (uint32_t)c->rln)] << 24;
    c->lane[5] += c->lane[6] ^ 0xe6ce9688u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pair_gap(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= sift_record(c, t1);
    t0 ^= drain_delta(c, t1);
    c->sched[11] = c->hash ^ sx_rl(c->lane[11], 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash = (c->hash * 0xa9479d21u) ^ sx_rr(c->hash, 30);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 2);
    c->lane[10] += c->lane[8] ^ 0xa0c6f198u;
    c->hash ^= c->lane[15] + 0xfcf41901u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc9b4e071u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 15);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 2);
    swap_track(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t seek_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58273u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x8991fb5fu) ^ sx_rr(c->hash, 9);
    c->sched[29] = c->hash ^ sx_rl(c->lane[8], 27);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t trim_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x51fab931u;
    c->hash = (c->hash * 0xcc1d8df1u) ^ sx_rr(c->hash, 6);
    c->hash ^= c->lane[4] + 0x8380556eu;
    c->lane[12] ^= sx_rl(c->lane[10], 20);
    c->hash = (c->hash * 0x6f3e2631u) ^ sx_rr(c->hash, 6);
    c->lane[12] += c->lane[14] ^ 0x6bfd61c9u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[1], 19);
    c->sched[21] = c->hash ^ sx_rl(c->lane[7], 9);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t swap_seat(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    chain_lease(c, &c->lane[11], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5adf1017u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xee5d0983u;
    latch_delta(c, t0, t1);
    fold_seat(c, &c->lane[3], 2);
    t2 += (uint32_t)coal_node(c);
    trim_list(c, &c->lane[6], 1);
    c->hash = (c->hash * 0x188416f3u) ^ sx_rr(c->hash, 6);
    c->hash = (c->hash * 0xb692326bu) ^ sx_rr(c->hash, 5);
    wrap_segment(c, t0, t1);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t split_cursor(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1f) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8430u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x0fd89361u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x964e492bu;
    store_block(c, t0, t1);
    c->sched[0] = c->hash ^ sx_rl(c->lane[3], 21);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int shift_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    sort_key(c, &c->lane[4], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 29282u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[4] += c->lane[2] ^ 0x7a2fe5c6u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[1], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5733u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb0c59efdu;
    t2 += (uint32_t)fold_layer_499(c);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 25);
    t2 += trace_token(c, c->rlo, c->rln);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t blend_offset(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] += c->lane[15] ^ 0x5b379ef1u;
    c->lane[15] ^= sx_rl(c->lane[2], 18);
    c->sched[9] = c->hash ^ sx_rl(c->lane[14], 4);
    t2 += trim_pool_477(c, c->slo, c->sln);
    c->hash = (c->hash * 0x7515cf09u) ^ sx_rr(c->hash, 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4cd09903u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4f9e19f3u;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 7);
    c->lane[4] += c->lane[13] ^ 0x38258125u;
    c->lane[8] ^= sx_rl(c->lane[15], 3);
    flush_region(c, t0, t1);
    c->hash ^= c->lane[8] + 0xc57da301u;
    c->lane[6] += c->lane[3] ^ 0xc6d8960au;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t parse_pool(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63349u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9c) << 0;
    flush_list(c, &c->lane[9], 3);
    latch_queue(c, &c->lane[4], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56064u) % (uint32_t)c->rln)] << 24;
    c->sched[29] = c->hash ^ sx_rl(c->lane[15], 6);
    c->hash ^= c->lane[7] + 0x83a966b8u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void tune_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa01650adu;
    c->hash = (c->hash * 0x404d8bddu) ^ sx_rr(c->hash, 26);
    c->sched[2] = c->hash ^ sx_rl(c->lane[15], 15);
    c->lane[7] ^= sx_rl(c->lane[5], 26);
    flush_store(c, &c->lane[3], 3);
    c->lane[10] ^= sx_rl(c->lane[6], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39432u) % (uint32_t)c->rln)] << 8;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 23);
    patch_entry(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] += c->lane[9] ^ 0xf769d0f4u;
    cache_stream(c, t0, t1);
    t2 += (uint32_t)trace_layer(c);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint32_t latch_view(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[1] += c->lane[4] ^ 0x3dd233ceu;
    t0 ^= flush_path_507(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[13] + 0xe7146e34u;
    c->lane[6] += c->lane[0] ^ 0x2c2d1003u;
    t1 ^= (uint32_t)reset_region(c, (uint8_t)(t0 >> 0), t2);
    t2 += fold_path(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x84759343u) ^ sx_rr(c->hash, 30);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t poll_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 349u) % (uint32_t)c->rln)] << 16;
    c->lane[1] ^= sx_rl(c->lane[2], 30);
    c->lane[1] += c->lane[12]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 3);
    swap_table(c, t0, t1);
    store_list(c, &c->lane[1], 2);
    c->hash = (c->hash * 0x676eff9fu) ^ sx_rr(c->hash, 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52744u) % (uint32_t)c->rln)] << 16;
    c->sched[28] = c->hash ^ sx_rl(c->lane[10], 11);
    settle_bound(c, &c->lane[8], 1);
    c->lane[13] ^= sx_rl(c->lane[4], 4);
    c->lane[15] += c->lane[14] ^ 0xb34b6f47u;
    c->raw[c->slo + (int)((t0 + 24438u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t rotate_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x7d) << 16;
    t2 = (t2 ^ c->sum) * 0x87a04febu;
    c->lane[13] ^= sx_rl(c->lane[0], 10);
    c->sum += t1;
    return t0 + t2;
}

static void prime_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    join_delta(c, &c->lane[0], 1);
    c->sched[18] = c->hash ^ sx_rl(c->lane[15], 30);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 8);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 26);
    t2 = (t2 ^ c->sum) * 0x30884391u;
    c->raw[c->slo + (int)((t0 + 29429u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 45964u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t shift_record(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xdb4e4f87u;
    c->raw[c->slo + (int)((t0 + 26476u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[4] + 0x1c39438eu;
    c->sched[26] = c->hash ^ sx_rl(c->lane[1], 27);
    c->sched[10] = c->hash ^ sx_rl(c->lane[15], 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t chain_record_438(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    move_queue(c, &c->lane[2], 4);
    c->lane[14] ^= sx_rl(c->lane[15], 23);
    c->sched[2] = c->hash ^ sx_rl(c->lane[8], 1);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    c->lane[4] += c->lane[1] ^ 0xc81ae30fu;
    t0 ^= reap_field(c, t1);
    c->lane[2] += c->lane[10]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 23);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 28);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t poll_frame(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[11] = c->hash ^ sx_rl(c->lane[8], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4bbb7575u;
    blend_pairing(c, &c->lane[3], 1);
    c->lane[15] ^= sx_rl(c->lane[9], 6);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 31);
    c->lane[15] += c->lane[2]; c->lane[7] ^= c->lane[15]; c->lane[7] = sx_rl(c->lane[7], 25);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t fold_unit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t1 ^= (uint32_t)settle_row_472(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 32867u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] += c->lane[11]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 24);
    c->hash ^= c->lane[2] + 0x4be32b3cu;
    step_pairing(c, &c->lane[3], 2);
    t1 ^= (uint32_t)reset_delta_455(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe3) << 8;
    t2 = (t2 ^ c->sum) * 0x93776915u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t chain_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    split_part(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30750u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[15] + 0x7643ffabu;
    c->lane[12] ^= sx_rl(c->lane[7], 8);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 12);
    c->sched[25] = c->hash ^ sx_rl(c->lane[12], 19);
    t2 = (t2 ^ c->sum) * 0x3dd41263u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0x8aa12dafu) ^ sx_rr(c->hash, 10);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void reset_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= push_line_459(c, t1);
    t2 += tune_limit(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd2) << 8;
    t2 = (t2 ^ c->sum) * 0x2310e53fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8d1240e7u;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 7);
    c->raw[c->slo + (int)((t0 + 23865u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x8a0b822fu) ^ sx_rr(c->hash, 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7112cf71u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x302b4f31u;
    c->hash = (c->hash * 0x0650bf3bu) ^ sx_rr(c->hash, 28);
    c->sched[10] = c->hash ^ sx_rl(c->lane[13], 28);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t drain_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x65af2757u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x256635d7u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 27);
    c->lane[13] ^= sx_rl(c->lane[11], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0b148699u;
    c->lane[2] += c->lane[13] ^ 0x2d91c12du;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t shift_scope(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] += c->lane[15] ^ 0x8c180f42u;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf3a859f7u;
    c->lane[11] += c->lane[14]; c->lane[5] ^= c->lane[11]; c->lane[5] = sx_rl(c->lane[5], 9);
    c->sched[10] = c->hash ^ sx_rl(c->lane[5], 11);
    t2 += pin_arena(c, c->rlo, c->rln);
    c->sched[18] = c->hash ^ sx_rl(c->lane[5], 12);
    t2 += (uint32_t)flush_item(c);
    c->hash ^= c->lane[1] + 0x687e7cb0u;
    c->lane[13] ^= sx_rl(c->lane[10], 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x4da8486fu;
    map_index(c, &c->lane[2], 4);
    c->lane[13] += c->lane[10] ^ 0xe2c796ceu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t clamp_tail(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 23224u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0xaf0789d1u;
    c->hash = (c->hash * 0x440a0d2bu) ^ sx_rr(c->hash, 1);
    c->raw[c->slo + (int)((t0 + 37996u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)mark_offset_490(c, (uint8_t)(t0 >> 8), t2);
    c->lane[13] ^= sx_rl(c->lane[6], 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t pick_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa6058ee9u;
    t1 ^= (uint32_t)load_row_474(c, (uint8_t)(t0 >> 8), t2);
    c->lane[7] += c->lane[11]; c->lane[10] ^= c->lane[7]; c->lane[10] = sx_rl(c->lane[10], 13);
    c->lane[9] += c->lane[3] ^ 0x0cc13f25u;
    c->lane[7] ^= sx_rl(c->lane[10], 3);
    t2 += close_bound(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 32731u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf9035c2du;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t sift_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 11);
    t1 ^= (uint32_t)split_range(c, (uint8_t)(t0 >> 8), t2);
    t2 = (t2 ^ c->sum) * 0x9272e373u;
    c->lane[3] += c->lane[0] ^ 0x82635283u;
    c->lane[4] ^= sx_rl(c->lane[9], 10);
    split_rate(c, &c->lane[9], 4);
    c->lane[6] ^= sx_rl(c->lane[0], 3);
    c->hash ^= c->lane[13] + 0x9788a1bau;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 22);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t rotate_limit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += tune_limit(c, c->rlo, c->rln);
    t1 ^= (uint32_t)mark_slot(c, (uint8_t)(t0 >> 8), t2);
    latch_queue(c, &c->lane[10], 1);
    c->lane[1] += c->lane[14] ^ 0xdc167e44u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t1 ^= (uint32_t)flush_unit(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[2] + 0xed86951fu;
    c->hash ^= c->lane[13] + 0x31b528ddu;
    c->lane[10] ^= sx_rl(c->lane[2], 14);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 29);
    c->lane[9] ^= sx_rl(c->lane[9], 16);
    c->lane[11] += c->lane[1]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 21);
    c->lane[5] ^= sx_rl(c->lane[10], 18);
    c->sum += t1;
    return t0 + t2;
}

static void chain_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xf605931bu) ^ sx_rr(c->hash, 14);
    t0 ^= reap_count(c, t1);
    t2 += fill_stream(c, c->slo, c->sln);
    link_part(c, t0, t1);
    c->lane[11] += c->lane[4]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 13);
    t2 = (t2 ^ c->sum) * 0x59147a3du;
    c->raw[c->slo + (int)((t0 + 33347u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf8431e6fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x58) << 0;
    t2 = (t2 ^ c->sum) * 0x86234009u;
    t2 = (t2 ^ c->sum) * 0x1f248c0fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int prime_level(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash ^= c->lane[7] + 0x47002adbu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[14] += c->lane[4] ^ 0xbf59091eu;
    load_page(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17024u) % (uint32_t)c->rln)] << 0;
    c->lane[8] ^= sx_rl(c->lane[4], 13);
    c->raw[c->slo + (int)((t0 + 29826u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[1] ^= sx_rl(c->lane[5], 19);
    t2 += slice_mask(c, c->slo, c->sln);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void cache_band(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[2], 18);
    c->raw[c->slo + (int)((t0 + 42698u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x0170e25bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xc4cc0e31u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38518u) % (uint32_t)c->rln)] << 0;
    c->sched[12] = c->hash ^ sx_rl(c->lane[1], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 ^= prime_pool(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43860u) % (uint32_t)c->rln)] << 16;
    t2 += settle_span(c, c->slo, c->sln);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t emit_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x629da909u;
    c->lane[12] += c->lane[9]; c->lane[8] ^= c->lane[12]; c->lane[8] = sx_rl(c->lane[8], 12);
    c->sched[9] = c->hash ^ sx_rl(c->lane[7], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    tune_scope(c, &c->lane[11], 3);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x75dda543u;
    t2 += defer_page(c, c->slo, c->sln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t trace_token(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 28);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 11);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t cache_segment_454(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] ^= sx_rl(c->lane[7], 2);
    c->sched[5] = c->hash ^ sx_rl(c->lane[15], 7);
    c->sched[7] = c->hash ^ sx_rl(c->lane[2], 6);
    c->raw[c->slo + (int)((t0 + 49429u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0xe6afb3a5u;
    c->raw[c->slo + (int)((t0 + 19800u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[10] += c->lane[12] ^ 0xa703d120u;
    t2 = (t2 ^ c->sum) * 0x56af016fu;
    t2 = (t2 ^ c->sum) * 0x79440311u;
    c->hash ^= c->lane[13] + 0x0bc63bccu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t reset_delta_455(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x7436b043u;
    c->hash = (c->hash * 0x6ab8f7adu) ^ sx_rr(c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[8] ^= sx_rl(c->lane[7], 13);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int coal_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x12) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52379u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x6b845277u;
    t2 = (t2 ^ c->sum) * 0x6253a023u;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void flush_list(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x0641a989u) ^ sx_rr(c->hash, 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8a71c4c1u;
    c->sched[10] = c->hash ^ sx_rl(c->lane[0], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x83) << 16;
    c->sched[20] = c->hash ^ sx_rl(c->lane[3], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int trace_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50637u) % (uint32_t)c->rln)] << 8;
    c->lane[4] += c->lane[2]; c->lane[10] ^= c->lane[4]; c->lane[10] = sx_rl(c->lane[10], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd9d68295u;
    c->lane[0] += c->lane[15] ^ 0x305aa1fau;
    c->hash = (c->hash * 0x70896e9fu) ^ sx_rr(c->hash, 12);
    c->lane[14] += c->lane[0]; c->lane[9] ^= c->lane[14]; c->lane[9] = sx_rl(c->lane[9], 11);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t push_line_459(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[3] ^ 0x824ac63cu;
    c->lane[1] += c->lane[3] ^ 0xcac6a3dbu;
    c->hash ^= c->lane[2] + 0x8c176dabu;
    c->sched[10] = c->hash ^ sx_rl(c->lane[13], 23);
    c->lane[1] ^= sx_rl(c->lane[8], 21);
    c->hash = (c->hash * 0x85ae39d3u) ^ sx_rr(c->hash, 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf0) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void flush_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 56423u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[0] += c->lane[5]; c->lane[9] ^= c->lane[0]; c->lane[9] = sx_rl(c->lane[9], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x742bdb37u;
    t2 = (t2 ^ c->sum) * 0x3da45f47u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 1);
    c->lane[12] += c->lane[14] ^ 0xbf7a1c0au;
    c->hash ^= c->lane[7] + 0x53eede6fu;
    c->hash = (c->hash * 0x1386755fu) ^ sx_rr(c->hash, 18);
    c->hash ^= c->lane[4] + 0x6fa199f8u;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t reset_region(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x822f7493u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x31) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe0eeaf23u;
    c->hash = (c->hash * 0xc818edafu) ^ sx_rr(c->hash, 14);
    c->lane[0] ^= sx_rl(c->lane[11], 28);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void store_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] += c->lane[1]; c->lane[13] ^= c->lane[3]; c->lane[13] = sx_rl(c->lane[13], 28);
    c->raw[c->slo + (int)((t0 + 63320u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] ^= sx_rl(c->lane[11], 16);
    c->raw[c->slo + (int)((t0 + 16400u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t tune_limit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] += c->lane[7]; c->lane[2] ^= c->lane[11]; c->lane[2] = sx_rl(c->lane[2], 18);
    c->lane[8] ^= sx_rl(c->lane[8], 1);
    c->sched[10] = c->hash ^ sx_rl(c->lane[10], 8);
    t2 = (t2 ^ c->sum) * 0xede124a9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52095u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[0] + 0x3d556732u;
    c->lane[6] ^= sx_rl(c->lane[10], 20);
    c->lane[5] ^= sx_rl(c->lane[1], 17);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t slice_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[0] += c->lane[6] ^ 0x9d2274cau;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[4] ^= sx_rl(c->lane[1], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0x78b3a401u) ^ sx_rr(c->hash, 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sched[3] = c->hash ^ sx_rl(c->lane[9], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xcf) << 8;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 31);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t drain_delta(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[22] = c->hash ^ sx_rl(c->lane[15], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[9] ^= sx_rl(c->lane[9], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1dea1015u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xaabd5da5u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t mark_slot(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[14] ^= sx_rl(c->lane[6], 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[15] += c->lane[3]; c->lane[14] ^= c->lane[15]; c->lane[14] = sx_rl(c->lane[14], 12);
    c->lane[3] ^= sx_rl(c->lane[2], 13);
    c->lane[6] += c->lane[8] ^ 0xc8f0b369u;
    c->hash ^= c->lane[5] + 0x883420d0u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2115u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 947u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc7) << 8;
    c->sched[10] = c->hash ^ sx_rl(c->lane[12], 15);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t settle_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xedac0117u;
    c->lane[13] += c->lane[0]; c->lane[7] ^= c->lane[13]; c->lane[7] = sx_rl(c->lane[7], 5);
    c->lane[3] ^= sx_rl(c->lane[0], 18);
    c->lane[9] += c->lane[14] ^ 0xd395278eu;
    t2 = (t2 ^ c->sum) * 0x9b8da3d7u;
    c->raw[c->slo + (int)((t0 + 8978u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void patch_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[9] += c->lane[0]; c->lane[3] ^= c->lane[9]; c->lane[3] = sx_rl(c->lane[3], 17);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 5);
    c->raw[c->slo + (int)((t0 + 34286u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[10] ^= sx_rl(c->lane[10], 18);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65111u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x3edb7009u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe3) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13371u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void load_page(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] += c->lane[5] ^ 0xf7fdf021u;
    c->hash = (c->hash * 0x09eb9667u) ^ sx_rr(c->hash, 25);
    c->lane[2] ^= sx_rl(c->lane[0], 20);
    c->hash ^= c->lane[10] + 0xd5aaf2e5u;
    c->lane[9] ^= sx_rl(c->lane[8], 11);
    c->hash ^= c->lane[6] + 0xc0527991u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t step_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[13] + 0x6189ca4fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52811u) % (uint32_t)c->rln)] << 8;
    c->lane[5] ^= sx_rl(c->lane[7], 1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t reap_field(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[1] = c->hash ^ sx_rl(c->lane[9], 28);
    c->lane[1] ^= sx_rl(c->lane[14], 10);
    c->hash = (c->hash * 0x8574d10bu) ^ sx_rr(c->hash, 26);
    c->raw[c->slo + (int)((t0 + 60941u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xb4401533u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 24);
    c->raw[c->slo + (int)((t0 + 10536u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 11);
    c->hash = (c->hash * 0xd3131ee5u) ^ sx_rr(c->hash, 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[3] += c->lane[14]; c->lane[13] ^= c->lane[3]; c->lane[13] = sx_rl(c->lane[13], 6);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t settle_row_472(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[3] + 0x479d9499u;
    c->lane[9] += c->lane[11]; c->lane[6] ^= c->lane[9]; c->lane[6] = sx_rl(c->lane[6], 20);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 1);
    c->lane[2] ^= sx_rl(c->lane[14], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 30);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t fold_path(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] += c->lane[4] ^ 0xe3e7c5c3u;
    c->hash ^= c->lane[9] + 0x40c9483fu;
    c->lane[10] += c->lane[5] ^ 0xc23eed65u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61886u) % (uint32_t)c->rln)] << 0;
    c->sched[21] = c->hash ^ sx_rl(c->lane[15], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] += c->lane[7]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 11);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t load_row_474(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] ^= sx_rl(c->lane[15], 1);
    c->lane[2] ^= sx_rl(c->lane[14], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17500u) % (uint32_t)c->rln)] << 0;
    c->sched[12] = c->hash ^ sx_rl(c->lane[12], 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 19607u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[23] = c->hash ^ sx_rl(c->lane[11], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xdf266fedu;
    c->hash = (c->hash * 0x4effa3a5u) ^ sx_rr(c->hash, 14);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void latch_delta(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[1] += c->lane[9] ^ 0x397f7f98u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash = (c->hash * 0xef5acc5bu) ^ sx_rr(c->hash, 24);
    c->hash ^= c->lane[1] + 0x46d3ef4eu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe2035b6bu;
    t2 = (t2 ^ c->sum) * 0xbc2f913bu;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void blend_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xee1a7ac9u) ^ sx_rr(c->hash, 13);
    c->hash ^= c->lane[2] + 0xec9dde89u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8a2a4bb9u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 26);
    c->hash = (c->hash * 0x9dbcd147u) ^ sx_rr(c->hash, 14);
    c->hash = (c->hash * 0x49427601u) ^ sx_rr(c->hash, 11);
    c->hash = (c->hash * 0xeec2f8b9u) ^ sx_rr(c->hash, 15);
    c->raw[c->slo + (int)((t0 + 26147u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[1] += c->lane[10] ^ 0x7e9650bdu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t trim_pool_477(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[20] = c->hash ^ sx_rl(c->lane[8], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60748u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x43878b81u;
    c->lane[5] ^= sx_rl(c->lane[8], 14);
    c->hash = (c->hash * 0x887e1229u) ^ sx_rr(c->hash, 1);
    c->sum += t1;
    return t0 + t2;
}

static void store_list(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] += c->lane[2]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 30);
    t2 = (t2 ^ c->sum) * 0x6ea13121u;
    c->raw[c->slo + (int)((t0 + 38040u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 23);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8657e0f1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41255u) % (uint32_t)c->rln)] << 0;
    c->lane[11] += c->lane[3] ^ 0xa0725669u;
    c->lane[5] += c->lane[2]; c->lane[10] ^= c->lane[5]; c->lane[10] = sx_rl(c->lane[10], 13);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 28);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void link_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[6] ^= sx_rl(c->lane[13], 28);
    c->lane[6] += c->lane[2]; c->lane[4] ^= c->lane[6]; c->lane[4] = sx_rl(c->lane[4], 20);
    c->hash ^= c->lane[7] + 0x555b2b88u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[6] ^= sx_rl(c->lane[15], 12);
    c->sched[5] = c->hash ^ sx_rl(c->lane[4], 14);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void cache_stream(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] ^= sx_rl(c->lane[0], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xdc) << 8;
    c->hash ^= c->lane[6] + 0xc6322410u;
    t2 = (t2 ^ c->sum) * 0x2b030651u;
    c->lane[3] += c->lane[0]; c->lane[2] ^= c->lane[3]; c->lane[2] = sx_rl(c->lane[2], 21);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 27);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t fill_stream(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xd4edd891u) ^ sx_rr(c->hash, 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe4cfff87u;
    t2 = (t2 ^ c->sum) * 0x1f4b4f3du;
    c->sched[4] = c->hash ^ sx_rl(c->lane[8], 1);
    c->lane[8] += c->lane[15]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 3);
    c->sum += t1;
    return t0 + t2;
}

static void fold_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[29] = c->hash ^ sx_rl(c->lane[1], 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35564u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0x88c05c0fu) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa7) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t flush_unit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[7] += c->lane[15]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 10);
    c->lane[13] += c->lane[14] ^ 0x34a882c8u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x55) << 0;
    c->lane[10] ^= sx_rl(c->lane[5], 19);
    c->hash ^= c->lane[7] + 0x6e0b8a18u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void sort_key(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 13);
    c->sched[29] = c->hash ^ sx_rl(c->lane[14], 2);
    c->sched[22] = c->hash ^ sx_rl(c->lane[13], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xdfba6d2bu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 6);
    c->lane[1] += c->lane[9]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 29);
    c->sched[5] = c->hash ^ sx_rl(c->lane[4], 7);
    t2 = (t2 ^ c->sum) * 0x00d41421u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x114f2ac3u;
    c->sched[28] = c->hash ^ sx_rl(c->lane[3], 14);
    c->hash = (c->hash * 0x4d90e649u) ^ sx_rr(c->hash, 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void map_index(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbd) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc0) << 0;
    c->lane[13] += c->lane[11] ^ 0xa5d20989u;
    c->raw[c->slo + (int)((t0 + 12004u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57361u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x9ca0babfu;
    c->hash ^= c->lane[0] + 0xb6296524u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void split_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[12], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[9] += c->lane[5] ^ 0x8074cad2u;
    c->raw[c->slo + (int)((t0 + 34493u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11618u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void chain_lease(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xec308a43u) ^ sx_rr(c->hash, 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2710u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 23626u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] ^= sx_rl(c->lane[1], 14);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void join_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 63890u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd5) << 8;
    c->lane[6] += c->lane[15]; c->lane[11] ^= c->lane[6]; c->lane[11] = sx_rl(c->lane[11], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x24) << 16;
    c->lane[11] += c->lane[9]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 13);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void swap_table(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc65906abu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[3] += c->lane[10] ^ 0xf64114bau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13434u) % (uint32_t)c->rln)] << 16;
    c->lane[0] += c->lane[1]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 21);
    c->lane[12] += c->lane[1]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 22);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 2);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint8_t mark_offset_490(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] += c->lane[4]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 9);
    c->lane[1] += c->lane[8]; c->lane[7] ^= c->lane[1]; c->lane[7] = sx_rl(c->lane[7], 15);
    c->lane[12] += c->lane[11]; c->lane[3] ^= c->lane[12]; c->lane[3] = sx_rl(c->lane[3], 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x12) << 0;
    c->lane[1] ^= sx_rl(c->lane[10], 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3c) << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void flush_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x76f3eefbu;
    c->lane[15] += c->lane[7] ^ 0x6fafb934u;
    c->sched[27] = c->hash ^ sx_rl(c->lane[14], 20);
    c->raw[c->slo + (int)((t0 + 33886u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[14] = c->hash ^ sx_rl(c->lane[11], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t defer_page(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x1709b85fu) ^ sx_rr(c->hash, 7);
    c->lane[4] += c->lane[9] ^ 0xac51a40cu;
    c->lane[11] += c->lane[6]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 16);
    c->lane[7] += c->lane[1]; c->lane[9] ^= c->lane[7]; c->lane[9] = sx_rl(c->lane[9], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xad2796dbu;
    swap_head(c, &c->lane[0], 4);
    c->sum += t1;
    return t0 + t2;
}

static void swap_track(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[12] ^= sx_rl(c->lane[1], 16);
    c->hash ^= c->lane[10] + 0xe007a6d7u;
    c->lane[0] += c->lane[2]; c->lane[4] ^= c->lane[0]; c->lane[4] = sx_rl(c->lane[4], 3);
    c->hash = (c->hash * 0x97acf5cbu) ^ sx_rr(c->hash, 10);
    c->raw[c->slo + (int)((t0 + 2870u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x132b7341u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint8_t split_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb2) << 0;
    c->hash = (c->hash * 0x4fa7b6b9u) ^ sx_rr(c->hash, 8);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x63070fd9u;
    c->lane[9] += c->lane[15]; c->lane[7] ^= c->lane[9]; c->lane[7] = sx_rl(c->lane[7], 23);
    c->raw[c->slo + (int)((t0 + 25467u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3144u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0xec9dd763u) ^ sx_rr(c->hash, 29);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void trim_list(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[15] ^ 0x584fabe9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd2948831u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 30);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 9);
    poll_unit(c, t0, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pin_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[12] = c->hash ^ sx_rl(c->lane[8], 27);
    c->hash ^= c->lane[9] + 0x06465fabu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 2);
    t2 = (t2 ^ c->sum) * 0x0c8bb269u;
    c->lane[3] += c->lane[2] ^ 0x1e11b9d8u;
    c->sum += t1;
    return t0 + t2;
}

static void wrap_segment(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[11] += c->lane[5] ^ 0xf5e9e9f7u;
    c->lane[8] ^= sx_rl(c->lane[3], 29);
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[12] += c->lane[0] ^ 0xb3f9c55au;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t close_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1326u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x3a19dae1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64215u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[10], 23);
    t2 = (t2 ^ c->sum) * 0x4fabd77bu;
    c->hash = (c->hash * 0x9fd081c3u) ^ sx_rr(c->hash, 22);
    c->lane[8] ^= sx_rl(c->lane[6], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static int fold_layer_499(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->lane[8] += c->lane[2] ^ 0x0feb1efdu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc4c1d233u;
    c->hash = (c->hash * 0x0f89c273u) ^ sx_rr(c->hash, 28);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t swap_entry(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9f) << 8;
    t2 = (t2 ^ c->sum) * 0x2afe5bdbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24181u) % (uint32_t)c->rln)] << 24;
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 6);
    c->raw[c->slo + (int)((t0 + 62775u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x011ddbd3u;
    c->lane[0] += c->lane[12] ^ 0xc0d69391u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xfa4bd879u;
    c->lane[15] += c->lane[2]; c->lane[13] ^= c->lane[15]; c->lane[13] = sx_rl(c->lane[13], 17);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void step_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x60) << 8;
    c->sched[2] = c->hash ^ sx_rl(c->lane[1], 6);
    c->lane[9] ^= sx_rl(c->lane[11], 26);
    c->lane[6] ^= sx_rl(c->lane[12], 12);
    c->sched[30] = c->hash ^ sx_rl(c->lane[2], 3);
    c->raw[c->slo + (int)((t0 + 13478u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t reap_count(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xeb87bcd3u;
    c->lane[10] ^= sx_rl(c->lane[6], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[8] + 0x82a1572fu;
    c->hash ^= c->lane[15] + 0x5c8b934fu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void move_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x1671d6e5u) ^ sx_rr(c->hash, 21);
    t2 = (t2 ^ c->sum) * 0x3e111e4bu;
    c->hash ^= c->lane[14] + 0x8cb41acfu;
    c->hash ^= c->lane[0] + 0xc1b5da36u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x745c2e47u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int flush_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[7] + 0x30e53cbfu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8111u) % (uint32_t)c->rln)] << 0;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void latch_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x29) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe7) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] += c->lane[3]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 20);
    c->lane[1] ^= sx_rl(c->lane[2], 27);
    c->raw[c->slo + (int)((t0 + 16692u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xfebd37f7u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void split_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x12a038e7u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 20);
    c->lane[2] += c->lane[3]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 19);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 27);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t flush_path_507(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[15], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcae7d47du;
    c->raw[c->slo + (int)((t0 + 47642u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[31] = c->hash ^ sx_rl(c->lane[6], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[11] ^= sx_rl(c->lane[4], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xdc28bca7u;
    c->lane[10] += c->lane[11] ^ 0xcee5b8afu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[31] = c->hash ^ sx_rl(c->lane[14], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t prime_pool(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 495u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0xc1e45633u) ^ sx_rr(c->hash, 21);
    c->lane[11] ^= sx_rl(c->lane[0], 24);
    c->lane[1] ^= sx_rl(c->lane[9], 12);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tune_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbe29cc43u;
    c->lane[4] ^= sx_rl(c->lane[0], 21);
    t2 = (t2 ^ c->sum) * 0x24f40b71u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 29);
    c->lane[12] ^= sx_rl(c->lane[4], 5);
    c->lane[11] += c->lane[3] ^ 0xa2d491f4u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[11], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa7d014cdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48196u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x122a69fdu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void settle_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x53ad1f9du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xcb) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[9] += c->lane[9] ^ 0xafb5f61du;
    c->lane[7] ^= sx_rl(c->lane[0], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5a3aa053u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26661u) % (uint32_t)c->rln)] << 16;
    c->lane[12] += c->lane[2] ^ 0x4c525be0u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t sift_record(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x607fc507u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 4976u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] += c->lane[13] ^ 0xa78fc37du;
    c->hash = (c->hash * 0x11e9eb4bu) ^ sx_rr(c->hash, 29);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pin_list(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    hold_row(c, t0, t1);
    t2 += (uint32_t)load_span_567(c);
    t0 ^= pin_frame(c, t1);
    mix_node_561(c, &c->lane[7], 3);
    sync_state(c, t0, t1);
    place_tail(c, &c->lane[4], 1);
    sort_pairing(c, t0, t1);
    close_span(c, t0, t1);
    trim_lease(c, &c->lane[8], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    rotate_head(c, t0, t1);
    slice_head(c, t0, t1);
    slice_batch(c, &c->lane[10], 2);
    c->lane[6] += c->lane[13]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 6);
    pin_level(c, t0, t1);
    t2 += (uint32_t)seek_rate(c);
    c->lane[12] += c->lane[9]; c->lane[3] ^= c->lane[12]; c->lane[3] = sx_rl(c->lane[3], 31);
    t2 += tally_index(c, c->rlo, c->rln);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc1fa83b9u;
    t2 += fill_frame(c, c->rlo, c->rln);
    align_record(c, t0, t1);
    chain_count(c, &c->lane[8], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void close_span(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 51664u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    push_page(c, t0, t1);
    c->hash ^= c->lane[2] + 0x49bfbbdfu;
    flush_block(c, &c->lane[10], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x60) << 0;
    yield_view(c, t0, t1);
    resize_field(c, &c->lane[11], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4254u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[14] + 0xa43161f6u;
    t1 ^= (uint32_t)fold_part(c, (uint8_t)(t0 >> 16), t2);
    defer_field(c, t0, t1);
    purge_tuple(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t pin_frame(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x3f) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x294d76b3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 += resize_delta(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 += cache_value(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 41491u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += (uint32_t)scan_row(c);
    c->lane[10] += c->lane[7]; c->lane[1] ^= c->lane[10]; c->lane[1] = sx_rl(c->lane[1], 6);
    t2 = (t2 ^ c->sum) * 0x09b1cf43u;
    t2 += relay_region(c, c->slo, c->sln);
    t2 += poll_arena(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32281u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0x8da4ea8bu) ^ sx_rr(c->hash, 10);
    c->lane[10] ^= sx_rl(c->lane[11], 26);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fill_frame(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    parse_queue(c, &c->lane[1], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x5edd49f7u) ^ sx_rr(c->hash, 30);
    t0 ^= resize_span(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 += clamp_page(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t0 ^= pin_lease(c, t1);
    t2 += blend_layer(c, c->slo, c->sln);
    t1 ^= (uint32_t)poll_view(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x687f945du;
    t2 += (uint32_t)link_run(c);
    c->sched[24] = c->hash ^ sx_rl(c->lane[5], 11);
    c->sum += t1;
    return t0 + t2;
}

static int link_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    t2 += (uint32_t)link_table(c);
    c->lane[1] += c->lane[8] ^ 0xbd61dd98u;
    place_tail(c, &c->lane[5], 4);
    t1 ^= (uint32_t)grow_row(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[10] + 0xb4a4128du;
    c->lane[13] ^= sx_rl(c->lane[6], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x46f4a97bu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 30);
    c->lane[3] ^= sx_rl(c->lane[15], 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf5) << 0;
    t2 = (t2 ^ c->sum) * 0xda243a89u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[7], 28);
    t0 ^= pin_lease(c, t1);
    t2 = (t2 ^ c->sum) * 0xc55648cbu;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void push_page(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xec6d1f3du;
    c->hash = (c->hash * 0xea68b0e9u) ^ sx_rr(c->hash, 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18804u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[5] + 0xbbeafca0u;
    c->hash ^= c->lane[12] + 0x6dee80d3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x91) << 16;
    c->lane[12] += c->lane[1]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 11);
    seek_record(c, &c->lane[11], 2);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void purge_tuple(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x0c9ef961u;
    reset_store(c, &c->lane[1], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0xc37183b1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static void defer_field(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[15] += c->lane[0] ^ 0xb9161cb0u;
    chain_slot(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb7c4a9a3u;
    t1 ^= (uint32_t)fold_span(c, (uint8_t)(t0 >> 8), t2);
    resize_field(c, &c->lane[2], 3);
    t1 ^= (uint32_t)stage_digest(c, (uint8_t)(t0 >> 0), t2);
    t2 += wrap_stack(c, c->rlo, c->rln);
    t2 += (uint32_t)yield_span(c);
    t2 = (t2 ^ c->sum) * 0xc1bc5a51u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50430u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61314u) % (uint32_t)c->rln)] << 0;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t clamp_page(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    clamp_seat(c, t0, t1);
    c->lane[12] += c->lane[14] ^ 0xaf773224u;
    c->hash = (c->hash * 0xf66f1de3u) ^ sx_rr(c->hash, 22);
    grow_queue(c, t0, t1);
    c->hash = (c->hash * 0x772f154bu) ^ sx_rr(c->hash, 1);
    c->raw[c->slo + (int)((t0 + 10196u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += (uint32_t)trim_queue(c);
    c->lane[11] += c->lane[15]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 26);
    c->lane[4] += c->lane[5] ^ 0xe7b74941u;
    c->sched[13] = c->hash ^ sx_rl(c->lane[11], 23);
    t1 ^= (uint32_t)fold_limit(c, (uint8_t)(t0 >> 16), t2);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t cache_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 38181u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)mark_cell(c);
    t2 = (t2 ^ c->sum) * 0x804fa5a9u;
    t2 += (uint32_t)latch_row(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[19] = c->hash ^ sx_rl(c->lane[5], 18);
    sort_pairing(c, t0, t1);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 25);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t resize_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xfac9c879u) ^ sx_rr(c->hash, 22);
    t1 ^= (uint32_t)map_pool(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x7c2bf553u) ^ sx_rr(c->hash, 30);
    t2 = (t2 ^ c->sum) * 0x01a2658fu;
    queue_offset(c, t0, t1);
    c->sched[16] = c->hash ^ sx_rl(c->lane[6], 10);
    c->hash ^= c->lane[11] + 0x27b0f64cu;
    c->sum += t1;
    return t0 + t2;
}

static void parse_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    align_record(c, t0, t1);
    c->hash ^= c->lane[11] + 0xe16ab045u;
    c->raw[c->slo + (int)((t0 + 39818u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 916u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    swap_ring(c, t0, t1);
    load_store(c, &c->lane[6], 3);
    c->lane[6] += c->lane[3]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 14);
    t0 ^= coal_unit(c, t1);
    c->lane[1] ^= sx_rl(c->lane[6], 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6fff98abu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45005u) % (uint32_t)c->rln)] << 16;
    flush_block(c, &c->lane[11], 4);
    c->raw[c->slo + (int)((t0 + 748u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xeef4072du;
    c->lane[5] += c->lane[13]; c->lane[15] ^= c->lane[5]; c->lane[15] = sx_rl(c->lane[15], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t poll_view(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += sift_entry_536(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xb01a81c5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa28feccfu;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 16);
    c->lane[12] += c->lane[6]; c->lane[13] ^= c->lane[12]; c->lane[13] = sx_rl(c->lane[13], 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t resize_span(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    trace_level(c, &c->lane[6], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcca69ac7u;
    c->sched[22] = c->hash ^ sx_rl(c->lane[4], 25);
    c->lane[6] += c->lane[2]; c->lane[8] ^= c->lane[6]; c->lane[8] = sx_rl(c->lane[8], 23);
    t1 ^= (uint32_t)fold_part(c, (uint8_t)(t0 >> 8), t2);
    sync_part(c, &c->lane[8], 4);
    tally_range(c, t0, t1);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfd7eaf7bu;
    t2 += push_stream_578(c, c->rlo, c->rln);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t wrap_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[12] += c->lane[11] ^ 0xc60e40c0u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x1e) << 0;
    c->lane[0] += c->lane[13] ^ 0x085463f8u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[14] += c->lane[12] ^ 0x70d08961u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33649u) % (uint32_t)c->rln)] << 0;
    c->lane[14] += c->lane[4] ^ 0x7cd76a35u;
    c->lane[14] ^= sx_rl(c->lane[12], 15);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 += (uint32_t)step_index(c);
    c->sum += t1;
    return t0 + t2;
}

static void resize_field(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20552u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 32932u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 19);
    t2 += (uint32_t)load_span_567(c);
    c->hash = (c->hash * 0xd9790d65u) ^ sx_rr(c->hash, 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[7] += c->lane[8]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 5);
    c->lane[13] += c->lane[9] ^ 0x70f644ceu;
    c->hash ^= c->lane[4] + 0x1d125abbu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[15] ^= sx_rl(c->lane[0], 11);
    c->lane[14] ^= sx_rl(c->lane[3], 27);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t fold_part(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19529u) % (uint32_t)c->rln)] << 16;
    c->lane[10] += c->lane[11] ^ 0x3357e7a3u;
    c->hash ^= c->lane[10] + 0xf5deb3b8u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->sched[3] = c->hash ^ sx_rl(c->lane[11], 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdf8c264bu;
    trim_lease(c, &c->lane[6], 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void swap_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4806u) % (uint32_t)c->rln)] << 16;
    t2 += push_batch(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0xd05c8741u;
    t1 ^= (uint32_t)patch_batch(c, (uint8_t)(t0 >> 16), t2);
    c->sched[13] = c->hash ^ sx_rl(c->lane[7], 30);
    stage_key(c, &c->lane[11], 2);
    c->sched[12] = c->hash ^ sx_rl(c->lane[14], 7);
    t2 += (uint32_t)cache_label(c);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void clamp_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    slice_batch(c, &c->lane[10], 3);
    c->raw[c->slo + (int)((t0 + 45687u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 20);
    c->hash = (c->hash * 0xcc3dc337u) ^ sx_rr(c->hash, 19);
    c->sched[9] = c->hash ^ sx_rl(c->lane[4], 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x52) << 0;
    t2 += coal_tuple(c, c->rlo, c->rln);
    grow_queue(c, t0, t1);
    t1 ^= (uint32_t)stage_digest(c, (uint8_t)(t0 >> 16), t2);
    c->lane[14] += c->lane[5] ^ 0x36a7ae00u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void queue_offset(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xbbb8ffefu) ^ sx_rr(c->hash, 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[9] ^= sx_rl(c->lane[0], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[2] += c->lane[10]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc5) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13145u) % (uint32_t)c->rln)] << 0;
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static void sync_part(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[3]; c->lane[8] ^= c->lane[5]; c->lane[8] = sx_rl(c->lane[8], 11);
    chain_count(c, &c->lane[11], 3);
    c->lane[4] += c->lane[15]; c->lane[7] ^= c->lane[4]; c->lane[7] = sx_rl(c->lane[7], 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xfcd8f0c3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9e) << 8;
    c->raw[c->slo + (int)((t0 + 23254u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 26);
    c->lane[9] ^= sx_rl(c->lane[1], 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39810u) % (uint32_t)c->rln)] << 16;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 18);
    c->sched[11] = c->hash ^ sx_rl(c->lane[9], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pin_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x36852ee7u;
    c->lane[10] += c->lane[14] ^ 0x5b3f626bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[15] += c->lane[5]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb0) << 0;
    t1 ^= (uint32_t)queue_layer(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0xada1f9a9u;
    c->lane[12] ^= sx_rl(c->lane[7], 3);
    fold_store(c, &c->lane[10], 2);
    t1 ^= (uint32_t)fill_gap(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x08eeb9bfu;
    t2 += pick_band(c, c->slo, c->sln);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void flush_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xcd) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa67c8fdbu;
    c->hash = (c->hash * 0x639b0b87u) ^ sx_rr(c->hash, 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x81445a75u;
    c->raw[c->slo + (int)((t0 + 24292u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[10] ^= sx_rl(c->lane[3], 23);
    mark_view(c, t0, t1);
    c->lane[14] ^= sx_rl(c->lane[3], 19);
    t1 ^= (uint32_t)patch_batch(c, (uint8_t)(t0 >> 16), t2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void place_tail(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x27580d87u;
    t2 = (t2 ^ c->sum) * 0x5d39ad7fu;
    t0 ^= relay_digest_568(c, t1);
    c->lane[9] += c->lane[6]; c->lane[10] ^= c->lane[9]; c->lane[10] = sx_rl(c->lane[10], 25);
    c->lane[7] ^= sx_rl(c->lane[5], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17197u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t sift_entry_536(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    cache_entry(c, &c->lane[10], 3);
    c->lane[10] ^= sx_rl(c->lane[8], 11);
    c->lane[4] += c->lane[10]; c->lane[14] ^= c->lane[4]; c->lane[14] = sx_rl(c->lane[14], 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37592u) % (uint32_t)c->rln)] << 0;
    c->lane[6] += c->lane[11]; c->lane[2] ^= c->lane[6]; c->lane[2] = sx_rl(c->lane[2], 14);
    c->hash = (c->hash * 0xc5482529u) ^ sx_rr(c->hash, 2);
    t2 += (uint32_t)latch_row(c);
    t2 += reap_count_546(c, c->slo, c->sln);
    c->hash ^= c->lane[0] + 0x932f9f32u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 15);
    hold_row(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[5] += c->lane[11] ^ 0x6126c9fdu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[0] ^= sx_rl(c->lane[6], 9);
    c->sum += t1;
    return t0 + t2;
}

static int mark_cell(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xeb) << 0;
    c->lane[9] += c->lane[0]; c->lane[5] ^= c->lane[9]; c->lane[5] = sx_rl(c->lane[5], 12);
    t0 ^= load_list(c, t1);
    c->lane[2] += c->lane[1]; c->lane[13] ^= c->lane[2]; c->lane[13] = sx_rl(c->lane[13], 11);
    c->raw[c->slo + (int)((t0 + 35940u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[9] += c->lane[10]; c->lane[13] ^= c->lane[9]; c->lane[13] = sx_rl(c->lane[13], 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[6] + 0x660564bbu;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sort_pairing(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 58678u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t1 ^= (uint32_t)fold_limit(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 21381u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x41) << 0;
    c->hash = (c->hash * 0x70091e2fu) ^ sx_rr(c->hash, 17);
    chain_label(c, &c->lane[7], 3);
    sort_arena(c, &c->lane[6], 3);
    t1 ^= (uint32_t)queue_layer(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= load_list(c, t1);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint8_t map_pool(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    mix_node_561(c, &c->lane[5], 2);
    c->lane[12] += c->lane[10]; c->lane[7] ^= c->lane[12]; c->lane[7] = sx_rl(c->lane[7], 17);
    c->lane[15] ^= sx_rl(c->lane[4], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41209u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xbe) << 0;
    c->hash = (c->hash * 0xffab8791u) ^ sx_rr(c->hash, 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44472u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)cache_frame(c, (uint8_t)(t0 >> 16), t2);
    c->lane[5] ^= sx_rl(c->lane[9], 5);
    t2 += (uint32_t)link_table(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int trim_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xde482443u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += push_stream_578(c, c->rlo, c->rln);
    peek_item(c, &c->lane[7], 1);
    c->lane[3] += c->lane[3] ^ 0xa738308au;
    c->hash = (c->hash * 0x3c5b06e9u) ^ sx_rr(c->hash, 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[13] += c->lane[13] ^ 0xddddf928u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17334u) % (uint32_t)c->rln)] << 16;
    c->sched[26] = c->hash ^ sx_rl(c->lane[4], 6);
    t2 = (t2 ^ c->sum) * 0xd8e1498du;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t coal_unit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    slice_head(c, t0, t1);
    t2 += seek_bound(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[8] += c->lane[10]; c->lane[4] ^= c->lane[8]; c->lane[4] = sx_rl(c->lane[4], 15);
    c->hash ^= c->lane[3] + 0x6f9b9f70u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[7], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xdd5524ebu;
    chain_slot(c, t0, t1);
    c->lane[9] ^= sx_rl(c->lane[4], 27);
    t2 += (uint32_t)yield_span(c);
    c->lane[6] += c->lane[13]; c->lane[9] ^= c->lane[6]; c->lane[9] = sx_rl(c->lane[9], 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13493u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void trace_level(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[2] ^ 0xc086a844u;
    c->raw[c->slo + (int)((t0 + 8184u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0xe1bb9c2du) ^ sx_rr(c->hash, 18);
    reset_store(c, &c->lane[7], 3);
    c->lane[7] ^= sx_rl(c->lane[1], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xed59a59bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x84) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x94) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void align_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 9613u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x0e743ff9u;
    c->lane[4] += c->lane[1]; c->lane[15] ^= c->lane[4]; c->lane[15] = sx_rl(c->lane[15], 7);
    stage_key(c, &c->lane[7], 4);
    wrap_store(c, t0, t1);
    c->lane[11] += c->lane[14] ^ 0xe92cabedu;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint8_t fold_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 2);
    t0 ^= trace_mask(c, t1);
    t2 = (t2 ^ c->sum) * 0xd29150cdu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x34) << 16;
    c->raw[c->slo + (int)((t0 + 59338u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[13] += c->lane[9] ^ 0x79ff6634u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8755u) % (uint32_t)c->rln)] << 0;
    wrap_store(c, t0, t1);
    c->hash ^= c->lane[2] + 0xc1999a0fu;
    t1 ^= (uint32_t)grow_row(c, (uint8_t)(t0 >> 8), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void load_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[1] + 0x6a3e4077u;
    c->hash = (c->hash * 0x3399f5f1u) ^ sx_rr(c->hash, 11);
    c->raw[c->slo + (int)((t0 + 54319u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[10] += c->lane[2] ^ 0xea0118d5u;
    c->lane[14] += c->lane[14] ^ 0x10301fd3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 ^= clamp_scope(c, t1);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 22);
    c->lane[2] += c->lane[15]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 2);
    c->sched[1] = c->hash ^ sx_rl(c->lane[0], 29);
    c->hash ^= c->lane[4] + 0x6c1e0469u;
    t2 += (uint32_t)place_region(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t reap_count_546(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    chain_gap(c, &c->lane[10], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    sync_port(c, &c->lane[0], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9d08d87bu;
    t0 ^= emit_limit(c, t1);
    c->sched[29] = c->hash ^ sx_rl(c->lane[4], 12);
    c->lane[15] += c->lane[14] ^ 0xac83c441u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x56) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8d64a4fdu;
    c->lane[6] ^= sx_rl(c->lane[5], 27);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t queue_layer(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[10] = c->hash ^ sx_rl(c->lane[15], 9);
    c->hash = (c->hash * 0x9657ceb7u) ^ sx_rr(c->hash, 12);
    c->sched[17] = c->hash ^ sx_rl(c->lane[8], 4);
    t2 += (uint32_t)align_level(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc3f4f6abu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb6) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x0f) << 16;
    t2 += (uint32_t)merge_label(c);
    c->sched[16] = c->hash ^ sx_rl(c->lane[1], 15);
    c->lane[5] += c->lane[7] ^ 0x36da24e1u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[5], 14);
    c->lane[12] += c->lane[9] ^ 0x926b1efcu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33361u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t seek_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3521d189u;
    c->lane[5] ^= sx_rl(c->lane[13], 2);
    c->raw[c->slo + (int)((t0 + 29353u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xabee7871u;
    t2 += (uint32_t)fold_tail(c);
    c->hash ^= c->lane[7] + 0x959ae99bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x59ab06c9u;
    c->sum += t1;
    return t0 + t2;
}

static int latch_row(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[11] ^= sx_rl(c->lane[6], 26);
    c->sched[11] = c->hash ^ sx_rl(c->lane[0], 29);
    t2 += purge_stack(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9c) << 8;
    c->lane[4] += c->lane[5] ^ 0xa6195debu;
    t2 += sort_tuple(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x897232b1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6169u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x93) << 0;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t trace_mask(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x22f10579u;
    c->hash ^= c->lane[8] + 0x7290240eu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x42fddda7u;
    t2 = (t2 ^ c->sum) * 0xfab1288bu;
    split_slot(c, &c->lane[1], 4);
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int cache_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->hash = (c->hash * 0x4d9c706du) ^ sx_rr(c->hash, 20);
    swap_head(c, &c->lane[9], 1);
    c->hash = (c->hash * 0x17925521u) ^ sx_rr(c->hash, 29);
    t2 += push_batch(c, c->slo, c->sln);
    coal_page(c, &c->lane[4], 4);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 24);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 2);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int step_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc7) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x6b) << 8;
    c->sched[19] = c->hash ^ sx_rl(c->lane[11], 29);
    c->sched[25] = c->hash ^ sx_rl(c->lane[10], 31);
    coal_page(c, &c->lane[4], 3);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void trim_lease(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc3) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x31) << 0;
    c->hash ^= c->lane[13] + 0x660253ecu;
    c->raw[c->slo + (int)((t0 + 12565u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xdc) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38734u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[14], 7);
    t2 = (t2 ^ c->sum) * 0xfd22b5b1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xaf) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    peek_store(c, &c->lane[3], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void hold_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    cache_batch(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 54833u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0c7b605fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    pin_level(c, t0, t1);
    c->sched[6] = c->hash ^ sx_rl(c->lane[5], 5);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 4);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint8_t grow_row(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] += c->lane[7]; c->lane[12] ^= c->lane[8]; c->lane[12] = sx_rl(c->lane[12], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47399u) % (uint32_t)c->rln)] << 16;
    t0 ^= clamp_scope(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 += (uint32_t)step_value(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[1] ^= sx_rl(c->lane[8], 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xe0dfb771u) ^ sx_rr(c->hash, 18);
    t2 = (t2 ^ c->sum) * 0xebd6f977u;
    c->lane[1] += c->lane[1] ^ 0x8fd1f3bau;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa910d9d7u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[14], 20);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int place_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    t2 += defer_token_636(c, c->slo, c->sln);
    c->sched[17] = c->hash ^ sx_rl(c->lane[15], 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7886u) % (uint32_t)c->rln)] << 8;
    c->lane[8] += c->lane[6]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 3);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t patch_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] ^= sx_rl(c->lane[3], 9);
    c->hash ^= c->lane[6] + 0xb50249d6u;
    c->hash ^= c->lane[7] + 0x35ebf6d4u;
    c->raw[c->slo + (int)((t0 + 50510u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    chain_label(c, &c->lane[10], 4);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 17);
    c->hash = (c->hash * 0x2589f05fu) ^ sx_rr(c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xcc) << 16;
    sync_view(c, t0, t1);
    c->hash ^= c->lane[13] + 0x1e8ff8f7u;
    c->lane[7] += c->lane[15]; c->lane[6] ^= c->lane[7]; c->lane[6] = sx_rl(c->lane[6], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    poll_unit(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void slice_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x40) << 0;
    c->raw[c->slo + (int)((t0 + 26631u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xb428781du;
    c->raw[c->slo + (int)((t0 + 4418u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 27531u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe1) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static void grow_queue(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    sift_span(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4292b879u;
    t2 += (uint32_t)pack_marker(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x41e31653u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash = (c->hash * 0x44a416bfu) ^ sx_rr(c->hash, 25);
    c->lane[3] += c->lane[4]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 19);
    c->hash = (c->hash * 0x502af4c3u) ^ sx_rr(c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x2f6d94bdu;
    t2 += (uint32_t)queue_count(c);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void sort_arena(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc5d1728fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[25] = c->hash ^ sx_rl(c->lane[1], 28);
    t2 = (t2 ^ c->sum) * 0xc1c7d11fu;
    c->hash = (c->hash * 0x875a48cdu) ^ sx_rr(c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9c) << 16;
    c->lane[5] ^= sx_rl(c->lane[12], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x73f3230du;
    c->hash ^= c->lane[8] + 0x82ba7ca4u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void mix_node_561(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[3]; c->lane[12] ^= c->lane[1]; c->lane[12] = sx_rl(c->lane[12], 27);
    c->lane[5] += c->lane[4]; c->lane[0] ^= c->lane[5]; c->lane[0] = sx_rl(c->lane[0], 10);
    t2 = (t2 ^ c->sum) * 0x42e148cbu;
    t2 += tally_index(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[12] += c->lane[6] ^ 0x212dcb4du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23737u) % (uint32_t)c->rln)] << 16;
    c->lane[10] += c->lane[1] ^ 0xc222589au;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int link_table(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[5] += c->lane[2] ^ 0x8a5541fau;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x28) << 0;
    t2 += blend_layer(c, c->rlo, c->rln);
    c->hash ^= c->lane[7] + 0x676cd903u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[14], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sched[7] = c->hash ^ sx_rl(c->lane[15], 16);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 29);
    c->sched[16] = c->hash ^ sx_rl(c->lane[2], 28);
    t2 += join_value(c, c->slo, c->sln);
    c->hash ^= c->lane[6] + 0x17c47610u;
    t2 = (t2 ^ c->sum) * 0x367b391bu;
    c->lane[15] += c->lane[12] ^ 0xad8458f9u;
    c->hash ^= c->lane[2] + 0x1acbb166u;
    merge_stream_613(c, t0, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void peek_item(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 12);
    t1 ^= (uint32_t)fill_gap(c, (uint8_t)(t0 >> 8), t2);
    c->sched[24] = c->hash ^ sx_rl(c->lane[0], 21);
    t2 += (uint32_t)align_level(c);
    c->hash = (c->hash * 0x22005e25u) ^ sx_rr(c->hash, 24);
    t2 = (t2 ^ c->sum) * 0x2c618a8bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xac) << 8;
    t0 ^= pick_level(c, t1);
    c->lane[4] += c->lane[13] ^ 0x99363f86u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void stage_key(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59457u) % (uint32_t)c->rln)] << 0;
    c->lane[0] += c->lane[8]; c->lane[11] ^= c->lane[0]; c->lane[11] = sx_rl(c->lane[11], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49090u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd9f1663fu;
    t2 += poll_arena(c, c->rlo, c->rln);
    c->lane[8] += c->lane[9]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void mark_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[8] += c->lane[11] ^ 0x2125aefdu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35730u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x874dabe3u) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0x3470f3d9u) ^ sx_rr(c->hash, 4);
    c->lane[6] += c->lane[4] ^ 0xcd2da559u;
    c->sched[1] = c->hash ^ sx_rl(c->lane[0], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52340u) % (uint32_t)c->rln)] << 24;
    t2 += relay_region(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 17);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int load_span_567(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64794u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0x0985736fu) ^ sx_rr(c->hash, 30);
    t0 ^= split_pairing(c, t1);
    t1 ^= (uint32_t)clamp_lease(c, (uint8_t)(t0 >> 8), t2);
    c->lane[14] += c->lane[10]; c->lane[8] ^= c->lane[14]; c->lane[8] = sx_rl(c->lane[8], 3);
    c->lane[13] += c->lane[7] ^ 0x411cad14u;
    t2 = (t2 ^ c->sum) * 0xc6f15a07u;
    close_region_606(c, &c->lane[9], 2);
    c->lane[10] += c->lane[0] ^ 0x14dd507du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t1 ^= (uint32_t)fill_tail_588(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t relay_digest_568(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] ^= sx_rl(c->lane[9], 26);
    c->lane[9] += c->lane[4]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 8);
    c->raw[c->slo + (int)((t0 + 35487u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd5) << 16;
    t2 += (uint32_t)align_scope_598(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[12] = c->hash ^ sx_rl(c->lane[13], 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10951u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbfee002bu;
    t2 += relay_view(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 19445u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t stage_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 10049u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 23);
    c->lane[14] += c->lane[5]; c->lane[11] ^= c->lane[14]; c->lane[11] = sx_rl(c->lane[11], 29);
    chain_row(c, &c->lane[6], 2);
    t0 ^= push_ring(c, t1);
    cache_batch(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x585fc0c3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47979u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa3) << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t load_list(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xe682c1a9u) ^ sx_rr(c->hash, 2);
    t2 += (uint32_t)scan_row(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x80e93d0bu;
    c->lane[4] += c->lane[13]; c->lane[12] ^= c->lane[4]; c->lane[12] = sx_rl(c->lane[12], 8);
    t2 = (t2 ^ c->sum) * 0x3e306839u;
    t2 = (t2 ^ c->sum) * 0x936b4ddbu;
    c->hash ^= c->lane[2] + 0xf3e1aff1u;
    c->lane[5] += c->lane[13] ^ 0x9692dd1au;
    c->hash ^= c->lane[10] + 0x01acaac4u;
    sync_view(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t fold_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[3] + 0x03c5ebc5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd72d1c41u;
    c->lane[15] ^= sx_rl(c->lane[0], 19);
    c->sched[6] = c->hash ^ sx_rl(c->lane[2], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 17);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void slice_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xaa31e789u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 22);
    prime_pairing(c, &c->lane[11], 1);
    c->sched[18] = c->hash ^ sx_rl(c->lane[8], 3);
    c->lane[9] ^= sx_rl(c->lane[9], 6);
    t2 = (t2 ^ c->sum) * 0x98f48ec9u;
    t1 ^= (uint32_t)cache_frame(c, (uint8_t)(t0 >> 8), t2);
    c->lane[4] += c->lane[8] ^ 0xd571c885u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54718u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x5ee7ce9du) ^ sx_rr(c->hash, 26);
    c->hash ^= c->lane[11] + 0xef8c8187u;
    t0 ^= wrap_lease(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int yield_span(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sched[14] = c->hash ^ sx_rl(c->lane[5], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37497u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= wrap_lease(c, t1);
    c->raw[c->slo + (int)((t0 + 33979u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27131u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 17604u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc0) << 0;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void chain_count(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] += c->lane[9]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 26);
    c->raw[c->slo + (int)((t0 + 459u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 30);
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    sync_state(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdd) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11201u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)sort_part(c, (uint8_t)(t0 >> 0), t2);
    c->sched[5] = c->hash ^ sx_rl(c->lane[8], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void chain_slot(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[25] = c->hash ^ sx_rl(c->lane[5], 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20538u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x89722a9fu;
    c->hash ^= c->lane[13] + 0xd1433ccfu;
    c->hash = (c->hash * 0x40b0869bu) ^ sx_rr(c->hash, 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x296397bbu;
    t2 += (uint32_t)align_scope_598(c);
    c->hash ^= c->lane[8] + 0x10e2313du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5d637035u;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void cache_entry(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    pick_level_633(c, &c->lane[0], 1);
    c->raw[c->slo + (int)((t0 + 20068u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    close_region_606(c, &c->lane[7], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 6);
    tap_state(c, &c->lane[5], 3);
    seek_record(c, &c->lane[11], 1);
    t2 += (uint32_t)seek_rate(c);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    tally_range(c, t0, t1);
    merge_stream_613(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xd41f4d51u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 15);
    t2 += swap_band_592(c, c->rlo, c->rln);
    t2 += fill_key(c, c->slo, c->sln);
    c->hash ^= c->lane[13] + 0x6a7bc87bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void wrap_store(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += purge_stack(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x0494bc29u;
    t2 += pick_band(c, c->rlo, c->rln);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t push_stream_578(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x108f24f3u) ^ sx_rr(c->hash, 22);
    c->sched[6] = c->hash ^ sx_rl(c->lane[14], 15);
    t2 += (uint32_t)tune_seat(c);
    c->lane[9] ^= sx_rl(c->lane[3], 4);
    swap_head(c, &c->lane[8], 4);
    c->raw[c->slo + (int)((t0 + 8707u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[8] += c->lane[0] ^ 0xb260bbceu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40363u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[8] + 0x556ac7f4u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t coal_tuple(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xebe21a8bu) ^ sx_rr(c->hash, 26);
    yield_view(c, t0, t1);
    c->lane[15] ^= sx_rl(c->lane[14], 27);
    c->lane[2] ^= sx_rl(c->lane[0], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += coal_digest_604(c, c->rlo, c->rln);
    c->lane[4] += c->lane[5]; c->lane[6] ^= c->lane[4]; c->lane[6] = sx_rl(c->lane[6], 22);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 3);
    c->lane[0] += c->lane[15] ^ 0x2a054e35u;
    c->hash = (c->hash * 0x6ec67dc3u) ^ sx_rr(c->hash, 19);
    c->sum += t1;
    return t0 + t2;
}

static void reset_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xfb) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    reap_group(c, &c->lane[8], 2);
    c->raw[c->slo + (int)((t0 + 22352u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27353u) % (uint32_t)c->rln)] << 8;
    rotate_head(c, t0, t1);
    c->lane[13] += c->lane[3] ^ 0x7bf72eb0u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 22);
    c->hash = (c->hash * 0x04838e57u) ^ sx_rr(c->hash, 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t fill_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[7] + 0x157f2855u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33560u) % (uint32_t)c->rln)] << 8;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24044u) % (uint32_t)c->rln)] << 8;
    c->lane[0] += c->lane[2]; c->lane[6] ^= c->lane[0]; c->lane[6] = sx_rl(c->lane[6], 13);
    c->lane[8] += c->lane[1] ^ 0x85815979u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t relay_view(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] ^= sx_rl(c->lane[15], 31);
    c->lane[2] += c->lane[13]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 18);
    c->lane[11] ^= sx_rl(c->lane[6], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29587u) % (uint32_t)c->rln)] << 16;
    c->lane[11] += c->lane[14] ^ 0xc1b55c87u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t pick_level(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[13] ^= sx_rl(c->lane[6], 30);
    t2 = (t2 ^ c->sum) * 0xd1a09127u;
    c->sched[7] = c->hash ^ sx_rl(c->lane[11], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16463u) % (uint32_t)c->rln)] << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[2], 24);
    t2 = (t2 ^ c->sum) * 0xb4e79a41u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28269u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x91e283a9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1273a1d3u;
    c->lane[2] += c->lane[12]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 15);
    c->raw[c->slo + (int)((t0 + 54927u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tally_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x149c8fa5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58621u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25997u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void chain_row(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51083u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 25818u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[20] = c->hash ^ sx_rl(c->lane[7], 24);
    t2 = (t2 ^ c->sum) * 0x411e2b15u;
    c->lane[15] += c->lane[14]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 20);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t split_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[9] + 0x53ceb249u;
    c->lane[9] += c->lane[12] ^ 0xad9693feu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58365u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 48855u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[11] + 0xf4b91234u;
    t2 = (t2 ^ c->sum) * 0x6a05ac97u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 40722u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 15);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void yield_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x86394e9du) ^ sx_rr(c->hash, 25);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 27);
    c->sched[26] = c->hash ^ sx_rl(c->lane[11], 1);
    c->raw[c->slo + (int)((t0 + 46562u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x6dbaf071u) ^ sx_rr(c->hash, 8);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    c->hash = (c->hash * 0xc628b20bu) ^ sx_rr(c->hash, 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sched[21] = c->hash ^ sx_rl(c->lane[8], 6);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint8_t fill_tail_588(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43641u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 23494u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[1] = c->hash ^ sx_rl(c->lane[2], 13);
    c->sched[17] = c->hash ^ sx_rl(c->lane[5], 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26832u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[6] + 0xf01f6accu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t sort_part(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7599u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 47870u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x71da8fb3u;
    c->hash ^= c->lane[10] + 0x07e0e3fau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7593u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void poll_unit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[13] + 0x8a6a88c8u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 1);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= c->lane[8] + 0x645ee6c9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[5] = c->hash ^ sx_rl(c->lane[15], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24673u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 53568u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void chain_gap(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] += c->lane[13]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 25);
    c->raw[c->slo + (int)((t0 + 52962u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3bb51025u;
    c->hash = (c->hash * 0x1abdb693u) ^ sx_rr(c->hash, 17);
    c->sched[8] = c->hash ^ sx_rl(c->lane[6], 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t swap_band_592(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xdc) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfeac1811u;
    t2 = (t2 ^ c->sum) * 0x66284545u;
    c->sched[8] = c->hash ^ sx_rl(c->lane[1], 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xfc) << 16;
    t2 = (t2 ^ c->sum) * 0x098f9489u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[8] + 0x868bd53cu;
    c->sum += t1;
    return t0 + t2;
}

static int scan_row(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[6] += c->lane[8] ^ 0xe93f5f0du;
    c->sched[20] = c->hash ^ sx_rl(c->lane[14], 2);
    prime_stack_758(c, &c->lane[4], 2);
    c->hash = (c->hash * 0x4a918e87u) ^ sx_rr(c->hash, 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbe) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17938u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[7] + 0xb4b3c3d6u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[5], 3);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void coal_page(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[13] = c->hash ^ sx_rl(c->lane[8], 11);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 6);
    c->lane[4] ^= sx_rl(c->lane[15], 19);
    c->sched[23] = c->hash ^ sx_rl(c->lane[2], 5);
    c->hash = (c->hash * 0x956089e3u) ^ sx_rr(c->hash, 30);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fill_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22316u) % (uint32_t)c->rln)] << 24;
    c->lane[11] ^= sx_rl(c->lane[7], 18);
    c->lane[14] += c->lane[0]; c->lane[11] ^= c->lane[14]; c->lane[11] = sx_rl(c->lane[11], 10);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 20);
    c->lane[0] += c->lane[15]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 1);
    c->sched[31] = c->hash ^ sx_rl(c->lane[14], 16);
    c->sum += t1;
    return t0 + t2;
}

static void cache_batch(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[12] += c->lane[11] ^ 0xe0a13956u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[6] ^= sx_rl(c->lane[4], 22);
    c->lane[3] += c->lane[4]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int seek_rate(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] += c->lane[14]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x820ee7a5u;
    c->raw[c->slo + (int)((t0 + 44815u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[12] += c->lane[6]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 15);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int align_scope_598(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb42eaac9u;
    c->lane[5] += c->lane[2]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 3);
    c->sched[21] = c->hash ^ sx_rl(c->lane[1], 9);
    c->hash = (c->hash * 0xad930217u) ^ sx_rr(c->hash, 19);
    c->lane[8] ^= sx_rl(c->lane[7], 14);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xff) << 16;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t clamp_scope(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbe52aa4fu;
    c->sched[18] = c->hash ^ sx_rl(c->lane[12], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34749u) % (uint32_t)c->rln)] << 8;
    c->lane[11] ^= sx_rl(c->lane[6], 11);
    c->hash = (c->hash * 0x0b3e29c5u) ^ sx_rr(c->hash, 11);
    c->lane[2] ^= sx_rl(c->lane[12], 14);
    c->raw[c->slo + (int)((t0 + 5504u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x446ffdd9u) ^ sx_rr(c->hash, 24);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 8);
    c->lane[3] ^= sx_rl(c->lane[6], 5);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void chain_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc7) << 8;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 7);
    c->lane[15] ^= sx_rl(c->lane[6], 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[5] += c->lane[2] ^ 0x27c88b70u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int align_level(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->hash ^= c->lane[2] + 0xe5d44e57u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44554u) % (uint32_t)c->rln)] << 16;
    c->sched[24] = c->hash ^ sx_rl(c->lane[11], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 4);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    c->lane[3] += c->lane[6]; c->lane[1] ^= c->lane[3]; c->lane[1] = sx_rl(c->lane[1], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[9] += c->lane[13] ^ 0xb7cc12dcu;
    c->lane[10] ^= sx_rl(c->lane[6], 5);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t push_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52499u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0xf40224bdu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 38722u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 2305u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x2bd0f807u;
    c->lane[13] += c->lane[12] ^ 0x1e16a907u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42410u) % (uint32_t)c->rln)] << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void sync_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] ^= sx_rl(c->lane[8], 17);
    c->lane[12] ^= sx_rl(c->lane[13], 8);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8eff2da5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd8651d47u;
    t2 = (t2 ^ c->sum) * 0xa2c88c4du;
    c->hash ^= c->lane[7] + 0x51f88e39u;
    c->lane[3] += c->lane[14]; c->lane[12] ^= c->lane[3]; c->lane[12] = sx_rl(c->lane[12], 9);
    t2 = (t2 ^ c->sum) * 0xf2ae85b3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x61bf164du;
    c->lane[7] += c->lane[9]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 26);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t coal_digest_604(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)clamp_head(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1813u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[0] + 0xfab1610eu;
    c->lane[10] += c->lane[3] ^ 0x8e4efd5cu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] += c->lane[10]; c->lane[2] ^= c->lane[8]; c->lane[2] = sx_rl(c->lane[2], 26);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t purge_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] += c->lane[7]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[5] + 0x605a1dfcu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 29);
    c->raw[c->slo + (int)((t0 + 52541u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[22] = c->hash ^ sx_rl(c->lane[12], 5);
    c->hash = (c->hash * 0xd51de50fu) ^ sx_rr(c->hash, 4);
    c->sum += t1;
    return t0 + t2;
}

static void close_region_606(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x27cbdb59u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37763u) % (uint32_t)c->rln)] << 24;
    c->lane[1] ^= sx_rl(c->lane[6], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void split_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x5e6a1d6fu) ^ sx_rr(c->hash, 26);
    c->lane[5] += c->lane[13]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12769u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[11] += c->lane[5]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 16);
    c->lane[9] += c->lane[14]; c->lane[1] ^= c->lane[9]; c->lane[1] = sx_rl(c->lane[1], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x928a8655u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t blend_layer(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1906489bu;
    c->sched[23] = c->hash ^ sx_rl(c->lane[11], 23);
    c->sched[10] = c->hash ^ sx_rl(c->lane[11], 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x136169ddu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4179u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62026u) % (uint32_t)c->rln)] << 0;
    c->lane[9] ^= sx_rl(c->lane[13], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += probe_state(c, c->slo, c->sln);
    c->sum += t1;
    return t0 + t2;
}

static void sync_state(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x71) << 16;
    c->hash ^= c->lane[15] + 0x9cd29092u;
    c->lane[8] ^= sx_rl(c->lane[15], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xe232e8cbu) ^ sx_rr(c->hash, 12);
    c->hash = (c->hash * 0xb3c5b835u) ^ sx_rr(c->hash, 22);
    c->lane[12] += c->lane[15] ^ 0x87d4c9a2u;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void sync_port(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] += c->lane[8] ^ 0x0c13f7beu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xae5c1ba5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39875u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int fold_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->raw[c->slo + (int)((t0 + 28586u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] ^= sx_rl(c->lane[15], 11);
    c->sched[27] = c->hash ^ sx_rl(c->lane[10], 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x869d2b73u;
    c->hash = (c->hash * 0xd50ed429u) ^ sx_rr(c->hash, 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe76edd07u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 24);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t poll_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[8] += c->lane[2]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6d25940bu;
    c->sched[27] = c->hash ^ sx_rl(c->lane[3], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 30);
    c->lane[10] += c->lane[3]; c->lane[1] ^= c->lane[10]; c->lane[1] = sx_rl(c->lane[1], 6);
    c->hash ^= c->lane[6] + 0xfd072a33u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc2248b2bu;
    t2 = (t2 ^ c->sum) * 0x13e111b7u;
    c->sum += t1;
    return t0 + t2;
}

static void merge_stream_613(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x26779d07u) ^ sx_rr(c->hash, 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x66380ec5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x95bd6ce7u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 29);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t wrap_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 17);
    c->lane[7] += c->lane[13]; c->lane[2] ^= c->lane[7]; c->lane[2] = sx_rl(c->lane[2], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x32080adfu;
    t2 = (t2 ^ c->sum) * 0x8ac88d51u;
    c->sched[11] = c->hash ^ sx_rl(c->lane[1], 31);
    c->lane[6] += c->lane[0]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 2);
    c->hash ^= c->lane[4] + 0x4dabd7f1u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29243u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x53) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38580u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pin_level(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[12] += c->lane[1] ^ 0xd415d0e1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0xd6e21803u) ^ sx_rr(c->hash, 5);
    c->hash = (c->hash * 0x5b98ea3du) ^ sx_rr(c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static int queue_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 21);
    c->lane[5] += c->lane[6] ^ 0x7f619d9bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sched[25] = c->hash ^ sx_rl(c->lane[6], 23);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 11);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int tune_seat(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->hash = (c->hash * 0x97176ab7u) ^ sx_rr(c->hash, 23);
    c->hash = (c->hash * 0x139e5943u) ^ sx_rr(c->hash, 31);
    c->hash = (c->hash * 0x85246185u) ^ sx_rr(c->hash, 18);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0d) << 8;
    c->lane[10] += c->lane[9] ^ 0x999ba1ebu;
    c->hash ^= c->lane[8] + 0x778040d7u;
    c->lane[6] += c->lane[14]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x7f) << 16;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void prime_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[12] += c->lane[11]; c->lane[9] ^= c->lane[12]; c->lane[9] = sx_rl(c->lane[9], 6);
    c->sched[27] = c->hash ^ sx_rl(c->lane[13], 1);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 10);
    c->hash = (c->hash * 0x2677d37du) ^ sx_rr(c->hash, 20);
    c->sched[30] = c->hash ^ sx_rl(c->lane[8], 11);
    c->lane[4] ^= sx_rl(c->lane[12], 2);
    c->lane[11] ^= sx_rl(c->lane[4], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe8) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xca88106fu;
    c->hash = (c->hash * 0x3ad6b175u) ^ sx_rr(c->hash, 15);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pick_band(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] += c->lane[3]; c->lane[15] ^= c->lane[7]; c->lane[15] = sx_rl(c->lane[15], 14);
    t2 = (t2 ^ c->sum) * 0x9df356f5u;
    c->lane[7] += c->lane[14] ^ 0xa9ec9912u;
    c->lane[12] += c->lane[9] ^ 0x1de61147u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x87718dafu;
    c->lane[15] ^= sx_rl(c->lane[5], 19);
    c->raw[c->slo + (int)((t0 + 32892u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] ^= sx_rl(c->lane[6], 26);
    c->raw[c->slo + (int)((t0 + 36025u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0xe3ee33e5u;
    c->lane[5] += c->lane[7]; c->lane[11] ^= c->lane[5]; c->lane[11] = sx_rl(c->lane[11], 18);
    c->sum += t1;
    return t0 + t2;
}

static void peek_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42792u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe7) << 8;
    c->lane[15] ^= sx_rl(c->lane[5], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xeb3e6077u;
    c->raw[c->slo + (int)((t0 + 41015u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x7335942bu;
    c->lane[0] += c->lane[15]; c->lane[14] ^= c->lane[0]; c->lane[14] = sx_rl(c->lane[14], 2);
    c->lane[6] += c->lane[2] ^ 0x70e05534u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t relay_region(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xd9179679u) ^ sx_rr(c->hash, 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49819u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[8] + 0x3a440c99u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[7], 29);
    c->hash ^= c->lane[9] + 0x29804998u;
    c->hash = (c->hash * 0xd8d263cfu) ^ sx_rr(c->hash, 19);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe35d6125u;
    c->lane[4] ^= sx_rl(c->lane[13], 6);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tally_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xd712ba79u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 18);
    c->hash = (c->hash * 0x7050ece7u) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0xbd3730a1u) ^ sx_rr(c->hash, 6);
    t2 = (t2 ^ c->sum) * 0x4bc26461u;
    c->sum += t1;
    return t0 + t2;
}

static int merge_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->sched[6] = c->hash ^ sx_rl(c->lane[3], 27);
    c->raw[c->slo + (int)((t0 + 24759u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x9fc86ef9u) ^ sx_rr(c->hash, 6);
    c->raw[c->slo + (int)((t0 + 31202u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf4faab2fu;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 1);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void swap_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x80916eefu;
    c->hash = (c->hash * 0xdcb2d0c7u) ^ sx_rr(c->hash, 13);
    c->lane[10] += c->lane[12]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 19);
    c->sched[3] = c->hash ^ sx_rl(c->lane[3], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59058u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[11] + 0x896b59a1u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 8);
    c->lane[9] ^= sx_rl(c->lane[11], 20);
    c->hash = (c->hash * 0xd2e26fa1u) ^ sx_rr(c->hash, 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27962u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void tap_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x74cbf18fu;
    c->sched[16] = c->hash ^ sx_rl(c->lane[4], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += probe_state(c, c->rlo, c->rln);
    c->hash ^= c->lane[13] + 0x164a5b3eu;
    c->lane[3] += c->lane[4]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 18);
    c->lane[13] ^= sx_rl(c->lane[2], 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37996u) % (uint32_t)c->rln)] << 16;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 30);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void seek_record(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17739u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[2] + 0xcada7120u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2e) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t cache_frame(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc7) << 8;
    c->raw[c->slo + (int)((t0 + 2806u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[7] = c->hash ^ sx_rl(c->lane[5], 13);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int step_value(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->hash ^= c->lane[4] + 0xc0f97bbcu;
    c->sched[8] = c->hash ^ sx_rl(c->lane[7], 27);
    c->lane[9] += c->lane[0] ^ 0x10946655u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x6b) << 0;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t emit_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[11]; c->lane[7] ^= c->lane[12]; c->lane[7] = sx_rl(c->lane[7], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53143u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[12] ^= sx_rl(c->lane[1], 21);
    c->lane[4] += c->lane[1] ^ 0xed430bdau;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static int pack_marker(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->raw[c->slo + (int)((t0 + 36033u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[3] += c->lane[2]; c->lane[9] ^= c->lane[3]; c->lane[9] = sx_rl(c->lane[9], 23);
    c->sched[12] = c->hash ^ sx_rl(c->lane[12], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30137u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t push_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 31972u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41524u) % (uint32_t)c->rln)] << 16;
    c->lane[4] += c->lane[7]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 26);
    c->hash = (c->hash * 0x2614e60fu) ^ sx_rr(c->hash, 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3a) << 0;
    c->lane[14] += c->lane[4]; c->lane[3] ^= c->lane[14]; c->lane[3] = sx_rl(c->lane[3], 26);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 16);
    c->lane[13] += c->lane[14]; c->lane[9] ^= c->lane[13]; c->lane[9] = sx_rl(c->lane[9], 17);
    c->sum += t1;
    return t0 + t2;
}

static void rotate_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53448u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 33219u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[3] += c->lane[13] ^ 0xff27b765u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xde) << 0;
    c->lane[15] ^= sx_rl(c->lane[12], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18852u) % (uint32_t)c->rln)] << 8;
    c->lane[8] += c->lane[4]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xadd0b05bu;
    t2 = (t2 ^ c->sum) * 0x1b9f85f9u;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static void pick_level_633(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x32a3ca6fu) ^ sx_rr(c->hash, 10);
    c->raw[c->slo + (int)((t0 + 15713u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[12] ^= sx_rl(c->lane[13], 11);
    c->lane[14] += c->lane[8]; c->lane[5] ^= c->lane[14]; c->lane[5] = sx_rl(c->lane[5], 9);
    c->raw[c->slo + (int)((t0 + 32894u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[11] += c->lane[7]; c->lane[13] ^= c->lane[11]; c->lane[13] = sx_rl(c->lane[13], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23348u) % (uint32_t)c->rln)] << 24;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void sift_span(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 39531u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x16b7b4b9u) ^ sx_rr(c->hash, 16);
    t2 = (t2 ^ c->sum) * 0x49f589f9u;
    t2 = (t2 ^ c->sum) * 0x89e05d75u;
    c->raw[c->slo + (int)((t0 + 46406u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 6);
    t2 = (t2 ^ c->sum) * 0x2c919c99u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xd60f4355u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint32_t sort_tuple(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[7]; c->lane[8] ^= c->lane[14]; c->lane[8] = sx_rl(c->lane[8], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42490u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0xda8aeb13u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t defer_token_636(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x7df910abu) ^ sx_rr(c->hash, 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 63345u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x9ba653cdu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t clamp_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] ^= sx_rl(c->lane[8], 5);
    c->raw[c->slo + (int)((t0 + 57504u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[30] = c->hash ^ sx_rl(c->lane[5], 19);
    t2 = (t2 ^ c->sum) * 0xb84296b1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[8] + 0x424b59a4u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t join_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[14] ^= sx_rl(c->lane[11], 21);
    t2 = (t2 ^ c->sum) * 0x957bd507u;
    c->hash ^= c->lane[14] + 0x2080a48bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x45f4020bu;
    c->sched[22] = c->hash ^ sx_rl(c->lane[7], 31);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 30);
    c->sum += t1;
    return t0 + t2;
}

static void reap_group(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 21);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xecb9a39bu;
    c->hash ^= c->lane[9] + 0x93b792f0u;
    c->raw[c->slo + (int)((t0 + 46673u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void hold_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)queue_lease(c);
    t2 += sift_batch(c, c->slo, c->sln);
    t2 += peek_bound(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37618u) % (uint32_t)c->rln)] << 0;
    fill_scope(c, t0, t1);
    t0 ^= emit_tuple(c, t1);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 19);
    cache_mask(c, t0, t1);
    t2 += (uint32_t)swap_segment(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    scan_gap(c, t0, t1);
    t0 ^= mark_region(c, t1);
    t2 += (uint32_t)rotate_gap(c);
    c->lane[12] += c->lane[1] ^ 0xb4ff3ddeu;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 27);
    c->hash = (c->hash * 0x19d4e7cfu) ^ sx_rr(c->hash, 14);
    reap_cursor_670(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xc21983fdu;
    trim_offset(c, &c->lane[5], 3);
    t0 ^= queue_band(c, t1);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 5);
    blend_state(c, &c->lane[7], 4);
    c->hash = (c->hash * 0x8d7b7307u) ^ sx_rr(c->hash, 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x94a5b49bu;
    t1 ^= (uint32_t)sort_marker(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= relay_arena_766(c, t1);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t sift_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)yield_stream(c, (uint8_t)(t0 >> 16), t2);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 21);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 19);
    yield_limit(c, t0, t1);
    c->lane[8] ^= sx_rl(c->lane[15], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += tap_slot(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x24) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 += grow_offset(c, c->slo, c->sln);
    t0 ^= clamp_marker(c, t1);
    t2 = (t2 ^ c->sum) * 0x2e35df5fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    coal_table(c, t0, t1);
    c->lane[4] += c->lane[10] ^ 0xdf9a3c8fu;
    c->sum += t1;
    return t0 + t2;
}

static int rotate_gap(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t2 += trace_segment(c, c->slo, c->sln);
    t1 ^= (uint32_t)pair_store(c, (uint8_t)(t0 >> 16), t2);
    flush_count(c, t0, t1);
    t2 += (uint32_t)trace_store(c);
    c->raw[c->slo + (int)((t0 + 35739u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9a4e70f9u;
    c->hash ^= c->lane[13] + 0xb18f3b78u;
    t2 += fetch_lease(c, c->rlo, c->rln);
    t0 ^= merge_delta(c, t1);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 28);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void cache_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)emit_cursor(c);
    probe_tuple(c, &c->lane[7], 3);
    swap_cursor(c, &c->lane[8], 2);
    t0 ^= chain_band(c, t1);
    c->lane[1] ^= sx_rl(c->lane[12], 2);
    blend_bucket(c, t0, t1);
    c->hash ^= c->lane[1] + 0x8c54ea23u;
    c->hash = (c->hash * 0x94f9db65u) ^ sx_rr(c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t clamp_marker(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xd0fa8fa3u;
    reap_cursor_670(c, t0, t1);
    t0 ^= emit_tuple(c, t1);
    t1 ^= (uint32_t)join_span(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 2170u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xfb) << 0;
    t0 ^= prime_layer(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tap_slot(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[7]; c->lane[1] ^= c->lane[14]; c->lane[1] = sx_rl(c->lane[1], 26);
    c->raw[c->slo + (int)((t0 + 17954u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51803u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x94) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[2] += c->lane[11]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 9);
    t2 += (uint32_t)fill_cell(c);
    c->lane[5] += c->lane[14]; c->lane[13] ^= c->lane[5]; c->lane[13] = sx_rl(c->lane[13], 15);
    t2 = (t2 ^ c->sum) * 0x8a0e4a85u;
    c->raw[c->slo + (int)((t0 + 43599u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[4] += c->lane[15] ^ 0xfeb0c088u;
    c->sum += t1;
    return t0 + t2;
}

static void probe_tuple(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[12]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 21);
    c->lane[11] ^= sx_rl(c->lane[3], 18);
    step_marker(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xe370f2c1u;
    t0 ^= sort_lease(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x17) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fetch_lease(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    chain_window(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash = (c->hash * 0x9bdee92fu) ^ sx_rr(c->hash, 5);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 20);
    t2 += (uint32_t)slice_count(c);
    t0 ^= mark_region(c, t1);
    align_record_661(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xc11c7e5fu;
    step_marker(c, t0, t1);
    t0 ^= prime_layer(c, t1);
    c->hash ^= c->lane[3] + 0x2166bde9u;
    c->sum += t1;
    return t0 + t2;
}

static void coal_table(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 25);
    trim_offset(c, &c->lane[3], 1);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 1);
    c->hash ^= c->lane[3] + 0x1cb668fdu;
    t2 += (uint32_t)pick_field(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += align_group(c, c->rlo, c->rln);
    c->lane[5] += c->lane[4] ^ 0x728acbddu;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void yield_limit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x49d08497u;
    t2 += (uint32_t)trace_store(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 36669u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[1] + 0x286d15e4u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2744u) % (uint32_t)c->rln)] << 0;
    fold_chunk(c, &c->lane[10], 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd12b219bu;
    t2 += slice_span(c, c->slo, c->sln);
    c->lane[4] ^= sx_rl(c->lane[9], 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x56) << 16;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t merge_delta(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)swap_segment(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x894d412fu;
    t2 += split_gap(c, c->slo, c->sln);
    c->sched[9] = c->hash ^ sx_rl(c->lane[15], 31);
    c->lane[7] += c->lane[4] ^ 0x1d4dacf7u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf5d65979u;
    t1 ^= (uint32_t)split_track(c, (uint8_t)(t0 >> 0), t2);
    c->lane[12] += c->lane[5] ^ 0xbb9e2711u;
    c->raw[c->slo + (int)((t0 + 20826u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbfcaa96fu;
    c->lane[6] += c->lane[2] ^ 0x661dc45fu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void blend_bucket(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xba) << 16;
    c->raw[c->slo + (int)((t0 + 4958u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[9] + 0xb1385bd7u;
    c->sched[17] = c->hash ^ sx_rl(c->lane[13], 20);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void flush_count(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[13], 24);
    t1 ^= (uint32_t)peek_table(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)align_slot(c);
    c->raw[c->slo + (int)((t0 + 6088u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    pair_window(c, t0, t1);
    t2 += (uint32_t)fill_index(c);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void swap_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= c->lane[7] + 0xe0fdb0b8u;
    c->sched[1] = c->hash ^ sx_rl(c->lane[8], 3);
    t2 += peek_bound(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    store_queue(c, t0, t1);
    c->hash = (c->hash * 0xcc29d4d1u) ^ sx_rr(c->hash, 30);
    t0 ^= defer_store(c, t1);
    c->lane[5] ^= sx_rl(c->lane[5], 10);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int swap_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 56513u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[8] ^= sx_rl(c->lane[6], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23895u) % (uint32_t)c->rln)] << 16;
    prime_stack_758(c, &c->lane[3], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xec) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20651u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe0a978d3u;
    c->lane[1] += c->lane[9] ^ 0xbadafbc8u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfc02e16fu;
    c->hash = (c->hash * 0x963e2e79u) ^ sx_rr(c->hash, 22);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void step_marker(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 55907u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[12] ^= sx_rl(c->lane[5], 21);
    c->sched[23] = c->hash ^ sx_rl(c->lane[2], 20);
    c->hash = (c->hash * 0x24447db7u) ^ sx_rr(c->hash, 7);
    t2 += sync_ring(c, c->slo, c->sln);
    pick_mask(c, &c->lane[7], 2);
    t1 ^= (uint32_t)scan_item_692(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)pair_store(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[4] + 0x952aa09eu;
    c->lane[2] += c->lane[12] ^ 0x6be6fb91u;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint32_t prime_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[11] + 0x031bb6feu;
    t1 ^= (uint32_t)trim_slot(c, (uint8_t)(t0 >> 8), t2);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 16);
    c->sched[10] = c->hash ^ sx_rl(c->lane[3], 25);
    c->lane[5] += c->lane[13]; c->lane[15] ^= c->lane[5]; c->lane[15] = sx_rl(c->lane[15], 22);
    c->lane[9] += c->lane[12] ^ 0x5249c170u;
    t2 = (t2 ^ c->sum) * 0x8660254fu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void chain_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[27] = c->hash ^ sx_rl(c->lane[5], 29);
    scan_gap(c, t0, t1);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 21);
    t2 += place_rate(c, c->rlo, c->rln);
    t2 += (uint32_t)queue_lease(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[11] += c->lane[14] ^ 0x86b172b1u;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t peek_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] += c->lane[2] ^ 0xe567bcc8u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64992u) % (uint32_t)c->rln)] << 24;
    t0 ^= poll_track(c, t1);
    t1 ^= (uint32_t)move_port(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x16e9a023u;
    drain_row(c, t0, t1);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 22);
    t2 += (uint32_t)peek_store_695(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51703u) % (uint32_t)c->rln)] << 8;
    t2 += join_band_688(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd2492e53u;
    cache_state(c, t0, t1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t emit_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[9] + 0x5ede5434u;
    t2 += fold_seat_699(c, c->rlo, c->rln);
    c->lane[1] += c->lane[13]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 17);
    t2 = (t2 ^ c->sum) * 0xea703f89u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 15);
    c->hash = (c->hash * 0xb0c9b8a1u) ^ sx_rr(c->hash, 29);
    c->hash = (c->hash * 0x6d523ef1u) ^ sx_rr(c->hash, 11);
    c->hash ^= c->lane[1] + 0x5a05f17cu;
    t2 += (uint32_t)swap_key(c);
    c->lane[14] += c->lane[1]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 28);
    t0 ^= queue_band(c, t1);
    c->hash = (c->hash * 0xd7917b45u) ^ sx_rr(c->hash, 29);
    c->lane[12] += c->lane[13]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 14);
    c->hash = (c->hash * 0x687816d7u) ^ sx_rr(c->hash, 13);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int align_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9c) << 8;
    t2 += (uint32_t)tally_label(c);
    c->hash ^= c->lane[11] + 0x4f1b67ddu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 45821u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44685u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x82b40709u;
    t2 += join_count(c, c->slo, c->sln);
    c->sched[28] = c->hash ^ sx_rl(c->lane[3], 4);
    t0 ^= tap_page(c, t1);
    c->lane[7] += c->lane[3] ^ 0xa275d975u;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void align_record_661(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18708u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[2] += c->lane[5]; c->lane[13] ^= c->lane[2]; c->lane[13] = sx_rl(c->lane[13], 14);
    c->lane[13] ^= sx_rl(c->lane[0], 8);
    t2 += (uint32_t)store_tail(c);
    c->sched[3] = c->hash ^ sx_rl(c->lane[6], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbb400c03u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x16) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56674u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63048u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t join_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[9] += c->lane[2] ^ 0xf07df7b2u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] += c->lane[1]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 20);
    c->raw[c->slo + (int)((t0 + 10597u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[6] += c->lane[11] ^ 0x97f47beau;
    c->hash = (c->hash * 0x4c629227u) ^ sx_rr(c->hash, 2);
    t2 += reset_mask(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] ^= sx_rl(c->lane[6], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x399b683fu;
    c->hash ^= c->lane[4] + 0x823ba425u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void trim_offset(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xfdd8c611u) ^ sx_rr(c->hash, 20);
    c->lane[8] += c->lane[14] ^ 0x36c1284au;
    t2 += (uint32_t)hold_line(c);
    c->lane[10] += c->lane[7]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 1);
    c->hash = (c->hash * 0xe74ad789u) ^ sx_rr(c->hash, 24);
    c->sched[15] = c->hash ^ sx_rl(c->lane[5], 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5d4fec97u;
    c->hash = (c->hash * 0xa5880de3u) ^ sx_rr(c->hash, 31);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 7);
    c->hash ^= c->lane[4] + 0x9aab31f9u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int fill_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa7d78a65u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xef) << 16;
    c->raw[c->slo + (int)((t0 + 53152u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xba) << 8;
    c->lane[15] += c->lane[1]; c->lane[6] ^= c->lane[15]; c->lane[6] = sx_rl(c->lane[6], 10);
    c->raw[c->slo + (int)((t0 + 32465u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0xf06a87e3u) ^ sx_rr(c->hash, 3);
    t1 ^= (uint32_t)pack_range(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x211f68e7u;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int fill_cell(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->raw[c->slo + (int)((t0 + 27857u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25221u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)grow_unit_719(c, (uint8_t)(t0 >> 16), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += align_store(c, c->slo, c->sln);
    c->lane[8] += c->lane[5]; c->lane[7] ^= c->lane[8]; c->lane[7] = sx_rl(c->lane[7], 4);
    t2 = (t2 ^ c->sum) * 0x6f8aa9dbu;
    c->raw[c->slo + (int)((t0 + 53301u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[4] = c->hash ^ sx_rl(c->lane[13], 30);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 3);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mark_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] ^= sx_rl(c->lane[7], 7);
    c->lane[7] += c->lane[12]; c->lane[2] ^= c->lane[7]; c->lane[2] = sx_rl(c->lane[2], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51123u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    move_cell(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int trace_store(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t1 ^= (uint32_t)grow_row_700(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[6] + 0x64a46ac9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9f) << 16;
    c->sched[31] = c->hash ^ sx_rl(c->lane[9], 22);
    c->hash = (c->hash * 0x44c75f07u) ^ sx_rr(c->hash, 20);
    fetch_node(c, &c->lane[6], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t2 += store_lease(c, c->slo, c->sln);
    t1 ^= (uint32_t)peek_table(c, (uint8_t)(t0 >> 8), t2);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] ^= sx_rl(c->lane[8], 29);
    t1 ^= (uint32_t)trim_slot(c, (uint8_t)(t0 >> 8), t2);
    c->lane[12] ^= sx_rl(c->lane[12], 21);
    t2 += tap_mask(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 3505u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t sort_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 27706u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    grow_group(c, &c->lane[2], 2);
    c->lane[0] ^= sx_rl(c->lane[11], 2);
    t2 += (uint32_t)emit_cursor(c);
    c->raw[c->slo + (int)((t0 + 32289u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27983u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int pick_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->hash ^= c->lane[6] + 0x9f86b6e5u;
    t2 = (t2 ^ c->sum) * 0x4538b2b5u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[7], 17);
    t0 ^= drain_part(c, t1);
    slice_stream_703(c, &c->lane[10], 2);
    pair_window(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb13d13cfu;
    t2 += latch_gap(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x933a82d7u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe45f0d89u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4085u) % (uint32_t)c->rln)] << 8;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void reap_cursor_670(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xf2192643u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1398u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)defer_window(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x7f81a643u;
    c->lane[11] += c->lane[4] ^ 0x73f2d0cau;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x6e) << 16;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t split_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x00ccd035u;
    t2 += slice_span(c, c->rlo, c->rln);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    c->lane[13] += c->lane[14]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 9);
    c->lane[3] += c->lane[12] ^ 0xa126d036u;
    c->lane[3] ^= sx_rl(c->lane[12], 5);
    t2 = (t2 ^ c->sum) * 0x8ae236f9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t align_group(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    pick_mask(c, &c->lane[1], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t2 += (uint32_t)parse_item(c);
    c->hash ^= c->lane[10] + 0x0010319cu;
    c->sched[0] = c->hash ^ sx_rl(c->lane[8], 30);
    t2 += trace_segment(c, c->rlo, c->rln);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 22);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2e) << 0;
    t2 = (t2 ^ c->sum) * 0xbf834011u;
    c->sum += t1;
    return t0 + t2;
}

static int slice_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t2 += latch_gap(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe98161b7u;
    t2 = (t2 ^ c->sum) * 0x056a256fu;
    c->raw[c->slo + (int)((t0 + 52526u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0xf8981577u) ^ sx_rr(c->hash, 27);
    c->sched[5] = c->hash ^ sx_rl(c->lane[7], 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc2) << 0;
    c->hash = (c->hash * 0x7f0d2d7bu) ^ sx_rr(c->hash, 17);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t pair_store(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[31] = c->hash ^ sx_rl(c->lane[6], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] += c->lane[15] ^ 0x74ad5c83u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22104u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x4f) << 8;
    c->hash = (c->hash * 0x8a034597u) ^ sx_rr(c->hash, 28);
    rotate_run_767(c, &c->lane[7], 3);
    c->lane[4] += c->lane[10] ^ 0xc99f3bb3u;
    c->lane[12] += c->lane[4] ^ 0xfdf15a0bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10103u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 += grow_offset(c, c->slo, c->sln);
    c->lane[15] ^= sx_rl(c->lane[0], 5);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void pair_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] += c->lane[13] ^ 0xe84e5161u;
    c->lane[11] ^= sx_rl(c->lane[11], 28);
    c->sched[21] = c->hash ^ sx_rl(c->lane[2], 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x25) << 8;
    c->hash = (c->hash * 0xe1e0e68du) ^ sx_rr(c->hash, 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18121u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 43055u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 20);
    c->lane[1] += c->lane[7]; c->lane[0] ^= c->lane[1]; c->lane[0] = sx_rl(c->lane[0], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t poll_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[2] = c->hash ^ sx_rl(c->lane[15], 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26982u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[12] + 0x13a012c1u;
    c->lane[7] += c->lane[8] ^ 0xeacc07d5u;
    c->raw[c->slo + (int)((t0 + 60463u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] += c->lane[11] ^ 0xf8cd2fafu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t trace_segment(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] += c->lane[4] ^ 0x65b2c660u;
    c->lane[10] += c->lane[0]; c->lane[2] ^= c->lane[10]; c->lane[2] = sx_rl(c->lane[2], 1);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 21);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t store_lease(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[5] + 0xd7617c28u;
    t1 ^= (uint32_t)step_stack(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash = (c->hash * 0x05dd6d5fu) ^ sx_rr(c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void cache_state(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)queue_port(c);
    prime_stack_758(c, &c->lane[10], 1);
    c->hash ^= c->lane[7] + 0x6eb7e89fu;
    c->hash ^= c->lane[1] + 0xfdbbf62fu;
    c->raw[c->slo + (int)((t0 + 36322u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[3] += c->lane[11] ^ 0x1fb5f4f2u;
    t2 = (t2 ^ c->sum) * 0x96f5d47bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16198u) % (uint32_t)c->rln)] << 0;
    c->sched[5] = c->hash ^ sx_rl(c->lane[10], 22);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t align_store(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)clamp_head(c);
    c->sched[7] = c->hash ^ sx_rl(c->lane[13], 14);
    c->hash ^= c->lane[3] + 0x0e5a3bdeu;
    c->lane[10] += c->lane[10] ^ 0x3020ececu;
    c->sched[29] = c->hash ^ sx_rl(c->lane[6], 16);
    c->hash = (c->hash * 0x2892e0dbu) ^ sx_rr(c->hash, 16);
    c->lane[0] += c->lane[5] ^ 0x4e00fda0u;
    t1 ^= (uint32_t)pack_range(c, (uint8_t)(t0 >> 16), t2);
    c->raw[c->slo + (int)((t0 + 48349u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum += t1;
    return t0 + t2;
}

static int parse_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    store_queue(c, t0, t1);
    c->lane[7] += c->lane[12]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 11);
    c->sched[6] = c->hash ^ sx_rl(c->lane[6], 1);
    t1 ^= (uint32_t)split_track(c, (uint8_t)(t0 >> 16), t2);
    c->lane[4] += c->lane[7]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 28);
    c->hash = (c->hash * 0xb3de71b7u) ^ sx_rr(c->hash, 16);
    t0 ^= latch_layer(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash = (c->hash * 0x560d92f3u) ^ sx_rr(c->hash, 30);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t peek_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x7adb2b69u) ^ sx_rr(c->hash, 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4427u) % (uint32_t)c->rln)] << 0;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 5);
    c->lane[0] ^= sx_rl(c->lane[6], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe09cabb1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39361u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xab) << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t drain_part(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb4) << 0;
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xba070b87u;
    blend_state(c, &c->lane[5], 3);
    c->hash = (c->hash * 0xb397d98bu) ^ sx_rr(c->hash, 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t latch_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[9] += c->lane[4] ^ 0x046d0761u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x864fa3cfu;
    c->sched[27] = c->hash ^ sx_rl(c->lane[3], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x492055ddu;
    c->lane[14] += c->lane[11] ^ 0xb285728fu;
    t2 = (t2 ^ c->sum) * 0x06baec79u;
    t0 ^= poll_line(c, t1);
    c->sched[9] = c->hash ^ sx_rl(c->lane[12], 15);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t place_rate(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)close_group(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x17a692b9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53841u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x0775d983u;
    t2 = (t2 ^ c->sum) * 0x5e5d05c5u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x63) << 16;
    t1 ^= (uint32_t)grow_segment(c, (uint8_t)(t0 >> 0), t2);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tap_page(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37432u) % (uint32_t)c->rln)] << 24;
    t0 ^= defer_store(c, t1);
    t0 ^= relay_arena_766(c, t1);
    c->sched[18] = c->hash ^ sx_rl(c->lane[0], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    rotate_count(c, &c->lane[2], 1);
    c->sched[29] = c->hash ^ sx_rl(c->lane[9], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5c) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t trim_slot(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[3] += c->lane[6]; c->lane[7] ^= c->lane[3]; c->lane[7] = sx_rl(c->lane[7], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1575u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 30786u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += scan_key_721(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[14] ^= sx_rl(c->lane[7], 11);
    t2 += probe_state(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[0] ^= sx_rl(c->lane[11], 15);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t join_band_688(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xdd7a5ba5u;
    hold_track(c, t0, t1);
    t1 ^= (uint32_t)latch_state(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[15] + 0x90cb1dc2u;
    c->raw[c->slo + (int)((t0 + 45344u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 31);
    push_mask(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x2ed165c3u;
    defer_chunk(c, &c->lane[6], 1);
    t1 ^= (uint32_t)grow_unit_719(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37102u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x87d2b281u) ^ sx_rr(c->hash, 2);
    c->lane[1] += c->lane[14] ^ 0x832459a8u;
    c->sum += t1;
    return t0 + t2;
}

static int emit_cursor(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= c->lane[13] + 0x41039eccu;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 10);
    c->hash = (c->hash * 0xc8f432e3u) ^ sx_rr(c->hash, 26);
    c->lane[10] ^= sx_rl(c->lane[4], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= c->lane[14] + 0x93a540bdu;
    fold_chunk(c, &c->lane[1], 2);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 19);
    t2 += queue_path(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x55b28041u) ^ sx_rr(c->hash, 20);
    fetch_node(c, &c->lane[7], 3);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int tally_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x84c8d011u;
    c->lane[2] ^= sx_rl(c->lane[3], 14);
    c->hash = (c->hash * 0xf9cad103u) ^ sx_rr(c->hash, 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t2 += (uint32_t)store_tail(c);
    c->sched[28] = c->hash ^ sx_rl(c->lane[11], 15);
    t1 ^= (uint32_t)move_port(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[14] + 0xb8f1e028u;
    c->lane[14] += c->lane[13]; c->lane[5] ^= c->lane[14]; c->lane[5] = sx_rl(c->lane[5], 4);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void grow_group(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 9138u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x970ca83du) ^ sx_rr(c->hash, 8);
    c->hash = (c->hash * 0x4dddc0adu) ^ sx_rr(c->hash, 28);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t scan_item_692(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[6] += c->lane[12]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 18);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23005u) % (uint32_t)c->rln)] << 8;
    c->lane[6] += c->lane[7]; c->lane[4] ^= c->lane[6]; c->lane[4] = sx_rl(c->lane[4], 8);
    c->sched[21] = c->hash ^ sx_rl(c->lane[12], 9);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 28);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x11fbd71fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41488u) % (uint32_t)c->rln)] << 16;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 1);
    c->lane[3] ^= sx_rl(c->lane[12], 14);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t sync_ring(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += join_count(c, c->slo, c->sln);
    c->hash ^= c->lane[15] + 0x6dc1c8e5u;
    t2 = (t2 ^ c->sum) * 0xb1b34799u;
    c->hash ^= c->lane[4] + 0x7ef89909u;
    flush_head(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc85016ebu;
    c->hash ^= c->lane[4] + 0xfdfdd8b8u;
    c->sum += t1;
    return t0 + t2;
}

static void pick_mask(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= settle_value(c, t1);
    t0 ^= drain_cell_765(c, t1);
    c->hash ^= c->lane[4] + 0xcc0f0d6fu;
    t2 = (t2 ^ c->sum) * 0x88c71155u;
    t1 ^= (uint32_t)shift_record_764(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)peek_limit(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)sort_marker(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25771u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x2db880e3u) ^ sx_rr(c->hash, 15);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int peek_store_695(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62702u) % (uint32_t)c->rln)] << 0;
    c->lane[14] ^= sx_rl(c->lane[8], 13);
    c->raw[c->slo + (int)((t0 + 17232u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[4] += c->lane[10]; c->lane[12] ^= c->lane[4]; c->lane[12] = sx_rl(c->lane[12], 28);
    c->hash ^= c->lane[10] + 0xa33c0ee6u;
    c->raw[c->slo + (int)((t0 + 50217u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t reset_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58928u) % (uint32_t)c->rln)] << 0;
    c->sched[22] = c->hash ^ sx_rl(c->lane[7], 5);
    c->hash = (c->hash * 0xa8b32f3du) ^ sx_rr(c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc2ee28abu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x238076fdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21801u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static void move_cell(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 37840u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48382u) % (uint32_t)c->rln)] << 0;
    t0 ^= split_record(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12154u) % (uint32_t)c->rln)] << 24;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 6);
    t2 += (uint32_t)parse_gap_735(c);
    c->lane[5] += c->lane[10]; c->lane[14] ^= c->lane[5]; c->lane[14] = sx_rl(c->lane[14], 19);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void scan_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x4c) << 8;
    t1 ^= (uint32_t)step_arena(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[0] + 0x65dd5f4bu;
    c->raw[c->slo + (int)((t0 + 621u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[15] += c->lane[6]; c->lane[8] ^= c->lane[15]; c->lane[8] = sx_rl(c->lane[8], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[1] += c->lane[14]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 7);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x01b26837u;
    flush_head(c, t0, t1);
    c->lane[9] += c->lane[2]; c->lane[10] ^= c->lane[9]; c->lane[10] = sx_rl(c->lane[10], 1);
    clamp_row(c, t0, t1);
    c->lane[7] ^= sx_rl(c->lane[11], 27);
    t0 ^= stage_label(c, t1);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t fold_seat_699(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)clamp_tail_711(c);
    c->hash ^= c->lane[2] + 0x9061bc90u;
    t2 += scan_key_721(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe8) << 8;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 2);
    c->raw[c->slo + (int)((t0 + 43307u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[1] ^ 0x357b73ebu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t grow_row_700(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 49896u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[15] += c->lane[2]; c->lane[9] ^= c->lane[15]; c->lane[9] = sx_rl(c->lane[9], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5c4db973u;
    c->hash = (c->hash * 0xf681aeadu) ^ sx_rr(c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int queue_lease(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf2) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7380u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0xd8b427a1u) ^ sx_rr(c->hash, 21);
    c->lane[1] += c->lane[3] ^ 0x2fba76aeu;
    t2 = (t2 ^ c->sum) * 0x5e4c5e97u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 36127u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6724u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[6] + 0x641614f1u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t tap_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] ^= sx_rl(c->lane[3], 10);
    t2 += (uint32_t)reset_batch(c);
    c->lane[15] ^= sx_rl(c->lane[12], 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    trace_rate(c, &c->lane[3], 3);
    load_label(c, t0, t1);
    c->hash ^= c->lane[13] + 0xf4dc8ea1u;
    c->lane[11] += c->lane[13] ^ 0xe0c3d26cu;
    c->hash ^= c->lane[4] + 0xbfdd3998u;
    t0 ^= drain_cell_765(c, t1);
    c->lane[2] += c->lane[1]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 31);
    c->sum += t1;
    return t0 + t2;
}

static void slice_stream_703(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xa277d317u) ^ sx_rr(c->hash, 16);
    t1 ^= (uint32_t)step_scope_756(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x92) << 8;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 18);
    t2 = (t2 ^ c->sum) * 0x3001af13u;
    c->sched[28] = c->hash ^ sx_rl(c->lane[8], 5);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 17);
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 14);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int defer_window(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6a92ea6fu;
    c->raw[c->slo + (int)((t0 + 62542u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 ^= cache_run(c, t1);
    c->lane[1] += c->lane[3]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 27);
    c->sched[25] = c->hash ^ sx_rl(c->lane[14], 16);
    cache_node(c, &c->lane[11], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1477u) % (uint32_t)c->rln)] << 0;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t slice_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xad) << 0;
    c->lane[3] += c->lane[9] ^ 0x465dc9b9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[3] += c->lane[8] ^ 0xa508ac50u;
    t2 += (uint32_t)sift_bucket(c);
    c->sched[4] = c->hash ^ sx_rl(c->lane[6], 5);
    c->sched[29] = c->hash ^ sx_rl(c->lane[2], 16);
    t2 = (t2 ^ c->sum) * 0x4935faddu;
    c->hash ^= c->lane[7] + 0x457f8b7cu;
    c->lane[0] += c->lane[11] ^ 0x2a350c4eu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11214u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2f4c0e2du;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t queue_band(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[2]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x2743d6bbu;
    t1 ^= (uint32_t)join_region(c, (uint8_t)(t0 >> 16), t2);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 15);
    t2 = (t2 ^ c->sum) * 0xdffa5d57u;
    c->lane[13] += c->lane[6]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 ^= chain_band(c, t1);
    c->lane[8] ^= sx_rl(c->lane[14], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static int swap_key(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash ^= c->lane[7] + 0x25f0460fu;
    c->sched[31] = c->hash ^ sx_rl(c->lane[8], 2);
    drain_row(c, t0, t1);
    c->hash = (c->hash * 0x2de1ba0du) ^ sx_rr(c->hash, 15);
    c->raw[c->slo + (int)((t0 + 58011u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += swap_scope(c, c->rlo, c->rln);
    c->hash ^= c->lane[1] + 0xb48bc0fcu;
    close_store(c, &c->lane[1], 2);
    t1 ^= (uint32_t)yield_stream(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0x46d82f31u) ^ sx_rr(c->hash, 30);
    t0 ^= defer_slot(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x680827d7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    latch_tail(c, t0, t1);
    c->sched[17] = c->hash ^ sx_rl(c->lane[14], 14);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int hold_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 11);
    c->raw[c->slo + (int)((t0 + 37043u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t1 ^= (uint32_t)close_state(c, (uint8_t)(t0 >> 0), t2);
    c->sched[10] = c->hash ^ sx_rl(c->lane[9], 31);
    fill_scope(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0xa8d374cfu) ^ sx_rr(c->hash, 5);
    c->sched[20] = c->hash ^ sx_rl(c->lane[4], 26);
    c->sched[13] = c->hash ^ sx_rl(c->lane[8], 30);
    t2 += (uint32_t)queue_port(c);
    c->hash ^= c->lane[13] + 0x3116a19du;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int sift_bucket(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash = (c->hash * 0xc2181481u) ^ sx_rr(c->hash, 15);
    c->hash = (c->hash * 0x8ea0a299u) ^ sx_rr(c->hash, 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x536a7275u;
    c->hash = (c->hash * 0x4ecab3b7u) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x70) << 8;
    c->lane[8] ^= sx_rl(c->lane[8], 28);
    t2 = (t2 ^ c->sum) * 0xa66e1735u;
    c->lane[14] += c->lane[3] ^ 0x5e66f493u;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64139u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa3) << 0;
    c->lane[13] += c->lane[5]; c->lane[11] ^= c->lane[13]; c->lane[11] = sx_rl(c->lane[11], 30);
    c->lane[5] += c->lane[15] ^ 0x3e46212fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int clamp_tail_711(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->lane[6] ^= sx_rl(c->lane[7], 18);
    c->hash ^= c->lane[11] + 0x3059bc07u;
    c->lane[2] ^= sx_rl(c->lane[9], 23);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xc9) << 16;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void close_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[13] + 0xe948c8f9u;
    c->raw[c->slo + (int)((t0 + 36327u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[0] = c->hash ^ sx_rl(c->lane[0], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57390u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t step_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34661u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xbb1ddea5u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t move_port(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x995a97bbu) ^ sx_rr(c->hash, 2);
    c->hash ^= c->lane[2] + 0xbf45a62au;
    c->sched[7] = c->hash ^ sx_rl(c->lane[1], 9);
    c->lane[7] += c->lane[3] ^ 0x23e00efau;
    c->raw[c->slo + (int)((t0 + 33028u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x76ee2745u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t swap_scope(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[10]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 29);
    c->lane[14] ^= sx_rl(c->lane[5], 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xaa6f0b0bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[11] += c->lane[12]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 31);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t cache_run(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7b651a9fu;
    c->lane[11] += c->lane[14] ^ 0x729e4b4eu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 6);
    c->lane[13] ^= sx_rl(c->lane[7], 2);
    c->lane[15] += c->lane[4]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7c) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40032u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t settle_value(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[9] + 0xf83d96fdu;
    c->lane[2] += c->lane[15]; c->lane[1] ^= c->lane[2]; c->lane[1] = sx_rl(c->lane[1], 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64639u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x62) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] += c->lane[9]; c->lane[11] ^= c->lane[7]; c->lane[11] = sx_rl(c->lane[11], 27);
    c->lane[9] += c->lane[14] ^ 0xb8dbd175u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xad) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t join_count(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57276u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x09311f25u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x30) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57231u) % (uint32_t)c->rln)] << 16;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x1687a877u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[3], 21);
    c->lane[11] ^= sx_rl(c->lane[15], 3);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t grow_unit_719(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[15] += c->lane[3] ^ 0x6adad66du;
    c->hash = (c->hash * 0x152bc649u) ^ sx_rr(c->hash, 9);
    c->hash = (c->hash * 0x33d2ed31u) ^ sx_rr(c->hash, 27);
    t2 = (t2 ^ c->sum) * 0x3f131777u;
    c->raw[c->slo + (int)((t0 + 10078u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25678u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x918eff8du) ^ sx_rr(c->hash, 15);
    c->hash ^= c->lane[4] + 0x18a03439u;
    t2 = (t2 ^ c->sum) * 0xf7e24dc7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void fill_scope(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0a) << 16;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 10);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 19);
    t2 = (t2 ^ c->sum) * 0x2f1e6867u;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t scan_key_721(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[12] += c->lane[10] ^ 0x8ac9fb64u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xac) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3bfc5573u;
    c->hash ^= c->lane[5] + 0x8a0c51a4u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44704u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3d) << 8;
    c->sum += t1;
    return t0 + t2;
}

static int clamp_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb1) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x59e45f8fu;
    t2 = (t2 ^ c->sum) * 0x776ba7a5u;
    c->lane[7] += c->lane[3]; c->lane[9] ^= c->lane[7]; c->lane[9] = sx_rl(c->lane[9], 20);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int queue_port(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->hash ^= c->lane[0] + 0xe5ed758eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x72f95fa5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[9] += c->lane[13] ^ 0x0611216eu;
    c->sched[7] = c->hash ^ sx_rl(c->lane[3], 5);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fetch_node(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 18);
    c->hash = (c->hash * 0xd8b71d3bu) ^ sx_rr(c->hash, 15);
    c->lane[3] += c->lane[1] ^ 0xd47e2c58u;
    c->hash = (c->hash * 0x0c867a8du) ^ sx_rr(c->hash, 24);
    c->sched[27] = c->hash ^ sx_rl(c->lane[1], 15);
    c->hash = (c->hash * 0x099ac42fu) ^ sx_rr(c->hash, 12);
    c->lane[6] += c->lane[11]; c->lane[9] ^= c->lane[6]; c->lane[9] = sx_rl(c->lane[9], 27);
    c->hash = (c->hash * 0x15cd4f4bu) ^ sx_rr(c->hash, 15);
    c->sched[20] = c->hash ^ sx_rl(c->lane[9], 5);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 6);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void blend_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x8b66448du;
    c->raw[c->slo + (int)((t0 + 13533u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x23) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf9) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc2) << 0;
    c->lane[14] += c->lane[14] ^ 0x4e0881b0u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41919u) % (uint32_t)c->rln)] << 24;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void clamp_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[14] + 0xc0399ac3u;
    c->hash = (c->hash * 0x463843e1u) ^ sx_rr(c->hash, 29);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t yield_stream(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 57753u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] += c->lane[13]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t split_track(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[20] = c->hash ^ sx_rl(c->lane[8], 12);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7b) << 16;
    c->raw[c->slo + (int)((t0 + 35279u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9d) << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t stage_label(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa8) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2e) << 0;
    c->sched[19] = c->hash ^ sx_rl(c->lane[6], 21);
    c->lane[0] += c->lane[11]; c->lane[4] ^= c->lane[0]; c->lane[4] = sx_rl(c->lane[4], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[23] = c->hash ^ sx_rl(c->lane[12], 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xcd) << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[0], 28);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void rotate_count(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= c->lane[1] + 0x92bbf575u;
    t2 = (t2 ^ c->sum) * 0xcbc5c223u;
    c->lane[14] ^= sx_rl(c->lane[1], 30);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 8);
    c->raw[c->slo + (int)((t0 + 13707u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[31] = c->hash ^ sx_rl(c->lane[12], 4);
    t2 = (t2 ^ c->sum) * 0x2b99bfedu;
    c->raw[c->slo + (int)((t0 + 34545u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x77b586f3u) ^ sx_rr(c->hash, 20);
    c->lane[10] += c->lane[14]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 6);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int store_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xbd) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[7], 11);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void push_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 5);
    c->lane[1] += c->lane[9] ^ 0x00a4992cu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[0] += c->lane[5]; c->lane[15] ^= c->lane[0]; c->lane[15] = sx_rl(c->lane[15], 13);
    c->lane[9] += c->lane[1] ^ 0xd205a1ccu;
    c->lane[2] += c->lane[9]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 2);
    t2 = (t2 ^ c->sum) * 0xa01a7bbfu;
    c->lane[8] ^= sx_rl(c->lane[9], 12);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 13);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void store_queue(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x81b0f561u) ^ sx_rr(c->hash, 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x16455711u;
    c->lane[7] ^= sx_rl(c->lane[13], 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50482u) % (uint32_t)c->rln)] << 0;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static int reset_batch(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf0) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40803u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[1] + 0x17de66d8u;
    c->hash = (c->hash * 0x14a134a7u) ^ sx_rr(c->hash, 7);
    c->lane[15] ^= sx_rl(c->lane[1], 6);
    c->lane[3] ^= sx_rl(c->lane[1], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc47a479du;
    c->hash ^= c->lane[4] + 0xf88f2667u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= c->lane[14] + 0xe5ff3befu;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int parse_gap_735(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x1c) << 0;
    c->hash ^= c->lane[8] + 0xc22269acu;
    c->hash = (c->hash * 0x703fb013u) ^ sx_rr(c->hash, 20);
    c->raw[c->slo + (int)((t0 + 58848u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t probe_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 9);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47456u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[8] + 0xd19cfc08u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 5);
    c->hash = (c->hash * 0x79d4ccddu) ^ sx_rr(c->hash, 5);
    c->sched[3] = c->hash ^ sx_rl(c->lane[10], 24);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t close_group(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[2] + 0x226ff5feu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf42074f3u;
    c->lane[4] += c->lane[13] ^ 0x020cd00fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2873u) % (uint32_t)c->rln)] << 24;
    c->sched[7] = c->hash ^ sx_rl(c->lane[10], 12);
    c->raw[c->slo + (int)((t0 + 8228u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[9] += c->lane[0]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 28);
    c->hash ^= c->lane[2] + 0xb7b8ff49u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void drain_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] += c->lane[13] ^ 0x6fe881c4u;
    c->lane[4] ^= sx_rl(c->lane[4], 21);
    c->raw[c->slo + (int)((t0 + 64216u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 43824u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[0] += c->lane[13] ^ 0x96e33452u;
    c->hash ^= c->lane[6] + 0xd765c7d4u;
    c->hash ^= c->lane[14] + 0x0375d96du;
    c->lane[6] += c->lane[12]; c->lane[14] ^= c->lane[6]; c->lane[14] = sx_rl(c->lane[14], 5);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static void hold_track(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 13440u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[8] = c->hash ^ sx_rl(c->lane[0], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9fec4d1du;
    c->lane[13] += c->lane[1] ^ 0xe2089891u;
    c->lane[5] += c->lane[12]; c->lane[6] ^= c->lane[5]; c->lane[6] = sx_rl(c->lane[6], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t grow_offset(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[1] += c->lane[6] ^ 0x2c79ce3cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[14] += c->lane[6]; c->lane[8] ^= c->lane[14]; c->lane[8] = sx_rl(c->lane[8], 15);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 19);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 10);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t pack_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] += c->lane[14]; c->lane[3] ^= c->lane[12]; c->lane[3] = sx_rl(c->lane[3], 22);
    c->lane[11] ^= sx_rl(c->lane[15], 17);
    c->raw[c->slo + (int)((t0 + 29207u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] ^= sx_rl(c->lane[13], 30);
    c->lane[3] += c->lane[10] ^ 0xee1f8022u;
    c->hash ^= c->lane[4] + 0xd864833cu;
    t2 = (t2 ^ c->sum) * 0xd4be839fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc8) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36016u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t latch_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x1ab981cfu) ^ sx_rr(c->hash, 10);
    c->lane[4] ^= sx_rl(c->lane[9], 28);
    c->sched[4] = c->hash ^ sx_rl(c->lane[11], 6);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 18);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 1);
    c->hash = (c->hash * 0x64642125u) ^ sx_rr(c->hash, 4);
    c->sched[13] = c->hash ^ sx_rl(c->lane[9], 25);
    c->lane[7] += c->lane[9]; c->lane[10] ^= c->lane[7]; c->lane[10] = sx_rl(c->lane[10], 27);
    c->hash ^= c->lane[14] + 0xd25297a8u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0xffbfdf4du) ^ sx_rr(c->hash, 18);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t close_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x83) << 0;
    c->lane[2] += c->lane[7]; c->lane[3] ^= c->lane[2]; c->lane[3] = sx_rl(c->lane[3], 11);
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 28);
    c->raw[c->slo + (int)((t0 + 17502u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t sort_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[13] += c->lane[12]; c->lane[15] ^= c->lane[13]; c->lane[15] = sx_rl(c->lane[15], 19);
    c->raw[c->slo + (int)((t0 + 23626u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x77ed96ffu;
    c->lane[7] ^= sx_rl(c->lane[3], 23);
    c->sched[12] = c->hash ^ sx_rl(c->lane[4], 4);
    c->lane[4] ^= sx_rl(c->lane[5], 27);
    c->lane[11] += c->lane[10]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 28);
    c->raw[c->slo + (int)((t0 + 20324u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void latch_tail(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 43957u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xad3a120du;
    c->lane[15] += c->lane[10]; c->lane[11] ^= c->lane[15]; c->lane[11] = sx_rl(c->lane[11], 4);
    c->lane[9] += c->lane[15]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 9);
    c->lane[2] += c->lane[9] ^ 0x9239bc97u;
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t defer_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xaddcb9bbu) ^ sx_rr(c->hash, 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13928u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6505u) % (uint32_t)c->rln)] << 16;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 23);
    c->lane[15] ^= sx_rl(c->lane[4], 11);
    c->hash ^= c->lane[7] + 0xe608ca89u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t split_record(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x0d558bf5u;
    c->raw[c->slo + (int)((t0 + 13319u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0x48198741u;
    c->lane[2] += c->lane[12] ^ 0x71232bcau;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x64) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void flush_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[9] ^= sx_rl(c->lane[14], 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc5253511u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[10] += c->lane[10] ^ 0x15ef1230u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[7] = c->hash ^ sx_rl(c->lane[9], 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61492u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 13831u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xec8a9081u;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint32_t queue_path(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x998afbc7u;
    c->hash ^= c->lane[2] + 0x2356d18du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x64e55559u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x28) << 0;
    c->hash = (c->hash * 0xcc799cb5u) ^ sx_rr(c->hash, 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49584u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x27c68f8du;
    c->lane[15] ^= sx_rl(c->lane[6], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26439u) % (uint32_t)c->rln)] << 8;
    c->sum += t1;
    return t0 + t2;
}

static void trace_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xed6c6397u) ^ sx_rr(c->hash, 11);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 31);
    c->lane[9] += c->lane[7] ^ 0x5af5d8dau;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xa0baac41u;
    c->sched[10] = c->hash ^ sx_rl(c->lane[9], 16);
    c->hash = (c->hash * 0x55a9e4efu) ^ sx_rr(c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[9], 16);
    c->hash ^= c->lane[1] + 0x3be741bau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36926u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void load_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 37396u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 35921u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 41981u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1422de85u;
    c->hash = (c->hash * 0x7b8b830bu) ^ sx_rr(c->hash, 29);
    c->lane[2] ^= sx_rl(c->lane[9], 6);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t defer_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xc3a3f9c5u) ^ sx_rr(c->hash, 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[1] + 0x693ec7beu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18399u) % (uint32_t)c->rln)] << 16;
    c->lane[4] += c->lane[10]; c->lane[12] ^= c->lane[4]; c->lane[12] = sx_rl(c->lane[12], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 58213u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38176u) % (uint32_t)c->rln)] << 16;
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 9);
    c->lane[8] ^= sx_rl(c->lane[4], 17);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t chain_band(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x18c0f339u;
    c->hash ^= c->lane[5] + 0x264552c3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0f3b85efu;
    c->sched[21] = c->hash ^ sx_rl(c->lane[12], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x53e0068du;
    c->lane[15] += c->lane[13]; c->lane[3] ^= c->lane[15]; c->lane[3] = sx_rl(c->lane[3], 27);
    c->lane[6] += c->lane[15]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 18);
    c->lane[13] ^= sx_rl(c->lane[15], 10);
    c->lane[13] ^= sx_rl(c->lane[13], 14);
    c->lane[5] += c->lane[6]; c->lane[9] ^= c->lane[5]; c->lane[9] = sx_rl(c->lane[9], 1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t peek_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 332u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x1f) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24348u) % (uint32_t)c->rln)] << 24;
    c->lane[4] ^= sx_rl(c->lane[15], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd52e7a85u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xba) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37220u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sched[26] = c->hash ^ sx_rl(c->lane[2], 14);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void defer_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[3] += c->lane[6] ^ 0x6a87d87fu;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 27);
    c->lane[13] ^= sx_rl(c->lane[11], 28);
    c->raw[c->slo + (int)((t0 + 58944u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[1] += c->lane[6] ^ 0xccbb081eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t step_scope_756(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[3] += c->lane[7] ^ 0xd3bab281u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 10);
    t2 = (t2 ^ c->sum) * 0xe450629bu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 17);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[3] + 0x71df2df5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1f50401bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4a) << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t poll_line(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x0636a9ddu) ^ sx_rr(c->hash, 27);
    c->sched[29] = c->hash ^ sx_rl(c->lane[12], 28);
    c->lane[0] += c->lane[14] ^ 0x4373547bu;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 7);
    c->lane[3] += c->lane[14] ^ 0x99bca3d8u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28810u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[8], 23);
    c->hash ^= c->lane[15] + 0xe0e324dfu;
    c->hash = (c->hash * 0xc264042bu) ^ sx_rr(c->hash, 6);
    c->lane[13] += c->lane[12] ^ 0x5900c217u;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 30);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void prime_stack_758(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52194u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[2] += c->lane[11] ^ 0xa02a505bu;
    c->lane[8] += c->lane[14]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 26);
    c->hash ^= c->lane[10] + 0xd827af5du;
    t2 = (t2 ^ c->sum) * 0x5e29ad2fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x81b75707u;
    c->hash = (c->hash * 0x4ba80853u) ^ sx_rr(c->hash, 28);
    c->hash ^= c->lane[15] + 0xafffa694u;
    t2 = (t2 ^ c->sum) * 0x7257d673u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t join_region(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe9) << 16;
    c->lane[5] ^= sx_rl(c->lane[1], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    c->raw[c->slo + (int)((t0 + 58934u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[7]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xfc) << 8;
    c->lane[7] += c->lane[9] ^ 0xebfad5c7u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[30] = c->hash ^ sx_rl(c->lane[8], 22);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t grow_segment(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xa668f7f5u;
    c->sched[27] = c->hash ^ sx_rl(c->lane[9], 10);
    c->lane[15] += c->lane[5]; c->lane[12] ^= c->lane[15]; c->lane[12] = sx_rl(c->lane[12], 9);
    c->lane[1] ^= sx_rl(c->lane[7], 23);
    c->lane[12] += c->lane[4]; c->lane[8] ^= c->lane[12]; c->lane[8] = sx_rl(c->lane[8], 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void cache_node(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 1263u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9e) << 0;
    c->lane[14] ^= sx_rl(c->lane[0], 19);
    c->lane[8] ^= sx_rl(c->lane[11], 7);
    c->lane[13] += c->lane[11] ^ 0x4c635e08u;
    c->hash = (c->hash * 0xc2ce0b4fu) ^ sx_rr(c->hash, 9);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t latch_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sched[28] = c->hash ^ sx_rl(c->lane[3], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf3) << 16;
    c->lane[4] ^= sx_rl(c->lane[6], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[14] += c->lane[6] ^ 0xf914aa4fu;
    c->hash ^= c->lane[10] + 0xc0f4532du;
    c->sched[31] = c->hash ^ sx_rl(c->lane[3], 3);
    c->hash = (c->hash * 0x47c66089u) ^ sx_rr(c->hash, 1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t step_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[3], 29);
    c->lane[1] ^= sx_rl(c->lane[12], 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37713u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3d78c15bu;
    c->sched[4] = c->hash ^ sx_rl(c->lane[6], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xf0c46cfbu;
    c->lane[3] += c->lane[8]; c->lane[12] ^= c->lane[3]; c->lane[12] = sx_rl(c->lane[12], 11);
    c->raw[c->slo + (int)((t0 + 41651u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x488945ffu) ^ sx_rr(c->hash, 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x70004e45u;
    c->lane[15] += c->lane[10] ^ 0x53a6075cu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t shift_record_764(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[6] += c->lane[4] ^ 0xeef889a8u;
    c->raw[c->slo + (int)((t0 + 61787u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[5] + 0xd1e3ec54u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t drain_cell_765(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 693u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5732d4cfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x27) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x84) << 0;
    c->lane[0] += c->lane[1] ^ 0x3535fdc0u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t relay_arena_766(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1c2a4aa3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xaff0ee41u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20100u) % (uint32_t)c->rln)] << 0;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x51) << 16;
    c->sched[27] = c->hash ^ sx_rl(c->lane[15], 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void rotate_run_767(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[16] = c->hash ^ sx_rl(c->lane[3], 23);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 20);
    c->lane[0] ^= sx_rl(c->lane[3], 16);
    c->lane[2] += c->lane[10]; c->lane[3] ^= c->lane[2]; c->lane[3] = sx_rl(c->lane[3], 5);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}


static const uint8_t sx_polys[16] = { 0x1d, 0x2b, 0x2d, 0x39, 0x3f, 0x4d, 0x5f, 0x63, 0x65, 0x69, 0x71, 0x77, 0x7b, 0x87, 0x8b, 0x8d };

static uint32_t step_field_768(uint32_t x) {
    return (x * 0x7fb5d329u) ^ sx_rl(x + 0x4e1d9a2bu, 11);
}

static int mark_stream(sx_state *c) {
    size_t got = 0;
#ifdef SX_DESIGNER
    {
        const char *forced = getenv("SX_DESIGNER_ID");
        if (forced != NULL) {
            got = strlen(forced);
            if (got > 63) got = 63;
            memcpy(c->name, forced, got);
        }
    }
#endif
    if (got == 0) {
        FILE *f = fopen("/proc/self/cmdline", "rb");
        if (f != NULL) {
            got = fread(c->name, 1, 63, f);
            fclose(f);
        }
    }
    c->name[got] = 0;
    c->nlen = (int)strlen((const char *)c->name);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x79265323u;
        t2 += (uint32_t)align_slot(c);
        t2 += push_stream_578(c, c->slo, c->sln);
        t0 ^= cache_chunk(c, t1);
        c->raw[c->slo + (int)((t0 + 57454u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
        c->lane[0] += c->lane[3]; c->lane[9] ^= c->lane[0]; c->lane[9] = sx_rl(c->lane[9], 26);
        c->lane[7] += c->lane[11]; c->lane[3] ^= c->lane[7]; c->lane[3] = sx_rl(c->lane[3], 18);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    return c->nlen;
}

static uint32_t emit_unit(uint32_t prime, uint32_t h, uint8_t x) {
    return (h ^ (uint32_t)x) * prime;
}

static uint32_t fold_cursor(sx_state *c, uint32_t nonce, int size) {
    uint32_t m = step_field_768((uint32_t)c->nlen);
    uint32_t basis = 0x16de7a0cu ^ m;
    uint32_t prime = 0x96c2e65au ^ m;
    uint32_t h = basis;
    int i;
    for (i = 0; i < c->nlen; i++) h = emit_unit(prime, h, c->name[i]);
    for (i = 0; i < 4; i++) h = emit_unit(prime, h, (uint8_t)(nonce >> (i * 8)));
    h = emit_unit(prime, h, (uint8_t)size);
#ifdef SX_TRACE
    fprintf(stderr, "T fnv_pre h=%08x m=%08x nlen=%d hash=%08x sum=%08x\n", h, m, c->nlen, c->hash, c->sum);
#endif
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
        c->lane[6] += c->lane[14] ^ 0x267cae53u;
        c->lane[3] ^= sx_rl(c->lane[15], 1);
        c->lane[3] += c->lane[1]; c->lane[5] ^= c->lane[3]; c->lane[5] = sx_rl(c->lane[5], 8);
        t1 ^= (uint32_t)link_queue(c, (uint8_t)(t0 >> 8), t2);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2049u) % (uint32_t)c->rln)] << 24;
        c->lane[1] += c->lane[0] ^ 0x5d874444u;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43315u) % (uint32_t)c->rln)] << 24;
        c->raw[c->slo + (int)((t0 + 2844u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
        c->hash ^= t0 ^ t1 ^ t2;
    }
#ifdef SX_TRACE
    fprintf(stderr, "T fnv_post h=%08x hash=%08x sum=%08x lane0=%08x\n", h, c->hash, c->sum, c->lane[0]);
#endif
    return h ^ c->hash;
}

static void hold_level(sx_state *c, uint32_t seed) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[12] += c->lane[10] ^ 0x60b627bau;
        c->raw[c->slo + (int)((t0 + 62765u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
        t0 ^= fold_mask(c, t1);
        t2 += shift_scope(c, c->slo, c->sln);
        c->sched[6] = c->hash ^ sx_rl(c->lane[3], 25);
        c->lane[8] += c->lane[3]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 10);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint8_t perm[256];
    uint32_t x;
    int a, b, i;
    c->gin[0] = 0;
    for (a = 1; a < 256; a++) {
        c->gin[a] = 0;
        for (b = 1; b < 256; b++) {
            if (sx_gf(c, (uint8_t)a, (uint8_t)b) == 1u) {
                c->gin[a] = (uint8_t)b;
                break;
            }
        }
    }
    for (i = 0; i < 256; i++) perm[i] = (uint8_t)i;
    x = (seed ^ c->hash) | 1u;
    for (i = 255; i > 0; i--) {
        uint8_t tmp;
        int j;
        x = sx_step(x);
        j = (int)(x % (uint32_t)(i + 1));
        tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
    for (i = 0; i < 256; i++) c->fwd[i] = perm[c->gin[i]];
    for (i = 0; i < 256; i++) c->rev[c->fwd[i]] = (uint8_t)i;
}

static void drain_track_772(sx_state *c, uint32_t master) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += (uint32_t)coal_head(c);
        c->hash = (c->hash * 0x376089efu) ^ sx_rr(c->hash, 8);
        trace_tail(c, &c->lane[0], 3);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4925u) % (uint32_t)c->rln)] << 8;
        t0 ^= slice_digest(c, t1);
        c->lane[13] ^= sx_rl(c->lane[1], 22);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x065aa47bu;
        t2 = (t2 ^ c->sum) * 0x0100a373u;
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5d) << 0;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t x = (master ^ c->hash) | 1u;
    int i;
    x = sx_step(x);
    c->poly = sx_polys[(x >> 3) & 15u];
    x = sx_step(x);
    c->cpoly = x | 0x80000001u;
    x = sx_step(x);
    c->step = x | 1u;
    for (i = 0; i < 4; i++) {
        x = sx_step(x);
        c->vec[i] = x;
    }
    hold_level(c, x);
}

static void defer_chunk_774(sx_state *c) {
    int i;
    c->shash = c->hash;
    c->ssum = c->sum;
    for (i = 0; i < 16; i++) c->slane[i] = c->lane[i];
    memcpy(c->sscr, c->raw + c->slo, (size_t)c->sln);
}

static void drain_row_775(sx_state *c) {
    int i;
    c->hash = c->shash;
    c->sum = c->ssum;
    for (i = 0; i < 16; i++) c->lane[i] = c->slane[i];
    memcpy(c->raw + c->slo, c->sscr, (size_t)c->sln);
}

static void fold_tail_776(sx_state *c, int lo, int ln) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 = (t2 ^ c->sum) * 0x0a0aa971u;
        c->raw[c->slo + (int)((t0 + 51659u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
        c->lane[14] ^= sx_rl(c->lane[7], 27);
        t1 ^= (uint32_t)cache_frame(c, (uint8_t)(t0 >> 0), t2);
        c->lane[2] ^= sx_rl(c->lane[7], 30);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13120u) % (uint32_t)c->rln)] << 0;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    int i;
    for (i = 0; i < ln; i++) {
        uint8_t v = c->raw[lo + i];
        c->hash = (c->hash ^ (uint32_t)v) * 0x0100019Du;
        c->sum = sx_crc(c, c->sum, (uint8_t)(v ^ (uint8_t)(i & 0xFF)));
        c->lane[i & 15] ^= sx_rl(c->hash + (uint32_t)i, (i % 29) + 1);
    }
}

static void defer_bound(sx_state *c, uint32_t *out) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash ^= c->lane[10] + 0xb5b19246u;
        c->hash = (c->hash * 0xc13679b1u) ^ sx_rr(c->hash, 1);
        c->lane[15] ^= sx_rl(c->lane[2], 28);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
        cache_stream(c, t0, t1);
        t2 = (t2 ^ c->sum) * 0xdea375f1u;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    int i;
    for (i = 0; i < 8; i++) {
        uint32_t v = c->sched[i];
        v ^= sx_rl(c->lane[i & 15], ((i * 3) % 29) + 1);
        v ^= c->hash * (uint32_t)(i + 1);
        v ^= sx_rl(c->sum, (i % 29) + 1);
        out[i] = v;
    }
}

static void pin_view(sx_state *c, uint32_t master, int rnd, int lo, int ln, uint32_t *out) {
    int i;
    c->hash = master ^ ((uint32_t)(rnd + 1) * 0x51A7B3C9u);
    c->sum = sx_rl(master, (rnd * 7) + 3);
    for (i = 0; i < 16; i++) c->lane[i] = master + ((uint32_t)(i + 1) * 0x2F1B3D57u) + (uint32_t)rnd;
    for (i = 0; i < 32; i++) c->sched[i] = 0;
    for (i = 0; i < c->sln; i++) {
        c->raw[c->slo + i] = (uint8_t)((master >> ((i & 3) * 8)) ^ (uint32_t)(rnd * 31 + i));
    }
    c->rlo = lo;
    c->rln = (ln > 0) ? ln : 1;
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += fill_list(c, c->slo, c->sln);
        c->sched[31] = c->hash ^ sx_rl(c->lane[9], 14);
        c->lane[4] += c->lane[14] ^ 0xa74c1a23u;
        tune_view(c, t0, t1);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        c->lane[8] += c->lane[4]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 27);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
        c->hash ^= t0 ^ t1 ^ t2;
    }
    fold_tail_776(c, lo, ln);
    switch (rnd) {
    case 0:
        relay_gap(c, c->hash, c->sum);
        sort_item(c, c->hash, c->sum);
        break;
    case 1:
        tune_bucket(c, c->hash, c->sum);
        mark_rate(c, c->hash, c->sum);
        break;
    default:
        pin_list(c, c->hash, c->sum);
        hold_entry(c, c->hash, c->sum);
        break;
    }
    defer_bound(c, out);
#ifdef SX_TRACE
    fprintf(stderr, "T rk%d %08x %08x %08x %08x %08x %08x %08x %08x\n", rnd,
            out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
#endif
}

static void pack_key(sx_state *c, const uint32_t *kk, uint32_t *out) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        parse_token(c, t0, t1);
        c->lane[13] += c->lane[13] ^ 0xe6dbdf39u;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20250u) % (uint32_t)c->rln)] << 24;
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
        clamp_offset(c, t0, t1);
        c->sched[26] = c->hash ^ sx_rl(c->lane[11], 13);
        c->lane[4] += c->lane[11] ^ 0x22b035b4u;
        c->lane[9] += c->lane[6]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 1);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x55) << 16;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    int i;
    for (i = 0; i < 4; i++) {
        uint32_t v = kk[i];
        v ^= sx_rl(c->hash, (i * 5) + 1);
        v ^= c->lane[i];
        v ^= sx_rl(c->sum, i + 1);
        out[i] = v;
    }
}

static void chain_list(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    drain_row_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += (uint32_t)latch_row(c);
        reap_cursor_670(c, t0, t1);
        t2 = (t2 ^ c->sum) * 0xa0d81899u;
        c->sched[25] = c->hash ^ sx_rl(c->lane[9], 22);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x34b27fefu;
        t1 ^= (uint32_t)blend_offset(c, (uint8_t)(t0 >> 8), t2);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint8_t tab[SX_PAY];
    uint8_t src[SX_PAY];
    uint32_t x;
    int i;
    pack_key(c, kk, w);
    for (i = 0; i < ln; i++) tab[i] = (uint8_t)i;
    x = w[0] | 1u;
    for (i = ln - 1; i > 0; i--) {
        uint8_t tmp;
        int j;
        x = sx_step(x);
        j = (int)(x % (uint32_t)(i + 1));
        tmp = tab[i];
        tab[i] = tab[j];
        tab[j] = tmp;
    }
    memcpy(src, c->raw + lo, (size_t)ln);
    if (fwd) {
        for (i = 0; i < ln; i++) c->raw[lo + i] = src[tab[i]];
    } else {
        for (i = 0; i < ln; i++) c->raw[lo + tab[i]] = src[i];
    }
}

static void flush_band(sx_state *c, const uint32_t *w, uint32_t *v) {
    uint32_t v0 = v[0], v1 = v[1], s = 0;
    int i;
    for (i = 0; i < 32; i++) {
        v0 += ((((v1 << 4) ^ (v1 >> 5)) + v1) ^ (s + w[s & 3]));
        s += c->step;
        v1 += ((((v0 << 4) ^ (v0 >> 5)) + v0) ^ (s + w[(s >> 11) & 3]));
    }
    v[0] = v0;
    v[1] = v1;
}

static void join_head(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    drain_row_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[7] += c->lane[11]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 14);
        c->lane[1] ^= sx_rl(c->lane[6], 13);
        t1 ^= (uint32_t)load_span(c, (uint8_t)(t0 >> 0), t2);
        t0 ^= relay_digest_568(c, t1);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54781u) % (uint32_t)c->rln)] << 16;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t v[2];
    uint8_t ks[8];
    int pos = 0, blk = 0, i;
    (void)fwd;
    pack_key(c, kk, w);
    while (pos < ln) {
        int rot = blk & 31;
        if (rot == 0) rot = 1;
        v[0] = (uint32_t)blk ^ w[0];
        v[1] = sx_rl(w[1], rot) ^ w[3];
        flush_band(c, w, v);
        for (i = 0; i < 4; i++) ks[i] = (uint8_t)(v[0] >> (i * 8));
        for (i = 0; i < 4; i++) ks[4 + i] = (uint8_t)(v[1] >> (i * 8));
        for (i = 0; i < 8 && pos + i < ln; i++) c->raw[lo + pos + i] ^= ks[i];
        pos += 8;
        blk++;
    }
}

static void emit_path(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    drain_row_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += (uint32_t)peek_store_695(c);
        c->lane[0] ^= sx_rl(c->lane[14], 17);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42716u) % (uint32_t)c->rln)] << 24;
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc91eb51fu;
        t2 = (t2 ^ c->sum) * 0xcb2df6e5u;
        cache_node(c, &c->lane[0], 1);
        relay_seat(c, &c->lane[3], 1);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint8_t m;
    int i;
    pack_key(c, kk, w);
    m = (uint8_t)((w[0] & 0xFFu) | 1u);
    if (fwd) {
        for (i = 1; i < ln; i++) c->raw[lo + i] ^= sx_gf(c, c->raw[lo + i - 1], m);
        for (i = 0; i < ln; i++) {
            uint8_t kbb = (uint8_t)(w[1 + (i & 1)] >> ((i & 3) * 8));
            int r = 1 + (int)(((w[2] >> ((i & 7) * 4)) & 7u) % 7u);
            c->raw[lo + i] = sx_bl(c->fwd[c->raw[lo + i] ^ kbb], r);
        }
    } else {
        for (i = 0; i < ln; i++) {
            uint8_t kbb = (uint8_t)(w[1 + (i & 1)] >> ((i & 3) * 8));
            int r = 1 + (int)(((w[2] >> ((i & 7) * 4)) & 7u) % 7u);
            c->raw[lo + i] = (uint8_t)(c->rev[sx_br(c->raw[lo + i], r)] ^ kbb);
        }
        for (i = ln - 1; i > 0; i--) c->raw[lo + i] ^= sx_gf(c, c->raw[lo + i - 1], m);
    }
}

static void slice_track(uint32_t *s, int a, int b, int d, int e) {
    s[a] += s[b]; s[e] = sx_rl(s[e] ^ s[a], 13);
    s[d] += s[e]; s[b] = sx_rl(s[b] ^ s[d], 9);
    s[a] += s[b]; s[e] = sx_rl(s[e] ^ s[a], 11);
    s[d] += s[e]; s[b] = sx_rl(s[b] ^ s[d], 6);
}

static void pin_node(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    drain_row_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8c) << 16;
        c->lane[4] += c->lane[6]; c->lane[15] ^= c->lane[4]; c->lane[15] = sx_rl(c->lane[15], 11);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
        pair_window(c, t0, t1);
        c->sched[22] = c->hash ^ sx_rl(c->lane[12], 27);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t base[16], s[16];
    uint8_t ks[64];
    int pos = 0, counter = 0, i, r;
    (void)fwd;
    pack_key(c, kk, w);
    while (pos < ln) {
        base[0] = c->vec[0]; base[1] = c->vec[1]; base[2] = c->vec[2]; base[3] = c->vec[3];
        base[4] = w[0]; base[5] = w[1]; base[6] = w[2]; base[7] = w[3];
        base[8] = sx_rl(w[0], 7); base[9] = sx_rl(w[1], 11);
        base[10] = sx_rl(w[2], 17); base[11] = sx_rl(w[3], 23);
        base[12] = (uint32_t)counter;
        base[13] = w[3] ^ 0x63D9A7C1u;
        base[14] = (uint32_t)c->size;
        base[15] = w[1] + w[2];
        for (i = 0; i < 16; i++) s[i] = base[i];
        for (r = 0; r < 10; r++) {
            slice_track(s, 0, 4, 8, 12);
            slice_track(s, 1, 5, 9, 13);
            slice_track(s, 2, 6, 10, 14);
            slice_track(s, 3, 7, 11, 15);
            slice_track(s, 0, 5, 10, 15);
            slice_track(s, 1, 6, 11, 12);
            slice_track(s, 2, 7, 8, 13);
            slice_track(s, 3, 4, 9, 14);
        }
        for (i = 0; i < 16; i++) {
            uint32_t v = s[i] + base[i];
            ks[i * 4 + 0] = (uint8_t)v;
            ks[i * 4 + 1] = (uint8_t)(v >> 8);
            ks[i * 4 + 2] = (uint8_t)(v >> 16);
            ks[i * 4 + 3] = (uint8_t)(v >> 24);
        }
        for (i = 0; i < 64 && pos + i < ln; i++) c->raw[lo + pos + i] ^= ks[i];
        pos += 64;
        counter++;
    }
}

static void pair_tuple(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    drain_row_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        trim_lease(c, &c->lane[2], 2);
        c->sched[18] = c->hash ^ sx_rl(c->lane[4], 21);
        c->lane[10] ^= sx_rl(c->lane[3], 19);
        c->lane[10] += c->lane[2] ^ 0xde79aeafu;
        c->lane[10] += c->lane[10] ^ 0x914ac330u;
        c->sched[16] = c->hash ^ sx_rl(c->lane[11], 23);
        c->lane[1] += c->lane[7]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 21);
        t2 = (t2 ^ c->sum) * 0x47c47a5du;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    int i;
    pack_key(c, kk, w);
    for (i = 0; i < ln; i++) {
        uint8_t m = (uint8_t)(((w[0] >> ((i & 3) * 8)) & 0xFFu) | 1u);
        int r = 1 + (int)(((w[1] >> ((i & 7) * 4)) & 7u) % 7u);
        if (fwd) {
            c->raw[lo + i] = sx_bl(sx_gf(c, c->raw[lo + i], m), r);
        } else {
            c->raw[lo + i] = sx_gf(c, sx_br(c->raw[lo + i], r), c->gin[m]);
        }
    }
}

static void reap_stream(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    drain_row_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6445u) % (uint32_t)c->rln)] << 0;
        c->lane[9] += c->lane[8]; c->lane[6] ^= c->lane[9]; c->lane[6] = sx_rl(c->lane[6], 17);
        t2 += (uint32_t)poll_range(c);
        fetch_range(c, t0, t1);
        c->lane[9] += c->lane[5]; c->lane[2] ^= c->lane[9]; c->lane[2] = sx_rl(c->lane[2], 19);
        c->lane[7] += c->lane[15] ^ 0x17ea9fd7u;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t x;
    int i;
    (void)fwd;
    pack_key(c, kk, w);
    x = w[0] | 1u;
    for (i = 0; i < ln; i++) {
        x = sx_step(x);
        c->raw[lo + i] ^= (uint8_t)x;
    }
}

static void move_layer(sx_state *c, int idx, int lo, int ln, const uint32_t *kk, int fwd) {
    switch (idx) {
    case 0: chain_list(c, lo, ln, kk, fwd); break;
    case 1: join_head(c, lo, ln, kk, fwd); break;
    case 2: emit_path(c, lo, ln, kk, fwd); break;
    case 3: pin_node(c, lo, ln, kk, fwd); break;
    case 4: pair_tuple(c, lo, ln, kk, fwd); break;
    default: reap_stream(c, lo, ln, kk, fwd); break;
    }
}

static void patch_stack(sx_state *c, uint32_t master, int rnd, int alo, int aln, int tlo, int tln, int s0, int s1, int fwd) {
    uint32_t kk[8];
    pin_view(c, master, rnd, alo, aln, kk);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += flush_span(c, c->slo, c->sln);
        store_list(c, &c->lane[7], 4);
        t2 += (uint32_t)join_frame(c);
        c->hash ^= c->lane[11] + 0x23b4a22cu;
        c->lane[3] ^= sx_rl(c->lane[14], 4);
        c->lane[13] += c->lane[5] ^ 0xbd01e147u;
        c->hash ^= c->lane[3] + 0x5c6d4390u;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    defer_chunk_774(c);
    if (fwd) {
        move_layer(c, s0, tlo, tln, &kk[0], 1);
        move_layer(c, s1, tlo, tln, &kk[4], 1);
    } else {
        move_layer(c, s1, tlo, tln, &kk[4], 0);
        move_layer(c, s0, tlo, tln, &kk[0], 0);
    }
}

void sx_entry(uint8_t *out, const uint8_t *in, int len, uint32_t nonce, int mode) {
    static const int PL[3][5] = { {0, 1, 0, 0, 1}, {1, 0, 1, 2, 3}, {2, 1, 0, 4, 5} };
    sx_state st;
    sx_state *c = &st;
    uint32_t base, master;
    int lows[2], lens[2];
    int h, i, k;

    if (in == NULL || out == NULL || len < 4 || len > SX_PAY) return;
    memset(&st, 0, sizeof(st));
    memcpy(c->raw, in, (size_t)len);
    c->size = len;
    c->slo = SX_PAY;
    c->sln = SX_SCR;
    c->rlo = SX_PAY;
    c->rln = SX_SCR;

    mark_stream(c);
    base = fold_cursor(c, nonce, len);
    {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        drain_block(c, &c->lane[11], 1);
        c->sched[1] = c->hash ^ sx_rl(c->lane[15], 1);
        c->sched[2] = c->hash ^ sx_rl(c->lane[7], 13);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x53e6f121u;
        c->lane[14] += c->lane[6] ^ 0x5013461eu;
        c->sched[6] = c->hash ^ sx_rl(c->lane[4], 23);
        t2 += (uint32_t)emit_cursor(c);
        c->lane[12] = sx_rr(c->lane[12] + c->hash, 9);
        t0 ^= mark_part(c, t1);
        c->lane[7] ^= sx_rl(c->lane[4], 8);
        c->hash = (c->hash * 0xc62c053bu) ^ sx_rr(c->hash, 25);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    }
    master = base ^ c->hash ^ c->sum ^ c->lane[3];
#ifdef SX_TRACE
    fprintf(stderr, "T master=%08x base=%08x\n", master, base);
#endif
    drain_track_772(c, master);
#ifdef SX_TRACE
    fprintf(stderr, "T poly=%02x cpoly=%08x step=%08x vec=%08x %08x %08x %08x fwd7=%02x\n",
            c->poly, c->cpoly, c->step, c->vec[0], c->vec[1], c->vec[2], c->vec[3], c->fwd[7]);
#endif

    h = len / 2;
    lows[0] = 0; lens[0] = h;
    lows[1] = h; lens[1] = len - h;

    for (k = 0; k < 3; k++) {
        i = mode ? k : (2 - k);
        patch_stack(c, master, PL[i][0],
                     lows[PL[i][1]], lens[PL[i][1]],
                     lows[PL[i][2]], lens[PL[i][2]],
                     PL[i][3], PL[i][4], mode);
    }

    memcpy(out, c->raw, (size_t)len);
    memset(&st, 0, sizeof(st));
}

