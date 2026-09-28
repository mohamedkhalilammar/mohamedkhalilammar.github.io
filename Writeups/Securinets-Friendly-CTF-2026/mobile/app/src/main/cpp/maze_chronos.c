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

static void merge_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t sift_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t chain_view(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void queue_table(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t yield_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t place_state(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t pack_delta(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pack_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void seek_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t reset_path(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int resize_bound(sx_state *c) __attribute__((used, noinline));
static int tap_window(sx_state *c) __attribute__((used, noinline));
static uint32_t fetch_rate(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t link_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t slice_cursor(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pack_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int place_line(sx_state *c) __attribute__((used, noinline));
static void merge_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int defer_head(sx_state *c) __attribute__((used, noinline));
static uint32_t pin_frame(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t swap_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t prime_queue(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t poll_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t flush_tuple(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t poll_span(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void close_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t drain_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_token(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void poll_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t mix_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void split_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void align_segment(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t drain_token(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int scan_key(sx_state *c) __attribute__((used, noinline));
static void join_pairing(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t hold_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t flush_group(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t yield_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_arena(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void map_node(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t yield_value(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void probe_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t close_gap(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tap_table(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t probe_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t map_block(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void defer_item(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t defer_head_49(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t mark_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int fetch_rate_51(sx_state *c) __attribute__((used, noinline));
static void mark_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int close_cursor(sx_state *c) __attribute__((used, noinline));
static int hold_unit(sx_state *c) __attribute__((used, noinline));
static void mark_band(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t hold_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t join_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t peek_pairing(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int store_slot(sx_state *c) __attribute__((used, noinline));
static uint32_t pick_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pair_store(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t trim_pairing(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void seek_cursor(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t defer_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_item(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t step_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t purge_part(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t grow_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tally_layer(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int parse_band(sx_state *c) __attribute__((used, noinline));
static int clamp_group(sx_state *c) __attribute__((used, noinline));
static uint32_t store_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void fetch_cell(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tap_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t flush_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t drain_pairing(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t resize_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int place_window(sx_state *c) __attribute__((used, noinline));
static uint32_t store_offset(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t patch_part(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t queue_ring(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t pick_key(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void fill_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void probe_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t scan_chunk(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t sync_layer(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void shift_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t trim_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t peek_count(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tally_tail(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void split_field(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t purge_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void load_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t move_record(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void map_port(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void yield_level(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void shift_token(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void map_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void clamp_pool(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int swap_bucket(sx_state *c) __attribute__((used, noinline));
static int defer_field(sx_state *c) __attribute__((used, noinline));
static uint32_t latch_head(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void queue_line(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void hold_arena(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t prime_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int defer_bucket(sx_state *c) __attribute__((used, noinline));
static uint8_t wrap_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fold_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t stage_count(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t settle_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t swap_count(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void place_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t reset_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t slice_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void settle_offset(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t grow_block(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void fill_group(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pair_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int parse_segment(sx_state *c) __attribute__((used, noinline));
static uint8_t load_index(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void relay_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void merge_limit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t reset_entry(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t stage_seat(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t join_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void link_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void probe_path(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void rotate_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void map_index(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t reset_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t split_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tally_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t reset_store(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void prime_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t shift_key(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void reap_bucket(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t drain_token_137(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void scan_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t parse_mask(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void latch_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void slice_bucket(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t sync_cursor(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t prime_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t probe_window(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void drain_field(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t drain_entry(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_port(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t grow_count(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t merge_limit_149(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t map_tail(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void blend_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t clamp_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t trim_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t parse_block(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void trace_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void push_item(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int parse_track(sx_state *c) __attribute__((used, noinline));
static uint8_t poll_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t pin_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t emit_offset(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t link_node(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_chunk_162(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_key(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t sync_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void clamp_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void grow_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t reset_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t resize_queue(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t mix_cell(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t coal_lease(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void drain_tuple(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t pick_slot(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t resize_list(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void chain_item(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t cache_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int mark_segment(sx_state *c) __attribute__((used, noinline));
static void link_table_177(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t hold_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int cache_chunk(sx_state *c) __attribute__((used, noinline));
static uint32_t purge_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int clamp_delta(sx_state *c) __attribute__((used, noinline));
static int defer_batch(sx_state *c) __attribute__((used, noinline));
static uint8_t map_track(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void grow_bucket(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t yield_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void load_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t latch_node(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int defer_tail(sx_state *c) __attribute__((used, noinline));
static int mark_count(sx_state *c) __attribute__((used, noinline));
static void shift_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t trim_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t cache_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t flush_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t wrap_cursor(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void seek_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t queue_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t link_key(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t defer_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t step_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void reset_page(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t emit_cursor(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void reset_key(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t mix_chunk(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int sift_token(sx_state *c) __attribute__((used, noinline));
static uint32_t mix_line(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int map_slot(sx_state *c) __attribute__((used, noinline));
static void relay_offset(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void split_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void tally_group(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t merge_page(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void push_stream(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t tally_stream(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int drain_page(sx_state *c) __attribute__((used, noinline));
static int tune_lease(sx_state *c) __attribute__((used, noinline));
static void store_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void coal_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void parse_tuple(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int close_queue(sx_state *c) __attribute__((used, noinline));
static void sync_cursor_220(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fold_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t pin_cursor(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fill_level(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t push_group(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void scan_token(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void defer_row(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t slice_window(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t fold_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int clamp_field(sx_state *c) __attribute__((used, noinline));
static uint8_t fetch_region(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t parse_record(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t sync_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pack_delta_234(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tally_run(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t mix_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void reset_index(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void fetch_span(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t stage_count_240(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void close_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t move_seat(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t purge_field(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void coal_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t hold_cursor(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t push_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int join_bound(sx_state *c) __attribute__((used, noinline));
static void resize_view(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t rotate_offset(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pin_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_band(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void probe_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t stage_slot(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void push_ring(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void pack_pairing(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void queue_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void flush_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void prime_stream(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t map_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void sift_value(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tune_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void blend_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void reset_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t blend_region_264(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int probe_entry(sx_state *c) __attribute__((used, noinline));
static uint8_t trace_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t cache_head(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void wrap_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void fill_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void patch_cell(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t latch_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void place_state_272(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int map_scope(sx_state *c) __attribute__((used, noinline));
static uint8_t fold_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t pick_bucket(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void rotate_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t reset_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void blend_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pack_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t place_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void push_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t sift_cursor(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t close_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t settle_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t rotate_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t reap_count(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void slice_band(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void stage_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int scan_node(sx_state *c) __attribute__((used, noinline));
static uint32_t fold_cursor(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t queue_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t rotate_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int flush_slot(sx_state *c) __attribute__((used, noinline));
static int link_layer(sx_state *c) __attribute__((used, noinline));
static int latch_level(sx_state *c) __attribute__((used, noinline));
static void queue_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t push_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t push_cursor(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t move_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pick_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t merge_head(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void stage_limit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t chain_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t emit_part(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pair_bucket(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int merge_tail(sx_state *c) __attribute__((used, noinline));
static uint8_t peek_region(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void close_marker(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t mix_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_head(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_offset(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int sync_view(sx_state *c) __attribute__((used, noinline));
static uint32_t clamp_head(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t drain_tuple_316(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tap_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t slice_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_index(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void sync_key(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int settle_scope(sx_state *c) __attribute__((used, noinline));
static void tap_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void hold_limit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void step_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int fetch_tail(sx_state *c) __attribute__((used, noinline));
static uint32_t trace_marker(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int purge_arena(sx_state *c) __attribute__((used, noinline));
static void stage_line(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int hold_tail(sx_state *c) __attribute__((used, noinline));
static void place_gap(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t mark_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t wrap_view(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t pack_count(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t move_index(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t scan_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int reset_count(sx_state *c) __attribute__((used, noinline));
static uint32_t load_pool(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int resize_cell(sx_state *c) __attribute__((used, noinline));
static uint32_t yield_list(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int mix_port(sx_state *c) __attribute__((used, noinline));
static void queue_lease(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t peek_pairing_342(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t poll_window(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void scan_bucket(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tune_page(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void shift_page(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void pick_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int place_queue(sx_state *c) __attribute__((used, noinline));
static uint32_t mix_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void cache_part(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t load_page(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void map_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void queue_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void relay_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int step_field(sx_state *c) __attribute__((used, noinline));
static int emit_record(sx_state *c) __attribute__((used, noinline));
static void blend_level(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int map_rate(sx_state *c) __attribute__((used, noinline));
static uint32_t pack_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t map_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int purge_part_361(sx_state *c) __attribute__((used, noinline));
static void fetch_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t probe_part(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int split_block_364(sx_state *c) __attribute__((used, noinline));
static uint8_t coal_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void relay_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void relay_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int map_unit(sx_state *c) __attribute__((used, noinline));
static void step_stream(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t sort_queue(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int settle_item(sx_state *c) __attribute__((used, noinline));
static uint8_t stage_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void swap_track(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t stage_count_374(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sort_band(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void fetch_value(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void latch_marker(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t stage_ring(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t rotate_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t shift_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t sift_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int stage_rate(sx_state *c) __attribute__((used, noinline));
static void fold_group(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void slice_index(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t hold_level(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t stage_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void join_list(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void emit_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t chain_unit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tap_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void parse_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t link_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int cache_page(sx_state *c) __attribute__((used, noinline));
static void shift_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t mix_segment(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t probe_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void defer_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int shift_table(sx_state *c) __attribute__((used, noinline));
static int coal_range(sx_state *c) __attribute__((used, noinline));
static void stage_frame(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int store_segment(sx_state *c) __attribute__((used, noinline));
static uint32_t drain_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t mark_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void grow_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t wrap_path(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t mix_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void trim_group(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_marker(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void chain_node(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int purge_port(sx_state *c) __attribute__((used, noinline));
static int fill_run(sx_state *c) __attribute__((used, noinline));
static int peek_track(sx_state *c) __attribute__((used, noinline));
static uint8_t tap_window_413(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void mix_mask(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void sort_stream(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t poll_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t move_pool(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int tune_chunk(sx_state *c) __attribute__((used, noinline));
static void place_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t slice_path(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t split_key(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void step_group(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t link_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void coal_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t stage_span(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t trim_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t link_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t trace_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_token(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t hold_label(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void probe_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t queue_ring_432(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int prime_bound(sx_state *c) __attribute__((used, noinline));
static uint32_t sort_state(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void push_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t reset_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void swap_list(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void blend_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int cache_block(sx_state *c) __attribute__((used, noinline));
static void tap_index_440(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void wrap_page(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t seek_row_442(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t defer_count(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_store(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void parse_band_445(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t prime_cursor(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_offset(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void place_mask(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t parse_pool(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void shift_slot(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t split_port(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void push_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t purge_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t swap_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void wrap_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t push_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void store_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t settle_marker(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int tap_unit(sx_state *c) __attribute__((used, noinline));
static void swap_region(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t map_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t scan_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t settle_window(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int reap_mask(sx_state *c) __attribute__((used, noinline));
static uint8_t stage_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t relay_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int align_frame(sx_state *c) __attribute__((used, noinline));
static uint32_t trace_batch_468(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t merge_tail_469(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_span_470(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t store_band(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int tally_count(sx_state *c) __attribute__((used, noinline));
static uint32_t mix_seat(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int shift_region(sx_state *c) __attribute__((used, noinline));
static uint32_t defer_batch_475(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t resize_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void split_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t parse_cell(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t blend_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t coal_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void shift_bucket(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t clamp_marker(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t scan_digest_483(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t pack_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void step_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void stage_range(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void merge_page_487(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int reap_stream(sx_state *c) __attribute__((used, noinline));
static void flush_span(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void flush_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void store_marker(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void sift_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int probe_unit(sx_state *c) __attribute__((used, noinline));
static uint32_t peek_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void reap_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void mark_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void tally_rate(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int step_unit(sx_state *c) __attribute__((used, noinline));
static uint8_t queue_entry(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t parse_node(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t step_gap(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t merge_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t queue_region(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t shift_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void poll_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t map_token(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t sync_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t purge_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int fill_span(sx_state *c) __attribute__((used, noinline));
static void place_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t shift_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_pool(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void flush_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t peek_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void prime_stack(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t emit_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void push_row(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t mix_chunk_518(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t patch_unit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void wrap_scope(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int swap_stream(sx_state *c) __attribute__((used, noinline));
static void patch_token(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t coal_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pair_unit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void load_bound(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void tally_arena(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tune_offset(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t prime_bucket(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void push_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int settle_scope_530(sx_state *c) __attribute__((used, noinline));
static uint32_t wrap_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t clamp_entry(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sync_port(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int split_group(sx_state *c) __attribute__((used, noinline));
static void clamp_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t blend_row(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void load_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void patch_stack(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void cache_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void defer_row_540(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t hold_unit_541(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t tune_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int tally_line(sx_state *c) __attribute__((used, noinline));
static void trim_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t map_unit_545(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tune_count(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void hold_bound(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void peek_key(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pick_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t defer_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t probe_stream(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t chain_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int move_span(sx_state *c) __attribute__((used, noinline));
static uint8_t sift_frame(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t split_line(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t move_tuple(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t coal_ring(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void cache_count(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void settle_marker_559(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void close_band(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int move_batch(sx_state *c) __attribute__((used, noinline));
static uint8_t stage_run(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void grow_tuple(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void latch_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t hold_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void map_key(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t seek_field(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void place_list(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int swap_state(sx_state *c) __attribute__((used, noinline));
static int latch_chunk(sx_state *c) __attribute__((used, noinline));
static void store_tail(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void emit_frame(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int reset_path_575(sx_state *c) __attribute__((used, noinline));
static int push_marker(sx_state *c) __attribute__((used, noinline));
static uint32_t tally_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void latch_cell(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t trim_index(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void poll_stream(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int sort_entry(sx_state *c) __attribute__((used, noinline));
static uint8_t shift_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_range(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t peek_delta(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t load_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void join_segment(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void scan_bucket_587(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t poll_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void parse_tail(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t blend_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int defer_segment(sx_state *c) __attribute__((used, noinline));
static int rotate_store_592(sx_state *c) __attribute__((used, noinline));
static void hold_index(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void yield_tuple(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int latch_region(sx_state *c) __attribute__((used, noinline));
static void prime_bucket_596(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int pin_count(sx_state *c) __attribute__((used, noinline));
static uint32_t yield_state_598(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int grow_bound(sx_state *c) __attribute__((used, noinline));
static uint32_t cache_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void trim_unit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void trace_tuple(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t push_queue(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int settle_value(sx_state *c) __attribute__((used, noinline));
static uint32_t grow_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int slice_value(sx_state *c) __attribute__((used, noinline));
static void cache_field(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t tally_page(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t slice_bucket_609(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t queue_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t slice_queue(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t scan_path(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void resize_key(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t defer_table_614(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t reap_lease(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int store_ring(sx_state *c) __attribute__((used, noinline));
static uint32_t defer_offset(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t load_token(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t grow_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int reset_state_620(sx_state *c) __attribute__((used, noinline));
static void split_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void emit_cell(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t hold_state_623(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void step_table(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t cache_part_625(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pair_item(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t wrap_frame(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t drain_bound(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void emit_field(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void align_tail(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t map_range(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t chain_pool(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t scan_layer(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void emit_token(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t sort_page(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void sort_region(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t place_pairing_637(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int chain_block(sx_state *c) __attribute__((used, noinline));
static void store_index(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void reap_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t join_stack(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t yield_stack(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tune_field_643(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void scan_tuple(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void reset_band(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int tune_list(sx_state *c) __attribute__((used, noinline));
static void tune_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t yield_region(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void merge_token_649(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t blend_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int pin_part(sx_state *c) __attribute__((used, noinline));
static uint8_t cache_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void step_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void pin_band(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t move_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int tune_state_656(sx_state *c) __attribute__((used, noinline));
static void poll_line_657(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int shift_state(sx_state *c) __attribute__((used, noinline));
static uint32_t reset_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t rotate_pairing(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pick_digest_661(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void tune_cursor(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t drain_port(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t yield_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void step_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t map_chunk(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int grow_store_667(sx_state *c) __attribute__((used, noinline));
static void scan_item(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void pair_field(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tally_layer_670(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t hold_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int hold_queue(sx_state *c) __attribute__((used, noinline));
static int settle_ring(sx_state *c) __attribute__((used, noinline));
static uint32_t chain_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void prime_bound_675(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t chain_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fill_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t grow_tail(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int seek_slot(sx_state *c) __attribute__((used, noinline));
static int cache_queue(sx_state *c) __attribute__((used, noinline));
static void mix_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t wrap_path_683(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t defer_index_684(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int shift_page_685(sx_state *c) __attribute__((used, noinline));
static void tap_unit_686(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int store_layer(sx_state *c) __attribute__((used, noinline));
static void push_stack(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int parse_block_689(sx_state *c) __attribute__((used, noinline));
static uint32_t tap_port_690(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_batch_691(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t store_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void emit_unit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t scan_item_694(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tap_unit_695(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t pin_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t prime_count(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t yield_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_limit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int tally_level(sx_state *c) __attribute__((used, noinline));
static int pin_label(sx_state *c) __attribute__((used, noinline));
static uint32_t latch_range(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t mix_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t clamp_group_704(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void store_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int join_pairing_706(sx_state *c) __attribute__((used, noinline));
static int purge_index(sx_state *c) __attribute__((used, noinline));
static uint8_t place_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t grow_bucket_709(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t emit_path(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void fetch_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t merge_state(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t mark_lease(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void grow_row(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void relay_line(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int emit_bound(sx_state *c) __attribute__((used, noinline));
static void mark_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int store_label(sx_state *c) __attribute__((used, noinline));
static void drain_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int scan_part(sx_state *c) __attribute__((used, noinline));
static void mark_range(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t align_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t sort_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t close_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t load_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t fill_list(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tune_token(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int poll_region(sx_state *c) __attribute__((used, noinline));
static uint32_t chain_region(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t drain_state(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int pack_field(sx_state *c) __attribute__((used, noinline));
static uint32_t chain_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tally_region_734(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t cache_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void sort_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t mark_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t step_gap_738(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void stage_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t blend_rate(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t defer_port(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int patch_chunk(sx_state *c) __attribute__((used, noinline));
static uint32_t move_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void swap_key(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void wrap_view_746(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t slice_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pair_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t move_store_749(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void prime_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void latch_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void tune_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t align_seat(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t close_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void resize_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t resize_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t shift_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void resize_queue_758(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t load_range(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void trace_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int fold_line(sx_state *c) __attribute__((used, noinline));
static uint32_t join_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void resize_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void swap_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t align_record(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void reap_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t fill_page(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));

static uint32_t prime_level(uint32_t x) __attribute__((used, noinline));
static int patch_row(sx_state *c) __attribute__((used, noinline));
static uint32_t close_count(uint32_t prime, uint32_t h, uint8_t x) __attribute__((used, noinline));
static uint32_t yield_state_771(sx_state *c, uint32_t nonce, int size) __attribute__((used, noinline));
static void fill_batch(sx_state *c, uint32_t master) __attribute__((used, noinline));
static void settle_record(sx_state *c, uint32_t seed) __attribute__((used, noinline));
static void settle_block(sx_state *c) __attribute__((used, noinline));
static void mix_chunk_775(sx_state *c) __attribute__((used, noinline));
static void split_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void grow_span(sx_state *c, uint32_t *out) __attribute__((used, noinline));
static void flush_table(sx_state *c, uint32_t master, int rnd, int lo, int ln, uint32_t *out) __attribute__((used, noinline));
static void prime_region(sx_state *c, const uint32_t *kk, uint32_t *out) __attribute__((used, noinline));
static void align_label(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void hold_pool(sx_state *c, const uint32_t *w, uint32_t *v) __attribute__((used, noinline));
static void pin_unit(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void sort_row(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void trim_entry(uint32_t *s, int a, int b, int d, int e) __attribute__((used, noinline));
static void tally_scope(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void stage_key(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void mix_tuple(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void shift_batch_788(sx_state *c, int idx, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void resize_record(sx_state *c, uint32_t master, int rnd, int alo, int aln, int tlo, int tln, int s0, int s1, int fwd) __attribute__((used, noinline));


static void merge_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= purge_part(c, t1);
    c->sched[23] = c->hash ^ sx_rl(c->lane[15], 23);
    align_segment(c, t0, t1);
    t1 ^= (uint32_t)mark_span(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)mix_limit(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= c->lane[15] + 0x9904c2aau;
    t2 += queue_ring(c, c->slo, c->sln);
    t2 += sift_label(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xefbe52dbu;
    c->lane[14] += c->lane[3]; c->lane[7] ^= c->lane[14]; c->lane[7] = sx_rl(c->lane[7], 29);
    t2 += (uint32_t)scan_key(c);
    t2 += stage_seat(c, c->slo, c->sln);
    t1 ^= (uint32_t)chain_view(c, (uint8_t)(t0 >> 16), t2);
    c->lane[15] += c->lane[3]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 22);
    c->hash = (c->hash * 0x07d3a9bfu) ^ sx_rr(c->hash, 24);
    t2 += (uint32_t)store_slot(c);
    queue_table(c, &c->lane[0], 2);
    shift_chunk(c, &c->lane[0], 2);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t sift_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += patch_span(c, c->slo, c->sln);
    t0 ^= grow_store(c, t1);
    t0 ^= reset_path(c, t1);
    mark_track(c, &c->lane[11], 4);
    close_block(c, &c->lane[11], 4);
    t1 ^= (uint32_t)link_table(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 31);
    c->sched[24] = c->hash ^ sx_rl(c->lane[4], 15);
    t2 += stage_count(c, c->slo, c->sln);
    t1 ^= (uint32_t)pack_delta(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x46b3ad4du;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t chain_view(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)resize_bound(c);
    t2 += (uint32_t)defer_field(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x76) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[15] + 0x01155427u;
    poll_digest(c, &c->lane[8], 3);
    c->raw[c->slo + (int)((t0 + 19930u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    seek_row(c, t0, t1);
    t1 ^= (uint32_t)flush_tuple(c, (uint8_t)(t0 >> 8), t2);
    c->hash = (c->hash * 0xa5a1e7a1u) ^ sx_rr(c->hash, 2);
    t2 += (uint32_t)tap_window(c);
    c->lane[6] += c->lane[14] ^ 0x3fdb8b56u;
    t2 += poll_state(c, c->rlo, c->rln);
    t0 ^= place_state(c, t1);
    map_store(c, &c->lane[8], 1);
    t2 += yield_state(c, c->slo, c->sln);
    t1 ^= (uint32_t)slice_cursor(c, (uint8_t)(t0 >> 8), t2);
    c->lane[0] += c->lane[10]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 15);
    t0 ^= resize_tail(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15779u) % (uint32_t)c->rln)] << 8;
    c->lane[4] += c->lane[9] ^ 0xcc89a1b8u;
    link_state(c, &c->lane[1], 1);
    t0 ^= store_digest(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xaf) << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void queue_table(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x890f92fdu;
    t1 ^= (uint32_t)settle_limit(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb426da65u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0e9e6001u;
    t2 += fetch_rate(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0x737b1589u;
    t2 = (t2 ^ c->sum) * 0x837a3e89u;
    t2 += latch_head(c, c->rlo, c->rln);
    c->hash ^= c->lane[8] + 0xfe7cb749u;
    pack_block(c, &c->lane[11], 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x54) << 0;
    t0 ^= defer_lease(c, t1);
    t2 = (t2 ^ c->sum) * 0x5bc2a1e9u;
    t2 = (t2 ^ c->sum) * 0x2928d711u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)defer_head(c);
    c->sched[28] = c->hash ^ sx_rl(c->lane[9], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10520u) % (uint32_t)c->rln)] << 0;
    seek_cursor(c, t0, t1);
    c->hash = (c->hash * 0xb6ba6b51u) ^ sx_rr(c->hash, 8);
    t1 ^= (uint32_t)swap_digest(c, (uint8_t)(t0 >> 16), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43441u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10535u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t1 ^= (uint32_t)slice_cursor(c, (uint8_t)(t0 >> 8), t2);
    c->sched[3] = c->hash ^ sx_rl(c->lane[12], 5);
    c->lane[1] += c->lane[7] ^ 0x92ea9b4eu;
    t2 += pin_frame(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static uint32_t yield_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)join_scope(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[12] + 0x09a0946fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63031u) % (uint32_t)c->rln)] << 16;
    c->sched[6] = c->hash ^ sx_rl(c->lane[13], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x88d87337u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6e7bddc5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[3] + 0x3353f4d2u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf6) << 8;
    pack_chunk(c, t0, t1);
    c->hash ^= c->lane[4] + 0x5b72242au;
    probe_range(c, t0, t1);
    c->hash = (c->hash * 0x429b739bu) ^ sx_rr(c->hash, 4);
    c->hash ^= c->lane[11] + 0x86521d80u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t place_state(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] ^= sx_rl(c->lane[9], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xad295869u;
    c->sched[21] = c->hash ^ sx_rl(c->lane[12], 25);
    t2 += poll_state(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    split_run(c, t0, t1);
    merge_record(c, t0, t1);
    c->sched[2] = c->hash ^ sx_rl(c->lane[7], 14);
    t2 += hold_key(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38115u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t pack_delta(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += fetch_rate(c, c->rlo, c->rln);
    t1 ^= (uint32_t)link_table(c, (uint8_t)(t0 >> 16), t2);
    t2 += seek_token(c, c->slo, c->sln);
    c->lane[7] ^= sx_rl(c->lane[3], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[5] ^= sx_rl(c->lane[15], 29);
    c->hash ^= c->lane[7] + 0x696f1383u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t grow_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[14] += c->lane[9] ^ 0x8ae881a9u;
    c->hash ^= c->lane[4] + 0x947012c9u;
    c->lane[1] ^= sx_rl(c->lane[13], 11);
    c->raw[c->slo + (int)((t0 + 16186u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 ^= drain_rate(c, t1);
    c->lane[9] += c->lane[1]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 18);
    c->sched[6] = c->hash ^ sx_rl(c->lane[5], 18);
    c->sched[18] = c->hash ^ sx_rl(c->lane[1], 18);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42003u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x22a0b0afu;
    c->hash = (c->hash * 0x093eacd7u) ^ sx_rr(c->hash, 14);
    c->lane[11] ^= sx_rl(c->lane[3], 14);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pack_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t1 ^= (uint32_t)flush_tuple(c, (uint8_t)(t0 >> 0), t2);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash = (c->hash * 0xed4e1f6fu) ^ sx_rr(c->hash, 6);
    t2 = (t2 ^ c->sum) * 0x8a1eae17u;
    c->hash = (c->hash * 0xd6aa5791u) ^ sx_rr(c->hash, 7);
    c->lane[10] += c->lane[9] ^ 0xe9d19b09u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 183u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x3fbae38bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void seek_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)prime_queue(c, (uint8_t)(t0 >> 16), t2);
    c->lane[1] += c->lane[8] ^ 0xd6b643b2u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[17] = c->hash ^ sx_rl(c->lane[0], 7);
    c->hash = (c->hash * 0xeb9aa28bu) ^ sx_rr(c->hash, 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63146u) % (uint32_t)c->rln)] << 16;
    split_run(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 8512u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[14] ^= sx_rl(c->lane[13], 1);
    close_block(c, &c->lane[2], 1);
    poll_digest(c, &c->lane[10], 1);
    t1 ^= (uint32_t)peek_pairing(c, (uint8_t)(t0 >> 8), t2);
    c->sched[25] = c->hash ^ sx_rl(c->lane[6], 31);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t reset_path(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x858a6cebu;
    t2 += (uint32_t)place_line(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x90) << 0;
    c->hash = (c->hash * 0xfeae70fdu) ^ sx_rr(c->hash, 4);
    c->hash ^= c->lane[15] + 0xa430817eu;
    t1 ^= (uint32_t)mix_limit(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= drain_rate(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4c2a57f1u;
    c->lane[9] ^= sx_rl(c->lane[4], 24);
    c->hash = (c->hash * 0x6f8f248fu) ^ sx_rr(c->hash, 30);
    c->lane[12] ^= sx_rl(c->lane[13], 15);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int resize_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->raw[c->slo + (int)((t0 + 3951u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 9);
    c->sched[9] = c->hash ^ sx_rl(c->lane[11], 13);
    c->hash ^= c->lane[10] + 0xb274b36bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] ^= sx_rl(c->lane[4], 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 19);
    c->hash ^= c->lane[8] + 0xcd8d687au;
    t0 ^= poll_span(c, t1);
    c->lane[4] ^= sx_rl(c->lane[4], 27);
    c->lane[0] ^= sx_rl(c->lane[7], 7);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int tap_window(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[7] ^= sx_rl(c->lane[15], 13);
    c->sched[17] = c->hash ^ sx_rl(c->lane[0], 29);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    align_segment(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2e) << 0;
    t2 += pin_frame(c, c->slo, c->sln);
    c->lane[8] ^= sx_rl(c->lane[10], 6);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t fetch_rate(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[1] += c->lane[3] ^ 0x631456a4u;
    t2 += yield_port(c, c->slo, c->sln);
    t1 ^= (uint32_t)slice_item(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)parse_segment(c);
    t2 = (t2 ^ c->sum) * 0x668ec6b1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc179bc1du;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t link_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb0ae2361u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += map_block(c, c->rlo, c->rln);
    c->lane[0] ^= sx_rl(c->lane[11], 13);
    c->sched[15] = c->hash ^ sx_rl(c->lane[11], 19);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6281a8f3u;
    c->lane[8] ^= sx_rl(c->lane[14], 11);
    c->sched[9] = c->hash ^ sx_rl(c->lane[15], 14);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf2) << 16;
    c->hash ^= c->lane[6] + 0x8fd6de7bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    seek_cursor(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t slice_cursor(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5e7c7a9bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 += step_value(c, c->slo, c->sln);
    c->lane[6] ^= sx_rl(c->lane[2], 25);
    c->hash = (c->hash * 0xd152edb1u) ^ sx_rr(c->hash, 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[5] += c->lane[5] ^ 0x243a0f2fu;
    t2 += (uint32_t)scan_key(c);
    c->lane[10] ^= sx_rl(c->lane[10], 1);
    c->hash = (c->hash * 0x3c2692c7u) ^ sx_rr(c->hash, 8);
    mark_band(c, &c->lane[6], 2);
    settle_offset(c, &c->lane[9], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void pack_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4ad55279u;
    c->sched[3] = c->hash ^ sx_rl(c->lane[8], 19);
    t2 += hold_key(c, c->rlo, c->rln);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xaf) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x1d09c0fbu;
    c->lane[4] += c->lane[7] ^ 0x96e7242du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t1 ^= (uint32_t)hold_state(c, (uint8_t)(t0 >> 0), t2);
    c->sched[29] = c->hash ^ sx_rl(c->lane[14], 4);
    c->sched[27] = c->hash ^ sx_rl(c->lane[12], 8);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int place_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    t2 += (uint32_t)close_cursor(c);
    merge_limit(c, t0, t1);
    c->lane[12] ^= sx_rl(c->lane[12], 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0e) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x7f29a583u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x82937c83u;
    t0 ^= defer_lease(c, t1);
    c->sched[10] = c->hash ^ sx_rl(c->lane[6], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6a) << 8;
    c->lane[12] += c->lane[0]; c->lane[5] ^= c->lane[12]; c->lane[5] = sx_rl(c->lane[5], 22);
    t2 = (t2 ^ c->sum) * 0xdd32cf87u;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void merge_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x0c985d41u) ^ sx_rr(c->hash, 16);
    c->sched[21] = c->hash ^ sx_rl(c->lane[9], 9);
    defer_item(c, &c->lane[3], 4);
    t2 = (t2 ^ c->sum) * 0xbcf6fcfbu;
    c->raw[c->slo + (int)((t0 + 24604u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[4] = c->hash ^ sx_rl(c->lane[10], 8);
    t2 += purge_table(c, c->rlo, c->rln);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 1);
    c->lane[2] ^= sx_rl(c->lane[14], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14758u) % (uint32_t)c->rln)] << 24;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static int defer_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 9);
    c->hash ^= c->lane[2] + 0xbb038d17u;
    c->lane[13] += c->lane[10]; c->lane[12] ^= c->lane[13]; c->lane[12] = sx_rl(c->lane[12], 18);
    c->hash = (c->hash * 0x005590cbu) ^ sx_rr(c->hash, 9);
    map_node(c, t0, t1);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 17);
    c->lane[11] += c->lane[4] ^ 0x7c5be926u;
    t0 ^= close_gap(c, t1);
    c->raw[c->slo + (int)((t0 + 52045u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= purge_part(c, t1);
    c->sched[16] = c->hash ^ sx_rl(c->lane[15], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 35140u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t pin_frame(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[7] += c->lane[14] ^ 0xea372c3eu;
    t1 ^= (uint32_t)trim_pairing(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54167u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb5caddbfu;
    c->sched[14] = c->hash ^ sx_rl(c->lane[13], 17);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t swap_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xce) << 8;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 29);
    t2 += defer_head_49(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 29324u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x8ebef7d5u) ^ sx_rr(c->hash, 28);
    c->lane[10] ^= sx_rl(c->lane[15], 5);
    c->raw[c->slo + (int)((t0 + 27884u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t prime_queue(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t1 ^= (uint32_t)grow_limit(c, (uint8_t)(t0 >> 16), t2);
    shift_token(c, &c->lane[2], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[15] += c->lane[13]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 5);
    c->raw[c->slo + (int)((t0 + 32583u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x19e97b81u;
    c->lane[13] += c->lane[11]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 2);
    t1 ^= (uint32_t)hold_state(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 ^= fold_slot(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t poll_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x2d0e096bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24507u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x64821cd3u) ^ sx_rr(c->hash, 7);
    tap_arena(c, &c->lane[2], 1);
    pair_store(c, t0, t1);
    c->lane[13] += c->lane[11]; c->lane[14] ^= c->lane[13]; c->lane[14] = sx_rl(c->lane[14], 29);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t flush_tuple(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 64188u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[5] ^= sx_rl(c->lane[0], 9);
    t2 += (uint32_t)store_slot(c);
    c->lane[11] += c->lane[10] ^ 0x1879f9f2u;
    mark_track(c, &c->lane[3], 1);
    c->hash = (c->hash * 0xd0f174a7u) ^ sx_rr(c->hash, 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t poll_span(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += reap_item(c, c->slo, c->sln);
    c->sched[20] = c->hash ^ sx_rl(c->lane[15], 6);
    t2 += (uint32_t)fetch_rate_51(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[9] += c->lane[14] ^ 0x2b1bec61u;
    t1 ^= (uint32_t)yield_value(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x2c) << 16;
    c->lane[3] += c->lane[9] ^ 0x2addf9a4u;
    t0 ^= scan_chunk(c, t1);
    c->sched[14] = c->hash ^ sx_rl(c->lane[12], 30);
    c->hash ^= c->lane[3] + 0x7ce78026u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void close_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43646u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sched[20] = c->hash ^ sx_rl(c->lane[12], 31);
    c->hash = (c->hash * 0x7b10bb4du) ^ sx_rr(c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    fetch_cell(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xb9a0ce4bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x83043637u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t drain_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[2] += c->lane[11]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 11);
    c->lane[14] += c->lane[14] ^ 0x57d92223u;
    t0 ^= probe_node(c, t1);
    t1 ^= (uint32_t)peek_pairing(c, (uint8_t)(t0 >> 0), t2);
    probe_mask(c, t0, t1);
    t1 ^= (uint32_t)join_scope(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53504u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] += c->lane[5] ^ 0xa2f4a293u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x994c1a31u;
    c->hash ^= c->lane[1] + 0xabe66922u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t seek_token(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x03c9a2cfu;
    t2 = (t2 ^ c->sum) * 0x32565f51u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x36e974a9u;
    t0 ^= tap_table(c, t1);
    c->sched[16] = c->hash ^ sx_rl(c->lane[14], 26);
    c->sum += t1;
    return t0 + t2;
}

static void poll_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3d53e723u;
    c->lane[13] ^= sx_rl(c->lane[9], 15);
    t2 += (uint32_t)hold_unit(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6d6e78f9u;
    t1 ^= (uint32_t)mark_span(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x92) << 16;
    join_pairing(c, t0, t1);
    t1 ^= (uint32_t)flush_group(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= drain_token(c, t1);
    c->hash ^= c->lane[5] + 0xce53dc0cu;
    c->raw[c->slo + (int)((t0 + 2320u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[7] + 0x88c6c92du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53790u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t mix_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48291u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x155bbbe3u;
    c->lane[14] += c->lane[14] ^ 0xc902163au;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62407u) % (uint32_t)c->rln)] << 0;
    t2 += (uint32_t)close_cursor(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void split_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] += c->lane[5] ^ 0x57418d20u;
    c->lane[7] += c->lane[14] ^ 0xe095b7c3u;
    mark_band(c, &c->lane[7], 4);
    c->lane[4] ^= sx_rl(c->lane[14], 15);
    c->lane[9] += c->lane[13] ^ 0x9a8caf48u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void align_segment(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43668u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 34289u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[8] ^= sx_rl(c->lane[14], 12);
    t2 += (uint32_t)clamp_group(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xab) << 16;
    c->lane[5] ^= sx_rl(c->lane[0], 30);
    t0 ^= pick_group(c, t1);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 17);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t drain_token(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= store_digest(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x15) << 0;
    c->hash = (c->hash * 0x8138771bu) ^ sx_rr(c->hash, 8);
    c->sched[23] = c->hash ^ sx_rl(c->lane[13], 13);
    c->sched[7] = c->hash ^ sx_rl(c->lane[10], 7);
    map_port(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 64042u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[4] = c->hash ^ sx_rl(c->lane[9], 1);
    t0 ^= fold_slot(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x67a568cfu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int scan_key(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7d) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe9) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x48) << 8;
    probe_range(c, t0, t1);
    queue_line(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64229u) % (uint32_t)c->rln)] << 0;
    c->lane[9] += c->lane[13]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x89) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void join_pairing(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] += c->lane[13] ^ 0xb43b10c4u;
    t1 ^= (uint32_t)pick_key(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x777e2987u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 1);
    c->raw[c->slo + (int)((t0 + 43751u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2f0b1805u;
    t2 = (t2 ^ c->sum) * 0xfc82dad5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[14] += c->lane[3]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 5);
    t0 ^= store_offset(c, t1);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t hold_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[15] += c->lane[2]; c->lane[10] ^= c->lane[15]; c->lane[10] = sx_rl(c->lane[10], 7);
    c->raw[c->slo + (int)((t0 + 44839u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[9] ^= sx_rl(c->lane[12], 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t1 ^= (uint32_t)reset_scope(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[2] + 0x8e18a1a3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t flush_group(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 15);
    c->sched[2] = c->hash ^ sx_rl(c->lane[12], 7);
    c->lane[11] ^= sx_rl(c->lane[14], 24);
    t1 ^= (uint32_t)load_index(c, (uint8_t)(t0 >> 8), t2);
    t2 += purge_table(c, c->slo, c->sln);
    c->lane[0] += c->lane[15]; c->lane[14] ^= c->lane[0]; c->lane[14] = sx_rl(c->lane[14], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xff) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0024dcd5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50781u) % (uint32_t)c->rln)] << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t yield_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe5) << 16;
    t2 = (t2 ^ c->sum) * 0xf045d521u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe116545bu;
    c->lane[9] ^= sx_rl(c->lane[1], 12);
    c->lane[7] += c->lane[14]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 21);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 20);
    c->lane[2] += c->lane[13] ^ 0xafdb6e14u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x49e39e79u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x401c7bffu;
    c->sum += t1;
    return t0 + t2;
}

static void tap_arena(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += prime_cell(c, c->rlo, c->rln);
    t2 += reset_entry(c, c->rlo, c->rln);
    c->lane[8] += c->lane[4] ^ 0xd8ac9a32u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61013u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25967u) % (uint32_t)c->rln)] << 0;
    t2 += (uint32_t)defer_field(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void map_node(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)place_window(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4f7c0999u;
    t2 = (t2 ^ c->sum) * 0xb97033c7u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] += c->lane[11] ^ 0x21202b53u;
    c->hash ^= c->lane[2] + 0x894f7333u;
    c->lane[14] += c->lane[1] ^ 0x8da87ec0u;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t yield_value(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[13], 4);
    tally_layer(c, &c->lane[0], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[5] ^= sx_rl(c->lane[3], 1);
    c->raw[c->slo + (int)((t0 + 28372u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    c->raw[c->slo + (int)((t0 + 38325u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 33509u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void probe_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)wrap_line(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[15] + 0x4fdfcb47u;
    link_state(c, &c->lane[2], 2);
    c->hash = (c->hash * 0x22f6541bu) ^ sx_rr(c->hash, 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sched[28] = c->hash ^ sx_rl(c->lane[10], 17);
    c->lane[0] += c->lane[4] ^ 0xf904eb8eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[19] = c->hash ^ sx_rl(c->lane[8], 9);
    c->hash = (c->hash * 0x37d9b2b5u) ^ sx_rr(c->hash, 6);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t close_gap(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd20ab185u;
    map_store(c, &c->lane[6], 3);
    t2 = (t2 ^ c->sum) * 0x64583d5bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x52) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x16) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18248u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9a) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x70ca4685u;
    t2 += (uint32_t)swap_bucket(c);
    t2 += (uint32_t)parse_segment(c);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tap_table(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= move_record(c, t1);
    hold_arena(c, &c->lane[2], 3);
    c->lane[8] += c->lane[1]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 8);
    c->lane[5] += c->lane[14] ^ 0x5ed6e269u;
    c->lane[1] ^= sx_rl(c->lane[1], 28);
    t2 += tap_cell(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xf8381b89u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x7c7d8ac1u) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x24) << 0;
    c->raw[c->slo + (int)((t0 + 65166u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t probe_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x33) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5f92ab0fu;
    fill_path(c, &c->lane[0], 3);
    c->lane[1] ^= sx_rl(c->lane[15], 20);
    c->lane[3] ^= sx_rl(c->lane[1], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0f) << 16;
    t2 += join_arena(c, c->slo, c->sln);
    t0 ^= scan_chunk(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t map_block(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[4] + 0x3b6bfa04u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[6], 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x33) << 16;
    c->lane[13] ^= sx_rl(c->lane[14], 10);
    c->raw[c->slo + (int)((t0 + 42831u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x56a5f195u) ^ sx_rr(c->hash, 25);
    t0 ^= swap_count(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x708c6a75u;
    c->lane[2] += c->lane[1] ^ 0x7272d800u;
    c->raw[c->slo + (int)((t0 + 11611u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x88b0a11fu) ^ sx_rr(c->hash, 7);
    c->lane[10] += c->lane[12]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 21);
    c->sum += t1;
    return t0 + t2;
}

static void defer_item(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= resize_tail(c, t1);
    t0 ^= patch_part(c, t1);
    c->lane[8] += c->lane[0] ^ 0xb04993e7u;
    c->hash = (c->hash * 0xf1bbf175u) ^ sx_rr(c->hash, 28);
    t2 += (uint32_t)parse_band(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[9] ^= sx_rl(c->lane[10], 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t defer_head_49(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 30);
    c->hash = (c->hash * 0x19ba0927u) ^ sx_rr(c->hash, 26);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 14);
    c->lane[5] += c->lane[6] ^ 0x1947abc8u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xad) << 0;
    c->hash = (c->hash * 0x0b9afc45u) ^ sx_rr(c->hash, 29);
    relay_limit(c, &c->lane[5], 3);
    c->hash ^= c->lane[14] + 0x3823d708u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= c->lane[12] + 0xe71c1c6bu;
    t2 = (t2 ^ c->sum) * 0x3e8ba959u;
    shift_chunk(c, &c->lane[5], 3);
    t2 += flush_delta(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum += t1;
    return t0 + t2;
}

static uint8_t mark_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x4cf42497u;
    c->lane[15] += c->lane[13] ^ 0x612d2802u;
    t1 ^= (uint32_t)sync_layer(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x2fd7d6c3u;
    c->lane[14] += c->lane[12]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 28);
    c->hash ^= c->lane[12] + 0x3071b911u;
    c->raw[c->slo + (int)((t0 + 64131u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[2] += c->lane[15]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x15) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4062u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int fetch_rate_51(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x38) << 0;
    c->raw[c->slo + (int)((t0 + 929u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 59833u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdad9b65bu;
    c->lane[13] += c->lane[14] ^ 0x63fbfab7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x3a) << 16;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mark_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)slice_item(c, (uint8_t)(t0 >> 8), t2);
    c->lane[3] += c->lane[9]; c->lane[15] ^= c->lane[3]; c->lane[15] = sx_rl(c->lane[15], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x64) << 16;
    c->hash = (c->hash * 0x628752efu) ^ sx_rr(c->hash, 3);
    t2 += (uint32_t)clamp_group(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63771u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2c997c6fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xcbbc46dfu;
    shift_token(c, &c->lane[6], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    fill_group(c, &c->lane[3], 3);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int close_cursor(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t1 ^= (uint32_t)load_index(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x4124d9c1u) ^ sx_rr(c->hash, 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21307u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[14] + 0xb179354bu;
    t0 ^= swap_count(c, t1);
    c->lane[2] += c->lane[0] ^ 0x93cfe522u;
    c->lane[2] ^= sx_rl(c->lane[2], 16);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int hold_unit(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->hash = (c->hash * 0x9a351a97u) ^ sx_rr(c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63109u) % (uint32_t)c->rln)] << 24;
    split_field(c, &c->lane[4], 2);
    t2 += queue_ring(c, c->slo, c->sln);
    t2 += stage_count(c, c->slo, c->sln);
    t2 += latch_head(c, c->rlo, c->rln);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 10);
    c->hash = (c->hash * 0x3f32b96du) ^ sx_rr(c->hash, 30);
    c->sched[19] = c->hash ^ sx_rl(c->lane[5], 12);
    t2 += pair_stack(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0x51e2f0d5u;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mark_band(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[7] ^= sx_rl(c->lane[2], 11);
    t2 += trim_span(c, c->slo, c->sln);
    clamp_pool(c, &c->lane[5], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2713u) % (uint32_t)c->rln)] << 16;
    merge_limit(c, t0, t1);
    c->hash ^= c->lane[10] + 0xe660c0c0u;
    c->lane[14] += c->lane[10]; c->lane[5] ^= c->lane[14]; c->lane[5] = sx_rl(c->lane[5], 29);
    c->sched[29] = c->hash ^ sx_rl(c->lane[8], 18);
    c->hash ^= c->lane[4] + 0x2da05a10u;
    c->lane[13] += c->lane[15] ^ 0x8538f50bu;
    c->hash ^= c->lane[9] + 0x99c301beu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t hold_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[12] += c->lane[3] ^ 0x16b1ceb3u;
    c->lane[8] ^= sx_rl(c->lane[11], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59031u) % (uint32_t)c->rln)] << 16;
    t1 ^= (uint32_t)grow_block(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x0787cf83u) ^ sx_rr(c->hash, 20);
    t2 += (uint32_t)parse_band(c);
    c->raw[c->slo + (int)((t0 + 15204u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe915b60bu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t join_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe8096a99u;
    c->sched[6] = c->hash ^ sx_rl(c->lane[5], 14);
    c->lane[11] += c->lane[1]; c->lane[13] ^= c->lane[11]; c->lane[13] = sx_rl(c->lane[13], 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xea2019b1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0d682cfbu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc6) << 0;
    c->raw[c->slo + (int)((t0 + 53481u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x71ecd09fu;
    fetch_cell(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t peek_pairing(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    place_chunk(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[2] += c->lane[9]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 11);
    c->lane[12] += c->lane[9]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 20);
    c->sched[16] = c->hash ^ sx_rl(c->lane[11], 8);
    c->raw[c->slo + (int)((t0 + 15783u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    load_chunk(c, &c->lane[3], 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int store_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    probe_path(c, t0, t1);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 11);
    t2 += trim_span(c, c->slo, c->sln);
    c->lane[14] += c->lane[10]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 19);
    c->sched[22] = c->hash ^ sx_rl(c->lane[0], 21);
    c->lane[7] ^= sx_rl(c->lane[13], 22);
    c->lane[2] += c->lane[12] ^ 0x1e910bf3u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 13);
    t1 ^= (uint32_t)peek_count(c, (uint8_t)(t0 >> 8), t2);
    c->lane[1] ^= sx_rl(c->lane[14], 15);
    t2 = (t2 ^ c->sum) * 0x4dd7d951u;
    c->hash = (c->hash * 0x7be02c85u) ^ sx_rr(c->hash, 9);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 23);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t pick_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[24] = c->hash ^ sx_rl(c->lane[4], 11);
    c->hash = (c->hash * 0xdc1f2d4du) ^ sx_rr(c->hash, 11);
    t1 ^= (uint32_t)settle_limit(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x8d11000fu;
    c->sched[9] = c->hash ^ sx_rl(c->lane[1], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash = (c->hash * 0x27024ad9u) ^ sx_rr(c->hash, 30);
    c->hash = (c->hash * 0x4a508f9du) ^ sx_rr(c->hash, 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pair_store(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x15) << 8;
    c->sched[1] = c->hash ^ sx_rl(c->lane[7], 13);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 5);
    c->raw[c->slo + (int)((t0 + 12305u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 42651u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf55e4d87u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[0] += c->lane[7]; c->lane[2] ^= c->lane[0]; c->lane[2] = sx_rl(c->lane[2], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc4279ee1u;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t trim_pairing(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += stage_seat(c, c->rlo, c->rln);
    c->lane[7] += c->lane[12] ^ 0xe5362ac9u;
    c->lane[0] += c->lane[2]; c->lane[15] ^= c->lane[0]; c->lane[15] = sx_rl(c->lane[15], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x79) << 8;
    t0 ^= store_offset(c, t1);
    t2 = (t2 ^ c->sum) * 0x262ad2b3u;
    c->lane[8] += c->lane[8] ^ 0xfbb06c22u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void seek_cursor(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x86b97cdbu) ^ sx_rr(c->hash, 23);
    t2 += drain_pairing(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x42ab522bu) ^ sx_rr(c->hash, 9);
    c->sched[27] = c->hash ^ sx_rl(c->lane[1], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf6) << 0;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 20);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t defer_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] += c->lane[0] ^ 0xaf8e44c4u;
    c->lane[12] += c->lane[14]; c->lane[8] ^= c->lane[12]; c->lane[8] = sx_rl(c->lane[8], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x91) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xc154b623u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x74e2f949u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t reap_item(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xf9252797u;
    c->raw[c->slo + (int)((t0 + 18332u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[9] += c->lane[6] ^ 0xd8e8d5e7u;
    c->sched[22] = c->hash ^ sx_rl(c->lane[2], 15);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t step_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf5118b6bu;
    c->hash = (c->hash * 0x9a8a4923u) ^ sx_rr(c->hash, 23);
    c->lane[7] += c->lane[12]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 16);
    c->lane[5] ^= sx_rl(c->lane[12], 10);
    settle_offset(c, &c->lane[7], 3);
    c->hash = (c->hash * 0x0a44aaa7u) ^ sx_rr(c->hash, 5);
    c->raw[c->slo + (int)((t0 + 36158u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x5b03917bu) ^ sx_rr(c->hash, 18);
    t2 = (t2 ^ c->sum) * 0x3c36f5d3u;
    c->hash = (c->hash * 0x5d45e26bu) ^ sx_rr(c->hash, 7);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t purge_part(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x58) << 16;
    c->sched[6] = c->hash ^ sx_rl(c->lane[9], 9);
    c->sched[22] = c->hash ^ sx_rl(c->lane[14], 9);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 1);
    split_field(c, &c->lane[7], 1);
    t2 += (uint32_t)defer_bucket(c);
    c->lane[3] += c->lane[6]; c->lane[8] ^= c->lane[3]; c->lane[8] = sx_rl(c->lane[8], 2);
    yield_level(c, &c->lane[6], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t grow_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xe3f72da7u) ^ sx_rr(c->hash, 2);
    c->hash = (c->hash * 0xab628067u) ^ sx_rr(c->hash, 19);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 11);
    c->hash ^= c->lane[10] + 0x85b0d46eu;
    c->sched[28] = c->hash ^ sx_rl(c->lane[1], 3);
    c->lane[3] += c->lane[10] ^ 0x9f4aadbcu;
    tally_tail(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 8);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void tally_layer(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[12] ^= sx_rl(c->lane[13], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28653u) % (uint32_t)c->rln)] << 16;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 2);
    c->raw[c->slo + (int)((t0 + 9627u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x47) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38052u) % (uint32_t)c->rln)] << 24;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int parse_band(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    t2 = (t2 ^ c->sum) * 0xb879c457u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 20);
    c->hash ^= c->lane[14] + 0xf0956892u;
    c->lane[14] ^= sx_rl(c->lane[11], 20);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int clamp_group(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->raw[c->slo + (int)((t0 + 44966u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[5] ^= sx_rl(c->lane[15], 16);
    c->lane[3] ^= sx_rl(c->lane[11], 13);
    c->hash = (c->hash * 0x852c64c5u) ^ sx_rr(c->hash, 11);
    c->lane[12] += c->lane[9]; c->lane[0] ^= c->lane[12]; c->lane[0] = sx_rl(c->lane[0], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9404u) % (uint32_t)c->rln)] << 0;
    c->sched[26] = c->hash ^ sx_rl(c->lane[5], 10);
    c->hash ^= c->lane[13] + 0x631fde0cu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x09) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[9] += c->lane[10]; c->lane[1] ^= c->lane[9]; c->lane[1] = sx_rl(c->lane[1], 4);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t store_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] += c->lane[14] ^ 0x3f971416u;
    c->hash = (c->hash * 0x2ac24d93u) ^ sx_rr(c->hash, 30);
    t2 = (t2 ^ c->sum) * 0x22cbd763u;
    c->lane[5] += c->lane[0] ^ 0x04677d1du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x29) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4736af9bu;
    c->sched[31] = c->hash ^ sx_rl(c->lane[13], 13);
    c->lane[1] += c->lane[15]; c->lane[14] ^= c->lane[1]; c->lane[14] = sx_rl(c->lane[14], 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void fetch_cell(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 23938u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 2165u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x3aa6330fu;
    c->raw[c->slo + (int)((t0 + 16442u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[2]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 14);
    t2 = (t2 ^ c->sum) * 0x04c327d3u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 11);
    c->hash = (c->hash * 0xf6fe0f05u) ^ sx_rr(c->hash, 12);
    t2 = (t2 ^ c->sum) * 0xacccb8f1u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 10);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t tap_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] ^= sx_rl(c->lane[6], 29);
    c->sched[6] = c->hash ^ sx_rl(c->lane[6], 20);
    c->raw[c->slo + (int)((t0 + 21703u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[1] + 0x8ffdfd2bu;
    c->lane[0] += c->lane[11]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 28);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t flush_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x39) << 8;
    c->lane[4] ^= sx_rl(c->lane[0], 20);
    c->sched[7] = c->hash ^ sx_rl(c->lane[0], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9437u) % (uint32_t)c->rln)] << 0;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 28);
    c->lane[11] += c->lane[4]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x39fc386fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf0) << 8;
    c->lane[10] += c->lane[4]; c->lane[15] ^= c->lane[10]; c->lane[15] = sx_rl(c->lane[15], 3);
    c->sched[28] = c->hash ^ sx_rl(c->lane[5], 13);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t drain_pairing(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[17] = c->hash ^ sx_rl(c->lane[13], 6);
    c->raw[c->slo + (int)((t0 + 7002u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[4] += c->lane[9] ^ 0xa74deaabu;
    c->hash = (c->hash * 0x97a0ed6bu) ^ sx_rr(c->hash, 29);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t resize_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[15] + 0x315fe2f4u;
    c->hash ^= c->lane[6] + 0x5c16c0a7u;
    c->hash = (c->hash * 0x631e86d5u) ^ sx_rr(c->hash, 1);
    c->lane[3] ^= sx_rl(c->lane[9], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x69f26ddfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x6e) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcd) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int place_window(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[15] ^= sx_rl(c->lane[8], 31);
    c->sched[11] = c->hash ^ sx_rl(c->lane[3], 5);
    c->lane[5] += c->lane[0] ^ 0x9f2739b9u;
    c->raw[c->slo + (int)((t0 + 2520u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[14] += c->lane[10] ^ 0xa276c165u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x095463bbu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] ^= sx_rl(c->lane[1], 12);
    c->hash = (c->hash * 0x6757e62du) ^ sx_rr(c->hash, 19);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t store_offset(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54975u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[0] + 0xc22303e3u;
    c->raw[c->slo + (int)((t0 + 13668u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[3] = c->hash ^ sx_rl(c->lane[4], 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t patch_part(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    c->lane[12] += c->lane[14] ^ 0x4525b5a0u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32257u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23667u) % (uint32_t)c->rln)] << 24;
    c->sched[12] = c->hash ^ sx_rl(c->lane[11], 21);
    c->hash = (c->hash * 0x949a18f3u) ^ sx_rr(c->hash, 20);
    c->hash ^= c->lane[2] + 0x81eafac1u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t queue_ring(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[0] = c->hash ^ sx_rl(c->lane[3], 21);
    c->lane[12] += c->lane[6]; c->lane[13] ^= c->lane[12]; c->lane[13] = sx_rl(c->lane[13], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1fff6981u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash = (c->hash * 0x614ad6bdu) ^ sx_rr(c->hash, 16);
    c->raw[c->slo + (int)((t0 + 41061u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t pick_key(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 26);
    c->hash = (c->hash * 0xefaa5959u) ^ sx_rr(c->hash, 29);
    t2 = (t2 ^ c->sum) * 0xce912c57u;
    c->lane[7] ^= sx_rl(c->lane[12], 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x6d) << 0;
    c->hash ^= c->lane[1] + 0xc0299a82u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64466u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void fill_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41127u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x703cd6bbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17681u) % (uint32_t)c->rln)] << 0;
    c->lane[2] ^= sx_rl(c->lane[14], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6dcac4cbu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6861ab0fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sched[4] = c->hash ^ sx_rl(c->lane[0], 2);
    c->hash ^= c->lane[14] + 0x7a14cb5bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[2] += c->lane[11]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 7);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void probe_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xfb20c40bu;
    c->lane[12] += c->lane[10] ^ 0xc451e214u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbf2ebfa3u;
    c->hash ^= c->lane[5] + 0xef27c81fu;
    c->lane[8] += c->lane[11]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x234e2a2fu;
    c->hash ^= c->lane[9] + 0x7780a713u;
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t scan_chunk(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x71397543u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5619bc29u;
    c->lane[5] += c->lane[9]; c->lane[3] ^= c->lane[5]; c->lane[3] = sx_rl(c->lane[3], 26);
    t2 = (t2 ^ c->sum) * 0x3e77f1f5u;
    c->lane[9] += c->lane[13]; c->lane[3] ^= c->lane[9]; c->lane[3] = sx_rl(c->lane[3], 6);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t sync_layer(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[6] += c->lane[12] ^ 0xeca21ff0u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44066u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x66) << 16;
    c->hash = (c->hash * 0x0934bfbdu) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 41172u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x5a4334cdu;
    c->raw[c->slo + (int)((t0 + 52268u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void shift_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[2]; c->lane[15] ^= c->lane[5]; c->lane[15] = sx_rl(c->lane[15], 26);
    c->lane[14] += c->lane[7] ^ 0x7bc8b3d7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd73e8433u;
    c->hash = (c->hash * 0x4c5a46efu) ^ sx_rr(c->hash, 30);
    c->raw[c->slo + (int)((t0 + 53446u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 9);
    t2 = (t2 ^ c->sum) * 0x7e9ebd2du;
    c->lane[9] ^= sx_rl(c->lane[7], 6);
    t2 += sync_mask(c, c->rlo, c->rln);
    c->lane[13] += c->lane[3]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x00841a81u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t trim_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 53310u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[0] += c->lane[5]; c->lane[11] ^= c->lane[0]; c->lane[11] = sx_rl(c->lane[11], 24);
    c->hash ^= c->lane[5] + 0x4d6ec8aau;
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 9);
    c->lane[12] += c->lane[6] ^ 0xef37b912u;
    c->hash = (c->hash * 0x1237bd71u) ^ sx_rr(c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[15] ^= sx_rl(c->lane[5], 3);
    c->lane[3] += c->lane[7] ^ 0xd818ece7u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t peek_count(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xddb15a93u;
    c->lane[8] ^= sx_rl(c->lane[10], 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x928560e9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35474u) % (uint32_t)c->rln)] << 24;
    c->lane[1] ^= sx_rl(c->lane[4], 31);
    c->sched[7] = c->hash ^ sx_rl(c->lane[6], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29086u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x51fa3f69u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void tally_tail(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x2e8259f9u) ^ sx_rr(c->hash, 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8c329be7u;
    c->hash ^= c->lane[2] + 0x723d3e6eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[25] = c->hash ^ sx_rl(c->lane[5], 20);
    c->raw[c->slo + (int)((t0 + 26295u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 19);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void split_field(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 37593u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[3] + 0xdb6aa65eu;
    c->lane[9] += c->lane[13]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13676u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[11] + 0x74c119eeu;
    c->raw[c->slo + (int)((t0 + 47305u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 13);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t purge_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[31] = c->hash ^ sx_rl(c->lane[6], 19);
    c->hash = (c->hash * 0xf8ba91f1u) ^ sx_rr(c->hash, 31);
    t2 = (t2 ^ c->sum) * 0x40447a7fu;
    c->sum += t1;
    return t0 + t2;
}

static void load_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x5aacec3fu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 2);
    c->hash = (c->hash * 0xd0218761u) ^ sx_rr(c->hash, 5);
    c->lane[8] += c->lane[9]; c->lane[11] ^= c->lane[8]; c->lane[11] = sx_rl(c->lane[11], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe427cca1u;
    c->lane[12] += c->lane[11] ^ 0x455b9673u;
    t2 = (t2 ^ c->sum) * 0x9eda4b55u;
    c->raw[c->slo + (int)((t0 + 32436u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t move_record(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[10] += c->lane[9]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[7] ^= sx_rl(c->lane[12], 4);
    c->lane[5] += c->lane[3] ^ 0x9ae942bbu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 15961u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    store_seat(c, &c->lane[8], 4);
    t2 = (t2 ^ c->sum) * 0x9fa0ebbdu;
    c->hash = (c->hash * 0xc2d36839u) ^ sx_rr(c->hash, 21);
    t2 = (t2 ^ c->sum) * 0x84fc76fdu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x0eb8f43du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void map_port(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[4], 13);
    c->sched[17] = c->hash ^ sx_rl(c->lane[15], 25);
    c->hash ^= c->lane[7] + 0xdb391081u;
    c->lane[1] += c->lane[3]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb5) << 8;
    t2 = (t2 ^ c->sum) * 0x062a9e45u;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void yield_level(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 4205u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[4] + 0x10ca2749u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 11187u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xdcb7873bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void shift_token(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x08ae0203u) ^ sx_rr(c->hash, 3);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 23);
    c->hash ^= c->lane[1] + 0x198884e7u;
    t2 = (t2 ^ c->sum) * 0x10fc769du;
    c->lane[4] += c->lane[14] ^ 0x7547de49u;
    t2 = (t2 ^ c->sum) * 0x122404c9u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void map_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35933u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 48532u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65259u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x93a8f94fu) ^ sx_rr(c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x97) << 16;
    c->lane[9] += c->lane[3]; c->lane[7] ^= c->lane[9]; c->lane[7] = sx_rl(c->lane[7], 1);
    c->raw[c->slo + (int)((t0 + 38174u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0xc652517bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void clamp_pool(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 60592u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[14] ^= sx_rl(c->lane[0], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc968e6c3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[9] += c->lane[8] ^ 0x88a7f385u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int swap_bucket(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x6b) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xad882c57u;
    c->lane[0] += c->lane[11]; c->lane[12] ^= c->lane[0]; c->lane[12] = sx_rl(c->lane[12], 23);
    c->lane[10] += c->lane[7] ^ 0x158da206u;
    c->lane[14] += c->lane[7] ^ 0x0e6d587eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd9) << 8;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int defer_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 5);
    c->lane[6] += c->lane[7] ^ 0x1cf0e91au;
    c->raw[c->slo + (int)((t0 + 3747u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x14ef852du) ^ sx_rr(c->hash, 11);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t latch_head(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc9) << 0;
    t2 = (t2 ^ c->sum) * 0x6b19a2e7u;
    c->hash ^= c->lane[5] + 0xf72a435au;
    c->sum += t1;
    return t0 + t2;
}

static void queue_line(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[12] += c->lane[10]; c->lane[0] ^= c->lane[12]; c->lane[0] = sx_rl(c->lane[0], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0x983a2f51u) ^ sx_rr(c->hash, 2);
    c->lane[9] += c->lane[15]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 7);
    c->hash = (c->hash * 0x5b6437d1u) ^ sx_rr(c->hash, 10);
    c->raw[c->slo + (int)((t0 + 7661u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0e) << 0;
    c->lane[13] += c->lane[5]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 28);
    c->lane[14] += c->lane[13]; c->lane[1] ^= c->lane[14]; c->lane[1] = sx_rl(c->lane[1], 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x02) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x6f) << 16;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void hold_arena(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 6153u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[9] += c->lane[2]; c->lane[3] ^= c->lane[9]; c->lane[3] = sx_rl(c->lane[3], 16);
    c->sched[14] = c->hash ^ sx_rl(c->lane[6], 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8e) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13713u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x597b9335u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36506u) % (uint32_t)c->rln)] << 16;
    c->lane[3] ^= sx_rl(c->lane[8], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t prime_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] += c->lane[15]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 27);
    c->raw[c->slo + (int)((t0 + 27722u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x569ffd4du;
    c->lane[12] += c->lane[10] ^ 0x49a4f455u;
    c->lane[4] += c->lane[14]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 4);
    c->lane[4] += c->lane[0]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 12);
    c->sum += t1;
    return t0 + t2;
}

static int defer_bucket(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12647u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x9c56fb19u) ^ sx_rr(c->hash, 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9993u) % (uint32_t)c->rln)] << 16;
    c->lane[7] += c->lane[12]; c->lane[15] ^= c->lane[7]; c->lane[15] = sx_rl(c->lane[15], 12);
    c->hash = (c->hash * 0xa28d128fu) ^ sx_rr(c->hash, 17);
    t2 = (t2 ^ c->sum) * 0xa9358eb5u;
    c->lane[11] ^= sx_rl(c->lane[2], 1);
    c->lane[5] += c->lane[15]; c->lane[10] ^= c->lane[5]; c->lane[10] = sx_rl(c->lane[10], 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0x932ff95fu) ^ sx_rr(c->hash, 23);
    c->raw[c->slo + (int)((t0 + 56658u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t wrap_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 23030u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xf455adbdu) ^ sx_rr(c->hash, 27);
    t2 = (t2 ^ c->sum) * 0x612629bbu;
    c->sched[1] = c->hash ^ sx_rl(c->lane[2], 1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t fold_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 64478u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8bb99b13u;
    c->hash ^= c->lane[6] + 0x11ddc5bau;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x23) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t stage_count(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[3] ^= sx_rl(c->lane[9], 21);
    c->hash = (c->hash * 0xd49905adu) ^ sx_rr(c->hash, 25);
    t2 = (t2 ^ c->sum) * 0x7ab20585u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[8], 22);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t settle_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[3] + 0xd146b661u;
    c->sched[20] = c->hash ^ sx_rl(c->lane[9], 23);
    c->lane[1] += c->lane[15]; c->lane[2] ^= c->lane[1]; c->lane[2] = sx_rl(c->lane[2], 28);
    c->hash = (c->hash * 0x13c0c7ebu) ^ sx_rr(c->hash, 29);
    c->lane[7] += c->lane[14] ^ 0x1fa384adu;
    c->lane[4] += c->lane[0] ^ 0x3da2b1d5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[13] = c->hash ^ sx_rl(c->lane[15], 19);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t swap_count(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd3) << 16;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= c->lane[1] + 0x02328c11u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x79aa531bu;
    c->lane[3] += c->lane[7] ^ 0x4e6fbe2au;
    c->hash = (c->hash * 0x631c3e01u) ^ sx_rr(c->hash, 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void place_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48746u) % (uint32_t)c->rln)] << 0;
    c->lane[11] += c->lane[6]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7ebd75b3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52065u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x9618bee5u;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t reset_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x592f8da9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe757c0a5u;
    c->hash = (c->hash * 0x98581f39u) ^ sx_rr(c->hash, 24);
    c->lane[11] += c->lane[11] ^ 0x69af8a58u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t slice_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x0acf2583u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 22);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 15);
    t2 = (t2 ^ c->sum) * 0x3add713bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7d672893u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x97) << 0;
    c->lane[4] += c->lane[10]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x85763003u;
    c->lane[10] += c->lane[10] ^ 0x3da2b944u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void settle_offset(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] ^= sx_rl(c->lane[7], 10);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 13);
    c->lane[4] += c->lane[15] ^ 0x9010a963u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[4] ^= sx_rl(c->lane[8], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x128f3f51u) ^ sx_rr(c->hash, 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2f448d43u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9bac6515u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t grow_block(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x27) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26664u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0xe1282da9u) ^ sx_rr(c->hash, 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61535u) % (uint32_t)c->rln)] << 16;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 9);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 7);
    c->raw[c->slo + (int)((t0 + 27801u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[15] + 0xa6366a13u;
    c->hash ^= c->lane[7] + 0x776aed20u;
    c->lane[10] += c->lane[11]; c->lane[15] ^= c->lane[10]; c->lane[15] = sx_rl(c->lane[15], 6);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void fill_group(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 ^= hold_cursor(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 1);
    c->lane[12] += c->lane[4] ^ 0xf331a99fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pair_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[1] += c->lane[14] ^ 0x0f8115cdu;
    t2 = (t2 ^ c->sum) * 0x60cb2e29u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8b) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 7);
    c->lane[4] += c->lane[5] ^ 0x4f1ece37u;
    c->sum += t1;
    return t0 + t2;
}

static int parse_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->lane[11] += c->lane[1] ^ 0x2b1872acu;
    c->hash = (c->hash * 0xd94f42abu) ^ sx_rr(c->hash, 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xad008e53u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31327u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[11] + 0xaa187a6cu;
    t2 = (t2 ^ c->sum) * 0x5bdf974fu;
    c->hash ^= c->lane[1] + 0xd941358du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30709u) % (uint32_t)c->rln)] << 8;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t load_index(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 7);
    c->hash = (c->hash * 0xd464b6adu) ^ sx_rr(c->hash, 27);
    c->hash = (c->hash * 0xfa4fe573u) ^ sx_rr(c->hash, 17);
    c->lane[0] += c->lane[10] ^ 0x5169fc03u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64824u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[0] + 0x5ed70869u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void relay_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xb8660495u) ^ sx_rr(c->hash, 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62260u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[2] + 0x356c7ab7u;
    c->lane[3] += c->lane[14] ^ 0x6c5cb47fu;
    c->lane[8] += c->lane[15]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 14);
    c->lane[10] ^= sx_rl(c->lane[1], 28);
    c->raw[c->slo + (int)((t0 + 54221u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x795e2a63u) ^ sx_rr(c->hash, 28);
    c->lane[1] ^= sx_rl(c->lane[15], 26);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void merge_limit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[11] += c->lane[2] ^ 0x85d80034u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25871u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x5a838f37u;
    c->lane[3] ^= sx_rl(c->lane[9], 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x53) << 8;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 30);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 22);
    c->lane[15] += c->lane[4] ^ 0xcab27765u;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t reset_entry(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 14);
    c->hash = (c->hash * 0x21628311u) ^ sx_rr(c->hash, 9);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 5);
    c->lane[12] += c->lane[2] ^ 0xca8bd68eu;
    c->lane[15] += c->lane[15] ^ 0x84007f00u;
    c->lane[9] += c->lane[8]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 17);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 31);
    c->lane[10] += c->lane[0]; c->lane[11] ^= c->lane[10]; c->lane[11] = sx_rl(c->lane[11], 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60643u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x7d75611bu) ^ sx_rr(c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t stage_seat(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[0] + 0xa37d9bc8u;
    c->lane[4] ^= sx_rl(c->lane[13], 17);
    c->hash ^= c->lane[2] + 0xc7d9c562u;
    t2 = (t2 ^ c->sum) * 0x9f523a5du;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t join_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[8] + 0x01e05733u;
    c->sched[28] = c->hash ^ sx_rl(c->lane[11], 10);
    c->lane[10] += c->lane[4] ^ 0x1384d818u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 16);
    t2 = (t2 ^ c->sum) * 0x8ae786d7u;
    c->sum += t1;
    return t0 + t2;
}

static void link_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xb0ffdab9u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 19);
    c->lane[3] += c->lane[14] ^ 0xd68f7b7au;
    c->raw[c->slo + (int)((t0 + 65092u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[1] ^= sx_rl(c->lane[8], 5);
    t2 = (t2 ^ c->sum) * 0x33a93525u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7b97b197u;
    t2 = (t2 ^ c->sum) * 0xf09ddf8fu;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void probe_path(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa0) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sched[16] = c->hash ^ sx_rl(c->lane[2], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xed22f50bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x43e0815du;
    c->raw[c->slo + (int)((t0 + 21993u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe7de104du;
    c->lane[5] += c->lane[2] ^ 0x46e8a9e6u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 2);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 22);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void rotate_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    map_index(c, &c->lane[4], 2);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 12);
    c->lane[12] ^= sx_rl(c->lane[10], 16);
    resize_view(c, &c->lane[10], 2);
    c->lane[13] += c->lane[3] ^ 0x3e7454bau;
    t1 ^= (uint32_t)link_node(c, (uint8_t)(t0 >> 16), t2);
    t2 += (uint32_t)mark_segment(c);
    t0 ^= queue_layer(c, t1);
    t1 ^= (uint32_t)merge_limit_149(c, (uint8_t)(t0 >> 0), t2);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 30);
    drain_tuple(c, t0, t1);
    t1 ^= (uint32_t)slice_window(c, (uint8_t)(t0 >> 8), t2);
    c->lane[9] += c->lane[1]; c->lane[2] ^= c->lane[9]; c->lane[2] = sx_rl(c->lane[2], 15);
    t1 ^= (uint32_t)split_range(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= hold_cursor(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)parse_record(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa4091a1bu;
    t2 += reset_label(c, c->rlo, c->rln);
    t0 ^= rotate_key(c, t1);
    t1 ^= (uint32_t)emit_offset(c, (uint8_t)(t0 >> 16), t2);
    t2 += fill_level(c, c->rlo, c->rln);
    blend_path(c, &c->lane[0], 2);
    pack_pairing(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x96a2e1ddu;
    t1 ^= (uint32_t)shift_chunk_162(c, (uint8_t)(t0 >> 16), t2);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static void map_index(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    push_item(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x92c5be27u;
    scan_cursor(c, &c->lane[10], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 ^= mix_digest(c, t1);
    reap_bucket(c, &c->lane[5], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    coal_frame(c, &c->lane[11], 1);
    c->raw[c->slo + (int)((t0 + 58075u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 18);
    latch_limit(c, &c->lane[10], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x678a890fu;
    t2 += (uint32_t)mark_count(c);
    c->lane[1] += c->lane[15] ^ 0x03d1b1bcu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54629u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 46568u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t reset_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x062d1a2du;
    t0 ^= parse_mask(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xba) << 0;
    t2 += (uint32_t)parse_track(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14185u) % (uint32_t)c->rln)] << 8;
    tally_batch(c, &c->lane[7], 1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 10);
    tap_port(c, &c->lane[10], 4);
    t0 ^= trim_layer(c, t1);
    c->lane[5] += c->lane[6]; c->lane[8] ^= c->lane[5]; c->lane[8] = sx_rl(c->lane[8], 11);
    coal_gap(c, t0, t1);
    slice_bucket(c, t0, t1);
    c->hash ^= c->lane[11] + 0xda668743u;
    c->lane[7] += c->lane[8]; c->lane[12] ^= c->lane[7]; c->lane[12] = sx_rl(c->lane[12], 4);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t split_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xb0c9835du) ^ sx_rr(c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x12bb42f3u;
    c->raw[c->slo + (int)((t0 + 41782u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x3ed975a3u) ^ sx_rr(c->hash, 2);
    c->raw[c->slo + (int)((t0 + 59975u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    push_stream(c, &c->lane[10], 3);
    c->sched[24] = c->hash ^ sx_rl(c->lane[11], 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    prime_head(c, &c->lane[11], 4);
    c->raw[c->slo + (int)((t0 + 34858u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)map_tail(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x76588357u;
    link_table_177(c, t0, t1);
    t0 ^= shift_key(c, t1);
    t0 ^= drain_token_137(c, t1);
    c->lane[15] += c->lane[2]; c->lane[11] ^= c->lane[15]; c->lane[11] = sx_rl(c->lane[11], 21);
    t1 ^= (uint32_t)reset_store(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)push_batch(c, (uint8_t)(t0 >> 0), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void tally_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa90ea96fu;
    t2 = (t2 ^ c->sum) * 0x28c671ebu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[4] += c->lane[1]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 18);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 30);
    t2 += trim_arena(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xba7692ebu;
    c->hash ^= c->lane[14] + 0xb60c7028u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54652u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t reset_store(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] += c->lane[3] ^ 0x6e8241b3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 9591u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x35a026f1u) ^ sx_rr(c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x25078cebu;
    push_item(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xdc) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5b) << 8;
    t2 += (uint32_t)defer_tail(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void prime_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[2] + 0x9dc067bau;
    t2 += drain_entry(c, c->slo, c->sln);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 16);
    t2 += (uint32_t)join_bound(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t2 += (uint32_t)tune_lease(c);
    t0 ^= rotate_offset(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t shift_key(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[0] + 0xc0d4c7cbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15299u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 54169u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += (uint32_t)parse_track(c);
    t1 ^= (uint32_t)link_node(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)grow_count(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0xb8821f2du;
    t1 ^= (uint32_t)pin_cursor(c, (uint8_t)(t0 >> 0), t2);
    blend_path(c, &c->lane[6], 3);
    c->lane[11] ^= sx_rl(c->lane[1], 8);
    c->lane[6] ^= sx_rl(c->lane[12], 1);
    c->sched[4] = c->hash ^ sx_rl(c->lane[10], 21);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void reap_bucket(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] ^= sx_rl(c->lane[13], 31);
    grow_layer(c, t0, t1);
    sync_cursor_220(c, &c->lane[0], 1);
    t2 += trim_arena(c, c->rlo, c->rln);
    t0 ^= link_key(c, t1);
    c->raw[c->slo + (int)((t0 + 42636u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    defer_row(c, &c->lane[7], 1);
    c->lane[4] += c->lane[9]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 1);
    c->hash = (c->hash * 0x0fc72d83u) ^ sx_rr(c->hash, 10);
    c->sched[29] = c->hash ^ sx_rl(c->lane[2], 16);
    t2 += clamp_table(c, c->slo, c->sln);
    trace_path(c, &c->lane[6], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)sync_cursor(c, (uint8_t)(t0 >> 0), t2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t drain_token_137(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xb11368b9u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 30);
    t1 ^= (uint32_t)parse_block(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= prime_entry(c, t1);
    push_ring(c, &c->lane[9], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x39) << 8;
    t1 ^= (uint32_t)merge_limit_149(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)poll_arena(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x00d5cc7du) ^ sx_rr(c->hash, 27);
    c->hash = (c->hash * 0x5c29581fu) ^ sx_rr(c->hash, 25);
    c->sched[15] = c->hash ^ sx_rl(c->lane[10], 19);
    clamp_block(c, &c->lane[1], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t1 ^= (uint32_t)pin_head(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    tap_port(c, &c->lane[10], 4);
    t2 = (t2 ^ c->sum) * 0x98999cb3u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void scan_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 13);
    c->lane[13] += c->lane[2] ^ 0x167e7c7cu;
    c->hash = (c->hash * 0x6f336c15u) ^ sx_rr(c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 46277u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x11f0399fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x733371f5u;
    t2 = (t2 ^ c->sum) * 0x886028a9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += probe_window(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x5b2bcb41u;
    c->raw[c->slo + (int)((t0 + 59852u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t parse_mask(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x003111e1u) ^ sx_rr(c->hash, 23);
    c->lane[3] += c->lane[14] ^ 0x2229babcu;
    t2 += yield_table(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash = (c->hash * 0xecd016abu) ^ sx_rr(c->hash, 16);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 19);
    c->raw[c->slo + (int)((t0 + 48814u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[6] + 0x9c46f5d1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xaa) << 8;
    scan_token(c, &c->lane[2], 2);
    c->hash ^= c->lane[12] + 0x26e91c6au;
    t0 ^= wrap_cursor(c, t1);
    c->hash ^= c->lane[1] + 0x30be7ce4u;
    c->lane[15] += c->lane[7]; c->lane[10] ^= c->lane[15]; c->lane[10] = sx_rl(c->lane[10], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void latch_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += clamp_table(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 20714u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)map_tail(c, (uint8_t)(t0 >> 8), t2);
    t2 += merge_page(c, c->slo, c->sln);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf9) << 0;
    t0 ^= mix_cell(c, t1);
    t2 = (t2 ^ c->sum) * 0xc5c87ff5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += tally_stream(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb6) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void slice_bucket(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 65300u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t1 ^= (uint32_t)emit_offset(c, (uint8_t)(t0 >> 0), t2);
    drain_field(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x4426f7e9u;
    c->lane[14] ^= sx_rl(c->lane[1], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x200384e5u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint8_t sync_cursor(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9c) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x9fb9ca1fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 17723u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x1ec22093u;
    t0 ^= hold_slot(c, t1);
    shift_rate(c, &c->lane[11], 2);
    c->lane[3] += c->lane[12]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 15);
    t0 ^= trim_layer(c, t1);
    t2 += reap_track(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61999u) % (uint32_t)c->rln)] << 24;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 26);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t prime_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[12] = c->hash ^ sx_rl(c->lane[15], 6);
    t2 += (uint32_t)clamp_delta(c);
    c->sched[14] = c->hash ^ sx_rl(c->lane[10], 8);
    t1 ^= (uint32_t)resize_list(c, (uint8_t)(t0 >> 8), t2);
    c->hash = (c->hash * 0x9b3c4d11u) ^ sx_rr(c->hash, 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcfda5b47u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 ^= mix_cell(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t probe_window(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xc0c15877u;
    t2 = (t2 ^ c->sum) * 0xf046101fu;
    c->hash = (c->hash * 0x7863b975u) ^ sx_rr(c->hash, 25);
    c->hash ^= c->lane[3] + 0xd4c8bb91u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45406u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26974u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x68e6a25du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 306u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34744u) % (uint32_t)c->rln)] << 16;
    c->lane[14] += c->lane[5] ^ 0xea840176u;
    link_table_177(c, t0, t1);
    c->lane[7] += c->lane[1] ^ 0x3414c59au;
    drain_tuple(c, t0, t1);
    c->sum += t1;
    return t0 + t2;
}

static void drain_field(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= flush_track(c, t1);
    grow_layer(c, t0, t1);
    t2 += (uint32_t)defer_tail(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x095dfe51u;
    c->lane[10] += c->lane[0]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 9);
    c->hash ^= c->lane[3] + 0xf85cfc6cu;
    grow_bucket(c, &c->lane[3], 4);
    t0 ^= step_view(c, t1);
    c->hash ^= c->lane[13] + 0x23f03b3au;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x196332e1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb4) << 8;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 17);
    c->hash ^= c->lane[2] + 0xc6d4b588u;
    clamp_block(c, &c->lane[3], 2);
    t2 = (t2 ^ c->sum) * 0x1261de4fu;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t drain_entry(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46269u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x04) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x51) << 8;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x74abb365u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[3], 30);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x3c9d54dbu;
    c->sum += t1;
    return t0 + t2;
}

static void tap_port(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    load_head(c, t0, t1);
    t2 += (uint32_t)drain_page(c);
    c->lane[1] += c->lane[6]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa18ecc3fu;
    c->lane[7] += c->lane[9] ^ 0xcd8dfae2u;
    c->hash ^= c->lane[13] + 0x577c823eu;
    c->hash = (c->hash * 0x0fa5bcc3u) ^ sx_rr(c->hash, 10);
    c->hash ^= c->lane[12] + 0x31ed34feu;
    c->raw[c->slo + (int)((t0 + 60665u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[9] ^= sx_rl(c->lane[0], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xde) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t grow_count(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x079587fdu;
    c->hash = (c->hash * 0x618095bfu) ^ sx_rr(c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t2 += (uint32_t)defer_batch(c);
    t0 ^= resize_queue(c, t1);
    t2 = (t2 ^ c->sum) * 0x3cca18f7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t merge_limit_149(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x13) << 0;
    c->hash = (c->hash * 0x5287d201u) ^ sx_rr(c->hash, 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34943u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 17);
    t0 ^= rotate_key(c, t1);
    c->hash ^= c->lane[13] + 0x52222dd5u;
    c->hash ^= c->lane[13] + 0xb9e214aau;
    c->lane[6] += c->lane[3]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 6);
    c->lane[6] ^= sx_rl(c->lane[1], 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t map_tail(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= cache_store(c, t1);
    c->lane[5] += c->lane[3] ^ 0xc5086940u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7781eb27u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60756u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46631u) % (uint32_t)c->rln)] << 8;
    c->lane[10] += c->lane[1]; c->lane[2] ^= c->lane[10]; c->lane[2] = sx_rl(c->lane[2], 29);
    t0 ^= wrap_cursor(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] += c->lane[2] ^ 0x29d8c044u;
    c->hash = (c->hash * 0xb226a8b9u) ^ sx_rr(c->hash, 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void blend_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 60362u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31214u) % (uint32_t)c->rln)] << 24;
    t0 ^= purge_pairing(c, t1);
    t2 = (t2 ^ c->sum) * 0x907b139bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash = (c->hash * 0xf10b3b1fu) ^ sx_rr(c->hash, 31);
    c->hash = (c->hash * 0xb3f349bbu) ^ sx_rr(c->hash, 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t clamp_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb3757073u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[5], 7);
    c->lane[4] += c->lane[7]; c->lane[12] ^= c->lane[4]; c->lane[12] = sx_rl(c->lane[12], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa4) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc0e5d329u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t trim_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xd6270fadu) ^ sx_rr(c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x30c2c647u;
    t0 ^= resize_queue(c, t1);
    c->lane[7] ^= sx_rl(c->lane[14], 30);
    c->lane[8] += c->lane[14] ^ 0xd06cbab3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t parse_block(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t1 ^= (uint32_t)map_track(c, (uint8_t)(t0 >> 0), t2);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x57c63abdu;
    c->lane[13] += c->lane[0] ^ 0x36e54221u;
    t2 += (uint32_t)cache_chunk(c);
    c->hash ^= c->lane[2] + 0xdc5d2269u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += (uint32_t)mark_count(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void trace_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x962955a7u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa6) << 0;
    t1 ^= (uint32_t)shift_chunk_162(c, (uint8_t)(t0 >> 16), t2);
    c->lane[9] += c->lane[2]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 2);
    c->sched[7] = c->hash ^ sx_rl(c->lane[10], 6);
    t2 += yield_table(c, c->rlo, c->rln);
    t0 ^= cache_slot(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void push_item(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)sync_item(c, (uint8_t)(t0 >> 0), t2);
    c->sched[13] = c->hash ^ sx_rl(c->lane[2], 5);
    c->raw[c->slo + (int)((t0 + 60506u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58750u) % (uint32_t)c->rln)] << 24;
    t2 += sync_mask(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 59305u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    reset_key(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x3058dc39u;
    t1 ^= (uint32_t)latch_node(c, (uint8_t)(t0 >> 0), t2);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static int parse_track(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[12] += c->lane[3]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24738u) % (uint32_t)c->rln)] << 16;
    c->lane[7] += c->lane[3] ^ 0x5641f78au;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x28bcc6efu;
    t2 += reset_state(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0xd03d5281u) ^ sx_rr(c->hash, 27);
    c->hash ^= c->lane[8] + 0xc1ca374fu;
    c->sched[14] = c->hash ^ sx_rl(c->lane[1], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31521u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 60811u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t poll_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 55310u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[25] = c->hash ^ sx_rl(c->lane[0], 15);
    c->lane[14] += c->lane[13] ^ 0x537e669cu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 19);
    c->lane[14] ^= sx_rl(c->lane[15], 9);
    t1 ^= (uint32_t)sync_item(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc216a1abu;
    t2 += (uint32_t)mark_segment(c);
    shift_rate(c, &c->lane[6], 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t pin_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xa5dab6f9u) ^ sx_rr(c->hash, 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[15] ^= sx_rl(c->lane[0], 20);
    seek_pairing(c, &c->lane[7], 3);
    c->lane[10] ^= sx_rl(c->lane[6], 29);
    c->hash = (c->hash * 0x1e19fdadu) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa094c9a3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8a) << 16;
    c->lane[0] += c->lane[14]; c->lane[11] ^= c->lane[0]; c->lane[11] = sx_rl(c->lane[11], 19);
    c->lane[3] += c->lane[5] ^ 0xa0007ab0u;
    c->lane[10] += c->lane[12] ^ 0xfb143c13u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t emit_offset(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 38122u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[7] + 0x18e4e094u;
    c->lane[6] += c->lane[4]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 15);
    c->lane[8] += c->lane[2] ^ 0x337b92b6u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 14);
    parse_tuple(c, &c->lane[6], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb0) << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t link_node(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[8] + 0x2f4c0cbfu;
    c->lane[10] += c->lane[13]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 5);
    t2 += coal_lease(c, c->slo, c->sln);
    c->lane[0] ^= sx_rl(c->lane[9], 21);
    chain_item(c, &c->lane[10], 1);
    t1 ^= (uint32_t)pick_slot(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[12] + 0x9e52a942u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t shift_chunk_162(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x43784f99u;
    c->lane[13] ^= sx_rl(c->lane[3], 31);
    c->lane[2] += c->lane[14]; c->lane[13] ^= c->lane[2]; c->lane[13] = sx_rl(c->lane[13], 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25069u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[13] + 0xd44c4e34u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x37) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa59f753du;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t rotate_key(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41425u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 13720u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 65217u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[13] ^= sx_rl(c->lane[15], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    parse_tuple(c, &c->lane[6], 4);
    c->lane[8] += c->lane[4]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 3);
    t0 ^= rotate_band(c, t1);
    c->sched[12] = c->hash ^ sx_rl(c->lane[13], 31);
    t1 ^= (uint32_t)push_batch(c, (uint8_t)(t0 >> 16), t2);
    c->lane[9] ^= sx_rl(c->lane[6], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb9e2eac1u;
    t2 = (t2 ^ c->sum) * 0x32bd071fu;
    close_range(c, t0, t1);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 13);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t sync_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[0] + 0x368e8914u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35347u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0xa72b98d5u) ^ sx_rr(c->hash, 28);
    c->lane[2] ^= sx_rl(c->lane[13], 21);
    c->hash = (c->hash * 0xa3c567a5u) ^ sx_rr(c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 28);
    t0 ^= queue_layer(c, t1);
    c->raw[c->slo + (int)((t0 + 8794u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x40bc9fa3u) ^ sx_rr(c->hash, 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32269u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void clamp_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t1 ^= (uint32_t)move_seat(c, (uint8_t)(t0 >> 0), t2);
    t2 += tally_stream(c, c->slo, c->sln);
    c->hash ^= c->lane[9] + 0x0b99580eu;
    c->sched[4] = c->hash ^ sx_rl(c->lane[9], 24);
    t2 = (t2 ^ c->sum) * 0x4d8efc17u;
    c->lane[11] += c->lane[6]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 15);
    t2 += stage_count_240(c, c->rlo, c->rln);
    store_seat(c, &c->lane[2], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 += (uint32_t)tune_lease(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void grow_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc0) << 0;
    pack_delta_234(c, t0, t1);
    t0 ^= mix_digest(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)pin_cursor(c, (uint8_t)(t0 >> 8), t2);
    c->lane[8] += c->lane[6] ^ 0x967aead3u;
    c->hash ^= c->lane[0] + 0x3e8c9ddcu;
    c->lane[12] += c->lane[3]; c->lane[9] ^= c->lane[12]; c->lane[9] = sx_rl(c->lane[9], 8);
    pack_pairing(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63271u) % (uint32_t)c->rln)] << 24;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t reset_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 16426u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[2] + 0xc814fa90u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t resize_queue(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x2a4e22afu) ^ sx_rr(c->hash, 12);
    t2 = (t2 ^ c->sum) * 0xfaaada7bu;
    resize_view(c, &c->lane[7], 2);
    c->lane[12] ^= sx_rl(c->lane[13], 30);
    c->lane[6] ^= sx_rl(c->lane[0], 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    reset_page(c, &c->lane[2], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x1f) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa43e6a33u;
    c->lane[14] += c->lane[5] ^ 0x2a19e5f6u;
    c->sched[10] = c->hash ^ sx_rl(c->lane[9], 23);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t mix_cell(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[14] += c->lane[15]; c->lane[7] ^= c->lane[14]; c->lane[7] = sx_rl(c->lane[7], 3);
    t0 ^= step_view(c, t1);
    c->hash ^= c->lane[0] + 0xb43a03d6u;
    c->raw[c->slo + (int)((t0 + 57522u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)purge_field(c, (uint8_t)(t0 >> 8), t2);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 27);
    close_range(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t coal_lease(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)map_slot(c);
    c->hash = (c->hash * 0x2c35e971u) ^ sx_rr(c->hash, 30);
    reset_key(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbe3dc2ebu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xcf65bae1u;
    t0 ^= link_key(c, t1);
    c->hash = (c->hash * 0xc6186d05u) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[3] ^= sx_rl(c->lane[7], 21);
    t0 ^= fold_view(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static void drain_tuple(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27833u) % (uint32_t)c->rln)] << 16;
    t1 ^= (uint32_t)parse_record(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x35) << 16;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t pick_slot(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x8335f6a7u) ^ sx_rr(c->hash, 16);
    c->lane[13] += c->lane[14] ^ 0xfca8a912u;
    t0 ^= seek_tuple(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10254u) % (uint32_t)c->rln)] << 24;
    t0 ^= hold_cursor(c, t1);
    c->lane[6] += c->lane[12]; c->lane[14] ^= c->lane[6]; c->lane[14] = sx_rl(c->lane[14], 13);
    c->lane[8] += c->lane[7] ^ 0x2d3bb5fdu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbdaf181fu;
    t2 += (uint32_t)sift_token(c);
    c->sched[13] = c->hash ^ sx_rl(c->lane[5], 31);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t resize_list(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] ^= sx_rl(c->lane[2], 2);
    tally_group(c, t0, t1);
    t1 ^= (uint32_t)purge_field(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x9a912dcfu;
    c->raw[c->slo + (int)((t0 + 20491u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50200u) % (uint32_t)c->rln)] << 8;
    merge_window(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x0b21f8d7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void chain_item(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] ^= sx_rl(c->lane[12], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13875u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x306bc0ebu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xce) << 0;
    t2 = (t2 ^ c->sum) * 0xd8546195u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf5) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t cache_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xc7f5b085u) ^ sx_rr(c->hash, 20);
    c->hash ^= c->lane[2] + 0x724c9f33u;
    push_stream(c, &c->lane[2], 4);
    t0 ^= pin_track(c, t1);
    c->hash ^= c->lane[4] + 0x7b5d2c3fu;
    c->lane[4] += c->lane[12] ^ 0x699e385du;
    c->hash ^= c->lane[8] + 0xcd4c8428u;
    c->lane[1] ^= sx_rl(c->lane[9], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6986u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xee) << 0;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 17);
    t2 += fill_level(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 26162u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 39630u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    relay_offset(c, &c->lane[7], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int mark_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash = (c->hash * 0x8b0b806fu) ^ sx_rr(c->hash, 19);
    c->lane[14] ^= sx_rl(c->lane[7], 29);
    t0 ^= rotate_band(c, t1);
    c->raw[c->slo + (int)((t0 + 3324u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 51885u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[4] + 0x805d862fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6bb2633fu;
    t0 ^= tally_run(c, t1);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void link_table_177(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[0] += c->lane[6] ^ 0x2603cd18u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] += c->lane[11] ^ 0x05826740u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[15], 20);
    c->hash ^= c->lane[5] + 0xbfc35be2u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[0] = c->hash ^ sx_rl(c->lane[8], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0xd482c4e3u) ^ sx_rr(c->hash, 6);
    c->hash ^= c->lane[2] + 0xbcbdf546u;
    c->lane[9] ^= sx_rl(c->lane[9], 26);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t hold_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 11);
    t1 ^= (uint32_t)fold_span(c, (uint8_t)(t0 >> 8), t2);
    c->sched[18] = c->hash ^ sx_rl(c->lane[15], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xf630b67du;
    c->sched[5] = c->hash ^ sx_rl(c->lane[8], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe3908c6fu;
    c->lane[7] += c->lane[1]; c->lane[15] ^= c->lane[7]; c->lane[15] = sx_rl(c->lane[15], 4);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int cache_chunk(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->lane[0] ^= sx_rl(c->lane[13], 15);
    c->lane[7] += c->lane[2] ^ 0xa2fcc8adu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63606u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65470u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x1a22aab3u) ^ sx_rr(c->hash, 25);
    c->hash = (c->hash * 0xfa4590d7u) ^ sx_rr(c->hash, 18);
    fetch_span(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xb2895cd9u;
    t2 += (uint32_t)close_queue(c);
    reset_index(c, &c->lane[7], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe29da5f9u;
    c->lane[15] ^= sx_rl(c->lane[11], 7);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t purge_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[16] = c->hash ^ sx_rl(c->lane[13], 10);
    t2 += mix_line(c, c->rlo, c->rln);
    c->sched[17] = c->hash ^ sx_rl(c->lane[9], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2a33e839u;
    t2 = (t2 ^ c->sum) * 0x88d3244du;
    c->hash = (c->hash * 0x42a0c209u) ^ sx_rr(c->hash, 22);
    c->sched[31] = c->hash ^ sx_rl(c->lane[12], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xba012b13u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc350eb75u;
    c->lane[6] += c->lane[10]; c->lane[1] ^= c->lane[6]; c->lane[1] = sx_rl(c->lane[1], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8327u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[10] + 0x5976609fu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int clamp_delta(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    t1 ^= (uint32_t)mix_chunk(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4b) << 0;
    c->lane[15] += c->lane[14] ^ 0x2ca8829au;
    t2 = (t2 ^ c->sum) * 0x91022c4du;
    c->lane[1] ^= sx_rl(c->lane[13], 30);
    push_ring(c, &c->lane[11], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc00f33f1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34743u) % (uint32_t)c->rln)] << 16;
    c->lane[2] += c->lane[9] ^ 0x6beba9c6u;
    t2 = (t2 ^ c->sum) * 0x914103b9u;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int defer_batch(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sched[8] = c->hash ^ sx_rl(c->lane[10], 23);
    c->lane[12] ^= sx_rl(c->lane[4], 24);
    c->raw[c->slo + (int)((t0 + 47872u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5e) << 0;
    c->lane[14] += c->lane[13] ^ 0xff15676du;
    c->hash ^= c->lane[4] + 0xeab8e47du;
    c->lane[15] += c->lane[12]; c->lane[9] ^= c->lane[15]; c->lane[9] = sx_rl(c->lane[9], 6);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 23);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t map_track(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x436d5221u;
    c->lane[2] += c->lane[15]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x2b) << 0;
    c->lane[14] += c->lane[13] ^ 0xe7cf1081u;
    t2 = (t2 ^ c->sum) * 0x7f04f975u;
    t2 += defer_table(c, c->rlo, c->rln);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 11);
    c->lane[8] += c->lane[6] ^ 0x5d221267u;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 28);
    c->lane[10] += c->lane[2]; c->lane[11] ^= c->lane[10]; c->lane[11] = sx_rl(c->lane[11], 9);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void grow_bucket(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59243u) % (uint32_t)c->rln)] << 16;
    c->lane[15] += c->lane[11]; c->lane[6] ^= c->lane[15]; c->lane[6] = sx_rl(c->lane[6], 1);
    c->hash = (c->hash * 0x7967e971u) ^ sx_rr(c->hash, 9);
    coal_gap(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9453c85du;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 28);
    t2 += (uint32_t)drain_page(c);
    c->hash ^= c->lane[6] + 0x485655e6u;
    t2 = (t2 ^ c->sum) * 0x48479febu;
    c->hash = (c->hash * 0x5ad564e5u) ^ sx_rr(c->hash, 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t yield_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 24000u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35116u) % (uint32_t)c->rln)] << 24;
    c->sched[11] = c->hash ^ sx_rl(c->lane[0], 1);
    t0 ^= rotate_offset(c, t1);
    c->lane[5] += c->lane[1] ^ 0x63b5e606u;
    c->lane[2] += c->lane[6]; c->lane[4] ^= c->lane[2]; c->lane[4] = sx_rl(c->lane[4], 17);
    c->hash = (c->hash * 0xfa297a03u) ^ sx_rr(c->hash, 31);
    t2 = (t2 ^ c->sum) * 0xca48a681u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum += t1;
    return t0 + t2;
}

static void load_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += push_group(c, c->rlo, c->rln);
    c->lane[12] += c->lane[13] ^ 0xaebf4082u;
    c->sched[27] = c->hash ^ sx_rl(c->lane[6], 31);
    t2 = (t2 ^ c->sum) * 0x8ed464cfu;
    c->lane[15] += c->lane[13]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 7);
    c->lane[2] += c->lane[1] ^ 0xc1118623u;
    c->raw[c->slo + (int)((t0 + 2873u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint8_t latch_node(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 59972u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += sync_mask(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 37645u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 2103u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += (uint32_t)join_bound(c);
    c->lane[2] ^= sx_rl(c->lane[8], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x89) << 0;
    t2 = (t2 ^ c->sum) * 0x36e0384fu;
    t1 ^= (uint32_t)fetch_region(c, (uint8_t)(t0 >> 8), t2);
    coal_frame(c, &c->lane[4], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t1 ^= (uint32_t)move_seat(c, (uint8_t)(t0 >> 16), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int defer_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    t2 += (uint32_t)map_slot(c);
    c->lane[9] += c->lane[12]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 29);
    c->sched[0] = c->hash ^ sx_rl(c->lane[11], 7);
    t2 += stage_slot(c, c->slo, c->sln);
    c->lane[14] += c->lane[7] ^ 0xf7e4a819u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 17);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int mark_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[9] += c->lane[2] ^ 0x0fbe9c2fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[26] = c->hash ^ sx_rl(c->lane[14], 23);
    t1 ^= (uint32_t)slice_window(c, (uint8_t)(t0 >> 8), t2);
    c->raw[c->slo + (int)((t0 + 18997u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x3ebc3ecfu) ^ sx_rr(c->hash, 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xeb) << 16;
    c->lane[13] += c->lane[3]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 21);
    c->sched[12] = c->hash ^ sx_rl(c->lane[6], 10);
    c->lane[0] += c->lane[13] ^ 0x685da4c8u;
    c->sched[17] = c->hash ^ sx_rl(c->lane[6], 16);
    c->lane[11] += c->lane[2]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 4);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void shift_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa3069023u;
    split_block(c, t0, t1);
    c->lane[11] += c->lane[1]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa3) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t trim_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 12062u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x46a1ce77u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[9], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24377u) % (uint32_t)c->rln)] << 0;
    c->lane[0] += c->lane[14]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 15);
    t2 = (t2 ^ c->sum) * 0x24b195fdu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t cache_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[7], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc499e7a1u;
    c->hash = (c->hash * 0xe18f9bd9u) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9c) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50630u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)clamp_field(c);
    scan_token(c, &c->lane[7], 1);
    c->hash = (c->hash * 0xb7138a7bu) ^ sx_rr(c->hash, 11);
    c->lane[15] += c->lane[13]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 10);
    defer_row(c, &c->lane[9], 1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t flush_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    sync_cursor_220(c, &c->lane[5], 1);
    t2 += emit_cursor(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x52) << 0;
    probe_slot(c, &c->lane[11], 2);
    c->lane[9] ^= sx_rl(c->lane[11], 14);
    c->lane[2] += c->lane[0] ^ 0xfc6a3fd3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6fe01c13u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t wrap_cursor(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 28710u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x642aa409u;
    c->lane[13] ^= sx_rl(c->lane[11], 24);
    c->hash ^= c->lane[15] + 0x004046d5u;
    c->raw[c->slo + (int)((t0 + 35813u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t reap_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60730u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[6] + 0x938d22e3u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] += c->lane[4]; c->lane[5] ^= c->lane[13]; c->lane[5] = sx_rl(c->lane[5], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[8] += c->lane[7] ^ 0x6d556894u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += merge_page(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9e) << 16;
    c->lane[4] += c->lane[14]; c->lane[10] ^= c->lane[4]; c->lane[10] = sx_rl(c->lane[10], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24232u) % (uint32_t)c->rln)] << 0;
    c->sum += t1;
    return t0 + t2;
}

static void seek_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x8428cd35u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[3] += c->lane[7] ^ 0x0dcaaecau;
    c->lane[5] += c->lane[8]; c->lane[12] ^= c->lane[5]; c->lane[12] = sx_rl(c->lane[12], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb1) << 16;
    c->sched[29] = c->hash ^ sx_rl(c->lane[13], 15);
    c->hash ^= c->lane[10] + 0x69738983u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26732u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0xa662a1a9u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t queue_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xca577197u) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5e) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x0d) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x21cac603u;
    c->hash ^= c->lane[8] + 0xfa38ba7au;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7b) << 16;
    c->lane[14] += c->lane[14] ^ 0x5ee847eau;
    c->lane[5] ^= sx_rl(c->lane[11], 2);
    c->sched[0] = c->hash ^ sx_rl(c->lane[10], 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t link_key(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[14] = c->hash ^ sx_rl(c->lane[6], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] ^= sx_rl(c->lane[7], 16);
    t2 = (t2 ^ c->sum) * 0x1fe2aef1u;
    c->raw[c->slo + (int)((t0 + 7200u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t defer_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21969u) % (uint32_t)c->rln)] << 16;
    c->lane[11] ^= sx_rl(c->lane[6], 2);
    c->hash ^= c->lane[5] + 0x76404363u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf7b7d0afu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61324u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[10] + 0xcab1fe56u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 20);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t step_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 53390u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x97) << 8;
    c->lane[9] ^= sx_rl(c->lane[15], 29);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void reset_page(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[3] + 0x5a014d5eu;
    t2 = (t2 ^ c->sum) * 0x946a2aadu;
    t2 = (t2 ^ c->sum) * 0xdab9868bu;
    c->lane[14] += c->lane[14] ^ 0xfc828faeu;
    c->lane[13] += c->lane[11] ^ 0xf8a8dfa0u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t emit_cursor(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 15398u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0xc7104f9du) ^ sx_rr(c->hash, 13);
    c->hash = (c->hash * 0xb0a0d8b3u) ^ sx_rr(c->hash, 6);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 31820u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x79f412dfu) ^ sx_rr(c->hash, 10);
    c->hash = (c->hash * 0x7e5e4bcfu) ^ sx_rr(c->hash, 11);
    c->hash = (c->hash * 0x3fa8f09fu) ^ sx_rr(c->hash, 15);
    c->sum += t1;
    return t0 + t2;
}

static void reset_key(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2c1b2537u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    c->hash = (c->hash * 0x95090725u) ^ sx_rr(c->hash, 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x3e) << 8;
    c->lane[0] += c->lane[12] ^ 0xbf413054u;
    t2 = (t2 ^ c->sum) * 0x212d8c03u;
    c->sched[23] = c->hash ^ sx_rl(c->lane[4], 17);
    c->hash ^= c->lane[9] + 0xf24f5077u;
    c->lane[6] ^= sx_rl(c->lane[3], 5);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint8_t mix_chunk(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    scan_bucket(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2512fed7u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 21);
    c->hash = (c->hash * 0x2e35c665u) ^ sx_rr(c->hash, 11);
    c->hash ^= c->lane[12] + 0x34e51c5bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int sift_token(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd8) << 0;
    c->lane[8] ^= sx_rl(c->lane[2], 20);
    t2 = (t2 ^ c->sum) * 0x31ca1ca7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb3) << 0;
    c->sched[26] = c->hash ^ sx_rl(c->lane[2], 13);
    c->hash = (c->hash * 0x3e598e89u) ^ sx_rr(c->hash, 5);
    c->sched[22] = c->hash ^ sx_rl(c->lane[8], 10);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mix_line(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xd9062881u;
    t2 = (t2 ^ c->sum) * 0x13382f8fu;
    c->lane[3] ^= sx_rl(c->lane[11], 31);
    c->hash = (c->hash * 0x5593d08bu) ^ sx_rr(c->hash, 29);
    c->raw[c->slo + (int)((t0 + 10720u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[9] + 0x9516a29eu;
    t2 = (t2 ^ c->sum) * 0xcf0d2a47u;
    c->hash ^= c->lane[9] + 0x6a640e2au;
    c->lane[9] ^= sx_rl(c->lane[3], 13);
    t2 = (t2 ^ c->sum) * 0xb2a94e5bu;
    c->sum += t1;
    return t0 + t2;
}

static int map_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->hash ^= c->lane[5] + 0x1d519f23u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x51) << 16;
    c->lane[12] += c->lane[11]; c->lane[7] ^= c->lane[12]; c->lane[7] = sx_rl(c->lane[7], 29);
    c->lane[7] ^= sx_rl(c->lane[9], 28);
    c->hash = (c->hash * 0xfba20f1fu) ^ sx_rr(c->hash, 11);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void relay_offset(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[0] + 0xa1f03e20u;
    c->hash = (c->hash * 0xa377e773u) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xeefae8e5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xa0ffe135u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void split_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[2] + 0x4047d39fu;
    c->lane[15] ^= sx_rl(c->lane[7], 26);
    c->lane[4] ^= sx_rl(c->lane[8], 12);
    c->raw[c->slo + (int)((t0 + 6556u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void tally_group(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[22] = c->hash ^ sx_rl(c->lane[5], 2);
    c->lane[11] ^= sx_rl(c->lane[6], 13);
    c->lane[10] += c->lane[0]; c->lane[12] ^= c->lane[10]; c->lane[12] = sx_rl(c->lane[12], 30);
    c->lane[10] += c->lane[4] ^ 0x36666460u;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint32_t merge_page(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xf0249983u) ^ sx_rr(c->hash, 14);
    c->lane[15] += c->lane[13]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 6);
    c->lane[2] += c->lane[9] ^ 0xb419a926u;
    c->lane[12] ^= sx_rl(c->lane[12], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe45e0491u;
    c->hash ^= c->lane[1] + 0x26fbcb36u;
    c->hash ^= c->lane[14] + 0x26870be7u;
    c->sum += t1;
    return t0 + t2;
}

static void push_stream(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[0], 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[4] += c->lane[8] ^ 0xe02f4f4cu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x17) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7284u) % (uint32_t)c->rln)] << 8;
    c->lane[14] += c->lane[9]; c->lane[5] ^= c->lane[14]; c->lane[5] = sx_rl(c->lane[5], 27);
    c->hash = (c->hash * 0x8219a83bu) ^ sx_rr(c->hash, 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t tally_stream(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5ea5226bu;
    c->lane[14] ^= sx_rl(c->lane[11], 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23444u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xde22968fu;
    c->lane[6] ^= sx_rl(c->lane[11], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[12] + 0x2b4e0b65u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32824u) % (uint32_t)c->rln)] << 24;
    c->sum += t1;
    return t0 + t2;
}

static int drain_page(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40893u) % (uint32_t)c->rln)] << 8;
    c->lane[1] ^= sx_rl(c->lane[5], 2);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15299u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[10] += c->lane[2] ^ 0x7755cccdu;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int tune_lease(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->raw[c->slo + (int)((t0 + 40607u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[6] = c->hash ^ sx_rl(c->lane[10], 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65405u) % (uint32_t)c->rln)] << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[7], 10);
    c->lane[5] ^= sx_rl(c->lane[5], 16);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void store_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] += c->lane[6]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 6);
    c->hash ^= c->lane[2] + 0xbb31ae21u;
    c->hash = (c->hash * 0xd6fdc725u) ^ sx_rr(c->hash, 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] += c->lane[0]; c->lane[9] ^= c->lane[12]; c->lane[9] = sx_rl(c->lane[9], 12);
    c->sched[9] = c->hash ^ sx_rl(c->lane[14], 11);
    c->lane[5] += c->lane[13]; c->lane[0] ^= c->lane[5]; c->lane[0] = sx_rl(c->lane[0], 21);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void coal_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] += c->lane[5]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 5);
    c->lane[3] += c->lane[15] ^ 0xc6aca71du;
    t2 = (t2 ^ c->sum) * 0x3b538157u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x15) << 0;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 15);
    c->hash = (c->hash * 0x02bc9b2du) ^ sx_rr(c->hash, 27);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void parse_tuple(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56254u) % (uint32_t)c->rln)] << 0;
    c->lane[2] += c->lane[12]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 7);
    t2 = (t2 ^ c->sum) * 0xc6e8a9fbu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8e1df25fu;
    c->lane[2] += c->lane[9]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 19);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 1);
    c->hash = (c->hash * 0x51d19edbu) ^ sx_rr(c->hash, 30);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int close_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->sched[20] = c->hash ^ sx_rl(c->lane[3], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x8b4cce67u;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sync_cursor_220(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45647u) % (uint32_t)c->rln)] << 8;
    c->lane[2] += c->lane[7]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 6);
    c->hash ^= c->lane[9] + 0x129974f5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64134u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x095ad283u) ^ sx_rr(c->hash, 16);
    c->lane[7] += c->lane[13] ^ 0xbbae8165u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fold_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x43) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] ^= sx_rl(c->lane[15], 22);
    c->lane[10] += c->lane[15]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 1);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9ce6aba7u;
    c->lane[0] ^= sx_rl(c->lane[2], 2);
    c->lane[13] += c->lane[0] ^ 0x9cda2b30u;
    c->hash = (c->hash * 0xf52775c1u) ^ sx_rr(c->hash, 16);
    c->lane[13] += c->lane[8] ^ 0x6b23da07u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9806u) % (uint32_t)c->rln)] << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t seek_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x34) << 0;
    c->lane[8] += c->lane[2]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 27);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 8);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 27);
    c->lane[13] += c->lane[14] ^ 0x1bed5c73u;
    t1 ^= (uint32_t)move_index(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t pin_cursor(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41142u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64079u) % (uint32_t)c->rln)] << 16;
    c->sched[0] = c->hash ^ sx_rl(c->lane[15], 4);
    c->hash = (c->hash * 0xfb068539u) ^ sx_rr(c->hash, 10);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t fill_level(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16944u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[11] + 0x87cdf485u;
    c->hash = (c->hash * 0x45afc11du) ^ sx_rr(c->hash, 2);
    c->lane[0] += c->lane[11] ^ 0xfaaffd8au;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[13] += c->lane[3] ^ 0x99ffd3d5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19301u) % (uint32_t)c->rln)] << 24;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t push_group(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[8] ^= sx_rl(c->lane[13], 15);
    c->sched[22] = c->hash ^ sx_rl(c->lane[2], 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb5) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9a) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[4] + 0x08f8e728u;
    t2 = (t2 ^ c->sum) * 0xdbe5337du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42813u) % (uint32_t)c->rln)] << 24;
    c->sum += t1;
    return t0 + t2;
}

static void scan_token(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb1) << 16;
    c->lane[6] += c->lane[1]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 22);
    c->lane[4] ^= sx_rl(c->lane[2], 7);
    c->raw[c->slo + (int)((t0 + 28701u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xcfd69e4du;
    c->sched[7] = c->hash ^ sx_rl(c->lane[15], 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0f5a62d7u;
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 14);
    c->hash = (c->hash * 0xcfb99537u) ^ sx_rr(c->hash, 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void defer_row(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x550b8109u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 6);
    c->hash ^= c->lane[9] + 0xe8de9027u;
    t2 = (t2 ^ c->sum) * 0xa8bccdf7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64138u) % (uint32_t)c->rln)] << 24;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t slice_window(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] += c->lane[0] ^ 0x576d380bu;
    t2 = (t2 ^ c->sum) * 0xbc9ad143u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0xcff4bd63u;
    c->lane[10] += c->lane[1] ^ 0x41f9cebfu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb6) << 0;
    c->hash ^= c->lane[10] + 0xf641f977u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t fold_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] += c->lane[1] ^ 0xaa336642u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61791u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42415u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[12] + 0xec87b9f4u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xc0ccd4c7u;
    c->lane[5] ^= sx_rl(c->lane[2], 31);
    c->raw[c->slo + (int)((t0 + 27710u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[7] + 0x4d07631eu;
    c->hash = (c->hash * 0x4b82c047u) ^ sx_rr(c->hash, 9);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int clamp_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->lane[0] += c->lane[2]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 10);
    c->hash ^= c->lane[2] + 0x521834deu;
    c->lane[3] += c->lane[4] ^ 0x3409dd2eu;
    c->hash = (c->hash * 0xf435b83du) ^ sx_rr(c->hash, 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57188u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0xfdfe67bfu) ^ sx_rr(c->hash, 27);
    c->lane[4] += c->lane[8]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 3);
    c->sched[28] = c->hash ^ sx_rl(c->lane[9], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t fetch_region(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5a226383u;
    c->sched[21] = c->hash ^ sx_rl(c->lane[7], 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcf170af7u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 12);
    c->sched[30] = c->hash ^ sx_rl(c->lane[1], 8);
    c->lane[1] ^= sx_rl(c->lane[13], 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 53593u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t parse_record(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 31);
    c->sched[6] = c->hash ^ sx_rl(c->lane[0], 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xec602347u;
    c->lane[0] ^= sx_rl(c->lane[6], 9);
    t2 = (t2 ^ c->sum) * 0xa9ab776fu;
    c->lane[0] += c->lane[5] ^ 0x0514c19cu;
    c->lane[7] ^= sx_rl(c->lane[4], 23);
    c->lane[2] ^= sx_rl(c->lane[5], 20);
    t2 = (t2 ^ c->sum) * 0x1d1be841u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t sync_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 15);
    c->hash = (c->hash * 0x5b705bf7u) ^ sx_rr(c->hash, 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51546u) % (uint32_t)c->rln)] << 24;
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 13);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[6] += c->lane[5] ^ 0x9a4f857fu;
    c->raw[c->slo + (int)((t0 + 62161u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28393u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static void pack_delta_234(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] += c->lane[3] ^ 0x7cb56da4u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[9], 3);
    t2 = (t2 ^ c->sum) * 0xa0ce49f7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa31ee77bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11957u) % (uint32_t)c->rln)] << 8;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 21);
    c->lane[3] += c->lane[15]; c->lane[8] ^= c->lane[3]; c->lane[8] = sx_rl(c->lane[8], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x433e9f65u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x741b3b97u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t tally_run(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x02b155bbu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb2) << 16;
    t2 = (t2 ^ c->sum) * 0x983f3fefu;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[9] + 0x65334e67u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[4] += c->lane[1] ^ 0xda90031bu;
    c->raw[c->slo + (int)((t0 + 11509u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 19);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint32_t mix_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xec) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x116235f9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe9602d45u;
    c->hash ^= c->lane[4] + 0x8b4602eau;
    c->lane[2] += c->lane[11]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdd) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x97) << 16;
    t2 = (t2 ^ c->sum) * 0x95ca7b01u;
    c->lane[7] += c->lane[14] ^ 0xc15d9240u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void reset_index(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa7) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] += c->lane[1] ^ 0xfa00d14eu;
    c->lane[12] += c->lane[0] ^ 0x128adac4u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 26);
    c->raw[c->slo + (int)((t0 + 17162u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12361u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void fetch_span(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[1] + 0x4f07e9f7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8e) << 0;
    c->lane[11] += c->lane[9] ^ 0x2669f6f1u;
    c->lane[9] += c->lane[8] ^ 0x5f0a2666u;
    c->raw[c->slo + (int)((t0 + 25901u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xaf) << 8;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t stage_count_240(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe9e7f99fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42930u) % (uint32_t)c->rln)] << 16;
    c->lane[4] ^= sx_rl(c->lane[10], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 14);
    t2 = (t2 ^ c->sum) * 0xca499357u;
    c->lane[1] ^= sx_rl(c->lane[0], 23);
    c->sched[12] = c->hash ^ sx_rl(c->lane[0], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void close_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] += c->lane[12] ^ 0xf13abc5au;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x49e941fbu;
    c->lane[0] += c->lane[10] ^ 0xf250e4e7u;
    c->raw[c->slo + (int)((t0 + 48541u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint8_t move_seat(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 14494u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xf2b1f1c3u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 3);
    c->hash ^= c->lane[4] + 0x150a8b6au;
    c->raw[c->slo + (int)((t0 + 51670u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t purge_field(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x9a361671u) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9f) << 8;
    c->hash ^= c->lane[7] + 0x97ea5dd7u;
    c->lane[1] += c->lane[5] ^ 0x4baaf425u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void coal_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x11ea1709u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf619fb6fu;
    t2 = (t2 ^ c->sum) * 0x694301cfu;
    c->hash ^= c->lane[3] + 0x97191f13u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41371u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x42248c1du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x49991622u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t hold_cursor(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24435u) % (uint32_t)c->rln)] << 24;
    c->lane[0] += c->lane[1]; c->lane[2] ^= c->lane[0]; c->lane[2] = sx_rl(c->lane[2], 8);
    t2 += (uint32_t)split_block_364(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[6] += c->lane[5]; c->lane[4] ^= c->lane[6]; c->lane[4] = sx_rl(c->lane[4], 29);
    c->sched[4] = c->hash ^ sx_rl(c->lane[13], 27);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t push_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[0] ^= sx_rl(c->lane[1], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[3] += c->lane[5] ^ 0x48d6c6a1u;
    c->lane[4] += c->lane[9] ^ 0x4c517b67u;
    c->lane[14] += c->lane[6]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int join_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33164u) % (uint32_t)c->rln)] << 8;
    c->lane[7] ^= sx_rl(c->lane[12], 26);
    t2 = (t2 ^ c->sum) * 0x3b41040du;
    c->raw[c->slo + (int)((t0 + 39451u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x36e91449u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa9) << 8;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void resize_view(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[12] + 0x50c3ccfau;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[16] = c->hash ^ sx_rl(c->lane[5], 29);
    c->lane[3] += c->lane[7] ^ 0xd34e8d0eu;
    c->lane[5] += c->lane[4]; c->lane[12] ^= c->lane[5]; c->lane[12] = sx_rl(c->lane[12], 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 7);
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 26);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t rotate_offset(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xf751257fu;
    c->hash = (c->hash * 0xa8193445u) ^ sx_rr(c->hash, 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0xf76cea81u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 23);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 28);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 24);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 12);
    c->hash = (c->hash * 0x340b33dfu) ^ sx_rr(c->hash, 13);
    c->lane[15] += c->lane[11]; c->lane[10] ^= c->lane[15]; c->lane[10] = sx_rl(c->lane[10], 30);
    c->raw[c->slo + (int)((t0 + 26237u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pin_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8f269835u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5226cd25u;
    t2 = (t2 ^ c->sum) * 0x942c6b3bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[11] += c->lane[12]; c->lane[13] ^= c->lane[11]; c->lane[13] = sx_rl(c->lane[13], 25);
    c->lane[6] += c->lane[4]; c->lane[8] ^= c->lane[6]; c->lane[8] = sx_rl(c->lane[8], 7);
    c->lane[0] += c->lane[8] ^ 0xc5335cffu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x55639407u;
    c->hash = (c->hash * 0x3a72f8ddu) ^ sx_rr(c->hash, 12);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t rotate_band(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] ^= sx_rl(c->lane[13], 19);
    c->hash = (c->hash * 0x3e527ba7u) ^ sx_rr(c->hash, 6);
    c->lane[3] ^= sx_rl(c->lane[3], 3);
    c->lane[2] += c->lane[11]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 17);
    c->lane[7] += c->lane[8] ^ 0x6d84b839u;
    c->hash = (c->hash * 0xaa3395d1u) ^ sx_rr(c->hash, 18);
    c->hash ^= c->lane[11] + 0x83e2d4adu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void probe_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[2] += c->lane[3] ^ 0xaeeae161u;
    c->lane[8] += c->lane[11] ^ 0xb7cac8c9u;
    t2 = (t2 ^ c->sum) * 0x23bfaa33u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 27);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t stage_slot(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x693932f1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x40) << 0;
    c->raw[c->slo + (int)((t0 + 22565u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xc1635211u;
    c->lane[6] += c->lane[6] ^ 0x4a71d9deu;
    c->lane[8] ^= sx_rl(c->lane[11], 6);
    c->sum += t1;
    return t0 + t2;
}

static void push_ring(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[2] ^= sx_rl(c->lane[15], 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] += c->lane[2] ^ 0x3ebab1deu;
    c->sched[23] = c->hash ^ sx_rl(c->lane[14], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44681u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 18);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void pack_pairing(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 9);
    c->hash = (c->hash * 0xc10e4a6fu) ^ sx_rr(c->hash, 14);
    t0 ^= probe_part(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25022u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8e8c6937u;
    c->lane[14] += c->lane[10] ^ 0xe71a0999u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void queue_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= drain_tuple_316(c, t1);
    t0 ^= map_tuple(c, t1);
    t2 += (uint32_t)link_layer(c);
    t2 += rotate_batch(c, c->rlo, c->rln);
    fetch_frame(c, &c->lane[10], 3);
    prime_stream(c, t0, t1);
    stage_pairing(c, &c->lane[5], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42234u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)reset_count(c);
    flush_bound(c, &c->lane[11], 4);
    t1 ^= (uint32_t)peek_pairing_342(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 27);
    fold_group(c, t0, t1);
    c->lane[3] += c->lane[6]; c->lane[11] ^= c->lane[3]; c->lane[11] = sx_rl(c->lane[11], 1);
    t2 += fold_cursor(c, c->slo, c->sln);
    t2 += merge_head(c, c->slo, c->sln);
    step_stream(c, t0, t1);
    t0 ^= place_pairing(c, t1);
    t0 ^= move_layer(c, t1);
    cache_part(c, &c->lane[10], 2);
    c->lane[3] ^= sx_rl(c->lane[11], 6);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void flush_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t1 ^= (uint32_t)mark_stack(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] += c->lane[12] ^ 0x2ccb0a0eu;
    c->sched[19] = c->hash ^ sx_rl(c->lane[7], 23);
    t2 += (uint32_t)place_queue(c);
    t2 += cache_head(c, c->rlo, c->rln);
    c->hash ^= c->lane[4] + 0xa8e40b05u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64783u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += (uint32_t)probe_entry(c);
    c->raw[c->slo + (int)((t0 + 24976u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x6d000d7bu) ^ sx_rr(c->hash, 15);
    t1 ^= (uint32_t)rotate_label(c, (uint8_t)(t0 >> 8), t2);
    c->hash = (c->hash * 0x14d2da01u) ^ sx_rr(c->hash, 28);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 9);
    t1 ^= (uint32_t)pack_count(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void prime_stream(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += tune_field(c, c->slo, c->sln);
    sift_value(c, t0, t1);
    blend_region(c, t0, t1);
    t1 ^= (uint32_t)reap_count(c, (uint8_t)(t0 >> 8), t2);
    pick_entry(c, t0, t1);
    t2 += (uint32_t)hold_tail(c);
    t1 ^= (uint32_t)trace_batch(c, (uint8_t)(t0 >> 8), t2);
    c->raw[c->slo + (int)((t0 + 26809u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= trim_digest(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8c2d3c7bu;
    t2 += push_cell(c, c->rlo, c->rln);
    t2 += (uint32_t)mix_port(c);
    reset_bound(c, &c->lane[6], 2);
    patch_cell(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += (uint32_t)map_unit(c);
    c->lane[2] += c->lane[13]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 20);
    t2 = (t2 ^ c->sum) * 0xd589f957u;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t map_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += stage_count_374(c, c->slo, c->sln);
    c->lane[14] ^= sx_rl(c->lane[5], 19);
    c->lane[8] += c->lane[11]; c->lane[14] ^= c->lane[8]; c->lane[14] = sx_rl(c->lane[14], 9);
    c->lane[4] ^= sx_rl(c->lane[11], 5);
    c->lane[10] += c->lane[6] ^ 0xcdaf381cu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)blend_region_264(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)pick_bucket(c, (uint8_t)(t0 >> 0), t2);
    fill_label(c, t0, t1);
    wrap_lease(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void sift_value(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)close_level(c, (uint8_t)(t0 >> 0), t2);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 26);
    place_state_272(c, &c->lane[1], 1);
    patch_cell(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 30699u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x69be527fu;
    t0 ^= place_pairing(c, t1);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 28);
    c->hash = (c->hash * 0xaaa29629u) ^ sx_rr(c->hash, 21);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t tune_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)sync_view(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf1cd2d49u;
    c->lane[11] += c->lane[3]; c->lane[1] ^= c->lane[11]; c->lane[1] = sx_rl(c->lane[1], 19);
    hold_limit(c, t0, t1);
    t2 += pack_mask(c, c->rlo, c->rln);
    t2 += (uint32_t)step_field(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16863u) % (uint32_t)c->rln)] << 16;
    relay_chunk(c, &c->lane[9], 1);
    c->hash = (c->hash * 0x2bf8bc79u) ^ sx_rr(c->hash, 19);
    t2 += (uint32_t)purge_part_361(c);
    stage_pairing(c, &c->lane[4], 4);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void blend_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)move_index(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xff1ebf95u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6d) << 8;
    c->lane[8] += c->lane[10] ^ 0x5102da7eu;
    blend_state(c, &c->lane[5], 3);
    c->lane[0] += c->lane[15]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 12);
    c->lane[11] ^= sx_rl(c->lane[12], 14);
    c->hash ^= c->lane[15] + 0x9a4c3833u;
    c->raw[c->slo + (int)((t0 + 20419u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= load_page(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[2] += c->lane[10] ^ 0x6a83b060u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void reset_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += reap_arena(c, c->slo, c->sln);
    c->hash ^= c->lane[4] + 0x66b6e050u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xca) << 8;
    push_block(c, &c->lane[0], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x18b074f5u;
    c->lane[0] += c->lane[14]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 17);
    c->lane[6] += c->lane[8] ^ 0xda07455eu;
    t2 += (uint32_t)split_block_364(c);
    t0 ^= reset_rate(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t blend_region_264(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[13] ^= sx_rl(c->lane[7], 3);
    t0 ^= latch_view(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x75) << 16;
    c->hash ^= c->lane[4] + 0x35e036f1u;
    c->hash = (c->hash * 0x3505b0a1u) ^ sx_rr(c->hash, 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5bd220a5u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int probe_entry(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    slice_band(c, t0, t1);
    t1 ^= (uint32_t)fold_head(c, (uint8_t)(t0 >> 16), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x9462d829u;
    c->hash = (c->hash * 0x353a4bdbu) ^ sx_rr(c->hash, 26);
    c->lane[9] += c->lane[15]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 23);
    rotate_block(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x28) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x52) << 8;
    t0 ^= trim_digest(c, t1);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t trace_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)map_scope(c);
    t2 = (t2 ^ c->sum) * 0xd72ed835u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x8bfc9db1u;
    t2 += (uint32_t)merge_tail(c);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 21);
    c->lane[9] += c->lane[5]; c->lane[8] ^= c->lane[9]; c->lane[8] = sx_rl(c->lane[8], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8fd206b5u;
    t1 ^= (uint32_t)stage_table(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 61488u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t1 ^= (uint32_t)pick_bucket(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)rotate_head(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= sift_cursor(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t cache_head(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x054d1c5fu) ^ sx_rr(c->hash, 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64102u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41989u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 20);
    c->raw[c->slo + (int)((t0 + 32280u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= reset_rate(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static void wrap_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += settle_field(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash = (c->hash * 0x3fbb1fa5u) ^ sx_rr(c->hash, 6);
    push_block(c, &c->lane[4], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe7) << 16;
    c->lane[14] += c->lane[4] ^ 0xc6908049u;
    t2 += queue_value(c, c->slo, c->sln);
    t1 ^= (uint32_t)reap_count(c, (uint8_t)(t0 >> 0), t2);
    c->lane[11] += c->lane[8]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 29);
    blend_state(c, &c->lane[4], 4);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static void fill_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)settle_item(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcca8cf65u;
    t2 = (t2 ^ c->sum) * 0xf048462bu;
    c->raw[c->slo + (int)((t0 + 2563u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[5] += c->lane[12]; c->lane[10] ^= c->lane[5]; c->lane[10] = sx_rl(c->lane[10], 7);
    t1 ^= (uint32_t)fold_head(c, (uint8_t)(t0 >> 16), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void patch_cell(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 41390u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[6] + 0xc2a2410du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 ^= grow_head(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbb) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xae96af85u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x30) << 16;
    c->lane[13] += c->lane[0]; c->lane[7] ^= c->lane[13]; c->lane[7] = sx_rl(c->lane[7], 31);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t latch_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    tap_path(c, &c->lane[7], 3);
    t0 ^= slice_digest(c, t1);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe6) << 8;
    t2 += (uint32_t)settle_scope(c);
    c->sched[23] = c->hash ^ sx_rl(c->lane[10], 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x55) << 0;
    c->lane[2] += c->lane[12] ^ 0xa64c1ac6u;
    c->lane[5] += c->lane[0]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 29);
    t1 ^= (uint32_t)wrap_view(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void place_state_272(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] ^= sx_rl(c->lane[10], 11);
    c->hash = (c->hash * 0xd774967bu) ^ sx_rr(c->hash, 9);
    t1 ^= (uint32_t)chain_token(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xaa) << 0;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 19);
    c->raw[c->slo + (int)((t0 + 56914u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += push_cursor(c, c->rlo, c->rln);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 6);
    c->hash ^= c->lane[1] + 0x1478c87eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xcc) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int map_scope(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)emit_part(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37278u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x92ac57e3u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 17);
    c->lane[0] ^= sx_rl(c->lane[3], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17038u) % (uint32_t)c->rln)] << 24;
    c->lane[13] += c->lane[10] ^ 0xd597d4c5u;
    c->lane[10] += c->lane[13]; c->lane[4] ^= c->lane[10]; c->lane[4] = sx_rl(c->lane[4], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb9) << 0;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t fold_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] ^= sx_rl(c->lane[3], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[13] + 0xe6c5398eu;
    c->lane[15] += c->lane[5] ^ 0xc4e5cc8cu;
    t0 ^= clamp_head(c, t1);
    c->lane[3] += c->lane[7] ^ 0x8618861bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21341u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[7] + 0x50a0efd8u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[14] += c->lane[2]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 31);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t trim_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[15] ^= sx_rl(c->lane[13], 4);
    c->raw[c->slo + (int)((t0 + 39029u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x8217aaf1u) ^ sx_rr(c->hash, 28);
    t2 = (t2 ^ c->sum) * 0xdc10760fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15390u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t pick_bucket(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)link_layer(c);
    t2 += (uint32_t)scan_node(c);
    t2 += map_label(c, c->rlo, c->rln);
    c->sched[21] = c->hash ^ sx_rl(c->lane[0], 12);
    c->hash ^= c->lane[2] + 0x6c955327u;
    t2 += reap_arena(c, c->slo, c->sln);
    c->lane[7] += c->lane[10] ^ 0xf268c3c4u;
    c->hash ^= c->lane[13] + 0x89658b1cu;
    t2 += (uint32_t)latch_level(c);
    c->lane[1] += c->lane[9]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void rotate_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[12] += c->lane[9] ^ 0x4e322feeu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xfd002161u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe7) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3220b42du;
    c->lane[11] += c->lane[1]; c->lane[0] ^= c->lane[11]; c->lane[0] = sx_rl(c->lane[0], 24);
    t2 = (t2 ^ c->sum) * 0x0eb9c609u;
    c->lane[10] ^= sx_rl(c->lane[0], 4);
    c->hash ^= c->lane[15] + 0x135ab807u;
    t2 += merge_head(c, c->rlo, c->rln);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t reset_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 43447u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += (uint32_t)flush_slot(c);
    tap_path(c, &c->lane[9], 1);
    pair_bucket(c, &c->lane[4], 1);
    c->lane[9] += c->lane[0]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 31);
    c->lane[15] ^= sx_rl(c->lane[14], 17);
    t2 = (t2 ^ c->sum) * 0x25fcc413u;
    c->lane[0] += c->lane[2]; c->lane[4] ^= c->lane[0]; c->lane[4] = sx_rl(c->lane[4], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[12] += c->lane[15]; c->lane[8] ^= c->lane[12]; c->lane[8] = sx_rl(c->lane[8], 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54374u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void blend_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[9] += c->lane[4] ^ 0x9c9b5dd9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44720u) % (uint32_t)c->rln)] << 16;
    c->sched[13] = c->hash ^ sx_rl(c->lane[12], 2);
    c->hash = (c->hash * 0xf0044273u) ^ sx_rr(c->hash, 10);
    c->sched[22] = c->hash ^ sx_rl(c->lane[13], 11);
    c->sched[23] = c->hash ^ sx_rl(c->lane[0], 10);
    t2 = (t2 ^ c->sum) * 0x0ede5d6bu;
    c->lane[12] += c->lane[11] ^ 0x3baddaabu;
    t2 += (uint32_t)merge_tail(c);
    c->lane[15] += c->lane[3] ^ 0x04d878d6u;
    t2 += push_cell(c, c->slo, c->sln);
    c->lane[3] += c->lane[11]; c->lane[4] ^= c->lane[3]; c->lane[4] = sx_rl(c->lane[4], 27);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pack_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x86726313u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash = (c->hash * 0x1e5ba2e1u) ^ sx_rr(c->hash, 9);
    t2 += stage_ring(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[12] + 0x1291bd96u;
    t2 += (uint32_t)settle_scope(c);
    c->lane[2] += c->lane[10]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51749u) % (uint32_t)c->rln)] << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[3], 9);
    close_marker(c, &c->lane[3], 3);
    merge_offset(c, &c->lane[11], 3);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t place_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 21195u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += fold_cursor(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    queue_lease(c, &c->lane[8], 4);
    sync_key(c, &c->lane[8], 3);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11165u) % (uint32_t)c->rln)] << 8;
    t0 ^= move_layer(c, t1);
    c->lane[13] += c->lane[2]; c->lane[10] ^= c->lane[13]; c->lane[10] = sx_rl(c->lane[10], 14);
    c->lane[7] ^= sx_rl(c->lane[13], 9);
    t0 ^= drain_tuple_316(c, t1);
    c->lane[3] ^= sx_rl(c->lane[0], 21);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void push_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[13] += c->lane[2] ^ 0x2c2b84e6u;
    t1 ^= (uint32_t)sort_queue(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x23765443u;
    c->lane[9] += c->lane[9] ^ 0x5c6e690au;
    c->sched[31] = c->hash ^ sx_rl(c->lane[15], 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 ^= clamp_head(c, t1);
    t0 ^= mix_index(c, t1);
    merge_offset(c, &c->lane[0], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t sift_cursor(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 18);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash = (c->hash * 0x5e6249d7u) ^ sx_rr(c->hash, 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[9] += c->lane[1] ^ 0x281f1053u;
    sync_key(c, &c->lane[9], 1);
    t2 = (t2 ^ c->sum) * 0x931a3829u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t close_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27980u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17856u) % (uint32_t)c->rln)] << 16;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 31);
    c->raw[c->slo + (int)((t0 + 34655u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x754c21edu;
    t1 ^= (uint32_t)shift_index(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[13] + 0xbe16c63au;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26362u) % (uint32_t)c->rln)] << 24;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t settle_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x58eb44a5u;
    c->lane[10] += c->lane[4] ^ 0xef50928au;
    pick_entry(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x41cd36d3u;
    c->sched[16] = c->hash ^ sx_rl(c->lane[7], 14);
    t2 = (t2 ^ c->sum) * 0x7d6cc063u;
    t0 ^= mix_tail(c, t1);
    t2 += (uint32_t)scan_node(c);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    stage_line(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc65ab7e1u;
    c->raw[c->slo + (int)((t0 + 60451u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[30] = c->hash ^ sx_rl(c->lane[13], 23);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t rotate_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] ^= sx_rl(c->lane[10], 3);
    c->hash = (c->hash * 0x3a909139u) ^ sx_rr(c->hash, 4);
    c->raw[c->slo + (int)((t0 + 40655u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += queue_value(c, c->slo, c->sln);
    c->hash ^= c->lane[10] + 0x6ce4e2c3u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t reap_count(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2194u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xea) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2a5f01e9u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[9], 11);
    step_track(c, &c->lane[6], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[5] ^= sx_rl(c->lane[15], 10);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void slice_band(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 30);
    t2 += tap_track(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x87) << 16;
    c->hash ^= c->lane[0] + 0x8d702620u;
    t1 ^= (uint32_t)peek_region(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)rotate_label(c, (uint8_t)(t0 >> 8), t2);
    stage_limit(c, t0, t1);
    c->lane[8] ^= sx_rl(c->lane[10], 9);
    t2 += (uint32_t)sync_view(c);
    hold_limit(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void stage_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= mix_tail(c, t1);
    t2 = (t2 ^ c->sum) * 0x415d49c9u;
    c->raw[c->slo + (int)((t0 + 41073u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[28] = c->hash ^ sx_rl(c->lane[13], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 += (uint32_t)fetch_tail(c);
    queue_track(c, &c->lane[1], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int scan_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 19870u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[16] = c->hash ^ sx_rl(c->lane[9], 24);
    t1 ^= (uint32_t)coal_state(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 30);
    t2 += (uint32_t)place_queue(c);
    c->sched[7] = c->hash ^ sx_rl(c->lane[7], 8);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t fold_cursor(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] ^= sx_rl(c->lane[3], 20);
    c->lane[8] += c->lane[12]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 29);
    cache_part(c, &c->lane[10], 3);
    c->lane[11] += c->lane[5] ^ 0xe5cb0bb2u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 17);
    c->lane[1] ^= sx_rl(c->lane[4], 8);
    t2 = (t2 ^ c->sum) * 0xe0f13b1du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe0dfc1e9u;
    c->sched[22] = c->hash ^ sx_rl(c->lane[5], 16);
    c->raw[c->slo + (int)((t0 + 7583u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += (uint32_t)emit_record(c);
    c->lane[8] += c->lane[5] ^ 0xab8ed5eau;
    c->lane[0] += c->lane[3]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 15);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t queue_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 8);
    t1 ^= (uint32_t)move_index(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x87ea996fu;
    scan_bucket(c, t0, t1);
    c->sched[22] = c->hash ^ sx_rl(c->lane[4], 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x87577343u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t rotate_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9f4b24d3u;
    c->hash = (c->hash * 0xf708b2cfu) ^ sx_rr(c->hash, 20);
    fetch_value(c, t0, t1);
    c->lane[10] += c->lane[13] ^ 0x25f0b42bu;
    c->lane[11] += c->lane[3]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10356u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t1 ^= (uint32_t)wrap_view(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29837u) % (uint32_t)c->rln)] << 24;
    place_gap(c, &c->lane[7], 3);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 9);
    c->sched[20] = c->hash ^ sx_rl(c->lane[0], 18);
    c->sched[21] = c->hash ^ sx_rl(c->lane[8], 11);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int flush_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->sched[28] = c->hash ^ sx_rl(c->lane[13], 1);
    c->hash = (c->hash * 0xa51f1e2fu) ^ sx_rr(c->hash, 25);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 12);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 20);
    t2 += (uint32_t)reset_count(c);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 4);
    t0 ^= load_pool(c, t1);
    c->raw[c->slo + (int)((t0 + 31534u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6d) << 16;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 20);
    t0 ^= mix_index(c, t1);
    c->sched[26] = c->hash ^ sx_rl(c->lane[2], 10);
    swap_track(c, t0, t1);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int link_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->raw[c->slo + (int)((t0 + 31442u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= tune_page(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xce) << 0;
    c->hash ^= c->lane[3] + 0xc65cdbbau;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 25);
    c->lane[5] ^= sx_rl(c->lane[8], 23);
    pick_digest(c, &c->lane[0], 1);
    c->raw[c->slo + (int)((t0 + 21193u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int latch_level(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe5) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17630u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[1] + 0x536fcdb5u;
    c->hash ^= c->lane[15] + 0xd34d4dd3u;
    t2 += stage_count_374(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x413bdf19u;
    fetch_value(c, t0, t1);
    c->hash = (c->hash * 0xd12a7397u) ^ sx_rr(c->hash, 20);
    c->hash ^= c->lane[8] + 0x2e09ddd3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[1] ^= sx_rl(c->lane[11], 18);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void queue_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53378u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe4) << 8;
    t2 = (t2 ^ c->sum) * 0x427af3cbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1773u) % (uint32_t)c->rln)] << 16;
    c->lane[11] += c->lane[15]; c->lane[0] ^= c->lane[11]; c->lane[0] = sx_rl(c->lane[0], 31);
    c->sched[27] = c->hash ^ sx_rl(c->lane[8], 20);
    t2 += (uint32_t)map_rate(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa3c74355u;
    c->hash ^= c->lane[7] + 0x9e3bc4d3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t push_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    pick_digest(c, &c->lane[10], 2);
    t2 += (uint32_t)settle_item(c);
    t0 ^= tune_page(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x12) << 8;
    c->sched[31] = c->hash ^ sx_rl(c->lane[11], 15);
    t2 = (t2 ^ c->sum) * 0x4a22bda7u;
    c->hash ^= c->lane[1] + 0x20a9b920u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= c->lane[12] + 0x76f931deu;
    c->hash = (c->hash * 0x790fc5b9u) ^ sx_rr(c->hash, 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x07) << 0;
    c->lane[15] ^= sx_rl(c->lane[7], 25);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t push_cursor(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbfb7aca1u;
    map_state(c, &c->lane[7], 4);
    c->lane[10] += c->lane[11]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 27);
    c->hash ^= c->lane[1] + 0xe2eab625u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t move_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)resize_cell(c);
    c->hash ^= c->lane[1] + 0xa0f8a597u;
    c->lane[6] += c->lane[3]; c->lane[12] ^= c->lane[6]; c->lane[12] = sx_rl(c->lane[12], 29);
    c->hash = (c->hash * 0x744e035bu) ^ sx_rr(c->hash, 9);
    t2 += shift_digest(c, c->rlo, c->rln);
    c->lane[5] ^= sx_rl(c->lane[12], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4ea6f6d1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x3f71e203u;
    c->raw[c->slo + (int)((t0 + 50059u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[14] = c->hash ^ sx_rl(c->lane[5], 6);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pick_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    queue_store(c, &c->lane[0], 4);
    t1 ^= (uint32_t)sift_table(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[7] + 0xa4355678u;
    c->hash ^= c->lane[10] + 0x65d4d847u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[1] ^= sx_rl(c->lane[12], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4092u) % (uint32_t)c->rln)] << 0;
    c->lane[8] += c->lane[6] ^ 0x15f9192bu;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint32_t merge_head(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63918u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 61853u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= trace_marker(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[5] += c->lane[13] ^ 0x3b04550du;
    c->hash = (c->hash * 0x5655e75fu) ^ sx_rr(c->hash, 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void stage_limit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xaa) << 8;
    c->hash ^= c->lane[0] + 0xdbbb6b84u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37754u) % (uint32_t)c->rln)] << 24;
    t1 ^= (uint32_t)mark_stack(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x40) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x93) << 8;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t chain_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)hold_tail(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sched[8] = c->hash ^ sx_rl(c->lane[14], 15);
    c->hash ^= c->lane[11] + 0x4783cf50u;
    t0 ^= yield_list(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdf800c25u;
    c->raw[c->slo + (int)((t0 + 38989u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14320u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t reap_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] ^= sx_rl(c->lane[15], 23);
    t1 ^= (uint32_t)poll_window(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb0da2ebbu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x438720e9u;
    c->hash = (c->hash * 0x6aec0e7du) ^ sx_rr(c->hash, 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 6728u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)emit_record(c);
    relay_head(c, &c->lane[4], 2);
    t2 += (uint32_t)purge_part_361(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x717b0afdu;
    c->lane[2] += c->lane[12]; c->lane[15] ^= c->lane[2]; c->lane[15] = sx_rl(c->lane[15], 18);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t emit_part(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 18);
    c->lane[12] ^= sx_rl(c->lane[12], 7);
    c->lane[15] ^= sx_rl(c->lane[7], 15);
    c->hash = (c->hash * 0xb06844a5u) ^ sx_rr(c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[14] += c->lane[5] ^ 0x85adbf49u;
    c->hash = (c->hash * 0x01477587u) ^ sx_rr(c->hash, 21);
    relay_chunk(c, &c->lane[2], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9b) << 0;
    c->lane[6] ^= sx_rl(c->lane[5], 11);
    c->lane[13] ^= sx_rl(c->lane[9], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1241u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void pair_bucket(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    fold_group(c, t0, t1);
    c->hash = (c->hash * 0x3a4a9729u) ^ sx_rr(c->hash, 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18874u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 26437u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x565c335bu;
    c->lane[1] ^= sx_rl(c->lane[6], 7);
    stage_line(c, t0, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int merge_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9404u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 += stage_ring(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc8) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x55) << 0;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t peek_region(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] += c->lane[0] ^ 0x8d7ec33fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9a60ec57u;
    c->lane[4] += c->lane[14]; c->lane[6] ^= c->lane[4]; c->lane[6] = sx_rl(c->lane[6], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    latch_marker(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 16);
    c->hash = (c->hash * 0xf7697937u) ^ sx_rr(c->hash, 28);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void close_marker(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] += c->lane[1] ^ 0xb2c0449au;
    c->lane[4] += c->lane[13] ^ 0x42e373a1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63133u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x76) << 16;
    queue_lease(c, &c->lane[7], 3);
    t1 ^= (uint32_t)sort_queue(c, (uint8_t)(t0 >> 16), t2);
    c->raw[c->slo + (int)((t0 + 44266u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 52171u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x664dcb0du) ^ sx_rr(c->hash, 10);
    c->hash = (c->hash * 0x96c65d2fu) ^ sx_rr(c->hash, 12);
    c->lane[1] += c->lane[11] ^ 0xf8e57e8cu;
    t2 += (uint32_t)map_rate(c);
    c->hash = (c->hash * 0x6d93e7e3u) ^ sx_rr(c->hash, 5);
    t2 += (uint32_t)mix_port(c);
    c->raw[c->slo + (int)((t0 + 6678u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t mix_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x4535d241u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    step_stream(c, t0, t1);
    c->lane[12] += c->lane[5]; c->lane[13] ^= c->lane[12]; c->lane[13] = sx_rl(c->lane[13], 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27574u) % (uint32_t)c->rln)] << 8;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37758u) % (uint32_t)c->rln)] << 16;
    sort_band(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t grow_head(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63868u) % (uint32_t)c->rln)] << 8;
    c->lane[14] += c->lane[11]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 26);
    t2 = (t2 ^ c->sum) * 0x76478b01u;
    c->hash = (c->hash * 0x632d8bcbu) ^ sx_rr(c->hash, 16);
    c->raw[c->slo + (int)((t0 + 16676u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[10] += c->lane[5] ^ 0x01fcd1e8u;
    c->lane[1] ^= sx_rl(c->lane[4], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3dff95cbu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_offset(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[14] + 0x8bd61936u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xffbf67f1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6191u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int sync_view(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[28] = c->hash ^ sx_rl(c->lane[11], 10);
    shift_page(c, &c->lane[0], 1);
    c->lane[6] += c->lane[10]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 22);
    c->sched[21] = c->hash ^ sx_rl(c->lane[15], 7);
    c->lane[1] ^= sx_rl(c->lane[11], 19);
    c->hash = (c->hash * 0x9b62e48bu) ^ sx_rr(c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    fetch_frame(c, &c->lane[1], 3);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t clamp_head(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[15] += c->lane[14]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 6);
    t1 ^= (uint32_t)pack_count(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += (uint32_t)purge_arena(c);
    blend_level(c, t0, t1);
    c->lane[12] ^= sx_rl(c->lane[13], 28);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t drain_tuple_316(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xcef3a607u) ^ sx_rr(c->hash, 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54810u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8b) << 0;
    c->hash ^= c->lane[5] + 0x96264b16u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 14335u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xf374c471u;
    c->raw[c->slo + (int)((t0 + 24261u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xf00b2a23u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tap_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xb8026d19u) ^ sx_rr(c->hash, 17);
    t2 += (uint32_t)step_field(c);
    t2 += (uint32_t)map_unit(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb39c155fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x9092f34bu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 13);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t slice_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] ^= sx_rl(c->lane[11], 22);
    t1 ^= (uint32_t)peek_pairing_342(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x309280b5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9606971fu;
    t2 = (t2 ^ c->sum) * 0x91ab46dbu;
    t2 += (uint32_t)stage_rate(c);
    c->lane[12] += c->lane[5] ^ 0xf0c42907u;
    c->hash ^= c->lane[8] + 0xf513758eu;
    c->lane[0] += c->lane[15]; c->lane[2] ^= c->lane[0]; c->lane[2] = sx_rl(c->lane[2], 23);
    c->lane[11] += c->lane[4]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 21);
    t0 ^= probe_part(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t shift_index(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += rotate_batch(c, c->slo, c->sln);
    c->lane[9] += c->lane[3]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 9);
    t2 = (t2 ^ c->sum) * 0x639fb98fu;
    relay_record(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 46042u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 16524u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += (uint32_t)fetch_tail(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16101u) % (uint32_t)c->rln)] << 0;
    t0 ^= scan_digest(c, t1);
    c->lane[7] += c->lane[5] ^ 0x7d064392u;
    c->hash = (c->hash * 0xa5ed12f7u) ^ sx_rr(c->hash, 2);
    t1 ^= (uint32_t)stage_table(c, (uint8_t)(t0 >> 0), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void sync_key(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0c5616f3u;
    c->hash = (c->hash * 0x8aa0795fu) ^ sx_rr(c->hash, 6);
    c->hash = (c->hash * 0xcbe89c9fu) ^ sx_rr(c->hash, 23);
    c->hash = (c->hash * 0x8fd075e7u) ^ sx_rr(c->hash, 1);
    c->sched[8] = c->hash ^ sx_rl(c->lane[3], 19);
    c->lane[5] += c->lane[12]; c->lane[6] ^= c->lane[5]; c->lane[6] = sx_rl(c->lane[6], 21);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 13);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int settle_scope(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] += c->lane[11]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 ^= load_page(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd7dff0a5u;
    t2 = (t2 ^ c->sum) * 0xf38f5a8fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc7) << 0;
    c->hash = (c->hash * 0x18fb2d9fu) ^ sx_rr(c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x335b519bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25092u) % (uint32_t)c->rln)] << 16;
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf0) << 8;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tap_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[4]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 10);
    c->lane[11] ^= sx_rl(c->lane[7], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5ce7d18du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[9] = c->hash ^ sx_rl(c->lane[8], 5);
    c->lane[3] += c->lane[13]; c->lane[2] ^= c->lane[3]; c->lane[2] = sx_rl(c->lane[2], 23);
    c->lane[4] ^= sx_rl(c->lane[14], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x20) << 0;
    c->hash ^= c->lane[4] + 0x2e678835u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void hold_limit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] += c->lane[10]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 17);
    c->lane[8] += c->lane[0]; c->lane[14] ^= c->lane[8]; c->lane[14] = sx_rl(c->lane[14], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd3) << 16;
    t2 += pack_stack(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x62) << 16;
    t2 += (uint32_t)split_block_364(c);
    c->sched[23] = c->hash ^ sx_rl(c->lane[14], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf8d5970du;
    c->lane[0] += c->lane[13]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 11);
    t2 = (t2 ^ c->sum) * 0xb5acffdbu;
    c->lane[6] ^= sx_rl(c->lane[6], 1);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void step_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 11);
    c->hash ^= c->lane[7] + 0xc3839fc2u;
    c->lane[6] ^= sx_rl(c->lane[7], 29);
    c->lane[14] += c->lane[5]; c->lane[8] ^= c->lane[14]; c->lane[8] = sx_rl(c->lane[8], 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x4443cac1u;
    t2 += map_label(c, c->rlo, c->rln);
    c->lane[10] += c->lane[0]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 30);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int fetch_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4a) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd151aacbu;
    c->raw[c->slo + (int)((t0 + 38215u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x18) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50152u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16427u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1e) << 0;
    c->lane[11] += c->lane[7]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->hash = (c->hash * 0x5ec1ffdbu) ^ sx_rr(c->hash, 17);
    t2 = (t2 ^ c->sum) * 0x193cc6a5u;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t trace_marker(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xa06f26adu) ^ sx_rr(c->hash, 15);
    c->hash ^= c->lane[6] + 0xeb2e3c48u;
    t2 = (t2 ^ c->sum) * 0x1a7b7bffu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x59) << 8;
    t2 = (t2 ^ c->sum) * 0xd9b2f8b5u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int purge_arena(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40959u) % (uint32_t)c->rln)] << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[5], 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb4) << 8;
    c->lane[13] += c->lane[7]; c->lane[11] ^= c->lane[13]; c->lane[11] = sx_rl(c->lane[11], 2);
    c->lane[14] += c->lane[11] ^ 0xd8d861e0u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[1] + 0x7354d11eu;
    c->hash ^= c->lane[3] + 0x30cc97aau;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void stage_line(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa36b8b71u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36616u) % (uint32_t)c->rln)] << 0;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int hold_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->hash = (c->hash * 0xcc8dee69u) ^ sx_rr(c->hash, 15);
    c->sched[20] = c->hash ^ sx_rl(c->lane[5], 29);
    c->hash ^= c->lane[11] + 0xffb8eed3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[14] += c->lane[10]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 1);
    c->sched[21] = c->hash ^ sx_rl(c->lane[1], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19132u) % (uint32_t)c->rln)] << 0;
    c->lane[0] ^= sx_rl(c->lane[15], 21);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void place_gap(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 2);
    c->lane[5] ^= sx_rl(c->lane[2], 3);
    c->lane[5] += c->lane[9]; c->lane[2] ^= c->lane[5]; c->lane[2] = sx_rl(c->lane[2], 24);
    c->lane[3] += c->lane[1]; c->lane[5] ^= c->lane[3]; c->lane[5] = sx_rl(c->lane[5], 23);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 18);
    c->hash = (c->hash * 0x19f76615u) ^ sx_rr(c->hash, 31);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t mark_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] += c->lane[8] ^ 0xc283d9e1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 11);
    c->raw[c->slo + (int)((t0 + 37753u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t wrap_view(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35970u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[2] + 0x16218522u;
    c->raw[c->slo + (int)((t0 + 44501u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t pack_count(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[5] += c->lane[3]; c->lane[13] ^= c->lane[5]; c->lane[13] = sx_rl(c->lane[13], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[14] += c->lane[0] ^ 0x2df16517u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf7) << 0;
    c->lane[15] ^= sx_rl(c->lane[9], 17);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 30);
    t2 = (t2 ^ c->sum) * 0xafeb9b2fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x298ac6a1u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t move_index(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[3] ^= sx_rl(c->lane[11], 3);
    c->hash = (c->hash * 0x9685eeb5u) ^ sx_rr(c->hash, 2);
    c->sched[26] = c->hash ^ sx_rl(c->lane[14], 10);
    c->hash = (c->hash * 0xc0e403ddu) ^ sx_rr(c->hash, 8);
    c->lane[2] += c->lane[11] ^ 0x95e39284u;
    t2 = (t2 ^ c->sum) * 0xf91bce9fu;
    c->lane[9] ^= sx_rl(c->lane[4], 4);
    c->hash = (c->hash * 0x283f81ffu) ^ sx_rr(c->hash, 10);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 29);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t scan_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[7] += c->lane[2]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 7);
    c->lane[4] += c->lane[10]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 30);
    c->hash ^= c->lane[12] + 0x386fe9b1u;
    t2 = (t2 ^ c->sum) * 0xe7a6bb8du;
    c->lane[7] += c->lane[5] ^ 0x1a055ab0u;
    c->lane[14] += c->lane[15] ^ 0x5fbee7d8u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int reset_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[14] ^= sx_rl(c->lane[0], 17);
    c->lane[7] += c->lane[0] ^ 0x1ee5d780u;
    c->hash = (c->hash * 0x24bc4217u) ^ sx_rr(c->hash, 7);
    t2 = (t2 ^ c->sum) * 0x4ba4a1bfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x26) << 16;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t load_pool(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 42959u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash = (c->hash * 0x0dcda78bu) ^ sx_rr(c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x398e2fd9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[3] = c->hash ^ sx_rl(c->lane[13], 18);
    c->hash ^= c->lane[2] + 0xf35116dfu;
    c->lane[10] += c->lane[6]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 15);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int resize_cell(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50463u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15639u) % (uint32_t)c->rln)] << 0;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 26);
    c->lane[4] ^= sx_rl(c->lane[2], 22);
    c->hash = (c->hash * 0x56620211u) ^ sx_rr(c->hash, 14);
    t2 = (t2 ^ c->sum) * 0xe7b5e9edu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61261u) % (uint32_t)c->rln)] << 16;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t yield_list(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[15] = c->hash ^ sx_rl(c->lane[7], 16);
    c->lane[0] += c->lane[8]; c->lane[2] ^= c->lane[0]; c->lane[2] = sx_rl(c->lane[2], 20);
    c->lane[15] += c->lane[9] ^ 0xff8bddc4u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 11);
    c->lane[9] += c->lane[14]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 27);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x91) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[1] += c->lane[15] ^ 0x2601f310u;
    c->hash = (c->hash * 0x2b5382b3u) ^ sx_rr(c->hash, 6);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int mix_port(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 32497u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[9] = c->hash ^ sx_rl(c->lane[12], 3);
    t2 = (t2 ^ c->sum) * 0x74d0aae5u;
    c->hash = (c->hash * 0x07e5319fu) ^ sx_rr(c->hash, 8);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void queue_lease(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[2] + 0xe4237e60u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8ba09ff7u;
    c->raw[c->slo + (int)((t0 + 28122u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[12] = c->hash ^ sx_rl(c->lane[14], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x608d69fdu;
    c->sched[0] = c->hash ^ sx_rl(c->lane[6], 14);
    c->lane[3] ^= sx_rl(c->lane[11], 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x62) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t peek_pairing_342(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 56490u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[12] += c->lane[10] ^ 0xe1abfdfcu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[8] ^= sx_rl(c->lane[5], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[9] = c->hash ^ sx_rl(c->lane[1], 3);
    c->raw[c->slo + (int)((t0 + 26436u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[4] + 0x392dfad8u;
    c->hash ^= c->lane[8] + 0x2e7d3ab7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t poll_window(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x546b68d1u) ^ sx_rr(c->hash, 23);
    c->hash ^= c->lane[0] + 0xaac27563u;
    t2 = (t2 ^ c->sum) * 0x0c4d2e2du;
    c->raw[c->slo + (int)((t0 + 28162u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[28] = c->hash ^ sx_rl(c->lane[9], 1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void scan_bucket(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc71e8defu;
    c->lane[14] ^= sx_rl(c->lane[12], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x85d3aacfu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 21);
    t2 = (t2 ^ c->sum) * 0xda4ffefbu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t tune_page(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64710u) % (uint32_t)c->rln)] << 0;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 23);
    c->sched[3] = c->hash ^ sx_rl(c->lane[5], 22);
    c->raw[c->slo + (int)((t0 + 39215u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[9] += c->lane[4]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void shift_page(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 6286u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x72) << 0;
    c->lane[0] += c->lane[2]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x550881f7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xe6792397u;
    c->hash ^= c->lane[12] + 0x4c4fb475u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void pick_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xdd80cacdu;
    c->lane[10] += c->lane[9]; c->lane[13] ^= c->lane[10]; c->lane[13] = sx_rl(c->lane[13], 13);
    c->lane[7] += c->lane[4]; c->lane[10] ^= c->lane[7]; c->lane[10] = sx_rl(c->lane[10], 25);
    c->lane[10] ^= sx_rl(c->lane[10], 29);
    c->sched[4] = c->hash ^ sx_rl(c->lane[8], 27);
    c->lane[12] ^= sx_rl(c->lane[14], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sched[22] = c->hash ^ sx_rl(c->lane[12], 21);
    c->raw[c->slo + (int)((t0 + 55755u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int place_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->lane[0] += c->lane[2]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50337u) % (uint32_t)c->rln)] << 8;
    c->sched[18] = c->hash ^ sx_rl(c->lane[15], 29);
    c->lane[5] += c->lane[12]; c->lane[3] ^= c->lane[5]; c->lane[3] = sx_rl(c->lane[3], 25);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mix_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[3] + 0xb1b92cbau;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x3717d9b9u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void cache_part(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa330dbe3u;
    c->raw[c->slo + (int)((t0 + 24766u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf93dadbbu;
    c->lane[1] += c->lane[9] ^ 0xe36766d4u;
    c->hash = (c->hash * 0x78c3807fu) ^ sx_rr(c->hash, 28);
    c->lane[8] ^= sx_rl(c->lane[5], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53087u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t load_page(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[8] = c->hash ^ sx_rl(c->lane[14], 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61929u) % (uint32_t)c->rln)] << 16;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 1);
    c->lane[12] += c->lane[13]; c->lane[11] ^= c->lane[12]; c->lane[11] = sx_rl(c->lane[11], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 23122u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4b020107u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void map_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] += c->lane[8]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 29);
    t2 = (t2 ^ c->sum) * 0x50925d61u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void queue_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[21] = c->hash ^ sx_rl(c->lane[15], 31);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 23);
    c->lane[1] += c->lane[5]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 30);
    c->lane[13] ^= sx_rl(c->lane[4], 5);
    c->raw[c->slo + (int)((t0 + 21899u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x683656bbu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void relay_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5c3ac0afu;
    c->sched[3] = c->hash ^ sx_rl(c->lane[4], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static int step_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[5] + 0x7a622a93u;
    c->hash ^= c->lane[0] + 0x1e291fffu;
    c->hash ^= c->lane[1] + 0x03f9628cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7f) << 0;
    c->sched[3] = c->hash ^ sx_rl(c->lane[4], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[10] ^= sx_rl(c->lane[10], 13);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 27);
    c->sched[30] = c->hash ^ sx_rl(c->lane[9], 20);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int emit_record(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[8] += c->lane[5]; c->lane[3] ^= c->lane[8]; c->lane[3] = sx_rl(c->lane[3], 8);
    c->hash = (c->hash * 0x2f4c3287u) ^ sx_rr(c->hash, 3);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xeda9b079u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61927u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc93a72c3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[0] + 0x9a9cee9fu;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void blend_level(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x49f4f76fu) ^ sx_rr(c->hash, 16);
    c->lane[6] ^= sx_rl(c->lane[10], 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 21957u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52024u) % (uint32_t)c->rln)] << 16;
    c->lane[0] += c->lane[15] ^ 0xdbe09a35u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46324u) % (uint32_t)c->rln)] << 8;
    c->lane[0] ^= sx_rl(c->lane[2], 14);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static int map_rate(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[8] += c->lane[3] ^ 0x37314ea7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x125eb357u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe9ebdeddu;
    c->hash ^= c->lane[4] + 0x6cab5253u;
    c->hash ^= c->lane[14] + 0xdb6fca82u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[16] = c->hash ^ sx_rl(c->lane[5], 15);
    c->hash ^= c->lane[7] + 0x87472a0au;
    c->lane[15] += c->lane[12] ^ 0xf3171f3eu;
    c->hash ^= c->lane[0] + 0x96e80d43u;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t pack_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf84d50f3u;
    c->raw[c->slo + (int)((t0 + 53108u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x2fc680c3u) ^ sx_rr(c->hash, 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 12563u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 22207u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t map_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[27] = c->hash ^ sx_rl(c->lane[13], 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x17) << 0;
    c->hash ^= c->lane[9] + 0x9129a209u;
    c->hash ^= c->lane[4] + 0xc5f18632u;
    c->sum += t1;
    return t0 + t2;
}

static int purge_part_361(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xde9287f1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x59) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fetch_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf32d14d7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x69) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24364u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t probe_part(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[10] ^ 0x3d56d981u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57922u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x4943f844u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 21);
    c->sched[31] = c->hash ^ sx_rl(c->lane[6], 15);
    c->hash = (c->hash * 0xa5385e51u) ^ sx_rr(c->hash, 24);
    c->lane[9] += c->lane[5] ^ 0x04262bccu;
    c->hash ^= c->lane[9] + 0x3f6bde2au;
    c->lane[11] += c->lane[3] ^ 0xe3a535b0u;
    t2 += (uint32_t)reap_stream(c);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int split_block_364(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xf2b688ffu;
    c->lane[13] += c->lane[9] ^ 0x301aeb6cu;
    c->lane[2] += c->lane[7]; c->lane[15] ^= c->lane[2]; c->lane[15] = sx_rl(c->lane[15], 3);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 25);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t coal_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] ^= sx_rl(c->lane[13], 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11522u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3737cc8du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] += c->lane[0]; c->lane[1] ^= c->lane[6]; c->lane[1] = sx_rl(c->lane[1], 27);
    c->lane[8] += c->lane[0]; c->lane[1] ^= c->lane[8]; c->lane[1] = sx_rl(c->lane[1], 27);
    t2 = (t2 ^ c->sum) * 0xff8b4e73u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42954u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 1842u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void relay_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 16175u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] += c->lane[2] ^ 0x4bef6ce5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[4] ^= sx_rl(c->lane[15], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x2c) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void relay_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] += c->lane[12]; c->lane[4] ^= c->lane[11]; c->lane[4] = sx_rl(c->lane[4], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] ^= sx_rl(c->lane[13], 22);
    c->hash ^= c->lane[1] + 0x01a5b1aau;
    c->lane[0] += c->lane[1]; c->lane[4] ^= c->lane[0]; c->lane[4] = sx_rl(c->lane[4], 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int map_unit(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->lane[2] += c->lane[6]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 8);
    c->lane[15] += c->lane[2]; c->lane[11] ^= c->lane[15]; c->lane[11] = sx_rl(c->lane[11], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50176u) % (uint32_t)c->rln)] << 24;
    c->lane[10] += c->lane[4]; c->lane[9] ^= c->lane[10]; c->lane[9] = sx_rl(c->lane[9], 26);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[7] += c->lane[0] ^ 0x68b92644u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[1] += c->lane[4] ^ 0x700afa89u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void step_stream(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[1] ^= sx_rl(c->lane[11], 7);
    c->lane[0] += c->lane[2] ^ 0x1c78c960u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0x387aad05u) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x03) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5019857fu;
    c->hash = (c->hash * 0x935a0f7bu) ^ sx_rr(c->hash, 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9bd21a89u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65184u) % (uint32_t)c->rln)] << 24;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint8_t sort_queue(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23325u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x91) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x42) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcfd67d89u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 11);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int settle_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->lane[4] += c->lane[8] ^ 0xdf486e7eu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x6b793ee1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x187822d7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t stage_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb0) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 14627u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[0] += c->lane[14] ^ 0xbf02e4abu;
    c->hash ^= c->lane[7] + 0x99ebcf94u;
    c->hash ^= c->lane[13] + 0xd0c7abe0u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[14] += c->lane[3]; c->lane[7] ^= c->lane[14]; c->lane[7] = sx_rl(c->lane[7], 14);
    c->lane[15] += c->lane[7] ^ 0xb0d77de4u;
    c->lane[12] += c->lane[9] ^ 0xe02af01au;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void swap_track(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xfeef1a5du) ^ sx_rr(c->hash, 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x4208dbb1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39078u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[12] + 0x270194f8u;
    c->lane[15] += c->lane[11] ^ 0x7ffb1b7bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6352u) % (uint32_t)c->rln)] << 16;
    c->sched[20] = c->hash ^ sx_rl(c->lane[14], 4);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 12);
    c->hash = (c->hash * 0xda5acf33u) ^ sx_rr(c->hash, 16);
    c->lane[15] += c->lane[4] ^ 0xdb0d5ba4u;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t stage_count_374(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[9] + 0xc647d761u;
    c->lane[6] += c->lane[11] ^ 0xea65fe44u;
    c->lane[1] += c->lane[3]; c->lane[0] ^= c->lane[1]; c->lane[0] = sx_rl(c->lane[0], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52492u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x8b135261u) ^ sx_rr(c->hash, 28);
    c->sched[1] = c->hash ^ sx_rl(c->lane[7], 21);
    c->lane[5] ^= sx_rl(c->lane[9], 14);
    c->hash ^= c->lane[14] + 0x7ddaeb68u;
    c->sum += t1;
    return t0 + t2;
}

static void sort_band(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 28);
    c->lane[14] += c->lane[7] ^ 0x16786a23u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 16);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48341u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 12813u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[3]; c->lane[10] ^= c->lane[5]; c->lane[10] = sx_rl(c->lane[10], 26);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static void fetch_value(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[30] = c->hash ^ sx_rl(c->lane[4], 27);
    c->lane[4] += c->lane[1] ^ 0x1dac4be8u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24759u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[5] += c->lane[3] ^ 0xa73469fau;
    c->raw[c->slo + (int)((t0 + 8138u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void latch_marker(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[10], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x78a75dd5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46885u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x079c61b5u;
    c->lane[7] ^= sx_rl(c->lane[12], 13);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 7);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 5);
    c->hash = (c->hash * 0xd76d462du) ^ sx_rr(c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x29d0cd3bu;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint32_t stage_ring(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 18789u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x96ba47fdu) ^ sx_rr(c->hash, 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7f) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x60) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7b) << 16;
    t2 = (t2 ^ c->sum) * 0x2f97265du;
    c->lane[9] ^= sx_rl(c->lane[8], 16);
    c->raw[c->slo + (int)((t0 + 30103u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 19);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t rotate_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50080u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8e) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4516u) % (uint32_t)c->rln)] << 0;
    c->lane[11] += c->lane[3]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 11);
    c->lane[5] += c->lane[9]; c->lane[3] ^= c->lane[5]; c->lane[3] = sx_rl(c->lane[3], 6);
    c->raw[c->slo + (int)((t0 + 14581u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 8);
    c->lane[10] ^= sx_rl(c->lane[3], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xaa6aa889u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t shift_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5649u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x22ed398bu;
    t2 = (t2 ^ c->sum) * 0x9f7e6ddbu;
    c->lane[5] ^= sx_rl(c->lane[3], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23434u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x699c6af3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x1dd97081u) ^ sx_rr(c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x163043f5u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t sift_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 26);
    c->lane[9] += c->lane[0]; c->lane[14] ^= c->lane[9]; c->lane[14] = sx_rl(c->lane[14], 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31243u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb6152dd5u;
    c->lane[6] ^= sx_rl(c->lane[6], 14);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xea) << 16;
    c->hash = (c->hash * 0x5c2ab0e7u) ^ sx_rr(c->hash, 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int stage_rate(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= c->lane[4] + 0x21eaabb8u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x91) << 0;
    c->raw[c->slo + (int)((t0 + 30577u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x97b08b2bu) ^ sx_rr(c->hash, 15);
    c->raw[c->slo + (int)((t0 + 64627u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[14] ^ 0xfc00344du;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_group(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe6e2f00fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x14) << 16;
    c->hash = (c->hash * 0x07e0b21bu) ^ sx_rr(c->hash, 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x3b) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 12540u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void slice_index(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x135c4c25u;
    t2 += shift_batch(c, c->slo, c->sln);
    t1 ^= (uint32_t)slice_path(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= sort_state(c, t1);
    reap_record(c, t0, t1);
    t1 ^= (uint32_t)stage_label(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)reset_token(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)tap_window_413(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= c->lane[4] + 0xdbb78cc2u;
    c->lane[4] ^= sx_rl(c->lane[7], 18);
    merge_page_487(c, t0, t1);
    t1 ^= (uint32_t)pack_state(c, (uint8_t)(t0 >> 0), t2);
    t2 += scan_digest_483(c, c->rlo, c->rln);
    t2 += (uint32_t)shift_region(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[13] ^= sx_rl(c->lane[9], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x02) << 8;
    step_group(c, &c->lane[9], 4);
    join_list(c, t0, t1);
    t0 ^= hold_level(c, t1);
    c->lane[11] ^= sx_rl(c->lane[2], 6);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t hold_level(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += tap_index(c, c->slo, c->sln);
    parse_band_445(c, &c->lane[2], 2);
    defer_bound(c, &c->lane[0], 3);
    c->lane[3] ^= sx_rl(c->lane[9], 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[11] += c->lane[12]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 22);
    t0 ^= stage_span(c, t1);
    t1 ^= (uint32_t)poll_line(c, (uint8_t)(t0 >> 16), t2);
    wrap_page(c, t0, t1);
    step_run(c, t0, t1);
    t0 ^= patch_marker(c, t1);
    t0 ^= hold_label(c, t1);
    emit_limit(c, &c->lane[4], 2);
    t1 ^= (uint32_t)defer_count(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= peek_entry(c, t1);
    t2 += mix_seat(c, c->slo, c->sln);
    c->lane[2] += c->lane[15] ^ 0x04d8cd91u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t stage_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= coal_track(c, t1);
    t1 ^= (uint32_t)mix_segment(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xcf820cf5u) ^ sx_rr(c->hash, 22);
    t2 += (uint32_t)fill_run(c);
    coal_chunk(c, &c->lane[6], 4);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 2);
    c->hash ^= c->lane[3] + 0xf2fff03fu;
    t2 += drain_track(c, c->rlo, c->rln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void join_list(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)stage_marker(c, (uint8_t)(t0 >> 0), t2);
    c->sched[9] = c->hash ^ sx_rl(c->lane[8], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21863u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 += (uint32_t)cache_page(c);
    c->lane[6] += c->lane[12]; c->lane[1] ^= c->lane[6]; c->lane[1] = sx_rl(c->lane[1], 26);
    parse_cursor(c, &c->lane[2], 3);
    t1 ^= (uint32_t)link_level(c, (uint8_t)(t0 >> 8), t2);
    t2 += settle_marker(c, c->rlo, c->rln);
    t2 += chain_unit(c, c->rlo, c->rln);
    t0 ^= parse_pool(c, t1);
    t2 += probe_state(c, c->slo, c->sln);
    shift_delta(c, &c->lane[5], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24056u) % (uint32_t)c->rln)] << 16;
    c->lane[5] += c->lane[0] ^ 0x6eec0780u;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void emit_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[2], 20);
    t2 = (t2 ^ c->sum) * 0x4450e05fu;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 19);
    c->hash = (c->hash * 0x519c9e07u) ^ sx_rr(c->hash, 6);
    c->hash = (c->hash * 0x760d1031u) ^ sx_rr(c->hash, 26);
    c->lane[4] += c->lane[14]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 23);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t chain_unit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xa5d09e0du) ^ sx_rr(c->hash, 31);
    t1 ^= (uint32_t)poll_line(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5d) << 0;
    t2 = (t2 ^ c->sum) * 0x1bca54cbu;
    t2 = (t2 ^ c->sum) * 0x96da6783u;
    t0 ^= link_block(c, t1);
    grow_rate(c, &c->lane[6], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59960u) % (uint32_t)c->rln)] << 24;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 20);
    tap_index_440(c, t0, t1);
    sort_stream(c, t0, t1);
    t2 += (uint32_t)shift_table(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xaf5c61bfu;
    t2 += queue_ring_432(c, c->slo, c->sln);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tap_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)peek_track(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23217u) % (uint32_t)c->rln)] << 16;
    c->lane[9] ^= sx_rl(c->lane[12], 12);
    t0 ^= mark_entry(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x85e689e3u;
    c->sched[6] = c->hash ^ sx_rl(c->lane[2], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe573a353u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void parse_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)purge_port(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3440e3f1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= c->lane[2] + 0x420a1e60u;
    trim_group(c, &c->lane[10], 3);
    t2 = (t2 ^ c->sum) * 0x62be6da5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[7] += c->lane[8] ^ 0xe8e814a2u;
    t0 ^= patch_marker(c, t1);
    c->hash ^= c->lane[1] + 0xc6da699cu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t link_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1c79d6cdu;
    t1 ^= (uint32_t)mix_label(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0xe5371b0fu) ^ sx_rr(c->hash, 16);
    c->raw[c->slo + (int)((t0 + 1717u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    sift_part(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    store_marker(c, &c->lane[8], 1);
    c->raw[c->slo + (int)((t0 + 3056u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += drain_track(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x347a1327u;
    t2 += resize_span(c, c->slo, c->sln);
    stage_frame(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfe21c0a5u;
    place_mask(c, &c->lane[6], 1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int cache_page(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    mix_mask(c, &c->lane[6], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] ^= sx_rl(c->lane[9], 2);
    t0 ^= mark_entry(c, t1);
    c->hash = (c->hash * 0x0f044411u) ^ sx_rr(c->hash, 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0d) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63424u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48351u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x032a7bc1u;
    c->raw[c->slo + (int)((t0 + 44863u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[3] + 0xfdc06541u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x966f4cf7u;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void shift_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[8] ^= sx_rl(c->lane[12], 2);
    c->lane[1] ^= sx_rl(c->lane[15], 24);
    t2 = (t2 ^ c->sum) * 0xf884f563u;
    c->sched[3] = c->hash ^ sx_rl(c->lane[7], 9);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 10);
    c->lane[15] += c->lane[11]; c->lane[13] ^= c->lane[15]; c->lane[13] = sx_rl(c->lane[13], 25);
    grow_rate(c, &c->lane[0], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15140u) % (uint32_t)c->rln)] << 16;
    t1 ^= (uint32_t)move_pool(c, (uint8_t)(t0 >> 8), t2);
    c->sched[27] = c->hash ^ sx_rl(c->lane[7], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3e3c8f39u;
    t2 = (t2 ^ c->sum) * 0xf222108fu;
    c->sched[27] = c->hash ^ sx_rl(c->lane[4], 17);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t mix_segment(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= wrap_path(c, t1);
    c->hash ^= c->lane[2] + 0x1db66cf8u;
    chain_node(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[14] += c->lane[5]; c->lane[1] ^= c->lane[14]; c->lane[1] = sx_rl(c->lane[1], 30);
    t0 ^= seek_row_442(c, t1);
    t1 ^= (uint32_t)tap_window_413(c, (uint8_t)(t0 >> 0), t2);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 11);
    c->hash ^= c->lane[2] + 0xf4f46a6eu;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[11] ^= sx_rl(c->lane[2], 7);
    c->lane[1] ^= sx_rl(c->lane[10], 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65118u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 23793u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += clamp_marker(c, c->slo, c->sln);
    t2 += (uint32_t)reap_stream(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t probe_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)coal_range(c);
    t1 ^= (uint32_t)scan_line(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)fill_run(c);
    t2 = (t2 ^ c->sum) * 0x54dd305fu;
    c->lane[0] += c->lane[15] ^ 0x45b2145du;
    c->sched[5] = c->hash ^ sx_rl(c->lane[11], 28);
    c->lane[8] += c->lane[2] ^ 0xeb55ed3au;
    t2 = (t2 ^ c->sum) * 0x543ad13fu;
    c->sum += t1;
    return t0 + t2;
}

static void defer_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 16171u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 1);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 24);
    t2 += (uint32_t)store_segment(c);
    t2 += (uint32_t)peek_track(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xaf) << 16;
    c->hash = (c->hash * 0xde3a2c31u) ^ sx_rr(c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 += rotate_store(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += (uint32_t)probe_unit(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int shift_table(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 11);
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 6);
    t0 ^= sort_state(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x06) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t1 ^= (uint32_t)defer_count(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x3bce3947u;
    t2 = (t2 ^ c->sum) * 0x68d37d15u;
    step_group(c, &c->lane[9], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15238u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0x7cdd005du) ^ sx_rr(c->hash, 1);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int coal_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash = (c->hash * 0x4023acfbu) ^ sx_rr(c->hash, 4);
    blend_delta(c, &c->lane[2], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22952u) % (uint32_t)c->rln)] << 24;
    c->sched[3] = c->hash ^ sx_rl(c->lane[10], 1);
    t2 += trace_batch_468(c, c->slo, c->sln);
    c->lane[3] += c->lane[13]; c->lane[9] ^= c->lane[3]; c->lane[9] = sx_rl(c->lane[9], 25);
    c->lane[4] += c->lane[10]; c->lane[1] ^= c->lane[4]; c->lane[1] = sx_rl(c->lane[1], 17);
    merge_token(c, &c->lane[0], 3);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void stage_frame(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 18);
    c->raw[c->slo + (int)((t0 + 377u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x8404d707u) ^ sx_rr(c->hash, 22);
    poll_slot(c, &c->lane[11], 2);
    t2 = (t2 ^ c->sum) * 0x3eb32e95u;
    c->hash = (c->hash * 0x566dec57u) ^ sx_rr(c->hash, 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x32) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x34) << 16;
    c->lane[5] += c->lane[13] ^ 0xfab421e1u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 25);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static int store_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->raw[c->slo + (int)((t0 + 10352u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)split_port(c, (uint8_t)(t0 >> 16), t2);
    c->lane[5] += c->lane[1] ^ 0x2d40af7bu;
    c->lane[9] += c->lane[12] ^ 0x1c8ff8a1u;
    push_value(c, &c->lane[10], 4);
    c->lane[8] += c->lane[12]; c->lane[14] ^= c->lane[8]; c->lane[14] = sx_rl(c->lane[14], 9);
    t2 += queue_ring_432(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x4a7e021du) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x91515f11u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t drain_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[31] = c->hash ^ sx_rl(c->lane[1], 16);
    t1 ^= (uint32_t)slice_path(c, (uint8_t)(t0 >> 8), t2);
    parse_band_445(c, &c->lane[3], 3);
    shift_slot(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17139u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[1] + 0x79d62541u;
    t2 += (uint32_t)tap_unit(c);
    c->hash ^= c->lane[11] + 0xd5a2d01bu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t mark_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x4792e80bu) ^ sx_rr(c->hash, 15);
    c->lane[2] += c->lane[6]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 17);
    c->lane[12] += c->lane[4]; c->lane[8] ^= c->lane[12]; c->lane[8] = sx_rl(c->lane[8], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc3) << 0;
    c->hash ^= c->lane[0] + 0x65a9cd34u;
    c->hash = (c->hash * 0xe300731du) ^ sx_rr(c->hash, 24);
    t2 = (t2 ^ c->sum) * 0x0b1d8c7bu;
    swap_list(c, &c->lane[6], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x009f417du;
    c->hash ^= c->lane[11] + 0x52be0207u;
    place_scope(c, &c->lane[2], 4);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void grow_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 48342u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x76d3d279u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[28] = c->hash ^ sx_rl(c->lane[11], 18);
    t0 ^= parse_pool(c, t1);
    t0 ^= link_block(c, t1);
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 22);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t wrap_path(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    t2 += rotate_store(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x672db4f5u;
    probe_scope(c, &c->lane[4], 4);
    c->lane[9] += c->lane[4] ^ 0xf6aa2689u;
    t0 ^= map_token(c, t1);
    c->hash = (c->hash * 0xabba1e57u) ^ sx_rr(c->hash, 30);
    c->lane[6] += c->lane[1] ^ 0x1f948136u;
    t1 ^= (uint32_t)split_key(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)tune_chunk(c);
    c->lane[5] += c->lane[2]; c->lane[11] ^= c->lane[5]; c->lane[11] = sx_rl(c->lane[11], 5);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t mix_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] ^= sx_rl(c->lane[1], 2);
    c->sched[31] = c->hash ^ sx_rl(c->lane[9], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[1] += c->lane[14] ^ 0xdea5af9eu;
    c->lane[6] += c->lane[13] ^ 0xdcecc9d7u;
    c->lane[10] += c->lane[9] ^ 0xc90eb956u;
    c->lane[8] ^= sx_rl(c->lane[14], 24);
    c->lane[4] += c->lane[15]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 5);
    t1 ^= (uint32_t)reset_token(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)trim_label(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x46) << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void trim_group(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[6] = c->hash ^ sx_rl(c->lane[9], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39834u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x5dab5235u;
    c->raw[c->slo + (int)((t0 + 40093u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xdc1f8f21u) ^ sx_rr(c->hash, 25);
    c->hash ^= c->lane[1] + 0x17015ca5u;
    c->lane[9] ^= sx_rl(c->lane[3], 17);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_marker(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[22] = c->hash ^ sx_rl(c->lane[14], 6);
    c->raw[c->slo + (int)((t0 + 40168u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += (uint32_t)cache_block(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 17334u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void chain_node(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= seek_row_442(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x57ddcb1bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[5] += c->lane[9]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 23);
    c->lane[13] ^= sx_rl(c->lane[11], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 31);
    c->raw[c->slo + (int)((t0 + 50568u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 2);
    t1 ^= (uint32_t)queue_entry(c, (uint8_t)(t0 >> 0), t2);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int purge_port(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[4] ^= sx_rl(c->lane[6], 10);
    t0 ^= prime_cursor(c, t1);
    c->hash ^= c->lane[2] + 0x1d37937bu;
    t1 ^= (uint32_t)split_key(c, (uint8_t)(t0 >> 16), t2);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 6);
    push_frame(c, &c->lane[2], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    wrap_page(c, t0, t1);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int fill_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    t2 += link_label(c, c->rlo, c->rln);
    c->lane[6] ^= sx_rl(c->lane[3], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[11] += c->lane[4] ^ 0x55c04751u;
    c->lane[1] += c->lane[13]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 19);
    t2 += (uint32_t)prime_bound(c);
    c->lane[4] ^= sx_rl(c->lane[1], 15);
    c->lane[11] += c->lane[8]; c->lane[14] ^= c->lane[11]; c->lane[14] = sx_rl(c->lane[14], 28);
    c->lane[1] += c->lane[5]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 25);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int peek_track(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57448u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x35b28a41u;
    c->lane[8] += c->lane[11] ^ 0x83798015u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x14) << 0;
    t0 ^= stage_span(c, t1);
    t0 ^= grow_offset(c, t1);
    c->lane[4] ^= sx_rl(c->lane[14], 21);
    t0 ^= hold_label(c, t1);
    t2 = (t2 ^ c->sum) * 0x95318383u;
    probe_scope(c, &c->lane[1], 3);
    c->raw[c->slo + (int)((t0 + 52670u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[14] = c->hash ^ sx_rl(c->lane[13], 7);
    c->lane[15] += c->lane[8]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 29);
    c->raw[c->slo + (int)((t0 + 55866u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[13] = c->hash ^ sx_rl(c->lane[7], 29);
    merge_token(c, &c->lane[3], 3);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t tap_window_413(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 8243u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39557u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x96215a1du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdc1aa697u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 25);
    t0 ^= trace_rate(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[6], 2);
    c->lane[13] ^= sx_rl(c->lane[11], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void mix_mask(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[10], 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x23) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x54) << 8;
    place_scope(c, &c->lane[2], 4);
    c->lane[0] += c->lane[15] ^ 0xff0b5614u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[12] ^= sx_rl(c->lane[0], 29);
    t2 = (t2 ^ c->sum) * 0xcda3233fu;
    c->sched[26] = c->hash ^ sx_rl(c->lane[10], 16);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 7);
    c->lane[3] += c->lane[10]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 14);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void sort_stream(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    place_mask(c, &c->lane[6], 2);
    c->hash = (c->hash * 0x589dfb3bu) ^ sx_rr(c->hash, 24);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x0b3036f9u;
    c->hash ^= c->lane[7] + 0xfdaaea25u;
    t2 = (t2 ^ c->sum) * 0xb8ea65b5u;
    tap_index_440(c, t0, t1);
    coal_chunk(c, &c->lane[11], 4);
    c->lane[3] ^= sx_rl(c->lane[3], 29);
    swap_list(c, &c->lane[4], 2);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint8_t poll_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbdc3730bu;
    t1 ^= (uint32_t)relay_lease(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0xf00498cbu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] ^= sx_rl(c->lane[2], 8);
    c->hash = (c->hash * 0x2c9fd929u) ^ sx_rr(c->hash, 3);
    c->lane[14] += c->lane[15] ^ 0x4820401cu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 30);
    blend_delta(c, &c->lane[9], 3);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t move_pool(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] += c->lane[11] ^ 0x812f9223u;
    t2 += purge_label(c, c->rlo, c->rln);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33012u) % (uint32_t)c->rln)] << 16;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 28);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int tune_chunk(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[8] += c->lane[10] ^ 0x91a3348au;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 17);
    c->lane[15] += c->lane[13]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 18);
    c->lane[8] += c->lane[0] ^ 0xeaeeae35u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 3);
    c->sched[27] = c->hash ^ sx_rl(c->lane[8], 10);
    place_range(c, t0, t1);
    c->lane[3] += c->lane[1] ^ 0xff6fc6efu;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void place_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 += (uint32_t)reap_mask(c);
    t2 += purge_stack(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] += c->lane[8]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->hash ^= c->lane[8] + 0xe33a8975u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t slice_path(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa5) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf3) << 16;
    sift_part(c, t0, t1);
    c->sched[3] = c->hash ^ sx_rl(c->lane[11], 10);
    c->raw[c->slo + (int)((t0 + 33051u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t split_key(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xc1c5da61u) ^ sx_rr(c->hash, 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37113u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35547u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xcbf74cf7u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 1);
    c->hash ^= c->lane[8] + 0xcc7bac6bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0f) << 0;
    t2 = (t2 ^ c->sum) * 0x4e60e6ddu;
    c->hash ^= c->lane[13] + 0x302a89d0u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void step_group(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[0] += c->lane[14] ^ 0x5ab21c9du;
    c->hash = (c->hash * 0x0d80c123u) ^ sx_rr(c->hash, 15);
    merge_page_487(c, t0, t1);
    c->hash ^= c->lane[0] + 0x487a6871u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t link_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[14] = c->hash ^ sx_rl(c->lane[0], 9);
    c->lane[11] ^= sx_rl(c->lane[9], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[1] = c->hash ^ sx_rl(c->lane[9], 31);
    t2 = (t2 ^ c->sum) * 0x262438cdu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void coal_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xaf) << 16;
    c->sched[31] = c->hash ^ sx_rl(c->lane[7], 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x80b545d7u;
    c->lane[11] += c->lane[8] ^ 0x0a2bb6cau;
    t0 ^= trim_span_470(c, t1);
    c->raw[c->slo + (int)((t0 + 60521u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc0) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t stage_span(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += queue_region(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 47591u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 38577u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] ^= sx_rl(c->lane[2], 14);
    c->hash ^= c->lane[0] + 0x2cd0aa98u;
    c->lane[2] ^= sx_rl(c->lane[4], 12);
    c->raw[c->slo + (int)((t0 + 59531u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcabf231fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x1a9224bbu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfcf01f23u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t trim_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x332ed823u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11613u) % (uint32_t)c->rln)] << 8;
    c->lane[10] ^= sx_rl(c->lane[7], 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xac) << 16;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 3);
    c->lane[7] ^= sx_rl(c->lane[4], 13);
    t2 = (t2 ^ c->sum) * 0xd1a55e4bu;
    c->lane[5] ^= sx_rl(c->lane[5], 19);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 9);
    t0 ^= settle_window(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t link_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 ^= map_token(c, t1);
    t2 += (uint32_t)tally_count(c);
    c->raw[c->slo + (int)((t0 + 19973u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[5] ^= sx_rl(c->lane[15], 19);
    c->lane[0] ^= sx_rl(c->lane[15], 16);
    c->raw[c->slo + (int)((t0 + 36941u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xb1b9f401u) ^ sx_rr(c->hash, 19);
    t1 ^= (uint32_t)queue_entry(c, (uint8_t)(t0 >> 0), t2);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 20);
    c->lane[5] ^= sx_rl(c->lane[5], 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static uint32_t trace_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[9] ^= sx_rl(c->lane[8], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x709dd80du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[1] + 0x72cd480du;
    c->sched[30] = c->hash ^ sx_rl(c->lane[13], 12);
    c->sched[22] = c->hash ^ sx_rl(c->lane[6], 15);
    c->hash = (c->hash * 0x074222edu) ^ sx_rr(c->hash, 29);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_token(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += resize_span(c, c->slo, c->sln);
    t2 += (uint32_t)probe_unit(c);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 15);
    shift_bucket(c, &c->lane[10], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xd683becdu;
    c->lane[11] += c->lane[11] ^ 0x43dbe8e5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26283u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x21) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[13] += c->lane[13] ^ 0x41e681bau;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t hold_label(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)tap_unit(c);
    t2 = (t2 ^ c->sum) * 0x9e78fc19u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1e48eabbu;
    c->hash ^= c->lane[11] + 0x26eff785u;
    t2 += (uint32_t)fill_span(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 31427u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x3d8a9ce1u) ^ sx_rr(c->hash, 20);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void probe_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += settle_marker(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 19462u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[12] = c->hash ^ sx_rl(c->lane[0], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58153u) % (uint32_t)c->rln)] << 24;
    flush_block(c, t0, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t queue_ring_432(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 32746u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xfeb1dc65u;
    c->lane[13] += c->lane[15] ^ 0x8774f7e2u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 22);
    c->hash = (c->hash * 0xff4c67d5u) ^ sx_rr(c->hash, 5);
    c->lane[5] += c->lane[0] ^ 0xcc5e924au;
    c->hash = (c->hash * 0x4344e6f7u) ^ sx_rr(c->hash, 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    step_run(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[5] += c->lane[7] ^ 0x74984141u;
    c->sum += t1;
    return t0 + t2;
}

static int prime_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    store_batch(c, &c->lane[6], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[10] + 0x5b8cf094u;
    c->lane[12] += c->lane[5] ^ 0x53acaed0u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x715bc25du;
    t0 ^= swap_layer(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x96) << 0;
    swap_region(c, &c->lane[10], 2);
    c->lane[3] ^= sx_rl(c->lane[15], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t sort_state(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x235c157bu;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 18);
    flush_span(c, &c->lane[6], 1);
    c->sched[19] = c->hash ^ sx_rl(c->lane[15], 16);
    c->lane[4] += c->lane[13]; c->lane[11] ^= c->lane[4]; c->lane[11] = sx_rl(c->lane[11], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc5f88ed5u;
    c->lane[4] += c->lane[11] ^ 0xac1a7473u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb0) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void push_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd4) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcac59d8bu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += (uint32_t)shift_region(c);
    c->hash = (c->hash * 0xb2580d17u) ^ sx_rr(c->hash, 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40650u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x033cf373u;
    t1 ^= (uint32_t)merge_tail_469(c, (uint8_t)(t0 >> 16), t2);
    t2 += shift_batch(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2d571e01u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[3] = c->hash ^ sx_rl(c->lane[14], 26);
    c->hash = (c->hash * 0xc205a943u) ^ sx_rr(c->hash, 7);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t reset_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xeab6c565u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61937u) % (uint32_t)c->rln)] << 8;
    t2 += mix_seat(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54688u) % (uint32_t)c->rln)] << 8;
    c->lane[2] ^= sx_rl(c->lane[2], 20);
    c->lane[15] += c->lane[9] ^ 0xe42675b7u;
    c->hash = (c->hash * 0x42d0aca7u) ^ sx_rr(c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd8) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t1 ^= (uint32_t)parse_node(c, (uint8_t)(t0 >> 0), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void swap_list(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[10] += c->lane[3] ^ 0xff45c077u;
    c->raw[c->slo + (int)((t0 + 11237u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 27129u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[15] + 0x2e42dcaau;
    c->sched[11] = c->hash ^ sx_rl(c->lane[14], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x86fe6e83u;
    t1 ^= (uint32_t)pack_state(c, (uint8_t)(t0 >> 0), t2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void blend_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= parse_cell(c, t1);
    t2 += scan_digest_483(c, c->rlo, c->rln);
    wrap_batch(c, &c->lane[2], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf404b155u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 14837u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x206091c1u) ^ sx_rr(c->hash, 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[9] += c->lane[7]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 22);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int cache_block(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xbb718f6fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x36) << 16;
    stage_range(c, &c->lane[7], 2);
    c->raw[c->slo + (int)((t0 + 62623u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= blend_node(c, t1);
    t2 = (t2 ^ c->sum) * 0xcfb5d379u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16765u) % (uint32_t)c->rln)] << 24;
    c->lane[11] ^= sx_rl(c->lane[2], 5);
    c->lane[8] ^= sx_rl(c->lane[1], 7);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0b281abfu;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tap_index_440(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)step_unit(c);
    mark_path(c, &c->lane[6], 3);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 24);
    t2 = (t2 ^ c->sum) * 0x2fa1fff9u;
    t0 ^= coal_track(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash = (c->hash * 0xd99a3f83u) ^ sx_rr(c->hash, 14);
    c->lane[4] ^= sx_rl(c->lane[2], 10);
    c->hash = (c->hash * 0x10378aabu) ^ sx_rr(c->hash, 16);
    c->hash = (c->hash * 0x41dddf8bu) ^ sx_rr(c->hash, 5);
    poll_slot(c, &c->lane[0], 1);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void wrap_page(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[12] = c->hash ^ sx_rl(c->lane[1], 23);
    t2 += map_batch(c, c->rlo, c->rln);
    split_pairing(c, &c->lane[9], 2);
    t2 += purge_label(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[9] += c->lane[9] ^ 0x0387c73bu;
    t2 = (t2 ^ c->sum) * 0xeb7c328fu;
    t2 += sync_list(c, c->rlo, c->rln);
    t2 += trace_batch_468(c, c->rlo, c->rln);
    c->hash ^= c->lane[7] + 0x0e30a322u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe4) << 16;
    c->sched[27] = c->hash ^ sx_rl(c->lane[13], 8);
    t1 ^= (uint32_t)store_band(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[6] + 0xa974ced9u;
    c->lane[12] += c->lane[12] ^ 0x6a09637du;
    c->sched[0] = c->hash ^ sx_rl(c->lane[13], 16);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t seek_row_442(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[0] ^= sx_rl(c->lane[12], 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t1 ^= (uint32_t)stage_marker(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= defer_batch_475(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa9b5e0efu;
    c->hash = (c->hash * 0x338eaa43u) ^ sx_rr(c->hash, 26);
    split_pairing(c, &c->lane[5], 3);
    t2 = (t2 ^ c->sum) * 0xf26b672fu;
    c->lane[10] += c->lane[3] ^ 0xefdd7f8eu;
    c->lane[14] += c->lane[1]; c->lane[11] ^= c->lane[14]; c->lane[11] = sx_rl(c->lane[11], 31);
    c->raw[c->slo + (int)((t0 + 51304u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)merge_gap(c, (uint8_t)(t0 >> 0), t2);
    stage_range(c, &c->lane[2], 3);
    c->hash ^= c->lane[3] + 0x6494ef6cu;
    c->lane[13] += c->lane[13] ^ 0x1100c1e3u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t defer_count(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    reap_record(c, t0, t1);
    c->sched[11] = c->hash ^ sx_rl(c->lane[13], 5);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x62) << 0;
    c->sched[27] = c->hash ^ sx_rl(c->lane[13], 24);
    c->raw[c->slo + (int)((t0 + 9348u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    c->hash ^= c->lane[5] + 0x2a90f244u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t rotate_store(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xfe6f66e5u) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] += c->lane[2]; c->lane[8] ^= c->lane[6]; c->lane[8] = sx_rl(c->lane[8], 22);
    t1 ^= (uint32_t)scan_line(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x4ee43dbdu;
    c->lane[7] ^= sx_rl(c->lane[6], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7ea06b1bu;
    place_range(c, t0, t1);
    c->lane[6] += c->lane[10] ^ 0x5c584cf8u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc0) << 0;
    c->sum += t1;
    return t0 + t2;
}

static void parse_band_445(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xf9e101e3u;
    c->lane[12] ^= sx_rl(c->lane[2], 25);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x60) << 16;
    c->raw[c->slo + (int)((t0 + 17153u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[9] += c->lane[10] ^ 0xc4d6722au;
    c->lane[0] += c->lane[15] ^ 0x8ecb4724u;
    t2 += purge_stack(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 54846u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 52664u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t prime_cursor(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd1d7d7fbu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x41b9df8bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16594u) % (uint32_t)c->rln)] << 24;
    c->sched[11] = c->hash ^ sx_rl(c->lane[7], 28);
    c->hash = (c->hash * 0x6de49f2bu) ^ sx_rr(c->hash, 19);
    tally_rate(c, t0, t1);
    c->hash ^= c->lane[8] + 0x178b8376u;
    c->hash ^= c->lane[5] + 0x99da4243u;
    c->hash = (c->hash * 0xce8ee5cdu) ^ sx_rr(c->hash, 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbc) << 0;
    c->lane[9] += c->lane[5]; c->lane[8] ^= c->lane[9]; c->lane[8] = sx_rl(c->lane[8], 28);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t grow_offset(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += shift_port(c, c->slo, c->sln);
    c->hash ^= c->lane[14] + 0x30a8e802u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x76ac0fadu;
    t2 += push_delta(c, c->slo, c->sln);
    t1 ^= (uint32_t)relay_lease(c, (uint8_t)(t0 >> 16), t2);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 17);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void place_mask(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 23207u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 3);
    c->sched[31] = c->hash ^ sx_rl(c->lane[13], 4);
    c->lane[13] ^= sx_rl(c->lane[14], 2);
    c->lane[4] ^= sx_rl(c->lane[11], 19);
    c->hash ^= c->lane[12] + 0x780f427du;
    t0 ^= peek_entry(c, t1);
    t2 = (t2 ^ c->sum) * 0x9c6d6563u;
    c->raw[c->slo + (int)((t0 + 55362u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] ^= sx_rl(c->lane[13], 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t parse_pool(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[9] += c->lane[3]; c->lane[7] ^= c->lane[9]; c->lane[7] = sx_rl(c->lane[7], 27);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x03ba11abu;
    c->hash = (c->hash * 0xbd9e776du) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0xaf83d565u) ^ sx_rr(c->hash, 10);
    c->hash ^= c->lane[4] + 0x7c57bee4u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x57fa0f2bu;
    c->lane[5] += c->lane[6]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 10);
    c->hash ^= c->lane[0] + 0x6040a158u;
    t0 ^= step_gap(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void shift_slot(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)align_frame(c);
    c->hash ^= c->lane[11] + 0x32a34bb4u;
    store_marker(c, &c->lane[6], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1fdcfbc7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= c->lane[9] + 0x89ab0205u;
    c->lane[0] += c->lane[11] ^ 0x1d64ebc6u;
    t2 = (t2 ^ c->sum) * 0x81f2005fu;
    t2 = (t2 ^ c->sum) * 0xffade33bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x93) << 16;
    c->lane[14] ^= sx_rl(c->lane[7], 29);
    t2 += clamp_marker(c, c->slo, c->sln);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint8_t split_port(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[25] = c->hash ^ sx_rl(c->lane[9], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa1) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x2a) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x023f6ac3u;
    c->hash = (c->hash * 0x5c09b3a3u) ^ sx_rr(c->hash, 15);
    c->lane[13] ^= sx_rl(c->lane[9], 30);
    c->hash ^= c->lane[2] + 0x62808642u;
    c->lane[14] ^= sx_rl(c->lane[2], 17);
    t2 = (t2 ^ c->sum) * 0xa17f8c1bu;
    flush_block(c, t0, t1);
    c->hash = (c->hash * 0x08a7268du) ^ sx_rr(c->hash, 25);
    c->hash ^= c->lane[5] + 0xc6148efau;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void push_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[10] += c->lane[15]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 16);
    t2 += (uint32_t)reap_stream(c);
    c->hash ^= c->lane[10] + 0x824c89aau;
    c->lane[4] ^= sx_rl(c->lane[4], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x6f) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x29) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t purge_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22372u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbfc8c03bu;
    c->raw[c->slo + (int)((t0 + 65455u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x93) << 8;
    c->sched[2] = c->hash ^ sx_rl(c->lane[13], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd0d524e1u;
    t2 = (t2 ^ c->sum) * 0xaa0129b5u;
    c->lane[3] ^= sx_rl(c->lane[15], 27);
    c->lane[4] += c->lane[10] ^ 0xe80911e6u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t swap_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20686u) % (uint32_t)c->rln)] << 8;
    c->lane[12] ^= sx_rl(c->lane[14], 7);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[6] += c->lane[14] ^ 0xa8c624c5u;
    t2 = (t2 ^ c->sum) * 0x6d432443u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64749u) % (uint32_t)c->rln)] << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void wrap_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0xe272acc1u;
    c->raw[c->slo + (int)((t0 + 50317u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x9c2a13fdu;
    c->hash ^= c->lane[4] + 0xd4a59416u;
    t2 = (t2 ^ c->sum) * 0xee16d541u;
    t2 = (t2 ^ c->sum) * 0xbd11e557u;
    t2 = (t2 ^ c->sum) * 0x4435d9c5u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t push_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[0] += c->lane[1]; c->lane[12] ^= c->lane[0]; c->lane[12] = sx_rl(c->lane[12], 18);
    c->lane[7] ^= sx_rl(c->lane[4], 4);
    c->sched[30] = c->hash ^ sx_rl(c->lane[7], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5177u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x610af9b5u) ^ sx_rr(c->hash, 1);
    c->lane[5] ^= sx_rl(c->lane[1], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2640u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 32615u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum += t1;
    return t0 + t2;
}

static void store_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[14] + 0x38806f0fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= c->lane[8] + 0xa4afb11cu;
    c->lane[4] ^= sx_rl(c->lane[3], 16);
    c->hash = (c->hash * 0x3fa962e1u) ^ sx_rr(c->hash, 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcb) << 0;
    c->lane[3] ^= sx_rl(c->lane[2], 21);
    t2 = (t2 ^ c->sum) * 0xbd1d0c4bu;
    c->sched[24] = c->hash ^ sx_rl(c->lane[6], 1);
    c->raw[c->slo + (int)((t0 + 45771u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t settle_marker(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] += c->lane[1] ^ 0xcd626fd1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x82) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17950u) % (uint32_t)c->rln)] << 8;
    c->sched[18] = c->hash ^ sx_rl(c->lane[15], 15);
    c->sum += t1;
    return t0 + t2;
}

static int tap_unit(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x85) << 0;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 26);
    c->lane[11] += c->lane[2] ^ 0x064d0cecu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9e3aed07u;
    c->lane[1] += c->lane[6] ^ 0x253940b4u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x20d7afd5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42912u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0xf9a81f5bu;
    c->lane[3] += c->lane[14]; c->lane[0] ^= c->lane[3]; c->lane[0] = sx_rl(c->lane[0], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x548fb86bu;
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void swap_region(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[3] + 0x3c2c3c30u;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 23);
    c->hash = (c->hash * 0x2491e0f9u) ^ sx_rr(c->hash, 11);
    c->sched[18] = c->hash ^ sx_rl(c->lane[8], 16);
    c->hash ^= c->lane[13] + 0xd29ea2f7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x59f36cffu;
    c->lane[3] += c->lane[4] ^ 0x24ade25cu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22794u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x4e) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t map_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x33b0eb8bu;
    t2 = (t2 ^ c->sum) * 0x54094affu;
    c->hash = (c->hash * 0x71c73283u) ^ sx_rr(c->hash, 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0a) << 16;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t scan_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] += c->lane[4]; c->lane[2] ^= c->lane[8]; c->lane[2] = sx_rl(c->lane[2], 2);
    c->lane[15] ^= sx_rl(c->lane[0], 6);
    c->lane[0] += c->lane[10] ^ 0x693aa903u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x30e9688bu;
    c->lane[15] ^= sx_rl(c->lane[5], 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33090u) % (uint32_t)c->rln)] << 8;
    c->lane[13] += c->lane[12]; c->lane[7] ^= c->lane[13]; c->lane[7] = sx_rl(c->lane[7], 5);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t settle_window(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 52971u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x15ecbc53u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[11] + 0x4a1bc29eu;
    c->hash = (c->hash * 0x3aeed4c1u) ^ sx_rr(c->hash, 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x56) << 16;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 19);
    c->raw[c->slo + (int)((t0 + 57266u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[9] += c->lane[15] ^ 0xa32baf06u;
    c->lane[11] += c->lane[2] ^ 0xe167e518u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int reap_mask(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x35368613u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf0) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[22] = c->hash ^ sx_rl(c->lane[3], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xc3) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9011u) % (uint32_t)c->rln)] << 0;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 10);
    c->lane[13] += c->lane[3] ^ 0x89040fb3u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 21);
    c->sched[4] = c->hash ^ sx_rl(c->lane[3], 15);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t stage_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 6197u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 18171u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xa8a4e8e3u;
    c->lane[3] += c->lane[9] ^ 0x92830ef2u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t relay_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x72be205bu) ^ sx_rr(c->hash, 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5d) << 0;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 12);
    c->lane[4] += c->lane[15]; c->lane[2] ^= c->lane[4]; c->lane[2] = sx_rl(c->lane[2], 29);
    c->lane[7] += c->lane[12]; c->lane[3] ^= c->lane[7]; c->lane[3] = sx_rl(c->lane[3], 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[7] += c->lane[4] ^ 0xdfa5a12du;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int align_frame(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= c->lane[7] + 0x072988bau;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb8d19d91u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36353u) % (uint32_t)c->rln)] << 16;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 16);
    c->hash ^= c->lane[10] + 0xbcf17ab5u;
    c->hash = (c->hash * 0xe1583207u) ^ sx_rr(c->hash, 4);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t trace_batch_468(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x1365533fu;
    c->hash = (c->hash * 0xccf8d50bu) ^ sx_rr(c->hash, 3);
    c->lane[12] += c->lane[3] ^ 0x8be311fdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37574u) % (uint32_t)c->rln)] << 8;
    c->sched[6] = c->hash ^ sx_rl(c->lane[7], 28);
    c->hash ^= c->lane[3] + 0x90923370u;
    c->hash ^= c->lane[7] + 0xbea39316u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe2) << 8;
    c->sched[11] = c->hash ^ sx_rl(c->lane[9], 22);
    t2 = (t2 ^ c->sum) * 0x6ebd87ebu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t merge_tail_469(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 4606u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 17);
    c->raw[c->slo + (int)((t0 + 51180u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[6] + 0x632107c0u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t trim_span_470(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x706a2687u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe17a2c5du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[18] = c->hash ^ sx_rl(c->lane[15], 16);
    c->hash ^= c->lane[11] + 0xd6917976u;
    c->raw[c->slo + (int)((t0 + 46346u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[9] = c->hash ^ sx_rl(c->lane[9], 20);
    t2 = (t2 ^ c->sum) * 0x170225adu;
    c->lane[0] += c->lane[3]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 29);
    t2 = (t2 ^ c->sum) * 0x26c4441bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t store_band(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x4daab1c1u) ^ sx_rr(c->hash, 6);
    c->lane[6] ^= sx_rl(c->lane[15], 20);
    c->raw[c->slo + (int)((t0 + 23289u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x73fdd8a5u;
    c->hash ^= c->lane[11] + 0x2c5f8850u;
    t2 += scan_path(c, c->rlo, c->rln);
    c->lane[15] += c->lane[15] ^ 0x5a1408d5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8f443bd3u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 20);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int tally_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x0408585bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= c->lane[13] + 0x731c9320u;
    c->sched[7] = c->hash ^ sx_rl(c->lane[10], 17);
    t2 = (t2 ^ c->sum) * 0x489c7055u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc6768de1u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[13], 8);
    c->sched[2] = c->hash ^ sx_rl(c->lane[15], 14);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mix_seat(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[0] += c->lane[11] ^ 0x92cdb511u;
    c->lane[9] ^= sx_rl(c->lane[5], 18);
    c->hash ^= c->lane[10] + 0x125824e1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 29);
    c->lane[0] += c->lane[5] ^ 0x4bcce923u;
    c->sched[7] = c->hash ^ sx_rl(c->lane[12], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1979a75fu;
    c->sched[28] = c->hash ^ sx_rl(c->lane[10], 9);
    c->sum += t1;
    return t0 + t2;
}

static int shift_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->sched[7] = c->hash ^ sx_rl(c->lane[1], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc2baf451u;
    scan_bucket_587(c, t0, t1);
    c->sched[21] = c->hash ^ sx_rl(c->lane[11], 29);
    c->raw[c->slo + (int)((t0 + 5581u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x2f83db51u) ^ sx_rr(c->hash, 25);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t defer_batch_475(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc5) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59061u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[2] += c->lane[10]; c->lane[13] ^= c->lane[2]; c->lane[13] = sx_rl(c->lane[13], 14);
    c->lane[10] ^= sx_rl(c->lane[14], 10);
    c->hash = (c->hash * 0xf5338df1u) ^ sx_rr(c->hash, 18);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t resize_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x47469dcbu) ^ sx_rr(c->hash, 22);
    c->lane[9] += c->lane[0]; c->lane[14] ^= c->lane[9]; c->lane[14] = sx_rl(c->lane[14], 10);
    c->lane[5] += c->lane[13] ^ 0x2dcc09acu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50997u) % (uint32_t)c->rln)] << 24;
    c->lane[5] += c->lane[6] ^ 0x1fc1e5f4u;
    t2 = (t2 ^ c->sum) * 0xa6327c87u;
    c->lane[3] ^= sx_rl(c->lane[13], 31);
    c->hash = (c->hash * 0x1f590d77u) ^ sx_rr(c->hash, 9);
    c->sum += t1;
    return t0 + t2;
}

static void split_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[14] + 0xaa86f5aeu;
    c->hash = (c->hash * 0xcc932f03u) ^ sx_rr(c->hash, 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[12] += c->lane[13] ^ 0xd2e47668u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[6], 21);
    c->hash = (c->hash * 0xde23390bu) ^ sx_rr(c->hash, 10);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 29);
    c->hash = (c->hash * 0x3ff39303u) ^ sx_rr(c->hash, 14);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t parse_cell(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x81) << 16;
    c->hash ^= c->lane[5] + 0xdecbb290u;
    c->hash ^= c->lane[10] + 0x79642a98u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[5] += c->lane[8]; c->lane[6] ^= c->lane[5]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->lane[6] += c->lane[13] ^ 0xff7142f2u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash = (c->hash * 0x10e52723u) ^ sx_rr(c->hash, 9);
    c->lane[11] ^= sx_rl(c->lane[2], 27);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t blend_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[5] + 0x9cb964d0u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3989u) % (uint32_t)c->rln)] << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 13);
    c->hash ^= c->lane[8] + 0x5f92d786u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x73) << 16;
    c->sched[8] = c->hash ^ sx_rl(c->lane[10], 3);
    c->hash = (c->hash * 0x7d0c2ba9u) ^ sx_rr(c->hash, 1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t coal_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x47706fc9u) ^ sx_rr(c->hash, 2);
    c->lane[11] ^= sx_rl(c->lane[8], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39301u) % (uint32_t)c->rln)] << 24;
    c->lane[15] ^= sx_rl(c->lane[15], 1);
    t2 = (t2 ^ c->sum) * 0x61875fdfu;
    t2 = (t2 ^ c->sum) * 0x43fd9c53u;
    t2 = (t2 ^ c->sum) * 0x56532f21u;
    t2 = (t2 ^ c->sum) * 0xf7f4a285u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x94ff5d19u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void shift_bucket(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x32019165u) ^ sx_rr(c->hash, 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd40425f3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[14], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc417c45bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t clamp_marker(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[5] + 0x008cb032u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49995u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x16ab9119u) ^ sx_rr(c->hash, 18);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61419u) % (uint32_t)c->rln)] << 16;
    c->lane[13] ^= sx_rl(c->lane[7], 26);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t scan_digest_483(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] ^= sx_rl(c->lane[14], 13);
    t2 = (t2 ^ c->sum) * 0x02eb8c13u;
    c->hash ^= c->lane[8] + 0x68b80d11u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60026u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[1] + 0xc866fb99u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8381u) % (uint32_t)c->rln)] << 24;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static uint8_t pack_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[4]; c->lane[5] ^= c->lane[10]; c->lane[5] = sx_rl(c->lane[5], 12);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4926u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x60) << 16;
    c->lane[3] += c->lane[8]; c->lane[15] ^= c->lane[3]; c->lane[15] = sx_rl(c->lane[15], 22);
    t2 = (t2 ^ c->sum) * 0x5bc104f9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19094u) % (uint32_t)c->rln)] << 8;
    t2 += grow_state(c, c->rlo, c->rln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void step_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xd40cbfa1u;
    c->lane[14] ^= sx_rl(c->lane[13], 8);
    t2 = (t2 ^ c->sum) * 0xafaf7635u;
    c->sched[13] = c->hash ^ sx_rl(c->lane[8], 26);
    c->lane[3] += c->lane[10]; c->lane[7] ^= c->lane[3]; c->lane[7] = sx_rl(c->lane[7], 12);
    c->sched[10] = c->hash ^ sx_rl(c->lane[3], 8);
    c->lane[2] += c->lane[6]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 26);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void stage_range(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x3cb6329fu) ^ sx_rr(c->hash, 18);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[4], 18);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash = (c->hash * 0x12de3703u) ^ sx_rr(c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd9547c6bu;
    c->hash ^= c->lane[13] + 0xd9094062u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void merge_page_487(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x7d163ca9u;
    c->hash = (c->hash * 0xc1f4b8efu) ^ sx_rr(c->hash, 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59919u) % (uint32_t)c->rln)] << 24;
    c->sched[4] = c->hash ^ sx_rl(c->lane[14], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62004u) % (uint32_t)c->rln)] << 0;
    c->lane[8] ^= sx_rl(c->lane[12], 25);
    c->lane[2] += c->lane[3]; c->lane[1] ^= c->lane[2]; c->lane[1] = sx_rl(c->lane[1], 5);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 23);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static int reap_stream(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2279ed27u;
    c->lane[11] += c->lane[13] ^ 0xeb40bda8u;
    c->lane[12] += c->lane[11]; c->lane[3] ^= c->lane[12]; c->lane[3] = sx_rl(c->lane[3], 23);
    c->sched[26] = c->hash ^ sx_rl(c->lane[7], 29);
    c->lane[4] ^= sx_rl(c->lane[3], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12206u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7451u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x84441599u;
    c->lane[9] += c->lane[4]; c->lane[1] ^= c->lane[9]; c->lane[1] = sx_rl(c->lane[1], 20);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void flush_span(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[12] = c->hash ^ sx_rl(c->lane[4], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[4] += c->lane[7]; c->lane[13] ^= c->lane[4]; c->lane[13] = sx_rl(c->lane[13], 8);
    c->raw[c->slo + (int)((t0 + 58244u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58220u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 29157u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 29);
    c->lane[2] += c->lane[15]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0c) << 8;
    c->raw[c->slo + (int)((t0 + 19512u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[5] + 0xed04823du;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void flush_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 18293u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xaa) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5b) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26025u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd7e11945u;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static void store_marker(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] += c->lane[4]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 31);
    c->sched[1] = c->hash ^ sx_rl(c->lane[8], 19);
    c->hash ^= c->lane[13] + 0x0373faa1u;
    c->lane[3] += c->lane[4] ^ 0xf094e6b6u;
    c->hash ^= c->lane[8] + 0xbbe3ca63u;
    c->lane[11] += c->lane[3] ^ 0x38cd44cau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1007u) % (uint32_t)c->rln)] << 8;
    c->sched[31] = c->hash ^ sx_rl(c->lane[2], 13);
    c->lane[2] += c->lane[5]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 5);
    c->hash ^= c->lane[11] + 0x20fb3ba6u;
    t2 = (t2 ^ c->sum) * 0xc029bd2bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void sift_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 8773u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[4] ^= sx_rl(c->lane[14], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 47366u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x437158f7u;
    c->lane[10] += c->lane[0]; c->lane[11] ^= c->lane[10]; c->lane[11] = sx_rl(c->lane[11], 16);
    c->raw[c->slo + (int)((t0 + 40323u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb9) << 8;
    c->lane[7] += c->lane[1] ^ 0xf0613a0cu;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int probe_unit(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf6) << 8;
    c->raw[c->slo + (int)((t0 + 45039u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] += c->lane[4] ^ 0x403b6209u;
    t2 = (t2 ^ c->sum) * 0x471efdddu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x82861d77u;
    c->lane[5] ^= sx_rl(c->lane[1], 2);
    c->lane[15] += c->lane[12] ^ 0x166eec30u;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t peek_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 18820u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x7e65a769u) ^ sx_rr(c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3e) << 0;
    c->raw[c->slo + (int)((t0 + 25322u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[10] + 0xc2940b96u;
    c->lane[15] += c->lane[8] ^ 0xca7ae7b8u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34692u) % (uint32_t)c->rln)] << 16;
    c->lane[9] ^= sx_rl(c->lane[15], 8);
    c->hash = (c->hash * 0x3575f2a3u) ^ sx_rr(c->hash, 15);
    c->raw[c->slo + (int)((t0 + 50517u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void reap_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] += c->lane[10]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc3) << 0;
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x065bd813u;
    c->raw[c->slo + (int)((t0 + 60990u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] += c->lane[14]; c->lane[15] ^= c->lane[10]; c->lane[15] = sx_rl(c->lane[15], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x04) << 0;
    c->lane[4] += c->lane[12]; c->lane[13] ^= c->lane[4]; c->lane[13] = sx_rl(c->lane[13], 29);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void mark_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 31823u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33529u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[4] + 0x66083c8eu;
    c->lane[4] += c->lane[3]; c->lane[10] ^= c->lane[4]; c->lane[10] = sx_rl(c->lane[10], 30);
    c->lane[14] += c->lane[2]; c->lane[13] ^= c->lane[14]; c->lane[13] = sx_rl(c->lane[13], 21);
    c->hash ^= c->lane[10] + 0x409a2303u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void tally_rate(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] ^= sx_rl(c->lane[4], 18);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 2);
    c->lane[9] ^= sx_rl(c->lane[13], 28);
    c->sched[21] = c->hash ^ sx_rl(c->lane[13], 9);
    c->lane[11] ^= sx_rl(c->lane[3], 24);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static int step_unit(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->sched[29] = c->hash ^ sx_rl(c->lane[14], 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8b0f3059u;
    c->hash ^= c->lane[6] + 0xc7aa17cbu;
    c->hash ^= c->lane[15] + 0xcf0f6799u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43104u) % (uint32_t)c->rln)] << 8;
    c->lane[13] ^= sx_rl(c->lane[15], 10);
    c->hash ^= c->lane[8] + 0x80d8b0d7u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0xf2be4dcfu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t queue_entry(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 35262u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x12ba2c33u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0xfdf10e1bu) ^ sx_rr(c->hash, 24);
    c->sched[31] = c->hash ^ sx_rl(c->lane[0], 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xdf1578a5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60552u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t parse_node(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] += c->lane[14] ^ 0xb95c6e7eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xa3) << 0;
    t2 = (t2 ^ c->sum) * 0xbe36ae9du;
    c->hash ^= c->lane[8] + 0x5b63c368u;
    c->raw[c->slo + (int)((t0 + 31575u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t step_gap(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xad1887c1u;
    c->lane[9] ^= sx_rl(c->lane[7], 13);
    t2 = (t2 ^ c->sum) * 0x5a135ac5u;
    c->hash ^= c->lane[0] + 0xd19f5fc7u;
    c->sched[21] = c->hash ^ sx_rl(c->lane[5], 27);
    c->lane[4] += c->lane[11]; c->lane[2] ^= c->lane[4]; c->lane[2] = sx_rl(c->lane[2], 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x52) << 16;
    c->lane[8] += c->lane[6]; c->lane[7] ^= c->lane[8]; c->lane[7] = sx_rl(c->lane[7], 2);
    c->sched[5] = c->hash ^ sx_rl(c->lane[12], 20);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t merge_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] ^= sx_rl(c->lane[14], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[1] + 0xefd6989bu;
    c->hash ^= c->lane[6] + 0x79f80c07u;
    c->raw[c->slo + (int)((t0 + 14373u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0xb18ac68du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 35896u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t queue_region(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x16e01535u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 54445u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf0) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x79) << 16;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 31);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t shift_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 37932u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] ^= sx_rl(c->lane[9], 18);
    t2 = (t2 ^ c->sum) * 0xc3dfd093u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[4] += c->lane[11] ^ 0x5b03780bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xfb) << 0;
    c->sum += t1;
    return t0 + t2;
}

static void poll_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[14] = c->hash ^ sx_rl(c->lane[11], 20);
    t2 = (t2 ^ c->sum) * 0x0fb1d545u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1d) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xef855b69u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 2);
    c->hash ^= c->lane[4] + 0x7084b583u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59909u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t map_token(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x103ea519u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[4] = c->hash ^ sx_rl(c->lane[12], 26);
    c->hash = (c->hash * 0xb25ad0f5u) ^ sx_rr(c->hash, 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t sync_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30736u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x274e2cb1u;
    c->raw[c->slo + (int)((t0 + 11744u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[5] = c->hash ^ sx_rl(c->lane[7], 12);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t purge_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xf232bed3u) ^ sx_rr(c->hash, 31);
    c->hash = (c->hash * 0xaa2d57bbu) ^ sx_rr(c->hash, 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xba) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xa50d5aebu;
    c->sum += t1;
    return t0 + t2;
}

static int fill_span(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    t2 = (t2 ^ c->sum) * 0x7603f413u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6c5d0361u;
    c->raw[c->slo + (int)((t0 + 3913u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] += c->lane[6] ^ 0x848df387u;
    c->lane[2] += c->lane[14]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 23);
    c->lane[11] += c->lane[11] ^ 0x9ea1c3c9u;
    c->lane[15] += c->lane[0] ^ 0xc7a356f3u;
    c->sched[7] = c->hash ^ sx_rl(c->lane[13], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void place_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48247u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[4] + 0xda174929u;
    c->lane[9] += c->lane[5] ^ 0x5167db7eu;
    c->raw[c->slo + (int)((t0 + 12257u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x03) << 8;
    c->hash ^= c->lane[1] + 0x615aab1fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbcae08b7u;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t shift_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[8] + 0xe744e43fu;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 24);
    t2 = (t2 ^ c->sum) * 0xc903552bu;
    c->sched[18] = c->hash ^ sx_rl(c->lane[8], 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60061u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x64c34221u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 20);
    c->sum += t1;
    return t0 + t2;
}

static void tap_pool(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    push_gap(c, t0, t1);
    t2 += peek_label(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 38192u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xc38e0bfbu;
    poll_stream(c, &c->lane[2], 2);
    t2 += (uint32_t)rotate_store_592(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    cache_count(c, t0, t1);
    t2 += (uint32_t)latch_region(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t2 += (uint32_t)grow_bound(c);
    c->sched[11] = c->hash ^ sx_rl(c->lane[8], 29);
    t2 += grow_state(c, c->rlo, c->rln);
    flush_head(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 41320u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0x00808ecfu;
    prime_stack(c, t0, t1);
    prime_bucket_596(c, &c->lane[8], 1);
    sync_port(c, t0, t1);
    t2 += (uint32_t)chain_block(c);
    patch_stack(c, t0, t1);
    t0 ^= load_token(c, t1);
    c->lane[4] += c->lane[8] ^ 0x82f89cbcu;
    t2 += cache_part_625(c, c->slo, c->sln);
    t2 += scan_layer(c, c->rlo, c->rln);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 1);
    join_segment(c, t0, t1);
    c->sched[18] = c->hash ^ sx_rl(c->lane[9], 17);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void flush_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5690bf3bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x20) << 0;
    t1 ^= (uint32_t)coal_batch(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35080u) % (uint32_t)c->rln)] << 0;
    t2 += patch_unit(c, c->rlo, c->rln);
    c->lane[10] += c->lane[5]; c->lane[11] ^= c->lane[10]; c->lane[11] = sx_rl(c->lane[11], 8);
    load_bound(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1942u) % (uint32_t)c->rln)] << 16;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 30);
    c->lane[14] += c->lane[15]; c->lane[12] ^= c->lane[14]; c->lane[12] = sx_rl(c->lane[12], 2);
    c->lane[13] += c->lane[7]; c->lane[12] ^= c->lane[13]; c->lane[12] = sx_rl(c->lane[12], 22);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t peek_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] ^= sx_rl(c->lane[4], 17);
    t2 += clamp_entry(c, c->slo, c->sln);
    c->lane[14] ^= sx_rl(c->lane[12], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t1 ^= (uint32_t)hold_unit_541(c, (uint8_t)(t0 >> 0), t2);
    sort_region(c, &c->lane[3], 1);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    patch_token(c, &c->lane[3], 2);
    t1 ^= (uint32_t)mix_chunk_518(c, (uint8_t)(t0 >> 8), t2);
    align_tail(c, &c->lane[5], 2);
    defer_row_540(c, &c->lane[7], 1);
    t2 += (uint32_t)settle_scope_530(c);
    wrap_scope(c, t0, t1);
    t0 ^= defer_index(c, t1);
    hold_bound(c, t0, t1);
    c->sched[21] = c->hash ^ sx_rl(c->lane[4], 3);
    c->sum += t1;
    return t0 + t2;
}

static void prime_stack(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)swap_stream(c);
    t1 ^= (uint32_t)emit_label(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= defer_offset(c, t1);
    c->sched[17] = c->hash ^ sx_rl(c->lane[8], 22);
    t2 += (uint32_t)pin_count(c);
    t1 ^= (uint32_t)chain_pool(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)drain_bound(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    pair_unit(c, t0, t1);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 12);
    push_row(c, &c->lane[1], 3);
    c->lane[8] ^= sx_rl(c->lane[11], 24);
    t2 = (t2 ^ c->sum) * 0xa50e66f7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7978u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbdad1ce3u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint8_t emit_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x503f70cfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc6) << 8;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xa8301a9fu;
    t1 ^= (uint32_t)hold_unit_541(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17114u) % (uint32_t)c->rln)] << 24;
    t0 ^= map_unit_545(c, t1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50150u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40819u) % (uint32_t)c->rln)] << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void push_row(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[3] += c->lane[9]; c->lane[0] ^= c->lane[3]; c->lane[0] = sx_rl(c->lane[0], 13);
    t2 = (t2 ^ c->sum) * 0xb8fe33c9u;
    t1 ^= (uint32_t)tune_marker(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8fc76dc9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41191u) % (uint32_t)c->rln)] << 8;
    c->lane[3] ^= sx_rl(c->lane[4], 6);
    t2 += (uint32_t)slice_value(c);
    t2 += (uint32_t)tally_line(c);
    c->lane[14] ^= sx_rl(c->lane[14], 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41677u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[5] + 0xf8101919u;
    tune_count(c, &c->lane[11], 2);
    c->raw[c->slo + (int)((t0 + 32367u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] ^= sx_rl(c->lane[2], 31);
    t2 = (t2 ^ c->sum) * 0x091fdc55u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t mix_chunk_518(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t1 ^= (uint32_t)probe_stream(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t1 ^= (uint32_t)prime_bucket(c, (uint8_t)(t0 >> 0), t2);
    t2 = (t2 ^ c->sum) * 0x97138efdu;
    c->lane[3] += c->lane[11]; c->lane[13] ^= c->lane[3]; c->lane[13] = sx_rl(c->lane[13], 3);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 17);
    c->lane[11] ^= sx_rl(c->lane[13], 13);
    patch_stack(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t patch_unit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    cache_seat(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc29758a3u;
    t1 ^= (uint32_t)tune_marker(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 16167u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[14] + 0xb88803ffu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xcf) << 0;
    trace_tuple(c, &c->lane[1], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x774533b9u;
    c->sched[10] = c->hash ^ sx_rl(c->lane[3], 8);
    c->sum += t1;
    return t0 + t2;
}

static void wrap_scope(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xeec573bdu;
    c->hash = (c->hash * 0x32ebe943u) ^ sx_rr(c->hash, 22);
    c->lane[1] ^= sx_rl(c->lane[0], 10);
    tally_arena(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbf971f1fu;
    c->sched[4] = c->hash ^ sx_rl(c->lane[1], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[9] += c->lane[1]; c->lane[8] ^= c->lane[9]; c->lane[8] = sx_rl(c->lane[8], 5);
    t2 = (t2 ^ c->sum) * 0xf56246dbu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5e) << 8;
    cache_seat(c, t0, t1);
    t2 += wrap_part(c, c->slo, c->sln);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static int swap_stream(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x32cacb8bu;
    c->hash = (c->hash * 0x859e1693u) ^ sx_rr(c->hash, 28);
    t2 = (t2 ^ c->sum) * 0xa81f3fc7u;
    store_tail(c, &c->lane[2], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)reset_path_575(c);
    t2 += tune_offset(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 2574u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void patch_token(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] += c->lane[14]; c->lane[1] ^= c->lane[4]; c->lane[1] = sx_rl(c->lane[1], 27);
    t2 += tune_offset(c, c->slo, c->sln);
    c->lane[15] ^= sx_rl(c->lane[11], 19);
    t2 += (uint32_t)move_span(c);
    c->hash ^= c->lane[12] + 0xea812d21u;
    c->lane[12] ^= sx_rl(c->lane[13], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57587u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[14] + 0x937ca579u;
    t2 = (t2 ^ c->sum) * 0xf32578bfu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 24);
    clamp_batch(c, &c->lane[1], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd0a02159u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x81b338a1u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t coal_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x71) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5b65293du;
    c->lane[10] += c->lane[15] ^ 0x3e4a6956u;
    t1 ^= (uint32_t)tally_page(c, (uint8_t)(t0 >> 0), t2);
    trim_head(c, &c->lane[5], 1);
    defer_row_540(c, &c->lane[3], 2);
    t2 += clamp_entry(c, c->rlo, c->rln);
    c->lane[5] += c->lane[15]; c->lane[14] ^= c->lane[5]; c->lane[14] = sx_rl(c->lane[14], 6);
    c->lane[5] ^= sx_rl(c->lane[7], 19);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void pair_unit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf899e5adu;
    c->hash ^= c->lane[12] + 0x3c1f0119u;
    t2 += blend_row(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x2e3a460du) ^ sx_rr(c->hash, 1);
    c->hash = (c->hash * 0x3a0b83b5u) ^ sx_rr(c->hash, 13);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 28);
    c->lane[10] ^= sx_rl(c->lane[2], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1847f46bu;
    t0 ^= yield_state_598(c, t1);
    t2 += (uint32_t)settle_scope_530(c);
    c->sched[0] = c->hash ^ sx_rl(c->lane[14], 6);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void load_bound(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 11);
    scan_bucket_587(c, t0, t1);
    push_gap(c, t0, t1);
    load_ring(c, t0, t1);
    t2 += (uint32_t)split_group(c);
    sync_port(c, t0, t1);
    c->sched[26] = c->hash ^ sx_rl(c->lane[12], 5);
    t2 = (t2 ^ c->sum) * 0x16f27709u;
    t2 += (uint32_t)move_batch(c);
    c->lane[1] += c->lane[8] ^ 0x2811e5ebu;
    c->raw[c->slo + (int)((t0 + 21603u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] += c->lane[3] ^ 0x0ec0c854u;
    t1 ^= (uint32_t)prime_bucket(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[7] + 0x7403af13u;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void tally_arena(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcc33fd41u;
    t1 ^= (uint32_t)stage_run(c, (uint8_t)(t0 >> 0), t2);
    grow_tuple(c, &c->lane[5], 2);
    c->sched[19] = c->hash ^ sx_rl(c->lane[6], 25);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 19);
    t2 = (t2 ^ c->sum) * 0x630f8975u;
    t0 ^= tally_region(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x16e06f83u;
    c->lane[3] += c->lane[0] ^ 0xd68b07d3u;
    c->hash ^= c->lane[7] + 0x870070a6u;
    t0 ^= chain_store(c, t1);
    c->lane[6] += c->lane[15]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 13);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 23);
    t1 ^= (uint32_t)sift_frame(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3086u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint32_t tune_offset(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[10] += c->lane[3]; c->lane[2] ^= c->lane[10]; c->lane[2] = sx_rl(c->lane[2], 10);
    hold_bound(c, t0, t1);
    cache_count(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 1732u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3f166057u;
    c->lane[7] += c->lane[14] ^ 0x9aa7f2d3u;
    hold_index(c, t0, t1);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 9);
    c->lane[12] += c->lane[1] ^ 0x99aea47eu;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t prime_bucket(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xe7f97561u;
    t2 += (uint32_t)move_batch(c);
    t2 = (t2 ^ c->sum) * 0xee6680b7u;
    c->hash ^= c->lane[4] + 0xf72f7363u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x7e7ac069u) ^ sx_rr(c->hash, 13);
    c->hash ^= c->lane[1] + 0xfbb6ef3fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5d2d82c9u;
    c->hash ^= c->lane[3] + 0xd47f486au;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void push_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[14] = c->hash ^ sx_rl(c->lane[11], 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x40c24d2bu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 19);
    t2 += pick_delta(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x04) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 += (uint32_t)move_span(c);
    latch_cell(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43643u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x6296cdf3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd63ef2a5u;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int settle_scope_530(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->hash = (c->hash * 0x58b477f3u) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0xdc50bc73u) ^ sx_rr(c->hash, 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash = (c->hash * 0x7c592df9u) ^ sx_rr(c->hash, 5);
    c->hash = (c->hash * 0x3eeaa451u) ^ sx_rr(c->hash, 2);
    c->hash = (c->hash * 0x5c00eabdu) ^ sx_rr(c->hash, 30);
    c->lane[11] += c->lane[8]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 29);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 11);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t wrap_part(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xeb) << 16;
    c->lane[10] += c->lane[9] ^ 0x02d39293u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x35) << 8;
    c->lane[8] ^= sx_rl(c->lane[3], 27);
    c->lane[5] += c->lane[3]; c->lane[0] ^= c->lane[5]; c->lane[0] = sx_rl(c->lane[0], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 49894u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] += c->lane[0]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 5);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t clamp_entry(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31351u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x241fc8ffu) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa6) << 16;
    emit_frame(c, t0, t1);
    close_band(c, &c->lane[6], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25877u) % (uint32_t)c->rln)] << 24;
    c->lane[15] += c->lane[7]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 2);
    t2 += (uint32_t)push_marker(c);
    c->sched[11] = c->hash ^ sx_rl(c->lane[8], 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x23) << 8;
    c->raw[c->slo + (int)((t0 + 44925u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 2);
    t2 = (t2 ^ c->sum) * 0xefc70ea7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7299af6du;
    c->sum += t1;
    return t0 + t2;
}

static void sync_port(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[6] += c->lane[5] ^ 0x1a2bbe1cu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[13] ^= sx_rl(c->lane[7], 3);
    c->hash = (c->hash * 0x72b31745u) ^ sx_rr(c->hash, 3);
    c->lane[1] += c->lane[3]; c->lane[8] ^= c->lane[1]; c->lane[8] = sx_rl(c->lane[8], 20);
    c->sched[11] = c->hash ^ sx_rl(c->lane[8], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t1 ^= (uint32_t)probe_stream(c, (uint8_t)(t0 >> 16), t2);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static int split_group(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf50383cdu;
    store_tail(c, &c->lane[0], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55182u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x13) << 8;
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x86) << 0;
    tune_count(c, &c->lane[6], 3);
    c->raw[c->slo + (int)((t0 + 21647u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x87) << 0;
    c->hash ^= c->lane[0] + 0xb499930bu;
    c->lane[4] += c->lane[11]; c->lane[2] ^= c->lane[4]; c->lane[2] = sx_rl(c->lane[2], 28);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void clamp_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)reset_path_575(c);
    c->lane[9] += c->lane[8] ^ 0xfa4979f6u;
    c->lane[7] += c->lane[12] ^ 0x73e465a0u;
    c->hash = (c->hash * 0x152e8a5fu) ^ sx_rr(c->hash, 12);
    c->lane[7] ^= sx_rl(c->lane[1], 25);
    c->lane[8] += c->lane[8] ^ 0xade0a4e8u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t blend_row(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)coal_ring(c, (uint8_t)(t0 >> 0), t2);
    c->lane[11] += c->lane[4] ^ 0x42ed6ad1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2cfc69a9u;
    c->lane[13] ^= sx_rl(c->lane[6], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xaf) << 0;
    t2 = (t2 ^ c->sum) * 0x88870f6fu;
    peek_key(c, &c->lane[6], 2);
    c->sum += t1;
    return t0 + t2;
}

static void load_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] += c->lane[10] ^ 0xbffa6c4fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17365u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0xcf5708b5u;
    map_key(c, &c->lane[1], 3);
    t0 ^= seek_field(c, t1);
    c->lane[6] += c->lane[6] ^ 0x7f3f0669u;
    t0 ^= blend_tail(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 37887u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] ^= sx_rl(c->lane[13], 4);
    t2 = (t2 ^ c->sum) * 0x15f1b705u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 20);
    t2 += (uint32_t)latch_chunk(c);
    t0 ^= hold_ring(c, t1);
    t2 = (t2 ^ c->sum) * 0x532f664du;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void patch_stack(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x0f4b4d73u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5b182f57u;
    t2 = (t2 ^ c->sum) * 0xa8198ca5u;
    c->lane[15] += c->lane[5]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 2);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 20);
    t2 = (t2 ^ c->sum) * 0x5c287dedu;
    c->hash = (c->hash * 0xfe9d9227u) ^ sx_rr(c->hash, 4);
    poll_stream(c, &c->lane[11], 2);
    c->lane[3] += c->lane[10] ^ 0x4f806d35u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x49) << 16;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void cache_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 5);
    place_list(c, &c->lane[4], 4);
    c->hash = (c->hash * 0x66a24dc1u) ^ sx_rr(c->hash, 6);
    t1 ^= (uint32_t)sift_frame(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] += c->lane[8]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    parse_tail(c, t0, t1);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 23);
    c->sched[28] = c->hash ^ sx_rl(c->lane[14], 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 330u) % (uint32_t)c->rln)] << 8;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 23);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static void defer_row_540(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[5] + 0x04b0c0cdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54820u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[2] + 0x3111cbdbu;
    c->lane[1] += c->lane[12]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x61bc38a5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4b) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x16) << 16;
    t2 += (uint32_t)swap_state(c);
    c->lane[5] += c->lane[7] ^ 0x49f45187u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x4f) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t hold_unit_541(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] += c->lane[1] ^ 0x54efb439u;
    c->hash ^= c->lane[4] + 0x7d6d4e62u;
    c->raw[c->slo + (int)((t0 + 19385u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += move_tuple(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30791u) % (uint32_t)c->rln)] << 0;
    t0 ^= split_line(c, t1);
    c->hash ^= c->lane[15] + 0x4e77c655u;
    t2 = (t2 ^ c->sum) * 0x73e9a323u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 ^= hold_ring(c, t1);
    t2 += patch_table(c, c->slo, c->sln);
    c->lane[4] ^= sx_rl(c->lane[14], 31);
    c->hash ^= c->lane[11] + 0xce78fab1u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t tune_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[0] + 0x8b83dfb3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] += c->lane[7] ^ 0xba4d91bau;
    c->hash = (c->hash * 0x92e635b1u) ^ sx_rr(c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xaf) << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int tally_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash = (c->hash * 0xb9e63a55u) ^ sx_rr(c->hash, 3);
    c->raw[c->slo + (int)((t0 + 25846u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 16);
    settle_marker_559(c, &c->lane[11], 2);
    t2 = (t2 ^ c->sum) * 0x0ec8341fu;
    c->sched[22] = c->hash ^ sx_rl(c->lane[2], 23);
    t2 = (t2 ^ c->sum) * 0xd71cbdc3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x31) << 8;
    c->lane[3] ^= sx_rl(c->lane[12], 10);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void trim_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 59872u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5ff6830du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54574u) % (uint32_t)c->rln)] << 16;
    t0 ^= defer_index(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t map_unit_545(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x43) << 0;
    c->lane[6] += c->lane[9] ^ 0xcbfe0eeau;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 7);
    t1 ^= (uint32_t)trim_index(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3d) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x108dec3du;
    t2 += patch_gap(c, c->slo, c->sln);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    latch_seat(c, &c->lane[8], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55995u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tune_count(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[7] + 0xe5d66a0eu;
    t2 += scan_path(c, c->slo, c->sln);
    c->sched[7] = c->hash ^ sx_rl(c->lane[7], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13635u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[4] += c->lane[5] ^ 0x7cc79d0cu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 30);
    t2 = (t2 ^ c->sum) * 0x0e82cc4du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void hold_bound(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42639u) % (uint32_t)c->rln)] << 24;
    align_tail(c, &c->lane[2], 1);
    c->hash ^= c->lane[8] + 0xff18cfe6u;
    c->raw[c->slo + (int)((t0 + 5847u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 47990u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] ^= sx_rl(c->lane[11], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x40) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xff) << 16;
    t0 ^= wrap_frame(c, t1);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void peek_key(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[2] + 0x39acd3e9u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[9], 11);
    c->hash = (c->hash * 0x6030a43du) ^ sx_rr(c->hash, 8);
    c->lane[4] += c->lane[10] ^ 0xbc23624cu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pick_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] ^= sx_rl(c->lane[7], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcab91a9bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)drain_bound(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0xd76d54fbu) ^ sx_rr(c->hash, 28);
    c->hash ^= c->lane[4] + 0x85921653u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xce26a463u;
    c->raw[c->slo + (int)((t0 + 65014u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[13] = c->hash ^ sx_rl(c->lane[0], 6);
    c->lane[9] += c->lane[10] ^ 0x6e13dbf1u;
    c->hash ^= c->lane[15] + 0x4fecd3c0u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t defer_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += scan_layer(c, c->slo, c->sln);
    c->hash = (c->hash * 0x11ee121bu) ^ sx_rr(c->hash, 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63361u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe6) << 16;
    t2 += scan_path(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19567u) % (uint32_t)c->rln)] << 24;
    c->lane[8] ^= sx_rl(c->lane[5], 31);
    c->lane[8] += c->lane[11]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 13);
    c->raw[c->slo + (int)((t0 + 43415u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[26] = c->hash ^ sx_rl(c->lane[1], 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t probe_stream(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x66564305u) ^ sx_rr(c->hash, 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32226u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[1] + 0xba359d93u;
    c->lane[12] ^= sx_rl(c->lane[0], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 9);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t chain_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3e4bcf99u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x68) << 8;
    c->hash = (c->hash * 0x668a6ee5u) ^ sx_rr(c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[5] ^= sx_rl(c->lane[5], 1);
    t2 += (uint32_t)reset_state_620(c);
    c->lane[14] += c->lane[3] ^ 0x51e2f751u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int move_span(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t2 += (uint32_t)grow_bound(c);
    c->lane[11] += c->lane[8]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 15);
    prime_bucket_596(c, &c->lane[5], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 19351u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd8) << 0;
    pair_item(c, t0, t1);
    c->hash ^= c->lane[11] + 0x0bdf743fu;
    c->raw[c->slo + (int)((t0 + 34144u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x9fe75473u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t sift_frame(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 35265u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] += c->lane[0]; c->lane[1] ^= c->lane[6]; c->lane[1] = sx_rl(c->lane[1], 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x88) << 0;
    c->raw[c->slo + (int)((t0 + 39347u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[3] + 0x53fd2a92u;
    t2 += (uint32_t)defer_segment(c);
    c->hash = (c->hash * 0xb743d205u) ^ sx_rr(c->hash, 22);
    trace_tuple(c, &c->lane[9], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3e) << 8;
    t2 += reap_lease(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[11] += c->lane[15] ^ 0xeab5e6e7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t split_line(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x9301826du) ^ sx_rr(c->hash, 29);
    c->sched[16] = c->hash ^ sx_rl(c->lane[8], 29);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 13);
    t2 = (t2 ^ c->sum) * 0x09240497u;
    c->lane[4] ^= sx_rl(c->lane[4], 17);
    t2 = (t2 ^ c->sum) * 0xbef5f9a5u;
    c->sched[25] = c->hash ^ sx_rl(c->lane[4], 31);
    c->lane[12] += c->lane[4]; c->lane[0] ^= c->lane[12]; c->lane[0] = sx_rl(c->lane[0], 18);
    c->raw[c->slo + (int)((t0 + 4503u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63684u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t move_tuple(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += defer_table_614(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xff) << 16;
    t2 += cache_part_625(c, c->rlo, c->rln);
    c->lane[6] += c->lane[11] ^ 0xf196d6d5u;
    scan_bucket_587(c, t0, t1);
    parse_tail(c, t0, t1);
    t0 ^= load_token(c, t1);
    c->raw[c->slo + (int)((t0 + 57392u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 12);
    t2 = (t2 ^ c->sum) * 0xf1b79fbfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc7) << 0;
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 24);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t coal_ring(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[11], 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17124u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0xbf2e5751u;
    step_table(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xaf) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8a) << 16;
    t1 ^= (uint32_t)hold_state_623(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xdb9b7679u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void cache_count(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x57bfe583u) ^ sx_rr(c->hash, 5);
    c->lane[8] += c->lane[15] ^ 0x70897e72u;
    c->hash ^= c->lane[3] + 0xac726dcau;
    c->hash = (c->hash * 0xbeeb810fu) ^ sx_rr(c->hash, 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdb) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb86d5037u;
    c->hash ^= c->lane[10] + 0x2b92300cu;
    c->lane[1] ^= sx_rl(c->lane[7], 1);
    c->hash ^= c->lane[1] + 0xce20bcb8u;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void settle_marker_559(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)pin_count(c);
    emit_field(c, &c->lane[6], 2);
    c->hash = (c->hash * 0xaf63d6ffu) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3668f74du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0xefd438bfu;
    c->lane[6] ^= sx_rl(c->lane[3], 27);
    t2 += push_queue(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0xa9af538fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void close_band(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23348u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[15] + 0x4f89bb71u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbe9622efu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58081u) % (uint32_t)c->rln)] << 16;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 6);
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 14);
    c->lane[11] += c->lane[0]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa0) << 16;
    t0 ^= defer_offset(c, t1);
    c->hash = (c->hash * 0x6a9c082bu) ^ sx_rr(c->hash, 7);
    c->hash ^= c->lane[1] + 0x4acbca2cu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int move_batch(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->lane[6] += c->lane[5]; c->lane[1] ^= c->lane[6]; c->lane[1] = sx_rl(c->lane[1], 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17176u) % (uint32_t)c->rln)] << 24;
    emit_cell(c, t0, t1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 6);
    t2 = (t2 ^ c->sum) * 0xaa9871e3u;
    c->lane[5] += c->lane[4]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 10);
    c->lane[7] += c->lane[9] ^ 0x5596f62fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x7743725bu;
    c->lane[15] += c->lane[8]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 26);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t stage_run(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x3560978fu;
    c->raw[c->slo + (int)((t0 + 49584u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += load_cell(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x444e8f79u;
    c->lane[4] += c->lane[13]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 8);
    t0 ^= trim_range(c, t1);
    t0 ^= yield_state_598(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 += reap_lease(c, c->slo, c->sln);
    c->hash ^= c->lane[7] + 0xea7f1daau;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 4);
    sort_region(c, &c->lane[5], 4);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 10);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void grow_tuple(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x4c) << 0;
    c->lane[7] += c->lane[14] ^ 0x6482ef89u;
    yield_tuple(c, t0, t1);
    c->hash ^= c->lane[6] + 0xbdac9e05u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[3] = c->hash ^ sx_rl(c->lane[3], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37781u) % (uint32_t)c->rln)] << 24;
    c->sched[11] = c->hash ^ sx_rl(c->lane[6], 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x3f) << 16;
    store_index(c, &c->lane[5], 1);
    t2 += (uint32_t)sort_entry(c);
    c->raw[c->slo + (int)((t0 + 33492u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[15] = c->hash ^ sx_rl(c->lane[8], 26);
    c->raw[c->slo + (int)((t0 + 14502u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x642bad7bu) ^ sx_rr(c->hash, 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3c) << 8;
    c->lane[6] ^= sx_rl(c->lane[1], 15);
    t1 ^= (uint32_t)place_pairing_637(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 29964u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x59) << 8;
    c->lane[10] ^= sx_rl(c->lane[5], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8a44e793u;
    c->sum += t1;
    return t0 + t2;
}

static void latch_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x8e0a07a5u) ^ sx_rr(c->hash, 10);
    c->lane[5] ^= sx_rl(c->lane[6], 7);
    resize_key(c, t0, t1);
    c->hash = (c->hash * 0xb7436d8fu) ^ sx_rr(c->hash, 9);
    t1 ^= (uint32_t)poll_marker(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)slice_value(c);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 23);
    t0 ^= blend_tail(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t hold_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x208f853fu;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20579u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[15] += c->lane[12] ^ 0x20a2725fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xeecd28a3u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void map_key(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    join_segment(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x54011a73u;
    t2 += (uint32_t)chain_block(c);
    cache_field(c, t0, t1);
    c->lane[10] += c->lane[10] ^ 0xa0edfc74u;
    trim_unit(c, t0, t1);
    c->hash = (c->hash * 0x3bdaaa7bu) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0x923d1edfu) ^ sx_rr(c->hash, 30);
    c->lane[13] ^= sx_rl(c->lane[14], 16);
    t1 ^= (uint32_t)shift_cell(c, (uint8_t)(t0 >> 16), t2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t seek_field(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 33854u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 16);
    t2 = (t2 ^ c->sum) * 0xe6b3d54fu;
    c->raw[c->slo + (int)((t0 + 12128u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x31) << 8;
    c->raw[c->slo + (int)((t0 + 6381u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x911eae4bu;
    c->sched[21] = c->hash ^ sx_rl(c->lane[7], 27);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void place_list(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[14] = c->hash ^ sx_rl(c->lane[8], 23);
    t2 = (t2 ^ c->sum) * 0xfc622b87u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 13);
    emit_token(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 20447u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 16753u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int swap_state(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] ^= sx_rl(c->lane[9], 7);
    c->lane[5] += c->lane[3] ^ 0xe436e6e2u;
    c->lane[0] ^= sx_rl(c->lane[1], 11);
    c->hash = (c->hash * 0x8c4be385u) ^ sx_rr(c->hash, 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x41d44a0bu;
    c->lane[1] += c->lane[4] ^ 0x5a6c8b46u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int latch_chunk(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[11] ^= sx_rl(c->lane[3], 11);
    c->sched[23] = c->hash ^ sx_rl(c->lane[10], 7);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 13);
    c->lane[2] += c->lane[15]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x08) << 0;
    t1 ^= (uint32_t)grow_batch(c, (uint8_t)(t0 >> 16), t2);
    c->sched[9] = c->hash ^ sx_rl(c->lane[4], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x19c792d1u;
    c->lane[7] += c->lane[2]; c->lane[15] ^= c->lane[7]; c->lane[15] = sx_rl(c->lane[15], 6);
    c->sched[25] = c->hash ^ sx_rl(c->lane[11], 1);
    hold_index(c, t0, t1);
    t2 += map_range(c, c->slo, c->sln);
    t2 += (uint32_t)rotate_store_592(c);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void store_tail(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    resize_key(c, t0, t1);
    c->lane[1] ^= sx_rl(c->lane[2], 5);
    c->hash = (c->hash * 0xbb3e81f9u) ^ sx_rr(c->hash, 28);
    c->sched[28] = c->hash ^ sx_rl(c->lane[14], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[8] + 0xf2ab0140u;
    t1 ^= (uint32_t)slice_bucket_609(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xcc) << 8;
    c->sched[9] = c->hash ^ sx_rl(c->lane[13], 3);
    t1 ^= (uint32_t)peek_delta(c, (uint8_t)(t0 >> 16), t2);
    c->sum += t1;
    return t0 + t2;
}

static void emit_frame(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 += (uint32_t)reset_state_620(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x17) << 8;
    t1 ^= (uint32_t)poll_marker(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[5] + 0x0894b876u;
    c->raw[c->slo + (int)((t0 + 42822u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int reset_path_575(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    step_table(c, t0, t1);
    c->hash ^= c->lane[2] + 0xf5ca3e43u;
    t1 ^= (uint32_t)tally_page(c, (uint8_t)(t0 >> 16), t2);
    t2 += cache_list(c, c->rlo, c->rln);
    c->lane[7] ^= sx_rl(c->lane[10], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[14] += c->lane[5]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int push_marker(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->sched[27] = c->hash ^ sx_rl(c->lane[14], 14);
    c->lane[5] += c->lane[13] ^ 0xfc1d740au;
    c->lane[0] += c->lane[9] ^ 0x01946767u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x44) << 0;
    t2 += (uint32_t)latch_region(c);
    c->hash ^= c->lane[9] + 0x47b36bbfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x11) << 0;
    t2 += (uint32_t)store_ring(c);
    t2 = (t2 ^ c->sum) * 0x9dfec9cdu;
    c->lane[1] += c->lane[5] ^ 0x1cb73a0cu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t tally_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[13] + 0x644867b9u;
    c->lane[4] ^= sx_rl(c->lane[8], 29);
    c->hash ^= c->lane[15] + 0xbb1a93c5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2c71027du;
    t2 = (t2 ^ c->sum) * 0xc52450f5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62216u) % (uint32_t)c->rln)] << 8;
    t0 ^= slice_queue(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[17] = c->hash ^ sx_rl(c->lane[15], 10);
    c->lane[7] += c->lane[13] ^ 0x607a830au;
    t2 += (uint32_t)settle_value(c);
    t1 ^= (uint32_t)sort_page(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void latch_cell(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45502u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x500335f7u;
    t2 = (t2 ^ c->sum) * 0x353aedfdu;
    c->hash ^= c->lane[1] + 0x69b16fcau;
    t1 ^= (uint32_t)queue_token(c, (uint8_t)(t0 >> 0), t2);
    c->lane[12] += c->lane[12] ^ 0x97808da2u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t trim_index(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 42926u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 19);
    split_label(c, &c->lane[3], 3);
    c->hash ^= c->lane[1] + 0x4be2bec4u;
    t1 ^= (uint32_t)chain_pool(c, (uint8_t)(t0 >> 8), t2);
    c->lane[0] += c->lane[1] ^ 0xe78898e5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40062u) % (uint32_t)c->rln)] << 16;
    c->sched[12] = c->hash ^ sx_rl(c->lane[11], 11);
    t2 = (t2 ^ c->sum) * 0x9167637bu;
    c->hash = (c->hash * 0x326934c1u) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xac4f4509u;
    c->lane[1] += c->lane[5]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 17);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void poll_stream(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->sched[10] = c->hash ^ sx_rl(c->lane[6], 24);
    c->raw[c->slo + (int)((t0 + 7917u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22136u) % (uint32_t)c->rln)] << 16;
    c->lane[13] += c->lane[4]; c->lane[12] ^= c->lane[13]; c->lane[12] = sx_rl(c->lane[12], 28);
    t2 += grow_state(c, c->slo, c->sln);
    c->hash ^= c->lane[10] + 0x07c9c539u;
    c->lane[13] += c->lane[6]; c->lane[5] ^= c->lane[13]; c->lane[5] = sx_rl(c->lane[5], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30628u) % (uint32_t)c->rln)] << 24;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int sort_entry(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->raw[c->slo + (int)((t0 + 2274u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 24863u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[0] += c->lane[4] ^ 0xfd9740e5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sched[8] = c->hash ^ sx_rl(c->lane[5], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[6] = c->hash ^ sx_rl(c->lane[0], 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14799u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9383u) % (uint32_t)c->rln)] << 0;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t shift_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd9) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2129u) % (uint32_t)c->rln)] << 8;
    c->sched[24] = c->hash ^ sx_rl(c->lane[13], 24);
    c->lane[10] += c->lane[2] ^ 0xe242878cu;
    c->lane[6] ^= sx_rl(c->lane[1], 3);
    c->hash = (c->hash * 0x7861fa2fu) ^ sx_rr(c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5e4d093fu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t trim_range(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] ^= sx_rl(c->lane[9], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb3f06429u;
    c->hash = (c->hash * 0x997b33b3u) ^ sx_rr(c->hash, 3);
    c->sched[3] = c->hash ^ sx_rl(c->lane[2], 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51377u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19992u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x809950f7u) ^ sx_rr(c->hash, 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf3e2009du;
    c->hash ^= c->lane[11] + 0x0cbbb208u;
    c->lane[5] ^= sx_rl(c->lane[0], 6);
    t2 = (t2 ^ c->sum) * 0x86ae012fu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t peek_delta(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[14] ^= sx_rl(c->lane[14], 14);
    c->lane[11] += c->lane[15]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= c->lane[2] + 0x2d146d2eu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t load_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xaa7af7e7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcb) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11290u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc4) << 0;
    c->sum += t1;
    return t0 + t2;
}

static void join_segment(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x49) << 0;
    c->lane[11] ^= sx_rl(c->lane[3], 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x88) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50033u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x6dee32e1u) ^ sx_rr(c->hash, 24);
    c->lane[15] += c->lane[7]; c->lane[3] ^= c->lane[15]; c->lane[3] = sx_rl(c->lane[3], 1);
    c->lane[11] ^= sx_rl(c->lane[12], 6);
    c->lane[0] += c->lane[8]; c->lane[15] ^= c->lane[0]; c->lane[15] = sx_rl(c->lane[15], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash = (c->hash * 0x9ecb9f4bu) ^ sx_rr(c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void scan_bucket_587(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61705u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7115u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[7] + 0xd3e7838bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfaa585c7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t poll_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59990u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x47aea9bdu;
    c->raw[c->slo + (int)((t0 + 63398u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xed) << 16;
    c->lane[2] += c->lane[2] ^ 0x5e5c68d2u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[15], 26);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void parse_tail(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xca) << 0;
    c->lane[13] ^= sx_rl(c->lane[7], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57291u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x05c646a7u;
    c->lane[1] += c->lane[12]; c->lane[8] ^= c->lane[1]; c->lane[8] = sx_rl(c->lane[8], 5);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 6);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 16);
    c->sched[2] = c->hash ^ sx_rl(c->lane[7], 15);
    c->hash ^= c->lane[0] + 0xa9873f11u;
    c->hash ^= c->lane[2] + 0x4478e88fu;
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t blend_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xd025bc8du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash = (c->hash * 0xb7cc86afu) ^ sx_rr(c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa838adadu;
    c->lane[8] += c->lane[15]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 9);
    c->lane[13] ^= sx_rl(c->lane[15], 2);
    t2 = (t2 ^ c->sum) * 0x16184061u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3726u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int defer_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->hash ^= c->lane[11] + 0xf3c4f741u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[11] += c->lane[0]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 31);
    c->hash ^= c->lane[0] + 0x2f95a8d2u;
    c->sched[9] = c->hash ^ sx_rl(c->lane[13], 5);
    c->raw[c->slo + (int)((t0 + 39621u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9095760bu;
    c->hash = (c->hash * 0x0076717du) ^ sx_rr(c->hash, 6);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 13);
    c->sched[0] = c->hash ^ sx_rl(c->lane[10], 3);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int rotate_store_592(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash = (c->hash * 0x98a60e9fu) ^ sx_rr(c->hash, 26);
    c->raw[c->slo + (int)((t0 + 2670u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 52855u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 20);
    t2 = (t2 ^ c->sum) * 0x75de8319u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash = (c->hash * 0x072fc6a7u) ^ sx_rr(c->hash, 9);
    t2 = (t2 ^ c->sum) * 0xdc3dd02du;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void hold_index(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x9ebfcac9u) ^ sx_rr(c->hash, 26);
    c->sched[10] = c->hash ^ sx_rl(c->lane[5], 23);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[8] += c->lane[11]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 3);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 2);
    c->raw[c->slo + (int)((t0 + 32575u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[7] = c->hash ^ sx_rl(c->lane[12], 28);
    c->sched[0] = c->hash ^ sx_rl(c->lane[9], 1);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void yield_tuple(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] += c->lane[6]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 23);
    c->lane[3] += c->lane[8] ^ 0x22c9a52bu;
    c->hash ^= c->lane[0] + 0xa68691a5u;
    c->sched[8] = c->hash ^ sx_rl(c->lane[5], 17);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static int latch_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x95) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x27) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[13] += c->lane[2]; c->lane[0] ^= c->lane[13]; c->lane[0] = sx_rl(c->lane[0], 7);
    c->lane[1] += c->lane[4]; c->lane[12] ^= c->lane[1]; c->lane[12] = sx_rl(c->lane[12], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void prime_bucket_596(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[6] = c->hash ^ sx_rl(c->lane[12], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 17478u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] += c->lane[1]; c->lane[7] ^= c->lane[12]; c->lane[7] = sx_rl(c->lane[7], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[0], 8);
    c->sched[10] = c->hash ^ sx_rl(c->lane[3], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int pin_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t2 = (t2 ^ c->sum) * 0xb1ffc4afu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 18);
    c->hash = (c->hash * 0xb5db91cdu) ^ sx_rr(c->hash, 4);
    c->lane[5] ^= sx_rl(c->lane[2], 13);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t yield_state_598(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8fb54163u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 2);
    c->lane[10] ^= sx_rl(c->lane[9], 23);
    c->hash ^= c->lane[11] + 0x2b569bb3u;
    c->sched[19] = c->hash ^ sx_rl(c->lane[4], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb6e7fdfbu;
    c->lane[13] ^= sx_rl(c->lane[11], 28);
    c->sched[30] = c->hash ^ sx_rl(c->lane[15], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4e75f219u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int grow_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->raw[c->slo + (int)((t0 + 59361u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x34c273bfu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40852u) % (uint32_t)c->rln)] << 16;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t cache_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum += t1;
    return t0 + t2;
}

static void trim_unit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x51fbbf97u;
    c->lane[14] += c->lane[8]; c->lane[9] ^= c->lane[14]; c->lane[9] = sx_rl(c->lane[9], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x73592361u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 1);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 2);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 8);
    c->raw[c->slo + (int)((t0 + 5295u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void trace_tuple(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[9] ^= sx_rl(c->lane[12], 23);
    c->sched[2] = c->hash ^ sx_rl(c->lane[5], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x455a3f49u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t push_queue(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] ^= sx_rl(c->lane[11], 16);
    c->hash = (c->hash * 0xe2dbe481u) ^ sx_rr(c->hash, 3);
    c->hash ^= c->lane[13] + 0x23d967dau;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59387u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x1f) << 0;
    c->lane[6] ^= sx_rl(c->lane[10], 1);
    c->sum += t1;
    return t0 + t2;
}

static int settle_value(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[1] ^= sx_rl(c->lane[8], 24);
    t2 = (t2 ^ c->sum) * 0x6d3f3595u;
    c->sched[22] = c->hash ^ sx_rl(c->lane[2], 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x06) << 8;
    c->lane[5] += c->lane[8] ^ 0x890f4cb6u;
    c->hash ^= c->lane[8] + 0x7440383au;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t grow_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 56266u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[26] = c->hash ^ sx_rl(c->lane[13], 12);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57002u) % (uint32_t)c->rln)] << 24;
    c->lane[3] += c->lane[0]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 3);
    c->hash = (c->hash * 0x9ec7e4c1u) ^ sx_rr(c->hash, 27);
    c->raw[c->slo + (int)((t0 + 57467u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] += c->lane[14]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 27);
    c->hash = (c->hash * 0x8c9ef825u) ^ sx_rr(c->hash, 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[10] += c->lane[12] ^ 0x64b10f3du;
    c->sum += t1;
    return t0 + t2;
}

static int slice_value(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash = (c->hash * 0x8ce8f387u) ^ sx_rr(c->hash, 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe1) << 16;
    c->lane[8] += c->lane[9]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 10);
    c->sched[31] = c->hash ^ sx_rl(c->lane[1], 5);
    c->hash ^= c->lane[2] + 0x5cb3e013u;
    c->raw[c->slo + (int)((t0 + 35344u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void cache_field(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd74d3b9bu;
    c->lane[1] ^= sx_rl(c->lane[3], 23);
    c->lane[5] += c->lane[8]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 10);
    c->lane[12] ^= sx_rl(c->lane[1], 26);
    c->sched[29] = c->hash ^ sx_rl(c->lane[9], 18);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46508u) % (uint32_t)c->rln)] << 8;
    c->lane[14] += c->lane[12]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 16);
    c->lane[8] += c->lane[2]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 9);
    c->hash ^= c->lane[15] + 0xd632c0ccu;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t tally_page(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x546e0d5fu) ^ sx_rr(c->hash, 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34135u) % (uint32_t)c->rln)] << 0;
    c->lane[5] += c->lane[7] ^ 0xb6ca3028u;
    c->lane[12] += c->lane[4] ^ 0xea78dc20u;
    c->lane[6] += c->lane[1]; c->lane[8] ^= c->lane[6]; c->lane[8] = sx_rl(c->lane[8], 1);
    t2 = (t2 ^ c->sum) * 0x12a58ac7u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[9], 3);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t slice_bucket_609(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcccbf887u;
    c->sched[16] = c->hash ^ sx_rl(c->lane[0], 12);
    c->sched[25] = c->hash ^ sx_rl(c->lane[1], 15);
    c->lane[2] += c->lane[7] ^ 0xfdd97699u;
    c->lane[10] += c->lane[8]; c->lane[4] ^= c->lane[10]; c->lane[4] = sx_rl(c->lane[4], 7);
    c->lane[1] += c->lane[2]; c->lane[10] ^= c->lane[1]; c->lane[10] = sx_rl(c->lane[10], 9);
    t2 = (t2 ^ c->sum) * 0x813d3399u;
    c->lane[12] += c->lane[6] ^ 0x7804583eu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t queue_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbb) << 0;
    c->lane[3] += c->lane[1]; c->lane[10] ^= c->lane[3]; c->lane[10] = sx_rl(c->lane[10], 20);
    c->hash ^= c->lane[7] + 0x490cb903u;
    c->raw[c->slo + (int)((t0 + 48648u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc6a55cb9u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t slice_queue(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xee9200e3u;
    c->raw[c->slo + (int)((t0 + 48826u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[14] = c->hash ^ sx_rl(c->lane[8], 19);
    c->lane[13] += c->lane[14]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 31);
    c->lane[6] += c->lane[6] ^ 0xdb6f1d03u;
    c->lane[11] ^= sx_rl(c->lane[2], 22);
    t2 = (t2 ^ c->sum) * 0x06583431u;
    c->sched[8] = c->hash ^ sx_rl(c->lane[7], 17);
    c->lane[6] += c->lane[9]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 3);
    t2 = (t2 ^ c->sum) * 0x6f23b7f1u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t scan_path(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] += c->lane[6] ^ 0xdfdac889u;
    c->lane[14] += c->lane[14] ^ 0x15893377u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x56ca2f25u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[8] ^= sx_rl(c->lane[3], 30);
    c->hash = (c->hash * 0x383ecf1du) ^ sx_rr(c->hash, 12);
    c->lane[11] += c->lane[2] ^ 0x5dd54cacu;
    c->lane[14] ^= sx_rl(c->lane[1], 22);
    c->lane[9] += c->lane[6]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 6);
    c->sum += t1;
    return t0 + t2;
}

static void resize_key(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x506b387du;
    c->lane[1] += c->lane[12] ^ 0x3de93fbfu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2442e21fu;
    c->hash ^= c->lane[11] + 0xe10638c4u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd5) << 0;
    c->lane[14] ^= sx_rl(c->lane[2], 11);
    c->lane[13] += c->lane[13] ^ 0x1bae830fu;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint32_t defer_table_614(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe93495fdu;
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 17);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 9);
    t2 = (t2 ^ c->sum) * 0xe04b48b5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7b) << 8;
    c->lane[1] += c->lane[2] ^ 0x354dc6cbu;
    c->lane[6] ^= sx_rl(c->lane[13], 13);
    c->lane[11] += c->lane[14]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 7);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 8);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t reap_lease(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[5] + 0x03f7e4afu;
    c->lane[0] += c->lane[14] ^ 0x3eaebe59u;
    t2 = (t2 ^ c->sum) * 0xf219698du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum += t1;
    return t0 + t2;
}

static int store_ring(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x0ef986d3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3b) << 0;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[20] = c->hash ^ sx_rl(c->lane[10], 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9ed988e5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 295u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0xbc178825u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28455u) % (uint32_t)c->rln)] << 8;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t defer_offset(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[5] += c->lane[3]; c->lane[9] ^= c->lane[5]; c->lane[9] = sx_rl(c->lane[9], 5);
    c->hash ^= c->lane[3] + 0x279f0583u;
    c->lane[4] ^= sx_rl(c->lane[15], 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59339u) % (uint32_t)c->rln)] << 0;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 1);
    c->lane[11] ^= sx_rl(c->lane[12], 21);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t load_token(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 28);
    c->lane[10] += c->lane[14] ^ 0xde945ff1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x28ab2883u;
    c->hash ^= c->lane[10] + 0xd3fc319du;
    c->sched[19] = c->hash ^ sx_rl(c->lane[1], 15);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t grow_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 22);
    c->raw[c->slo + (int)((t0 + 41764u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[8] + 0xfc66adbeu;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x6d2c4b0du;
    c->lane[6] ^= sx_rl(c->lane[2], 12);
    c->raw[c->slo + (int)((t0 + 7750u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int reset_state_620(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x61) << 8;
    t2 = (t2 ^ c->sum) * 0x48adf2b3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf8) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xecd56299u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44722u) % (uint32_t)c->rln)] << 0;
    c->lane[11] += c->lane[4] ^ 0x0670e20du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1f6b9f89u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36247u) % (uint32_t)c->rln)] << 0;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void split_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 2);
    c->raw[c->slo + (int)((t0 + 5891u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x86f7c0e5u;
    c->lane[3] += c->lane[2]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 19);
    c->raw[c->slo + (int)((t0 + 4080u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void emit_cell(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xf1a5187du) ^ sx_rr(c->hash, 20);
    c->hash ^= c->lane[12] + 0x31bf913eu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfa606409u;
    c->sched[31] = c->hash ^ sx_rl(c->lane[12], 17);
    t2 = (t2 ^ c->sum) * 0x71609949u;
    c->lane[6] ^= sx_rl(c->lane[10], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8a) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t hold_state_623(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe7) << 0;
    c->lane[12] += c->lane[8] ^ 0x598545a8u;
    c->lane[13] ^= sx_rl(c->lane[14], 16);
    c->hash ^= c->lane[11] + 0x2e318026u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xee3ff197u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void step_table(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xbd5d5fa5u) ^ sx_rr(c->hash, 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47972u) % (uint32_t)c->rln)] << 16;
    c->sched[15] = c->hash ^ sx_rl(c->lane[3], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash = (c->hash * 0x8361f4ffu) ^ sx_rr(c->hash, 28);
    t2 = (t2 ^ c->sum) * 0x78a2ca0fu;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t cache_part_625(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] ^= sx_rl(c->lane[5], 9);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 31);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x910edffdu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 59325u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 22);
    c->lane[8] += c->lane[14] ^ 0x40ec45c1u;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 18);
    c->lane[1] += c->lane[9]; c->lane[8] ^= c->lane[1]; c->lane[8] = sx_rl(c->lane[8], 26);
    c->sum += t1;
    return t0 + t2;
}

static void pair_item(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x78060f89u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[5], 21);
    c->lane[7] += c->lane[12]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 23);
    t2 = (t2 ^ c->sum) * 0x9d75732bu;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t wrap_frame(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x62f212c9u) ^ sx_rr(c->hash, 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[4] += c->lane[3] ^ 0x361554c2u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 21343u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] ^= sx_rl(c->lane[4], 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t drain_bound(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    resize_store(c, &c->lane[2], 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4dab3f01u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55810u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[5] + 0xe8ccb9d8u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[12], 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void emit_field(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xa8b76491u;
    c->hash = (c->hash * 0x7a071465u) ^ sx_rr(c->hash, 3);
    c->hash = (c->hash * 0xaf015ff9u) ^ sx_rr(c->hash, 15);
    c->raw[c->slo + (int)((t0 + 11483u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[5] + 0x4698575bu;
    c->lane[9] += c->lane[14]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[13] + 0xb98226bcu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3a) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30777u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void align_tail(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[2] += c->lane[0]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[9] = c->hash ^ sx_rl(c->lane[9], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x17) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t map_range(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x69) << 16;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 17);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1aa94b07u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x07) << 8;
    c->lane[11] += c->lane[2] ^ 0x1eb81981u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8f) << 16;
    c->raw[c->slo + (int)((t0 + 43573u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum += t1;
    return t0 + t2;
}

static uint8_t chain_pool(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] += c->lane[12] ^ 0x39117e7fu;
    t0 ^= reap_view(c, t1);
    c->lane[11] += c->lane[0] ^ 0x3d0dccccu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x68) << 16;
    c->raw[c->slo + (int)((t0 + 36235u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t scan_layer(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 8486u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd0071961u;
    t2 = (t2 ^ c->sum) * 0xa5d616d7u;
    c->hash ^= c->lane[6] + 0x416de2abu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47210u) % (uint32_t)c->rln)] << 8;
    c->sum += t1;
    return t0 + t2;
}

static void emit_token(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x2627ee13u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x2f6d7f39u;
    c->hash ^= c->lane[10] + 0xf6c8b54bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x84521ca9u;
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint8_t sort_page(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[11] += c->lane[7] ^ 0x989e8278u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[2] ^= sx_rl(c->lane[3], 21);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void sort_region(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbc) << 8;
    c->hash = (c->hash * 0x88a64acdu) ^ sx_rr(c->hash, 28);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->lane[3] += c->lane[6] ^ 0xff7cb170u;
    c->lane[4] += c->lane[0]; c->lane[13] ^= c->lane[4]; c->lane[13] = sx_rl(c->lane[13], 30);
    c->hash ^= c->lane[10] + 0x800d2505u;
    c->lane[6] += c->lane[14] ^ 0xdd6799bcu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 23680u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x21) << 8;
    c->hash = (c->hash * 0xdf154925u) ^ sx_rr(c->hash, 9);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t place_pairing_637(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 7847u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x37) << 0;
    c->hash = (c->hash * 0x7571e01fu) ^ sx_rr(c->hash, 9);
    c->lane[14] += c->lane[5]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 10);
    c->hash ^= c->lane[7] + 0x217cf788u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int chain_block(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->sched[17] = c->hash ^ sx_rl(c->lane[4], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x36ef3367u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 7);
    c->raw[c->slo + (int)((t0 + 35098u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x58d34fefu;
    c->hash ^= c->lane[9] + 0x2e9b72dcu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 13);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void store_index(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] ^= sx_rl(c->lane[2], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21220u) % (uint32_t)c->rln)] << 16;
    c->lane[4] += c->lane[5] ^ 0xbcaa7cc4u;
    c->raw[c->slo + (int)((t0 + 43420u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void reap_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= scan_item_694(c, t1);
    c->raw[c->slo + (int)((t0 + 5601u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += rotate_pairing(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 1253u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] += c->lane[9]; c->lane[7] ^= c->lane[15]; c->lane[7] = sx_rl(c->lane[7], 6);
    t2 = (t2 ^ c->sum) * 0x181d578bu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 19);
    scan_item(c, t0, t1);
    t0 ^= yield_stack(c, t1);
    c->lane[10] += c->lane[14] ^ 0x251b11fbu;
    tap_unit_695(c, &c->lane[0], 4);
    t0 ^= defer_port(c, t1);
    c->hash ^= c->lane[9] + 0x787f0f3du;
    t0 ^= tune_field_643(c, t1);
    c->lane[4] += c->lane[13] ^ 0x15062929u;
    t0 ^= latch_range(c, t1);
    t2 += (uint32_t)pin_label(c);
    t2 += (uint32_t)patch_chunk(c);
    t0 ^= join_stack(c, t1);
    relay_line(c, &c->lane[10], 4);
    poll_line_657(c, &c->lane[1], 3);
    pin_band(c, &c->lane[2], 3);
    t2 += (uint32_t)grow_store_667(c);
    tune_cursor(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t join_stack(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    pick_digest_661(c, &c->lane[7], 1);
    t2 += (uint32_t)cache_queue(c);
    t0 ^= chain_rate(c, t1);
    tune_state(c, &c->lane[10], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x55) << 16;
    c->raw[c->slo + (int)((t0 + 41416u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t1 ^= (uint32_t)wrap_path_683(c, (uint8_t)(t0 >> 0), t2);
    tap_run(c, t0, t1);
    c->lane[6] += c->lane[2] ^ 0x7ce20220u;
    t2 += (uint32_t)shift_page_685(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x83) << 8;
    t2 += yield_bound(c, c->rlo, c->rln);
    stage_mask(c, t0, t1);
    t2 += blend_digest(c, c->slo, c->sln);
    step_delta(c, &c->lane[1], 1);
    c->raw[c->slo + (int)((t0 + 1409u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += slice_list(c, c->slo, c->sln);
    c->sched[19] = c->hash ^ sx_rl(c->lane[11], 4);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t yield_stack(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    scan_tuple(c, &c->lane[7], 4);
    prime_bound_675(c, &c->lane[5], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbeaa50ebu;
    merge_token_649(c, t0, t1);
    t2 += (uint32_t)tune_list(c);
    c->sched[15] = c->hash ^ sx_rl(c->lane[13], 22);
    t2 = (t2 ^ c->sum) * 0x8500246bu;
    c->sched[15] = c->hash ^ sx_rl(c->lane[14], 4);
    t2 += (uint32_t)pin_part(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x78) << 16;
    fetch_mask(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf408d6d7u;
    t0 ^= chain_tuple(c, t1);
    t1 ^= (uint32_t)align_record(c, (uint8_t)(t0 >> 0), t2);
    step_store(c, &c->lane[9], 4);
    c->lane[0] ^= sx_rl(c->lane[5], 21);
    t0 ^= reset_pairing(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59779u) % (uint32_t)c->rln)] << 8;
    c->sched[23] = c->hash ^ sx_rl(c->lane[4], 23);
    reset_band(c, &c->lane[10], 1);
    t0 ^= chain_limit(c, t1);
    t1 ^= (uint32_t)cache_limit(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)clamp_group_704(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tune_field_643(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[14] += c->lane[3]; c->lane[1] ^= c->lane[14]; c->lane[1] = sx_rl(c->lane[1], 23);
    tap_limit(c, t0, t1);
    t1 ^= (uint32_t)yield_region(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0x6ecd6273u) ^ sx_rr(c->hash, 17);
    t1 ^= (uint32_t)close_digest(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)parse_block_689(c);
    pair_field(c, t0, t1);
    c->lane[2] ^= sx_rl(c->lane[8], 21);
    c->lane[2] += c->lane[7]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe8) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x38b9fd41u;
    t2 += blend_rate(c, c->slo, c->sln);
    t1 ^= (uint32_t)resize_lease(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void scan_tuple(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    mix_queue(c, &c->lane[3], 1);
    pair_field(c, t0, t1);
    c->lane[5] += c->lane[11]; c->lane[14] ^= c->lane[5]; c->lane[14] = sx_rl(c->lane[14], 12);
    c->hash ^= c->lane[4] + 0xfa4f4fd9u;
    t0 ^= drain_port(c, t1);
    c->lane[10] += c->lane[5]; c->lane[6] ^= c->lane[10]; c->lane[6] = sx_rl(c->lane[6], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void reset_band(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 12);
    t1 ^= (uint32_t)hold_stack(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x36) << 16;
    c->raw[c->slo + (int)((t0 + 44257u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42794u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[12] + 0x9b0bc8ddu;
    c->raw[c->slo + (int)((t0 + 46193u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int tune_list(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    t2 = (t2 ^ c->sum) * 0x10bf5181u;
    c->lane[3] += c->lane[10]; c->lane[8] ^= c->lane[3]; c->lane[8] = sx_rl(c->lane[8], 19);
    c->sched[18] = c->hash ^ sx_rl(c->lane[10], 23);
    t2 = (t2 ^ c->sum) * 0x7b7601d1u;
    c->raw[c->slo + (int)((t0 + 6804u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    scan_item(c, t0, t1);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tune_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] += c->lane[3]; c->lane[7] ^= c->lane[0]; c->lane[7] = sx_rl(c->lane[7], 5);
    t2 += (uint32_t)shift_state(c);
    c->lane[11] ^= sx_rl(c->lane[3], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x60) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xef) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x74) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[12] ^= sx_rl(c->lane[7], 31);
    c->raw[c->slo + (int)((t0 + 10647u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[12] += c->lane[8] ^ 0x02cccb4fu;
    t2 = (t2 ^ c->sum) * 0x2a0dcea7u;
    step_store(c, &c->lane[2], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t yield_region(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x53c869e9u;
    c->hash = (c->hash * 0x6428e091u) ^ sx_rr(c->hash, 26);
    t2 += (uint32_t)hold_queue(c);
    t2 += (uint32_t)grow_store_667(c);
    c->raw[c->slo + (int)((t0 + 27876u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50401u) % (uint32_t)c->rln)] << 16;
    tune_cursor(c, t0, t1);
    t0 ^= yield_track(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void merge_token_649(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xaf57bbe9u;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 28);
    c->lane[5] += c->lane[2]; c->lane[13] ^= c->lane[5]; c->lane[13] = sx_rl(c->lane[13], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x45) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf9) << 16;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 8);
    t2 += map_chunk(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x55f4e863u;
    c->hash ^= c->lane[9] + 0x8e3c567fu;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t blend_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[15] + 0x20323a81u;
    t2 += map_chunk(c, c->slo, c->sln);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 11);
    c->lane[8] += c->lane[4] ^ 0x5271280fu;
    pin_band(c, &c->lane[1], 4);
    store_bound(c, &c->lane[1], 2);
    pick_digest_661(c, &c->lane[5], 4);
    t2 = (t2 ^ c->sum) * 0xb26b6ed7u;
    t1 ^= (uint32_t)mark_item(c, (uint8_t)(t0 >> 16), t2);
    c->sched[16] = c->hash ^ sx_rl(c->lane[15], 10);
    t0 ^= reset_pairing(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 7);
    c->sum += t1;
    return t0 + t2;
}

static int pin_part(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[4] = c->hash ^ sx_rl(c->lane[4], 29);
    t2 += (uint32_t)tune_state_656(c);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 21);
    t2 += rotate_pairing(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 214u) % (uint32_t)c->rln)] << 0;
    c->sched[5] = c->hash ^ sx_rl(c->lane[3], 18);
    t0 ^= move_store(c, t1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t cache_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x05193493u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x23a9915du;
    c->lane[4] += c->lane[7]; c->lane[15] ^= c->lane[4]; c->lane[15] = sx_rl(c->lane[15], 6);
    t0 ^= move_store(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x30fdcc53u;
    c->hash ^= c->lane[15] + 0xdba46db3u;
    c->lane[5] ^= sx_rl(c->lane[8], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void step_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[14] ^= sx_rl(c->lane[15], 22);
    t2 = (t2 ^ c->sum) * 0x867dbb83u;
    t1 ^= (uint32_t)emit_path(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)fill_list(c, (uint8_t)(t0 >> 8), t2);
    c->lane[14] += c->lane[14] ^ 0x8d069c81u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x07) << 16;
    c->raw[c->slo + (int)((t0 + 21788u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    poll_line_657(c, &c->lane[0], 3);
    c->sched[22] = c->hash ^ sx_rl(c->lane[7], 12);
    c->sched[26] = c->hash ^ sx_rl(c->lane[11], 17);
    c->hash = (c->hash * 0x3e10c573u) ^ sx_rr(c->hash, 28);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    prime_label(c, t0, t1);
    t0 ^= tally_layer_670(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x64) << 0;
    t2 += (uint32_t)settle_ring(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void pin_band(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[9] ^= sx_rl(c->lane[13], 27);
    t0 ^= latch_range(c, t1);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53090u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfe9448b1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x27) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[4] += c->lane[4] ^ 0x32a8d009u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t move_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)pack_field(c);
    c->hash = (c->hash * 0xbdd0514du) ^ sx_rr(c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x56c87aa9u) ^ sx_rr(c->hash, 29);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 9);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 27);
    t0 ^= move_limit(c, t1);
    tap_limit(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int tune_state_656(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    t1 ^= (uint32_t)pin_level(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7c) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57140u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x7ed7c5c5u) ^ sx_rr(c->hash, 9);
    t2 = (t2 ^ c->sum) * 0x06f98819u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8229u) % (uint32_t)c->rln)] << 16;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 14);
    c->hash ^= c->lane[6] + 0xce851979u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x7a8ee789u) ^ sx_rr(c->hash, 11);
    t2 = (t2 ^ c->sum) * 0xe4af3b5fu;
    prime_bound_675(c, &c->lane[0], 3);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void poll_line_657(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x68465e41u;
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 2);
    t1 ^= (uint32_t)prime_count(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= chain_rate(c, t1);
    c->lane[9] += c->lane[10]; c->lane[12] ^= c->lane[9]; c->lane[12] = sx_rl(c->lane[12], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48758u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)grow_tail(c, (uint8_t)(t0 >> 0), t2);
    t2 += mix_gap(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0x95dc5b5bu;
    c->lane[0] ^= sx_rl(c->lane[10], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51989u) % (uint32_t)c->rln)] << 8;
    c->lane[8] += c->lane[6]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int shift_state(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[11] ^= sx_rl(c->lane[1], 14);
    c->sched[30] = c->hash ^ sx_rl(c->lane[0], 3);
    t2 = (t2 ^ c->sum) * 0x007ce6fdu;
    t2 += (uint32_t)tally_level(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[11] += c->lane[9] ^ 0x47213af1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1bedbf15u;
    c->lane[1] ^= sx_rl(c->lane[0], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x84f2cb11u;
    c->hash ^= c->lane[1] + 0x88baa716u;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t reset_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[15] += c->lane[0] ^ 0xf1e67c5cu;
    c->hash = (c->hash * 0x3f7f92cdu) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb301360fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x2d) << 0;
    c->lane[8] ^= sx_rl(c->lane[4], 19);
    c->raw[c->slo + (int)((t0 + 17834u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    store_bound(c, &c->lane[8], 3);
    c->lane[15] ^= sx_rl(c->lane[0], 11);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t rotate_pairing(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 9);
    c->raw[c->slo + (int)((t0 + 60796u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[3] += c->lane[0] ^ 0xfff1291du;
    t2 += (uint32_t)pin_label(c);
    c->lane[8] ^= sx_rl(c->lane[7], 25);
    c->lane[9] += c->lane[14] ^ 0x01690720u;
    tap_unit_686(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 25191u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38961u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static void pick_digest_661(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb62b02a3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    mix_queue(c, &c->lane[0], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 30);
    c->hash ^= c->lane[2] + 0xb4bd5e31u;
    t2 += (uint32_t)tally_level(c);
    c->raw[c->slo + (int)((t0 + 53603u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4ac19247u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54686u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x851f1bf7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41427u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void tune_cursor(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] += c->lane[5]; c->lane[13] ^= c->lane[6]; c->lane[13] = sx_rl(c->lane[13], 11);
    c->raw[c->slo + (int)((t0 + 19876u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 33049u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= defer_index_684(c, t1);
    c->hash = (c->hash * 0x00bc6f23u) ^ sx_rr(c->hash, 24);
    c->sched[29] = c->hash ^ sx_rl(c->lane[6], 23);
    c->hash = (c->hash * 0x05ff5d2du) ^ sx_rr(c->hash, 5);
    emit_unit(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x090dd179u;
    t1 ^= (uint32_t)clamp_group_704(c, (uint8_t)(t0 >> 16), t2);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t drain_port(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[13] = c->hash ^ sx_rl(c->lane[15], 13);
    c->hash ^= c->lane[11] + 0x2acb1c77u;
    c->lane[8] ^= sx_rl(c->lane[4], 17);
    t2 += fill_track(c, c->rlo, c->rln);
    c->hash = (c->hash * 0xeb877799u) ^ sx_rr(c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 ^= tap_port_690(c, t1);
    c->lane[4] += c->lane[1]; c->lane[13] ^= c->lane[4]; c->lane[13] = sx_rl(c->lane[13], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5c4c5c29u;
    t2 += yield_bound(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x096ae60bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t yield_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x6c8ffc4du;
    t0 ^= grow_batch_691(c, t1);
    t2 += mix_gap(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x83) << 0;
    c->lane[15] += c->lane[2] ^ 0xafe8f515u;
    c->lane[3] ^= sx_rl(c->lane[10], 11);
    t0 ^= chain_limit(c, t1);
    t2 += (uint32_t)seek_slot(c);
    c->sched[20] = c->hash ^ sx_rl(c->lane[4], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc916992du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void step_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)join_pairing_706(c);
    c->hash = (c->hash * 0x2ff90e55u) ^ sx_rr(c->hash, 2);
    c->lane[5] += c->lane[7] ^ 0x393877ddu;
    c->lane[15] ^= sx_rl(c->lane[14], 26);
    push_stack(c, &c->lane[11], 3);
    c->lane[0] += c->lane[4]; c->lane[13] ^= c->lane[0]; c->lane[13] = sx_rl(c->lane[13], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35065u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)store_gap(c, (uint8_t)(t0 >> 8), t2);
    c->sched[17] = c->hash ^ sx_rl(c->lane[11], 22);
    t1 ^= (uint32_t)wrap_path_683(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[10] + 0xfe5dccedu;
    t2 += (uint32_t)scan_part(c);
    t2 = (t2 ^ c->sum) * 0xf4b2b4cbu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t map_chunk(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 2088u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 46516u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= drain_state(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static int grow_store_667(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8e) << 0;
    c->hash = (c->hash * 0x4875db07u) ^ sx_rr(c->hash, 28);
    t2 = (t2 ^ c->sum) * 0x7edd1ab3u;
    c->lane[8] += c->lane[13] ^ 0xd2433d97u;
    c->lane[11] ^= sx_rl(c->lane[5], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1cd88cc5u;
    t0 ^= scan_item_694(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[1] += c->lane[14] ^ 0x18bae675u;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void scan_item(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa3d270b7u;
    c->raw[c->slo + (int)((t0 + 45968u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[28] = c->hash ^ sx_rl(c->lane[10], 22);
    c->hash ^= c->lane[10] + 0xef149bf1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x84cae9a3u) ^ sx_rr(c->hash, 17);
    c->lane[5] ^= sx_rl(c->lane[3], 4);
    c->lane[3] ^= sx_rl(c->lane[11], 22);
    t2 = (t2 ^ c->sum) * 0x29c773fdu;
    sort_label(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xe4a3022fu;
    t2 += (uint32_t)parse_block_689(c);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void pair_field(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[6], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sched[0] = c->hash ^ sx_rl(c->lane[14], 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39987u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x37) << 0;
    c->hash ^= c->lane[6] + 0x03002e4eu;
    c->raw[c->slo + (int)((t0 + 22361u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[14] += c->lane[11]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 16);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t tally_layer_670(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    tap_unit_695(c, &c->lane[7], 2);
    c->sched[7] = c->hash ^ sx_rl(c->lane[3], 31);
    c->raw[c->slo + (int)((t0 + 35453u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 57091u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[9] += c->lane[6] ^ 0x08b6985bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t hold_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x7f1166c3u) ^ sx_rr(c->hash, 13);
    c->hash = (c->hash * 0x0fdda52du) ^ sx_rr(c->hash, 13);
    t2 += patch_value(c, c->rlo, c->rln);
    c->hash ^= c->lane[10] + 0x0bb93835u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xc62328f5u) ^ sx_rr(c->hash, 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 15);
    c->lane[3] += c->lane[15]; c->lane[12] ^= c->lane[3]; c->lane[12] = sx_rl(c->lane[12], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52473u) % (uint32_t)c->rln)] << 16;
    c->lane[5] ^= sx_rl(c->lane[14], 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int hold_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t1 ^= (uint32_t)place_stack(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33166u) % (uint32_t)c->rln)] << 16;
    t2 += (uint32_t)shift_page_685(c);
    t2 += (uint32_t)store_layer(c);
    t2 += (uint32_t)cache_queue(c);
    c->hash = (c->hash * 0x4ca92017u) ^ sx_rr(c->hash, 11);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int settle_ring(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x554793a1u;
    c->sched[29] = c->hash ^ sx_rl(c->lane[1], 13);
    c->hash = (c->hash * 0x71250d25u) ^ sx_rr(c->hash, 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x573d059fu;
    t2 = (t2 ^ c->sum) * 0xfac9eb9fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x216a4f3bu;
    c->sched[13] = c->hash ^ sx_rl(c->lane[11], 18);
    t2 += (uint32_t)purge_index(c);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t chain_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 36852u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 21);
    sort_label(c, t0, t1);
    c->hash = (c->hash * 0xfb2e2167u) ^ sx_rr(c->hash, 8);
    c->lane[12] ^= sx_rl(c->lane[2], 12);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void prime_bound_675(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xecd2c8e5u;
    c->lane[6] += c->lane[0] ^ 0x9c4ff56cu;
    c->sched[18] = c->hash ^ sx_rl(c->lane[14], 24);
    pair_label(c, &c->lane[8], 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4f073481u;
    swap_key(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[5] + 0x7c861703u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t chain_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 25371u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += sort_value(c, c->rlo, c->rln);
    c->lane[6] += c->lane[15] ^ 0x964024c9u;
    t2 = (t2 ^ c->sum) * 0xd09620e7u;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 19);
    t0 ^= chain_region(c, t1);
    drain_label(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x92) << 8;
    c->lane[13] ^= sx_rl(c->lane[13], 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fill_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46224u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0xec850e99u;
    c->lane[12] += c->lane[11]; c->lane[0] ^= c->lane[12]; c->lane[0] = sx_rl(c->lane[0], 17);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 31);
    c->lane[13] ^= sx_rl(c->lane[3], 15);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t grow_tail(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x366d2c51u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33599u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)poll_region(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[7], 6);
    c->lane[10] += c->lane[15] ^ 0x89722c2bu;
    t0 ^= move_store_749(c, t1);
    t0 ^= reap_view(c, t1);
    t0 ^= drain_state(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int seek_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[15] += c->lane[6]; c->lane[8] ^= c->lane[15]; c->lane[8] = sx_rl(c->lane[8], 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)close_item(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)grow_bucket_709(c, (uint8_t)(t0 >> 0), t2);
    c->lane[15] += c->lane[1] ^ 0xce173df1u;
    t0 ^= move_store_749(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x4f) << 0;
    c->hash ^= c->lane[7] + 0x521b9f1eu;
    c->lane[10] ^= sx_rl(c->lane[8], 7);
    c->lane[15] ^= sx_rl(c->lane[13], 24);
    t1 ^= (uint32_t)resize_lease(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0xfff6a06fu) ^ sx_rr(c->hash, 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xaa) << 8;
    t2 = (t2 ^ c->sum) * 0x85ceaf63u;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int cache_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t0 ^= tally_region_734(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40938u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 30515u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 26);
    t2 = (t2 ^ c->sum) * 0xad46121fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28458u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[4] + 0xa79b9530u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8b9e14f7u;
    t2 += (uint32_t)fold_line(c);
    c->sched[19] = c->hash ^ sx_rl(c->lane[7], 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5aa76c07u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mix_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x24863b93u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27345u) % (uint32_t)c->rln)] << 24;
    c->sched[23] = c->hash ^ sx_rl(c->lane[8], 18);
    c->lane[12] += c->lane[7]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 21);
    c->lane[4] += c->lane[4] ^ 0x9415ed97u;
    c->sched[20] = c->hash ^ sx_rl(c->lane[12], 18);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 22);
    t0 ^= step_gap_738(c, t1);
    c->sched[10] = c->hash ^ sx_rl(c->lane[3], 20);
    c->sched[23] = c->hash ^ sx_rl(c->lane[10], 2);
    c->hash = (c->hash * 0xc63477b1u) ^ sx_rr(c->hash, 4);
    c->hash = (c->hash * 0xa3061c2bu) ^ sx_rr(c->hash, 30);
    c->lane[5] += c->lane[12]; c->lane[14] ^= c->lane[5]; c->lane[14] = sx_rl(c->lane[14], 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[0] += c->lane[13] ^ 0x6dbbd4a8u;
    c->hash = (c->hash * 0xb59c83c9u) ^ sx_rr(c->hash, 21);
    t2 = (t2 ^ c->sum) * 0xa17cab13u;
    trace_cursor(c, &c->lane[5], 1);
    c->lane[12] ^= sx_rl(c->lane[6], 17);
    t2 += load_stack(c, c->slo, c->sln);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t wrap_path_683(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[3] ^= sx_rl(c->lane[12], 26);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 1);
    c->hash = (c->hash * 0x8c720479u) ^ sx_rr(c->hash, 29);
    t2 = (t2 ^ c->sum) * 0xce14a471u;
    t1 ^= (uint32_t)emit_path(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] ^= sx_rl(c->lane[13], 6);
    t2 = (t2 ^ c->sum) * 0x11acd9ddu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x71f1746fu;
    c->lane[5] += c->lane[4] ^ 0xb6c9d63cu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa4) << 8;
    t2 = (t2 ^ c->sum) * 0x8bc95c03u;
    t1 ^= (uint32_t)close_item(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x64c2fb9fu) ^ sx_rr(c->hash, 7);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t defer_index_684(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[30] = c->hash ^ sx_rl(c->lane[3], 14);
    grow_row(c, &c->lane[1], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x59) << 0;
    c->sched[6] = c->hash ^ sx_rl(c->lane[8], 8);
    c->lane[8] += c->lane[11]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 5);
    c->lane[5] += c->lane[9] ^ 0xb7e2477fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[14] += c->lane[5]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 23);
    c->hash ^= c->lane[15] + 0x58b0891au;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int shift_page_685(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x95) << 0;
    resize_store(c, &c->lane[1], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31927u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x07) << 16;
    t1 ^= (uint32_t)close_digest(c, (uint8_t)(t0 >> 0), t2);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void tap_unit_686(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    prime_label(c, t0, t1);
    c->sched[2] = c->hash ^ sx_rl(c->lane[7], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xddb88ff9u;
    c->hash = (c->hash * 0x3e3008afu) ^ sx_rr(c->hash, 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 42547u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)scan_part(c);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static int store_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t2 += (uint32_t)emit_bound(c);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 28);
    c->hash = (c->hash * 0xae6bb51bu) ^ sx_rr(c->hash, 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x26616343u;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 24);
    t2 = (t2 ^ c->sum) * 0x1bfe196du;
    t2 += mark_lease(c, c->rlo, c->rln);
    resize_queue_758(c, &c->lane[9], 3);
    c->lane[2] += c->lane[13] ^ 0xda13ba5fu;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void push_stack(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd0) << 16;
    t2 = (t2 ^ c->sum) * 0x185cc2e3u;
    c->raw[c->slo + (int)((t0 + 59924u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xab4c807du) ^ sx_rr(c->hash, 16);
    c->sched[21] = c->hash ^ sx_rl(c->lane[4], 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t2 += slice_list(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x1170af27u;
    c->lane[7] += c->lane[14]; c->lane[10] ^= c->lane[7]; c->lane[10] = sx_rl(c->lane[10], 10);
    c->sched[18] = c->hash ^ sx_rl(c->lane[9], 28);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[13] += c->lane[4]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 31);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int parse_block_689(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    wrap_view_746(c, t0, t1);
    c->hash = (c->hash * 0x50aea447u) ^ sx_rr(c->hash, 17);
    c->hash = (c->hash * 0xe0a764a9u) ^ sx_rr(c->hash, 7);
    t0 ^= defer_port(c, t1);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 22);
    tap_run(c, t0, t1);
    c->lane[11] += c->lane[6]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 1);
    t1 ^= (uint32_t)fill_page(c, (uint8_t)(t0 >> 16), t2);
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 29);
    t2 = (t2 ^ c->sum) * 0xa5893f57u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x73) << 0;
    t2 = (t2 ^ c->sum) * 0xc8aa2a4bu;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t tap_port_690(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[4] + 0x08562b5fu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb6) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35808u) % (uint32_t)c->rln)] << 16;
    c->lane[0] ^= sx_rl(c->lane[8], 15);
    c->lane[1] += c->lane[6] ^ 0xc2da1b9bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1157u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t grow_batch_691(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45832u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcb) << 8;
    c->lane[4] += c->lane[2]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 10);
    c->lane[7] ^= sx_rl(c->lane[1], 13);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t store_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[7] ^ 0xeff58adau;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7f68ed0fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 ^= join_node(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t1 ^= (uint32_t)mark_item(c, (uint8_t)(t0 >> 8), t2);
    pair_label(c, &c->lane[6], 4);
    c->lane[2] += c->lane[11]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 25);
    c->lane[1] += c->lane[9] ^ 0xb668e7b6u;
    c->hash ^= c->lane[13] + 0xc85bc5a0u;
    c->hash = (c->hash * 0x1672f191u) ^ sx_rr(c->hash, 5);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void emit_unit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] += c->lane[5] ^ 0xb097b926u;
    fetch_mask(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x55fd3867u;
    tune_block(c, t0, t1);
    c->hash ^= c->lane[15] + 0x267bc654u;
    c->raw[c->slo + (int)((t0 + 48752u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[5] + 0x95b775a8u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[10], 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33526u) % (uint32_t)c->rln)] << 24;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t scan_item_694(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += blend_rate(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x2dd846c3u;
    t0 ^= align_seat(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24423u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] ^= sx_rl(c->lane[14], 29);
    tune_token(c, &c->lane[9], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] += c->lane[5] ^ 0xb30d9a7bu;
    c->lane[1] ^= sx_rl(c->lane[13], 26);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 17);
    c->lane[3] ^= sx_rl(c->lane[15], 28);
    c->lane[12] += c->lane[9] ^ 0x3321d657u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tap_unit_695(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    swap_label(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xbb) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[5] += c->lane[2] ^ 0x234f8033u;
    t0 ^= load_range(c, t1);
    c->lane[3] += c->lane[10]; c->lane[8] ^= c->lane[3]; c->lane[8] = sx_rl(c->lane[8], 22);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    c->sched[7] = c->hash ^ sx_rl(c->lane[12], 3);
    resize_limit(c, &c->lane[0], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdd388b4fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t pin_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] += c->lane[13] ^ 0x558bec04u;
    reap_delta(c, &c->lane[5], 1);
    c->lane[10] ^= sx_rl(c->lane[7], 26);
    c->lane[2] ^= sx_rl(c->lane[6], 4);
    t1 ^= (uint32_t)fill_list(c, (uint8_t)(t0 >> 16), t2);
    c->sched[9] = c->hash ^ sx_rl(c->lane[8], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64343u) % (uint32_t)c->rln)] << 0;
    c->sched[3] = c->hash ^ sx_rl(c->lane[15], 12);
    c->hash ^= c->lane[13] + 0xcbb74111u;
    c->hash = (c->hash * 0x76d07479u) ^ sx_rr(c->hash, 6);
    t0 ^= move_limit(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t prime_count(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45929u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9b) << 16;
    t2 = (t2 ^ c->sum) * 0xe27d5667u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x76) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x3a) << 0;
    c->hash = (c->hash * 0x2e46f4a9u) ^ sx_rr(c->hash, 16);
    t2 = (t2 ^ c->sum) * 0x66fea0f5u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t yield_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x8d) << 0;
    c->sched[16] = c->hash ^ sx_rl(c->lane[6], 24);
    t0 ^= shift_entry(c, t1);
    t2 = (t2 ^ c->sum) * 0x42425b05u;
    t1 ^= (uint32_t)align_batch(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)store_label(c);
    t2 = (t2 ^ c->sum) * 0x2452d589u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void tap_limit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb9) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe2) << 16;
    mark_range(c, &c->lane[5], 2);
    c->lane[2] += c->lane[13]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 24);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 30);
    t2 = (t2 ^ c->sum) * 0x24056eddu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52951u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += sort_value(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 29);
    t2 += (uint32_t)pack_field(c);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static int tally_level(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    t2 = (t2 ^ c->sum) * 0x41269733u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[8], 10);
    t0 ^= chain_tuple(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xff) << 0;
    c->raw[c->slo + (int)((t0 + 40289u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x6a4abf57u) ^ sx_rr(c->hash, 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10806u) % (uint32_t)c->rln)] << 16;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int pin_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->raw[c->slo + (int)((t0 + 24736u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7409u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[15] + 0x62cf5feau;
    c->lane[6] ^= sx_rl(c->lane[9], 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 16954u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 27010u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] ^= sx_rl(c->lane[9], 18);
    c->lane[4] += c->lane[15] ^ 0x22bc1677u;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t latch_range(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t1 ^= (uint32_t)cache_lease(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[21] = c->hash ^ sx_rl(c->lane[4], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x739a4dcfu;
    t2 += (uint32_t)patch_chunk(c);
    c->sched[5] = c->hash ^ sx_rl(c->lane[2], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf7) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26035u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t mix_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[9] = c->hash ^ sx_rl(c->lane[13], 17);
    c->hash = (c->hash * 0x7c917109u) ^ sx_rr(c->hash, 24);
    c->lane[15] ^= sx_rl(c->lane[6], 9);
    mark_block(c, t0, t1);
    c->hash ^= c->lane[6] + 0x9215e8a4u;
    t2 = (t2 ^ c->sum) * 0xcc239817u;
    c->hash = (c->hash * 0x73d6ed8fu) ^ sx_rr(c->hash, 26);
    c->lane[10] += c->lane[14] ^ 0x0c82b654u;
    t1 ^= (uint32_t)merge_state(c, (uint8_t)(t0 >> 0), t2);
    relay_line(c, &c->lane[2], 3);
    c->raw[c->slo + (int)((t0 + 46605u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40791u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13669u) % (uint32_t)c->rln)] << 16;
    swap_key(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4f8fd855u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t clamp_group_704(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    stage_mask(c, t0, t1);
    latch_delta(c, &c->lane[4], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24589u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[5] ^= sx_rl(c->lane[6], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18161u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xec7ece7bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7e854db9u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void store_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28084u) % (uint32_t)c->rln)] << 8;
    c->sched[23] = c->hash ^ sx_rl(c->lane[15], 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x07fa7391u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9a) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int join_pairing_706(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->hash ^= c->lane[5] + 0xcc1637b5u;
    c->hash = (c->hash * 0xa5f8d57bu) ^ sx_rr(c->hash, 19);
    c->lane[0] += c->lane[4] ^ 0x0a2bac46u;
    c->hash = (c->hash * 0x0ba4f1a5u) ^ sx_rr(c->hash, 25);
    t2 = (t2 ^ c->sum) * 0x15610a7fu;
    c->sched[18] = c->hash ^ sx_rl(c->lane[7], 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x63c84eb5u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int purge_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xaa8b582du;
    c->hash ^= c->lane[11] + 0xbfd688a2u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x70) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47158u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[7] + 0xc2dc5953u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x715162f5u;
    t1 ^= (uint32_t)align_record(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 65453u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7c) << 16;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t place_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x43141055u;
    c->lane[4] += c->lane[5] ^ 0x8e9339dcu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x02001f91u;
    c->lane[10] += c->lane[10] ^ 0x6b5fb030u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa9) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= c->lane[8] + 0xaa99441cu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t grow_bucket_709(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x4c3e2cafu) ^ sx_rr(c->hash, 9);
    c->hash = (c->hash * 0xabac0531u) ^ sx_rr(c->hash, 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17580u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xfa) << 8;
    c->lane[2] += c->lane[2] ^ 0x69bc2abcu;
    c->raw[c->slo + (int)((t0 + 22352u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[15] += c->lane[12]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 19);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 22);
    c->lane[1] += c->lane[15]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 18);
    t2 = (t2 ^ c->sum) * 0x5cd4f671u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t emit_path(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[0] ^= sx_rl(c->lane[1], 30);
    c->hash = (c->hash * 0xeeccc97bu) ^ sx_rr(c->hash, 1);
    c->raw[c->slo + (int)((t0 + 57832u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc1b0570fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x2c179641u) ^ sx_rr(c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5578u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 42482u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[6] = c->hash ^ sx_rl(c->lane[4], 10);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void fetch_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32293u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3c) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[6] ^= sx_rl(c->lane[7], 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53512u) % (uint32_t)c->rln)] << 8;
    c->lane[0] ^= sx_rl(c->lane[5], 29);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint8_t merge_state(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] ^= sx_rl(c->lane[0], 2);
    c->raw[c->slo + (int)((t0 + 20525u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[1] += c->lane[3]; c->lane[12] ^= c->lane[1]; c->lane[12] = sx_rl(c->lane[12], 31);
    c->hash = (c->hash * 0xcabdc77fu) ^ sx_rr(c->hash, 9);
    c->raw[c->slo + (int)((t0 + 31471u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48053u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x254d4df7u) ^ sx_rr(c->hash, 21);
    c->hash = (c->hash * 0xd4270a41u) ^ sx_rr(c->hash, 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t mark_lease(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] ^= sx_rl(c->lane[6], 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58414u) % (uint32_t)c->rln)] << 8;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xee5daa7fu;
    c->hash = (c->hash * 0x45d6ef61u) ^ sx_rr(c->hash, 28);
    c->sum += t1;
    return t0 + t2;
}

static void tap_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[15] + 0xe5eae262u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 1);
    c->lane[9] += c->lane[6]; c->lane[13] ^= c->lane[9]; c->lane[13] = sx_rl(c->lane[13], 25);
    c->sched[25] = c->hash ^ sx_rl(c->lane[6], 1);
    c->lane[5] ^= sx_rl(c->lane[1], 4);
    c->lane[12] ^= sx_rl(c->lane[0], 6);
    c->hash = (c->hash * 0x155a8d0du) ^ sx_rr(c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void grow_row(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 16);
    c->lane[8] += c->lane[8] ^ 0x2071cba2u;
    c->lane[1] += c->lane[13] ^ 0x3ca13371u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[10], 28);
    c->lane[0] ^= sx_rl(c->lane[5], 23);
    t2 = (t2 ^ c->sum) * 0x842318fdu;
    c->lane[5] ^= sx_rl(c->lane[8], 15);
    c->lane[0] += c->lane[4]; c->lane[12] ^= c->lane[0]; c->lane[12] = sx_rl(c->lane[12], 31);
    t2 = (t2 ^ c->sum) * 0xa1a477dbu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void relay_line(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[4] + 0xa84347fbu;
    t2 = (t2 ^ c->sum) * 0xeecff6d9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xaccaa737u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5109u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[10] += c->lane[9]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 25);
    c->lane[10] += c->lane[10] ^ 0x9f202b2au;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int emit_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x1670d54bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21973u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x23) << 0;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mark_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7722u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49808u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xea) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42818u) % (uint32_t)c->rln)] << 0;
    c->sched[20] = c->hash ^ sx_rl(c->lane[5], 7);
    c->hash ^= c->lane[11] + 0xacfd923du;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static int store_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash = (c->hash * 0xbf545641u) ^ sx_rr(c->hash, 11);
    c->hash ^= c->lane[14] + 0x3077dd05u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x36) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x496207afu;
    c->hash = (c->hash * 0x0fc2f531u) ^ sx_rr(c->hash, 25);
    c->sched[14] = c->hash ^ sx_rl(c->lane[1], 4);
    c->raw[c->slo + (int)((t0 + 50333u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void drain_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x7a7b9d6du;
    c->sched[22] = c->hash ^ sx_rl(c->lane[15], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0acbfadfu;
    c->lane[4] ^= sx_rl(c->lane[11], 16);
    c->hash ^= c->lane[9] + 0x7a46c05cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= c->lane[1] + 0x58f0f68du;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static int scan_part(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->raw[c->slo + (int)((t0 + 42139u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[9] = c->hash ^ sx_rl(c->lane[0], 26);
    c->hash = (c->hash * 0xfcaf2061u) ^ sx_rr(c->hash, 27);
    c->hash = (c->hash * 0x9ef582cdu) ^ sx_rr(c->hash, 7);
    c->lane[4] += c->lane[3]; c->lane[7] ^= c->lane[4]; c->lane[7] = sx_rl(c->lane[7], 17);
    c->sched[11] = c->hash ^ sx_rl(c->lane[7], 20);
    c->raw[c->slo + (int)((t0 + 10190u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] ^= sx_rl(c->lane[13], 6);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mark_range(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[6] += c->lane[15] ^ 0xa9833d94u;
    t2 = (t2 ^ c->sum) * 0x2383667fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[5] += c->lane[1]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 21);
    c->hash = (c->hash * 0x3799efefu) ^ sx_rr(c->hash, 20);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t align_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44909u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x6fecb3cbu;
    c->raw[c->slo + (int)((t0 + 64930u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x98b83a8fu) ^ sx_rr(c->hash, 26);
    c->lane[11] ^= sx_rl(c->lane[13], 6);
    c->lane[0] += c->lane[11]; c->lane[9] ^= c->lane[0]; c->lane[9] = sx_rl(c->lane[9], 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf0) << 8;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash = (c->hash * 0x063d70d1u) ^ sx_rr(c->hash, 3);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t sort_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] ^= sx_rl(c->lane[12], 10);
    c->lane[1] ^= sx_rl(c->lane[10], 2);
    c->hash ^= c->lane[11] + 0x37c602a8u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 26);
    c->hash = (c->hash * 0xcdbf81c7u) ^ sx_rr(c->hash, 15);
    c->raw[c->slo + (int)((t0 + 29981u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t close_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 59928u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 21325u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] += c->lane[2] ^ 0x892cbfb3u;
    c->hash = (c->hash * 0x8f2a4ebbu) ^ sx_rr(c->hash, 7);
    c->raw[c->slo + (int)((t0 + 2911u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xca) << 8;
    c->raw[c->slo + (int)((t0 + 63937u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x71ea86bbu) ^ sx_rr(c->hash, 18);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t load_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[30] = c->hash ^ sx_rl(c->lane[12], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7615u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[11] + 0x03905eafu;
    c->lane[7] += c->lane[12]; c->lane[6] ^= c->lane[7]; c->lane[6] = sx_rl(c->lane[6], 29);
    c->lane[4] += c->lane[8] ^ 0x484a1ba6u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18754u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0xf2bf4483u) ^ sx_rr(c->hash, 19);
    c->sched[30] = c->hash ^ sx_rl(c->lane[0], 27);
    c->lane[0] += c->lane[9] ^ 0x338442fcu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x2b) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum += t1;
    return t0 + t2;
}

static uint8_t fill_list(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x81925c35u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44990u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[30] = c->hash ^ sx_rl(c->lane[4], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15953u) % (uint32_t)c->rln)] << 0;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 24);
    c->raw[c->slo + (int)((t0 + 12101u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 39868u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void tune_token(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int poll_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[9] += c->lane[5] ^ 0x11e9b9aau;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x23) << 8;
    t2 = (t2 ^ c->sum) * 0x3c4aadd7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x1a) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x336ee795u;
    t2 = (t2 ^ c->sum) * 0xf5fc591fu;
    c->hash ^= c->lane[0] + 0xf690423au;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6680u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[14] + 0x36a3f7dcu;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t chain_region(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[4] += c->lane[4] ^ 0x69faf7d5u;
    c->lane[12] += c->lane[4] ^ 0x24520d42u;
    c->raw[c->slo + (int)((t0 + 29841u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x105b64a1u;
    c->hash ^= c->lane[7] + 0x7e1a61b2u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    c->lane[2] += c->lane[5] ^ 0xdeb62e2bu;
    c->lane[3] ^= sx_rl(c->lane[0], 10);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t drain_state(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash = (c->hash * 0xc89a88b5u) ^ sx_rr(c->hash, 22);
    c->lane[1] ^= sx_rl(c->lane[3], 31);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int pack_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 27);
    c->lane[6] += c->lane[11]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x6bfbd432u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5f) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6d2ea0cdu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xecc2e921u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x83) << 8;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 7);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t chain_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[27] = c->hash ^ sx_rl(c->lane[7], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[4] += c->lane[12] ^ 0xd0a7b933u;
    c->hash ^= c->lane[3] + 0xbb2963d1u;
    c->lane[4] += c->lane[6] ^ 0xf5ea44ceu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x73) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5af10c27u;
    c->lane[9] += c->lane[2]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 26);
    c->hash ^= c->lane[7] + 0x9cc88f0du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xaa) << 0;
    t2 = (t2 ^ c->sum) * 0x5454e15du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tally_region_734(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43006u) % (uint32_t)c->rln)] << 8;
    c->lane[3] ^= sx_rl(c->lane[1], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x1eff36e7u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[5], 31);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 25);
    t2 = (t2 ^ c->sum) * 0x95d12af5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3a21a52bu;
    c->raw[c->slo + (int)((t0 + 15804u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t cache_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] += c->lane[3] ^ 0x4bd5c9dfu;
    c->lane[3] += c->lane[11] ^ 0xd9c120e4u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0c) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x99) << 8;
    c->lane[12] += c->lane[7] ^ 0x00853ed5u;
    c->hash ^= c->lane[15] + 0xc670903eu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void sort_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[13] += c->lane[10] ^ 0x1def950cu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t mark_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb1b5a7cdu;
    t2 = (t2 ^ c->sum) * 0x088a5d09u;
    c->lane[2] += c->lane[3]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] += c->lane[15] ^ 0xda2f93cbu;
    c->lane[13] ^= sx_rl(c->lane[13], 10);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t step_gap_738(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xb4438ec7u;
    c->raw[c->slo + (int)((t0 + 59036u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61067u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x96c3ec33u) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdebfdf77u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash = (c->hash * 0xdab5ebcbu) ^ sx_rr(c->hash, 4);
    c->raw[c->slo + (int)((t0 + 18153u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t reap_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x65ced76fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[15] += c->lane[7] ^ 0x8632bc82u;
    c->hash = (c->hash * 0x3108c2a3u) ^ sx_rr(c->hash, 2);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void stage_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf9) << 0;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 22);
    c->hash ^= c->lane[9] + 0x0fb375b2u;
    c->lane[3] += c->lane[12]; c->lane[10] ^= c->lane[3]; c->lane[10] = sx_rl(c->lane[10], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x12154687u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xda) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 5);
    c->sched[14] = c->hash ^ sx_rl(c->lane[8], 16);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint32_t blend_rate(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 14196u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[2] += c->lane[1]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5589u) % (uint32_t)c->rln)] << 8;
    c->lane[5] ^= sx_rl(c->lane[4], 28);
    c->sched[15] = c->hash ^ sx_rl(c->lane[3], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0649445fu;
    c->lane[15] += c->lane[11]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 31);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t defer_port(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x077e5c57u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[6], 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x81) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[6] += c->lane[10]; c->lane[3] ^= c->lane[6]; c->lane[3] = sx_rl(c->lane[3], 12);
    c->lane[12] ^= sx_rl(c->lane[7], 10);
    c->lane[2] += c->lane[4] ^ 0x76406d10u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7ca16653u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31360u) % (uint32_t)c->rln)] << 24;
    c->lane[13] += c->lane[15]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 13);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int patch_chunk(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->lane[6] += c->lane[14]; c->lane[0] ^= c->lane[6]; c->lane[0] = sx_rl(c->lane[0], 8);
    c->hash = (c->hash * 0x13b3a5bfu) ^ sx_rr(c->hash, 28);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x7f) << 16;
    c->sched[12] = c->hash ^ sx_rl(c->lane[4], 14);
    t2 = (t2 ^ c->sum) * 0xbb76ffafu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    t2 = (t2 ^ c->sum) * 0x00dc5febu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 4);
    c->raw[c->slo + (int)((t0 + 28706u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t move_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 46433u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xbc) << 8;
    c->hash = (c->hash * 0xf72a905fu) ^ sx_rr(c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[3] += c->lane[5] ^ 0xa35a5a72u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9146u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void swap_key(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[9] = c->hash ^ sx_rl(c->lane[8], 30);
    c->hash ^= c->lane[10] + 0xc484e6a5u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 3);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void wrap_view_746(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x344a7b45u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] ^= sx_rl(c->lane[8], 15);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t slice_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x0102b4b5u) ^ sx_rr(c->hash, 21);
    c->lane[0] ^= sx_rl(c->lane[13], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2837f4a5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x752210edu) ^ sx_rr(c->hash, 6);
    c->sum += t1;
    return t0 + t2;
}

static void pair_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34918u) % (uint32_t)c->rln)] << 24;
    c->sched[7] = c->hash ^ sx_rl(c->lane[10], 2);
    c->lane[15] += c->lane[11] ^ 0x6279c89du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45615u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xea) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t move_store_749(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xfac2cac3u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 10);
    t2 = (t2 ^ c->sum) * 0x79b406e9u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[14], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void prime_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] ^= sx_rl(c->lane[7], 16);
    t2 = (t2 ^ c->sum) * 0x46b1f41fu;
    c->sched[14] = c->hash ^ sx_rl(c->lane[5], 26);
    c->hash = (c->hash * 0xcf818077u) ^ sx_rr(c->hash, 2);
    c->sched[22] = c->hash ^ sx_rl(c->lane[3], 8);
    c->lane[12] += c->lane[8]; c->lane[13] ^= c->lane[12]; c->lane[13] = sx_rl(c->lane[13], 19);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[3] += c->lane[1]; c->lane[4] ^= c->lane[3]; c->lane[4] = sx_rl(c->lane[4], 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15979u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void latch_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[7] + 0xc4a24571u;
    c->hash ^= c->lane[10] + 0x54aa9b5cu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51659u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[6] += c->lane[15] ^ 0xb1508616u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void tune_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8b5d6e49u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 2);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xed) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3e) << 0;
    c->sched[31] = c->hash ^ sx_rl(c->lane[4], 20);
    t2 = (t2 ^ c->sum) * 0xf7592237u;
    c->lane[12] ^= sx_rl(c->lane[7], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3859u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x05553807u;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t align_seat(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[6] += c->lane[4]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x078d020bu) ^ sx_rr(c->hash, 13);
    c->lane[15] += c->lane[8]; c->lane[7] ^= c->lane[15]; c->lane[7] = sx_rl(c->lane[7], 2);
    c->hash = (c->hash * 0xf08ae72bu) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4c) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 17);
    c->sched[22] = c->hash ^ sx_rl(c->lane[3], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xbf) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t close_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 30735u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[5] ^= sx_rl(c->lane[4], 11);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 3);
    c->hash = (c->hash * 0x3217976du) ^ sx_rr(c->hash, 22);
    c->sched[28] = c->hash ^ sx_rl(c->lane[0], 2);
    c->sched[16] = c->hash ^ sx_rl(c->lane[6], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void resize_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[8] += c->lane[9] ^ 0x0ba16dacu;
    c->raw[c->slo + (int)((t0 + 39275u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= c->lane[0] + 0x49ec0fceu;
    c->sched[8] = c->hash ^ sx_rl(c->lane[7], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xfa) << 16;
    c->lane[9] ^= sx_rl(c->lane[8], 3);
    t2 = (t2 ^ c->sum) * 0x2d0eae21u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15729u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t resize_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12282u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[0] + 0x813b5824u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x483e95a7u;
    c->hash ^= c->lane[3] + 0x93109687u;
    c->lane[9] ^= sx_rl(c->lane[6], 13);
    c->raw[c->slo + (int)((t0 + 23737u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x7a68e3d3u) ^ sx_rr(c->hash, 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t shift_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x2e6585e3u) ^ sx_rr(c->hash, 6);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] ^= sx_rl(c->lane[9], 17);
    c->raw[c->slo + (int)((t0 + 11006u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 11);
    t2 = (t2 ^ c->sum) * 0x4bfdebd7u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void resize_queue_758(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x1fc1e37bu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t load_range(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3251u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 8409u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[13] += c->lane[1]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 24);
    c->hash ^= c->lane[5] + 0x98061286u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void trace_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 17858u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe379f2e1u;
    c->hash = (c->hash * 0x1230bc9du) ^ sx_rr(c->hash, 24);
    c->sched[0] = c->hash ^ sx_rl(c->lane[7], 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int fold_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash = (c->hash * 0x3345b1a5u) ^ sx_rr(c->hash, 3);
    c->raw[c->slo + (int)((t0 + 519u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 16988u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x793d51cfu;
    c->lane[15] ^= sx_rl(c->lane[14], 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[5] += c->lane[14] ^ 0xab7bb0a7u;
    t2 = (t2 ^ c->sum) * 0xa3aa1389u;
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t join_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[10] = c->hash ^ sx_rl(c->lane[0], 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x854de79fu;
    t2 = (t2 ^ c->sum) * 0xca9bb49bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void resize_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6120e0afu;
    c->lane[5] += c->lane[10]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26314u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0xd572a779u;
    c->raw[c->slo + (int)((t0 + 32912u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[3] ^= sx_rl(c->lane[0], 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x25c47a47u) ^ sx_rr(c->hash, 10);
    c->lane[14] ^= sx_rl(c->lane[5], 11);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void swap_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[4] ^= sx_rl(c->lane[2], 29);
    c->raw[c->slo + (int)((t0 + 20415u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[3] += c->lane[15] ^ 0xbf5f9f57u;
    c->hash = (c->hash * 0xc9085183u) ^ sx_rr(c->hash, 31);
    c->sched[30] = c->hash ^ sx_rl(c->lane[6], 10);
    c->hash ^= c->lane[3] + 0x6d5b4789u;
    c->sched[23] = c->hash ^ sx_rl(c->lane[8], 19);
    c->hash ^= c->lane[8] + 0x695a4194u;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t align_record(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[10] = c->hash ^ sx_rl(c->lane[8], 19);
    c->lane[7] += c->lane[11] ^ 0x30e04c5cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 4);
    c->raw[c->slo + (int)((t0 + 14146u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[3] += c->lane[6]; c->lane[1] ^= c->lane[3]; c->lane[1] = sx_rl(c->lane[1], 28);
    c->lane[14] += c->lane[12] ^ 0xd445cd76u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[2] += c->lane[15]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 10);
    c->lane[9] += c->lane[0] ^ 0x37fb0705u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void reap_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7bad0e6du;
    c->raw[c->slo + (int)((t0 + 10536u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 41614u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[14] += c->lane[2] ^ 0x1a15d3e0u;
    c->lane[13] += c->lane[15]; c->lane[8] ^= c->lane[13]; c->lane[8] = sx_rl(c->lane[8], 17);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x04a15d17u;
    c->lane[7] += c->lane[12]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 18);
    c->lane[7] += c->lane[13]; c->lane[11] ^= c->lane[7]; c->lane[11] = sx_rl(c->lane[11], 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t fill_page(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[2], 25);
    c->hash = (c->hash * 0xac59bb8fu) ^ sx_rr(c->hash, 29);
    c->raw[c->slo + (int)((t0 + 45506u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd0c30b8bu;
    t2 = (t2 ^ c->sum) * 0xe7d9bac3u;
    c->hash = (c->hash * 0x03f20077u) ^ sx_rr(c->hash, 18);
    c->hash = (c->hash * 0xb1dde7c1u) ^ sx_rr(c->hash, 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}


static const uint8_t sx_polys[16] = { 0x1d, 0x2b, 0x2d, 0x39, 0x3f, 0x4d, 0x5f, 0x63, 0x65, 0x69, 0x71, 0x77, 0x7b, 0x87, 0x8b, 0x8d };

static uint32_t prime_level(uint32_t x) {
    return (x * 0x7fb5d329u) ^ sx_rl(x + 0x4e1d9a2bu, 11);
}

static int patch_row(sx_state *c) {
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
        relay_head(c, &c->lane[10], 4);
        c->raw[c->slo + (int)((t0 + 515u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
        c->sched[9] = c->hash ^ sx_rl(c->lane[4], 17);
        c->hash ^= c->lane[6] + 0x7f5e2d05u;
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
        c->hash = (c->hash * 0x5eab59d9u) ^ sx_rr(c->hash, 8);
        push_frame(c, &c->lane[5], 1);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    return c->nlen;
}

static uint32_t close_count(uint32_t prime, uint32_t h, uint8_t x) {
    return (h ^ (uint32_t)x) * prime;
}

static uint32_t yield_state_771(sx_state *c, uint32_t nonce, int size) {
    uint32_t m = prime_level((uint32_t)c->nlen);
    uint32_t basis = 0x16de7a0cu ^ m;
    uint32_t prime = 0x96c2e65au ^ m;
    uint32_t h = basis;
    int i;
    for (i = 0; i < c->nlen; i++) h = close_count(prime, h, c->name[i]);
    for (i = 0; i < 4; i++) h = close_count(prime, h, (uint8_t)(nonce >> (i * 8)));
    h = close_count(prime, h, (uint8_t)size);
#ifdef SX_TRACE
    fprintf(stderr, "T fnv_pre h=%08x m=%08x nlen=%d hash=%08x sum=%08x\n", h, m, c->nlen, c->hash, c->sum);
#endif
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        stage_range(c, &c->lane[11], 4);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x23) << 8;
        t2 += purge_stack(c, c->slo, c->sln);
        c->lane[5] += c->lane[13]; c->lane[3] ^= c->lane[5]; c->lane[3] = sx_rl(c->lane[3], 16);
        c->hash ^= c->lane[12] + 0x5124766au;
        t1 ^= (uint32_t)shift_chunk_162(c, (uint8_t)(t0 >> 0), t2);
        c->raw[c->slo + (int)((t0 + 13848u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x39fe7e21u;
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        c->hash ^= t0 ^ t1 ^ t2;
    }
#ifdef SX_TRACE
    fprintf(stderr, "T fnv_post h=%08x hash=%08x sum=%08x lane0=%08x\n", h, c->hash, c->sum, c->lane[0]);
#endif
    return h ^ c->hash;
}

static void settle_record(sx_state *c, uint32_t seed) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x25) << 16;
        t2 += (uint32_t)swap_bucket(c);
        c->lane[10] += c->lane[11] ^ 0xc3a3eb6au;
        t2 = (t2 ^ c->sum) * 0x3bedb45du;
        c->raw[c->slo + (int)((t0 + 27785u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
        t2 += (uint32_t)clamp_field(c);
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

static void fill_batch(sx_state *c, uint32_t master) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t1 ^= (uint32_t)hold_state(c, (uint8_t)(t0 >> 16), t2);
        store_seat(c, &c->lane[6], 1);
        c->raw[c->slo + (int)((t0 + 26241u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
        drain_tuple(c, t0, t1);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        c->lane[6] += c->lane[10] ^ 0x43eddb6bu;
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
        c->hash = (c->hash * 0x343592dbu) ^ sx_rr(c->hash, 30);
        c->lane[3] ^= sx_rl(c->lane[5], 11);
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
    settle_record(c, x);
}

static void settle_block(sx_state *c) {
    int i;
    c->shash = c->hash;
    c->ssum = c->sum;
    for (i = 0; i < 16; i++) c->slane[i] = c->lane[i];
    memcpy(c->sscr, c->raw + c->slo, (size_t)c->sln);
}

static void mix_chunk_775(sx_state *c) {
    int i;
    c->hash = c->shash;
    c->sum = c->ssum;
    for (i = 0; i < 16; i++) c->lane[i] = c->slane[i];
    memcpy(c->raw + c->slo, c->sscr, (size_t)c->sln);
}

static void split_part(sx_state *c, int lo, int ln) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += (uint32_t)step_field(c);
        c->sched[18] = c->hash ^ sx_rl(c->lane[5], 17);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
        c->lane[4] ^= sx_rl(c->lane[13], 15);
        c->lane[2] += c->lane[3] ^ 0x3eed49e8u;
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

static void grow_span(sx_state *c, uint32_t *out) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[5] += c->lane[10]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 15);
        t2 = (t2 ^ c->sum) * 0xae80323du;
        latch_marker(c, t0, t1);
        t1 ^= (uint32_t)defer_count(c, (uint8_t)(t0 >> 8), t2);
        c->lane[10] += c->lane[15] ^ 0x1482bec0u;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64572u) % (uint32_t)c->rln)] << 16;
        t2 += clamp_table(c, c->rlo, c->rln);
        c->lane[5] += c->lane[10] ^ 0x9385b825u;
        c->lane[12] += c->lane[3]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 28);
        c->raw[c->slo + (int)((t0 + 59769u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x0d1e79a7u;
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

static void flush_table(sx_state *c, uint32_t master, int rnd, int lo, int ln, uint32_t *out) {
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
        t1 ^= (uint32_t)poll_window(c, (uint8_t)(t0 >> 0), t2);
        t0 ^= wrap_path(c, t1);
        c->lane[0] ^= sx_rl(c->lane[4], 18);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb0) << 0;
        t0 ^= patch_part(c, t1);
        t2 = (t2 ^ c->sum) * 0x2c646221u;
        c->raw[c->slo + (int)((t0 + 17896u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
        t2 = (t2 ^ c->sum) * 0x66fe466du;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    split_part(c, lo, ln);
    switch (rnd) {
    case 0:
        merge_chunk(c, c->hash, c->sum);
        rotate_chunk(c, c->hash, c->sum);
        break;
    case 1:
        queue_window(c, c->hash, c->sum);
        slice_index(c, c->hash, c->sum);
        break;
    default:
        tap_pool(c, c->hash, c->sum);
        reap_seat(c, c->hash, c->sum);
        break;
    }
    grow_span(c, out);
#ifdef SX_TRACE
    fprintf(stderr, "T rk%d %08x %08x %08x %08x %08x %08x %08x %08x\n", rnd,
            out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
#endif
}

static void prime_region(sx_state *c, const uint32_t *kk, uint32_t *out) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[11] += c->lane[10]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 11);
        c->lane[6] = sx_rr(c->lane[6] + c->hash, 26);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12917u) % (uint32_t)c->rln)] << 0;
        tap_unit_686(c, t0, t1);
        t1 ^= (uint32_t)parse_node(c, (uint8_t)(t0 >> 0), t2);
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

static void align_label(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    mix_chunk_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        align_segment(c, t0, t1);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7b) << 16;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22335u) % (uint32_t)c->rln)] << 24;
        c->lane[1] ^= sx_rl(c->lane[11], 19);
        c->raw[c->slo + (int)((t0 + 52432u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
        c->sched[0] = c->hash ^ sx_rl(c->lane[4], 6);
        t2 += tally_stream(c, c->slo, c->sln);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint8_t tab[SX_PAY];
    uint8_t src[SX_PAY];
    uint32_t x;
    int i;
    prime_region(c, kk, w);
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

static void hold_pool(sx_state *c, const uint32_t *w, uint32_t *v) {
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

static void pin_unit(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    mix_chunk_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
        c->lane[10] ^= sx_rl(c->lane[5], 18);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5c464697u;
        t0 ^= mix_tail(c, t1);
        blend_delta(c, &c->lane[3], 1);
        t2 = (t2 ^ c->sum) * 0x744b03a9u;
        c->lane[7] += c->lane[9] ^ 0x19eabe55u;
        c->lane[3] ^= sx_rl(c->lane[1], 13);
        c->hash ^= c->lane[11] + 0x9d584c9du;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t v[2];
    uint8_t ks[8];
    int pos = 0, blk = 0, i;
    (void)fwd;
    prime_region(c, kk, w);
    while (pos < ln) {
        int rot = blk & 31;
        if (rot == 0) rot = 1;
        v[0] = (uint32_t)blk ^ w[0];
        v[1] = sx_rl(w[1], rot) ^ w[3];
        hold_pool(c, w, v);
        for (i = 0; i < 4; i++) ks[i] = (uint8_t)(v[0] >> (i * 8));
        for (i = 0; i < 4; i++) ks[4 + i] = (uint8_t)(v[1] >> (i * 8));
        for (i = 0; i < 8 && pos + i < ln; i++) c->raw[lo + pos + i] ^= ks[i];
        pos += 8;
        blk++;
    }
}

static void sort_row(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    mix_chunk_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        reap_record(c, t0, t1);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29139u) % (uint32_t)c->rln)] << 8;
        t2 = (t2 ^ c->sum) * 0xfb24f717u;
        t1 ^= (uint32_t)reset_token(c, (uint8_t)(t0 >> 8), t2);
        c->hash ^= c->lane[12] + 0x72500effu;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18261u) % (uint32_t)c->rln)] << 16;
        t2 = (t2 ^ c->sum) * 0x3108de65u;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint8_t m;
    int i;
    prime_region(c, kk, w);
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

static void trim_entry(uint32_t *s, int a, int b, int d, int e) {
    s[a] += s[b]; s[e] = sx_rl(s[e] ^ s[a], 13);
    s[d] += s[e]; s[b] = sx_rl(s[b] ^ s[d], 9);
    s[a] += s[b]; s[e] = sx_rl(s[e] ^ s[a], 11);
    s[d] += s[e]; s[b] = sx_rl(s[b] ^ s[d], 6);
}

static void tally_scope(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    mix_chunk_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
        c->lane[15] ^= sx_rl(c->lane[11], 3);
        t2 = (t2 ^ c->sum) * 0x00f5c069u;
        t2 += prime_cell(c, c->slo, c->sln);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5ffb170bu;
        c->hash = (c->hash * 0xc0580dc9u) ^ sx_rr(c->hash, 25);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t base[16], s[16];
    uint8_t ks[64];
    int pos = 0, counter = 0, i, r;
    (void)fwd;
    prime_region(c, kk, w);
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
            trim_entry(s, 0, 4, 8, 12);
            trim_entry(s, 1, 5, 9, 13);
            trim_entry(s, 2, 6, 10, 14);
            trim_entry(s, 3, 7, 11, 15);
            trim_entry(s, 0, 5, 10, 15);
            trim_entry(s, 1, 6, 11, 12);
            trim_entry(s, 2, 7, 8, 13);
            trim_entry(s, 3, 4, 9, 14);
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

static void stage_key(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    mix_chunk_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        poll_line_657(c, &c->lane[4], 2);
        c->sched[7] = c->hash ^ sx_rl(c->lane[13], 20);
        c->lane[8] ^= sx_rl(c->lane[4], 31);
        t2 = (t2 ^ c->sum) * 0x19f9cad1u;
        c->hash = (c->hash * 0x0d66112fu) ^ sx_rr(c->hash, 4);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62952u) % (uint32_t)c->rln)] << 24;
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    int i;
    prime_region(c, kk, w);
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

static void mix_tuple(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    mix_chunk_775(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[3] += c->lane[13] ^ 0xd7967388u;
        c->lane[15] = sx_rr(c->lane[15] + c->hash, 6);
        t0 ^= grow_offset(c, t1);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
        close_block(c, &c->lane[10], 1);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        align_segment(c, t0, t1);
        c->lane[8] += c->lane[11]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 3);
        c->raw[c->slo + (int)((t0 + 41430u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
        c->lane[1] += c->lane[10]; c->lane[2] ^= c->lane[1]; c->lane[2] = sx_rl(c->lane[2], 25);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t x;
    int i;
    (void)fwd;
    prime_region(c, kk, w);
    x = w[0] | 1u;
    for (i = 0; i < ln; i++) {
        x = sx_step(x);
        c->raw[lo + i] ^= (uint8_t)x;
    }
}

static void shift_batch_788(sx_state *c, int idx, int lo, int ln, const uint32_t *kk, int fwd) {
    switch (idx) {
    case 0: align_label(c, lo, ln, kk, fwd); break;
    case 1: pin_unit(c, lo, ln, kk, fwd); break;
    case 2: sort_row(c, lo, ln, kk, fwd); break;
    case 3: tally_scope(c, lo, ln, kk, fwd); break;
    case 4: stage_key(c, lo, ln, kk, fwd); break;
    default: mix_tuple(c, lo, ln, kk, fwd); break;
    }
}

static void resize_record(sx_state *c, uint32_t master, int rnd, int alo, int aln, int tlo, int tln, int s0, int s1, int fwd) {
    uint32_t kk[8];
    flush_table(c, master, rnd, alo, aln, kk);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[13] += c->lane[0]; c->lane[3] ^= c->lane[13]; c->lane[3] = sx_rl(c->lane[3], 25);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
        c->sched[2] = c->hash ^ sx_rl(c->lane[5], 24);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
        t2 += mix_seat(c, c->slo, c->sln);
        t1 ^= (uint32_t)close_digest(c, (uint8_t)(t0 >> 0), t2);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    settle_block(c);
    if (fwd) {
        shift_batch_788(c, s0, tlo, tln, &kk[0], 1);
        shift_batch_788(c, s1, tlo, tln, &kk[4], 1);
    } else {
        shift_batch_788(c, s1, tlo, tln, &kk[4], 0);
        shift_batch_788(c, s0, tlo, tln, &kk[0], 0);
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

    patch_row(c);
    base = yield_state_771(c, nonce, len);
    {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash = (c->hash * 0x5bc85223u) ^ sx_rr(c->hash, 17);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47061u) % (uint32_t)c->rln)] << 24;
        t1 ^= (uint32_t)wrap_path_683(c, (uint8_t)(t0 >> 0), t2);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x88) << 0;
        c->raw[c->slo + (int)((t0 + 30486u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
        c->lane[0] = sx_rr(c->lane[0] + c->hash, 5);
        t2 = (t2 ^ c->sum) * 0xcc75b19du;
        t2 += (uint32_t)tune_state_656(c);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    }
    master = base ^ c->hash ^ c->sum ^ c->lane[3];
#ifdef SX_TRACE
    fprintf(stderr, "T master=%08x base=%08x\n", master, base);
#endif
    fill_batch(c, master);
#ifdef SX_TRACE
    fprintf(stderr, "T poly=%02x cpoly=%08x step=%08x vec=%08x %08x %08x %08x fwd7=%02x\n",
            c->poly, c->cpoly, c->step, c->vec[0], c->vec[1], c->vec[2], c->vec[3], c->fwd[7]);
#endif

    h = len / 2;
    lows[0] = 0; lens[0] = h;
    lows[1] = h; lens[1] = len - h;

    for (k = 0; k < 3; k++) {
        i = mode ? k : (2 - k);
        resize_record(c, master, PL[i][0],
                     lows[PL[i][1]], lens[PL[i][1]],
                     lows[PL[i][2]], lens[PL[i][2]],
                     PL[i][3], PL[i][4], mode);
    }

    memcpy(out, c->raw, (size_t)len);
    memset(&st, 0, sizeof(st));
}

