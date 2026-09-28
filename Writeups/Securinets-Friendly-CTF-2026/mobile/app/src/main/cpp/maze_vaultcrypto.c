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

static void prime_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t split_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int fetch_cursor(sx_state *c) __attribute__((used, noinline));
static uint32_t pack_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t blend_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int scan_frame(sx_state *c) __attribute__((used, noinline));
static void drain_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t push_head(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int swap_range(sx_state *c) __attribute__((used, noinline));
static uint32_t queue_path(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t queue_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void trace_value(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void defer_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void store_queue(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t defer_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void map_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t rotate_tuple(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int sort_scope(sx_state *c) __attribute__((used, noinline));
static uint8_t join_unit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t chain_pairing(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t purge_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t latch_stream(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t store_unit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t trace_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tune_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t merge_unit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t stage_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void rotate_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t place_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void trace_track(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t coal_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int poll_layer(sx_state *c) __attribute__((used, noinline));
static void merge_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t store_stream(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t grow_label(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void peek_group(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t prime_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t tally_window(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void emit_store(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int drain_view(sx_state *c) __attribute__((used, noinline));
static int settle_frame(sx_state *c) __attribute__((used, noinline));
static void prime_table(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t stage_segment(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void fold_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t swap_count(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void link_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void emit_field(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t step_record(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int mix_tail(sx_state *c) __attribute__((used, noinline));
static void fold_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t stage_limit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t mark_chunk(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int fetch_range(sx_state *c) __attribute__((used, noinline));
static int push_head_53(sx_state *c) __attribute__((used, noinline));
static uint32_t seek_head(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t split_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t pair_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int relay_layer(sx_state *c) __attribute__((used, noinline));
static void latch_offset(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t queue_unit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int settle_field(sx_state *c) __attribute__((used, noinline));
static void map_unit(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void move_frame(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t close_record(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t sort_band(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int reset_gap(sx_state *c) __attribute__((used, noinline));
static void pick_cell(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t seek_offset(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t pick_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int sift_head(sx_state *c) __attribute__((used, noinline));
static int reset_marker(sx_state *c) __attribute__((used, noinline));
static int scan_value(sx_state *c) __attribute__((used, noinline));
static void probe_segment(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void seek_node(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void resize_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void wrap_tail(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void mix_layer(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int probe_span(sx_state *c) __attribute__((used, noinline));
static uint32_t store_state(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void wrap_group(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tally_group(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t push_track(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t step_block(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int peek_stream(sx_state *c) __attribute__((used, noinline));
static void peek_line(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int merge_token(sx_state *c) __attribute__((used, noinline));
static uint8_t fold_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t sift_queue(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t emit_marker(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void pair_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int patch_segment(sx_state *c) __attribute__((used, noinline));
static uint32_t chain_token(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void close_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tap_part(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t settle_frame_94(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void resize_frame(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void pin_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void sift_region(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t purge_ring(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t align_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void trace_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t prime_index_101(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t relay_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int fold_view(sx_state *c) __attribute__((used, noinline));
static uint8_t place_rate(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void load_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t flush_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int pack_frame(sx_state *c) __attribute__((used, noinline));
static void chain_table(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int slice_bound(sx_state *c) __attribute__((used, noinline));
static int fill_seat(sx_state *c) __attribute__((used, noinline));
static uint32_t merge_path(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t yield_chunk(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void store_stream_113(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t swap_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pack_band(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int sift_digest(sx_state *c) __attribute__((used, noinline));
static uint32_t mix_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void emit_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void reset_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void emit_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t resize_chunk(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int settle_layer(sx_state *c) __attribute__((used, noinline));
static int latch_record(sx_state *c) __attribute__((used, noinline));
static uint8_t coal_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void latch_tuple(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int reset_label(sx_state *c) __attribute__((used, noinline));
static uint32_t pack_window(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void peek_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void trim_block(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int prime_stream(sx_state *c) __attribute__((used, noinline));
static uint8_t align_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void store_scope(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t parse_run(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void load_lease_134(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int queue_view(sx_state *c) __attribute__((used, noinline));
static int align_count(sx_state *c) __attribute__((used, noinline));
static int close_window(sx_state *c) __attribute__((used, noinline));
static void hold_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void coal_port(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int trim_queue(sx_state *c) __attribute__((used, noinline));
static uint32_t trim_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void wrap_batch(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t yield_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t resize_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t prime_node(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t swap_pool(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void trim_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t probe_index(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void link_label_149(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t join_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pair_field(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void patch_item(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t fold_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int merge_port(sx_state *c) __attribute__((used, noinline));
static void patch_delta(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int fold_range(sx_state *c) __attribute__((used, noinline));
static void fold_mask(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int tally_arena(sx_state *c) __attribute__((used, noinline));
static void map_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t seek_span(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t stage_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_part_162(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int reap_index(sx_state *c) __attribute__((used, noinline));
static void parse_run_164(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int swap_bucket(sx_state *c) __attribute__((used, noinline));
static uint8_t tally_entry(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_scope(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t peek_segment(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t sift_level(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int rotate_item(sx_state *c) __attribute__((used, noinline));
static uint8_t coal_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t tune_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t sync_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void mix_page(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int pair_mask(sx_state *c) __attribute__((used, noinline));
static uint8_t defer_part_176(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int sync_index(sx_state *c) __attribute__((used, noinline));
static uint8_t place_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t sort_unit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t sync_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void coal_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t align_key(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t settle_list(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t queue_layer(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int coal_offset(sx_state *c) __attribute__((used, noinline));
static uint32_t rotate_pairing(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void link_bucket(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t probe_window(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t map_band(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t latch_entry(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void cache_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t close_pool(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int emit_lease(sx_state *c) __attribute__((used, noinline));
static uint8_t relay_field(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t hold_queue(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void scan_label(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void pair_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t wrap_cursor(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int emit_marker_199(sx_state *c) __attribute__((used, noinline));
static int clamp_bound(sx_state *c) __attribute__((used, noinline));
static int sift_chunk(sx_state *c) __attribute__((used, noinline));
static uint8_t split_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void poll_rate(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t drain_tuple(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void slice_level(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void seek_slot(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void fetch_window(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int map_node(sx_state *c) __attribute__((used, noinline));
static uint8_t close_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t step_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t split_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t fetch_bucket(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t tally_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int purge_scope(sx_state *c) __attribute__((used, noinline));
static void fold_bucket(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void merge_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t resize_pool(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int sort_region(sx_state *c) __attribute__((used, noinline));
static void move_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t close_pairing(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void load_store(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void probe_block(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t purge_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int grow_stream(sx_state *c) __attribute__((used, noinline));
static uint32_t step_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t slice_level_226(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void mark_table(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t blend_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t probe_delta(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t relay_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t scan_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int purge_pool(sx_state *c) __attribute__((used, noinline));
static uint8_t fetch_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void poll_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t settle_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fetch_page(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t drain_region(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int store_value(sx_state *c) __attribute__((used, noinline));
static void yield_pairing(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t sift_stream(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t purge_marker(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t drain_mask(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pair_pool(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t queue_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t peek_record(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int rotate_slot(sx_state *c) __attribute__((used, noinline));
static int fold_seat(sx_state *c) __attribute__((used, noinline));
static int settle_tail(sx_state *c) __attribute__((used, noinline));
static int stage_marker(sx_state *c) __attribute__((used, noinline));
static uint8_t place_rate_251(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t join_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t wrap_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t yield_marker(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int align_page(sx_state *c) __attribute__((used, noinline));
static void reset_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int reap_tail(sx_state *c) __attribute__((used, noinline));
static uint32_t wrap_rate_258(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tap_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t prime_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t sort_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void peek_level(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t load_unit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void rotate_range(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void poll_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void join_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void resize_marker(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tap_marker(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tap_path(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int stage_key(sx_state *c) __attribute__((used, noinline));
static void purge_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t split_layer(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t reap_pool(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void emit_port(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void tune_arena(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void emit_record(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t align_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t move_frame_278(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t fill_row(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t scan_bound(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_ring(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t pick_digest(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t prime_stream_283(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t parse_part(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fetch_batch(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void resize_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t latch_port(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void poll_group(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t queue_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t pack_batch(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t purge_ring_291(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int emit_rate(sx_state *c) __attribute__((used, noinline));
static uint8_t step_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void coal_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t map_item(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t slice_layer(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t load_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t wrap_marker(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tally_slot(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void split_window(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void coal_batch(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void peek_record_302(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void defer_field_303(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t wrap_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t peek_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t stage_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t align_cursor(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void swap_tail(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t close_bucket(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t defer_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t mark_item(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t load_line(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t tap_segment(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int push_head_314(sx_state *c) __attribute__((used, noinline));
static void map_window(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void shift_offset(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void split_scope_317(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void mark_node(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int cache_window(sx_state *c) __attribute__((used, noinline));
static int link_run(sx_state *c) __attribute__((used, noinline));
static uint8_t prime_ring(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void yield_pairing_322(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t chain_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t resize_lease_324(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int yield_track(sx_state *c) __attribute__((used, noinline));
static void sync_record(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int merge_region(sx_state *c) __attribute__((used, noinline));
static uint32_t probe_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t defer_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void step_tail(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t cache_window_331(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void blend_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t mix_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t sync_cell_335(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int queue_span(sx_state *c) __attribute__((used, noinline));
static void load_pairing(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t pick_unit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void chain_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t relay_span_340(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t move_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void purge_group(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pin_entry(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fetch_window_344(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fold_unit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void seek_gap(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t store_token(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pack_entry(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t tap_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int sift_block(sx_state *c) __attribute__((used, noinline));
static uint32_t fetch_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int slice_item(sx_state *c) __attribute__((used, noinline));
static void shift_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int latch_node(sx_state *c) __attribute__((used, noinline));
static void pick_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int parse_token(sx_state *c) __attribute__((used, noinline));
static int patch_line(sx_state *c) __attribute__((used, noinline));
static int tune_tuple(sx_state *c) __attribute__((used, noinline));
static int relay_batch(sx_state *c) __attribute__((used, noinline));
static void sift_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int align_table(sx_state *c) __attribute__((used, noinline));
static void place_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void purge_list(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int stage_cursor(sx_state *c) __attribute__((used, noinline));
static uint32_t drain_queue(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void settle_index(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void purge_region(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t step_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t slice_list(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void mark_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int pin_region(sx_state *c) __attribute__((used, noinline));
static int fold_list(sx_state *c) __attribute__((used, noinline));
static void coal_port_373(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void shift_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void purge_gap(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int pick_tail(sx_state *c) __attribute__((used, noinline));
static uint32_t clamp_marker(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void mark_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t pair_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t mark_unit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void purge_count(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t blend_mask(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void coal_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void pack_index(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t grow_field(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void purge_count_386(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t resize_delta(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void stage_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void peek_run(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void relay_stack(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t store_stream_391(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void settle_cursor(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t settle_head(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t sync_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pin_label_395(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void slice_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void tune_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t stage_list(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t move_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void load_key(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t purge_run(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t map_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int settle_range(sx_state *c) __attribute__((used, noinline));
static uint8_t mark_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t align_scope(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t hold_page(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t place_bound(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_line(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void sync_node_410(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void slice_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void yield_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int push_digest(sx_state *c) __attribute__((used, noinline));
static void trim_segment(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t hold_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void map_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t rotate_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t close_slot(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t clamp_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t peek_block_420(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t join_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void mark_lease_422(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t split_frame(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t fold_segment(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void parse_count(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tune_marker(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int chain_line(sx_state *c) __attribute__((used, noinline));
static uint32_t fetch_batch_428(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int cache_list(sx_state *c) __attribute__((used, noinline));
static uint32_t swap_offset(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fill_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t patch_label(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t tune_row(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t map_range(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t store_path(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t slice_record(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t trim_pool(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t trim_mask_439(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void mark_track(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t swap_token(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void cache_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void blend_span(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t chain_batch(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t tally_stack(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t chain_batch_446(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void drain_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void drain_cell(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t drain_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void wrap_pool(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t place_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int blend_run(sx_state *c) __attribute__((used, noinline));
static int reap_token(sx_state *c) __attribute__((used, noinline));
static uint32_t poll_field(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t merge_band(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t mark_frame(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void probe_stream(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void pick_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t drain_table_459(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void latch_token(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int mix_key(sx_state *c) __attribute__((used, noinline));
static uint32_t stage_span(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void mix_bucket(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t yield_queue(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sync_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int pair_line(sx_state *c) __attribute__((used, noinline));
static uint32_t stage_range_467(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void join_marker(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void settle_region(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void chain_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void place_range(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t yield_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void close_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void parse_bound(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t stage_offset(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t patch_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t seek_port(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int pick_queue(sx_state *c) __attribute__((used, noinline));
static void stage_page(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t push_track_480(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void hold_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pin_lease(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t cache_chunk(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void queue_lease_484(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t trim_item(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void reset_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t patch_record(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t pack_path(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t chain_tail(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pair_store(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t flush_gap(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void shift_slot(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int swap_pool_493(sx_state *c) __attribute__((used, noinline));
static uint32_t join_unit_494(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t tune_store(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pick_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t stage_block_497(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t clamp_state(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void purge_chunk(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int mix_slot(sx_state *c) __attribute__((used, noinline));
static uint32_t align_band(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void close_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int sync_range(sx_state *c) __attribute__((used, noinline));
static uint8_t pin_rate(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void move_offset(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int place_store(sx_state *c) __attribute__((used, noinline));
static int pick_delta(sx_state *c) __attribute__((used, noinline));
static uint8_t chain_stream(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t join_group(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t defer_rate(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sift_stack(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void peek_digest(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void rotate_track(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t grow_index(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tune_head(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t settle_segment(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t parse_node(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void emit_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int split_window_519(sx_state *c) __attribute__((used, noinline));
static uint32_t sort_view(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t fetch_bound(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t align_span(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void fill_head(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int hold_gap(sx_state *c) __attribute__((used, noinline));
static uint32_t sort_pairing_525(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void chain_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void poll_arena(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t fold_count(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tune_store_529(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t merge_range(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void load_ring(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t split_bucket(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int store_label(sx_state *c) __attribute__((used, noinline));
static void defer_slot(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t load_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t close_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int drain_gap(sx_state *c) __attribute__((used, noinline));
static uint32_t merge_stack(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tune_state_539(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fill_rate(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void merge_layer(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t pin_level(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void split_path(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fold_arena(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int parse_rate(sx_state *c) __attribute__((used, noinline));
static void flush_limit(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void swap_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void probe_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void yield_digest(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t sync_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void fold_value(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t push_lease(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t resize_seat(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int drain_offset(sx_state *c) __attribute__((used, noinline));
static uint32_t wrap_stack(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t peek_table(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void clamp_lease(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void drain_lease(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t swap_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int settle_stack(sx_state *c) __attribute__((used, noinline));
static uint32_t slice_tuple(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t fold_limit(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t poll_block(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void sift_segment(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t load_offset(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void move_ring_566(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void move_table(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void peek_scope(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t probe_track(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sync_gap_570(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int sync_bound(sx_state *c) __attribute__((used, noinline));
static void clamp_value(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int store_run(sx_state *c) __attribute__((used, noinline));
static uint32_t step_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tap_count(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t resize_cell(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int tap_cursor(sx_state *c) __attribute__((used, noinline));
static int probe_region(sx_state *c) __attribute__((used, noinline));
static int grow_stream_579(sx_state *c) __attribute__((used, noinline));
static void prime_count(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t trace_delta(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void drain_seat(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void hold_layer(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t relay_bucket(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t swap_queue(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void drain_span(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void reset_range(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t sift_node(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void tune_head_589(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int shift_record(sx_state *c) __attribute__((used, noinline));
static uint32_t wrap_chunk(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pair_pairing(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void reap_marker(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void push_seat(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t seek_group(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t fetch_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void probe_pairing(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void reap_ring(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int poll_token(sx_state *c) __attribute__((used, noinline));
static void shift_line_600(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t patch_stack(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t tap_gap(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t reap_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t scan_key(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void tune_queue(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t yield_count(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t rotate_mask(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t seek_table(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t align_table_609(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_entry(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t load_offset_611(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t poll_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t drain_tail(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void push_field(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t fold_span(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t stage_value(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void join_pool(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int split_bucket_618(sx_state *c) __attribute__((used, noinline));
static void peek_port(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t blend_head_620(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t emit_track(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void purge_node(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t trim_node(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t map_cursor(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void settle_region_625(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void pair_item(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void link_queue(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t pin_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int drain_delta(sx_state *c) __attribute__((used, noinline));
static void blend_token(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t hold_page_631(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t probe_arena(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void hold_scope(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t prime_label(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void place_ring(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t close_layer(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void peek_entry(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tally_pairing(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t sift_slot(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void purge_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void blend_part(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t reset_arena(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void split_arena(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t clamp_queue(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_ring(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void fetch_tuple(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t store_tail(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t parse_line(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t reap_range(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void coal_part(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int stage_node_651(sx_state *c) __attribute__((used, noinline));
static int pack_queue(sx_state *c) __attribute__((used, noinline));
static uint8_t cache_page(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void merge_list(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void parse_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t rotate_pool(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pack_state(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t seek_span_658(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void fill_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tally_span(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void patch_page(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t hold_port(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t drain_span_663(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int blend_range(sx_state *c) __attribute__((used, noinline));
static void flush_segment(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t flush_level(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int trim_queue_667(sx_state *c) __attribute__((used, noinline));
static int parse_marker(sx_state *c) __attribute__((used, noinline));
static uint8_t sort_region_669(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void pick_marker(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void fill_stream(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static int latch_record_672(sx_state *c) __attribute__((used, noinline));
static uint32_t relay_head(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t prime_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void swap_label(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int parse_field(sx_state *c) __attribute__((used, noinline));
static void mix_row(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t fold_item(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t trace_stack_679(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static int emit_field_680(sx_state *c) __attribute__((used, noinline));
static int sync_line(sx_state *c) __attribute__((used, noinline));
static uint8_t map_stream(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void move_page(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t align_label(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void join_batch(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint8_t rotate_gap(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void store_node(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static void scan_layer(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t pack_node(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void tap_window(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t link_layer(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t peek_delta(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int pair_part_693(sx_state *c) __attribute__((used, noinline));
static int reap_scope_694(sx_state *c) __attribute__((used, noinline));
static int mix_node(sx_state *c) __attribute__((used, noinline));
static uint32_t defer_marker(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t probe_token(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void seek_chunk(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t reap_slot(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t trace_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static int load_stream(sx_state *c) __attribute__((used, noinline));
static uint32_t slice_part(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t sync_key(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t load_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t peek_scope_705(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t sync_mask(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint8_t mark_page(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t pair_index(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void fill_path(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t seek_block(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void chain_row(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t mark_list(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint8_t cache_list_713(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void relay_table(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t close_key(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void trim_view(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t pin_value(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void scan_cell(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t align_arena(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t drain_digest(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t tune_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t poll_limit(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t tune_level(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void sift_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static int prime_stream_725(sx_state *c) __attribute__((used, noinline));
static int split_table(sx_state *c) __attribute__((used, noinline));
static int fold_span_727(sx_state *c) __attribute__((used, noinline));
static int push_ring(sx_state *c) __attribute__((used, noinline));
static uint32_t trace_cursor(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void slice_mask(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t swap_token_731(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void coal_region(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t step_delta(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t split_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int fold_tuple(sx_state *c) __attribute__((used, noinline));
static uint32_t mark_window(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void hold_track(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t merge_pairing(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t chain_limit(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void patch_line_740(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t close_table(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t pin_bound(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void pin_stack(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t coal_group(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t grow_item(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint8_t shift_key(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t clamp_store(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static uint32_t swap_cell(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int reap_bound(sx_state *c) __attribute__((used, noinline));
static uint8_t clamp_scope(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static uint32_t split_tail(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int defer_gap(sx_state *c) __attribute__((used, noinline));
static void fold_scope(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static void mix_level(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint32_t trim_digest(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void join_value(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t grow_mask(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int yield_delta(sx_state *c) __attribute__((used, noinline));
static uint32_t grow_slot(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void patch_port(sx_state *c, uint32_t *v, int k) __attribute__((used, noinline));
static uint8_t stage_tuple(sx_state *c, uint8_t v, uint32_t a) __attribute__((used, noinline));
static void split_key(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));
static uint32_t tap_batch(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static int blend_node(sx_state *c) __attribute__((used, noinline));
static uint32_t pick_lease(sx_state *c, uint32_t a) __attribute__((used, noinline));
static uint32_t shift_view(sx_state *c, uint32_t a) __attribute__((used, noinline));
static void chain_row_767(sx_state *c, uint32_t a, uint32_t b) __attribute__((used, noinline));

static uint32_t stage_limit_768(uint32_t x) __attribute__((used, noinline));
static int fill_gap(sx_state *c) __attribute__((used, noinline));
static uint32_t rotate_list(uint32_t prime, uint32_t h, uint8_t x) __attribute__((used, noinline));
static uint32_t stage_ring(sx_state *c, uint32_t nonce, int size) __attribute__((used, noinline));
static void emit_row(sx_state *c, uint32_t master) __attribute__((used, noinline));
static void push_stream(sx_state *c, uint32_t seed) __attribute__((used, noinline));
static void probe_tail(sx_state *c) __attribute__((used, noinline));
static void pin_store(sx_state *c) __attribute__((used, noinline));
static void pick_ring_776(sx_state *c, int lo, int ln) __attribute__((used, noinline));
static void drain_port(sx_state *c, uint32_t *out) __attribute__((used, noinline));
static void tap_port(sx_state *c, uint32_t master, int rnd, int lo, int ln, uint32_t *out) __attribute__((used, noinline));
static void prime_marker(sx_state *c, const uint32_t *kk, uint32_t *out) __attribute__((used, noinline));
static void load_node(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void emit_bucket(sx_state *c, const uint32_t *w, uint32_t *v) __attribute__((used, noinline));
static void split_value(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void settle_track(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void swap_queue_784(uint32_t *s, int a, int b, int d, int e) __attribute__((used, noinline));
static void pair_lease(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void prime_token(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void shift_queue(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void mix_limit(sx_state *c, int idx, int lo, int ln, const uint32_t *kk, int fwd) __attribute__((used, noinline));
static void probe_item(sx_state *c, uint32_t master, int rnd, int alo, int aln, int tlo, int tln, int s0, int s1, int fwd) __attribute__((used, noinline));


static void prime_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[7] = c->hash ^ sx_rl(c->lane[6], 23);
    t2 += rotate_tuple(c, c->slo, c->sln);
    t0 ^= pack_ring(c, t1);
    t1 ^= (uint32_t)join_unit(c, (uint8_t)(t0 >> 0), t2);
    t2 += resize_chunk(c, c->slo, c->sln);
    t2 += (uint32_t)patch_segment(c);
    reset_store(c, &c->lane[6], 1);
    c->hash ^= c->lane[9] + 0x28fb3f0au;
    t2 += (uint32_t)sift_head(c);
    t0 ^= emit_marker(c, t1);
    mix_layer(c, &c->lane[2], 4);
    t2 += (uint32_t)poll_layer(c);
    t1 ^= (uint32_t)purge_digest(c, (uint8_t)(t0 >> 0), t2);
    resize_ring(c, t0, t1);
    t1 ^= (uint32_t)tally_window(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)place_rate(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)split_digest(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6369d567u;
    t1 ^= (uint32_t)sort_band(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= tap_part(c, t1);
    c->raw[c->slo + (int)((t0 + 63806u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x3d035ab7u) ^ sx_rr(c->hash, 19);
    t2 += (uint32_t)fetch_cursor(c);
    t2 += store_unit(c, c->rlo, c->rln);
    chain_table(c, &c->lane[7], 2);
    pair_part(c, t0, t1);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 9);
    c->sched[8] = c->hash ^ sx_rl(c->lane[14], 18);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t split_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= swap_count(c, t1);
    c->hash ^= c->lane[1] + 0x1cc62131u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 11);
    trace_value(c, t0, t1);
    t2 += (uint32_t)peek_stream(c);
    c->raw[c->slo + (int)((t0 + 38353u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += (uint32_t)slice_bound(c);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 28);
    c->lane[9] += c->lane[1] ^ 0xf40ddb83u;
    store_queue(c, t0, t1);
    t2 += (uint32_t)swap_range(c);
    t2 += (uint32_t)mix_tail(c);
    c->lane[2] ^= sx_rl(c->lane[2], 11);
    c->lane[5] += c->lane[0] ^ 0x763c74bbu;
    t2 += (uint32_t)scan_frame(c);
    emit_layer(c, t0, t1);
    t2 += stage_limit(c, c->slo, c->sln);
    t2 += blend_list(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65387u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int fetch_cursor(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 ^= push_head(c, t1);
    t2 += mark_chunk(c, c->slo, c->sln);
    t1 ^= (uint32_t)coal_span(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x71fde2d5u;
    defer_part(c, t0, t1);
    t2 += queue_path(c, c->rlo, c->rln);
    c->hash ^= c->lane[1] + 0x11e91ca1u;
    t2 = (t2 ^ c->sum) * 0x8c78be95u;
    c->raw[c->slo + (int)((t0 + 45105u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    fold_chunk(c, t0, t1);
    c->lane[11] += c->lane[0]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 8);
    t2 = (t2 ^ c->sum) * 0x5d4c53ffu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x7bb1e99bu;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t pack_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    store_stream_113(c, &c->lane[0], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60185u) % (uint32_t)c->rln)] << 16;
    t2 += (uint32_t)probe_span(c);
    c->sched[10] = c->hash ^ sx_rl(c->lane[0], 13);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 15);
    c->lane[3] ^= sx_rl(c->lane[9], 8);
    t0 ^= queue_lease(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x21ff3a23u;
    t1 ^= (uint32_t)seek_offset(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53717u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa70d41bbu;
    c->lane[12] += c->lane[8] ^ 0x81beecb9u;
    c->lane[1] ^= sx_rl(c->lane[13], 4);
    drain_entry(c, t0, t1);
    c->lane[10] ^= sx_rl(c->lane[15], 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t blend_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    merge_part(c, t0, t1);
    c->lane[5] += c->lane[6] ^ 0x078c95abu;
    c->hash ^= c->lane[11] + 0xcfc64c06u;
    c->raw[c->slo + (int)((t0 + 6884u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30758u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)place_arena(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbd9178e5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41581u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)poll_layer(c);
    c->lane[10] += c->lane[15] ^ 0x407cd359u;
    c->sum += t1;
    return t0 + t2;
}

static int scan_frame(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44319u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    latch_offset(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    map_view(c, t0, t1);
    c->lane[10] += c->lane[9]; c->lane[14] ^= c->lane[10]; c->lane[14] = sx_rl(c->lane[14], 8);
    c->sched[10] = c->hash ^ sx_rl(c->lane[4], 18);
    t1 ^= (uint32_t)purge_digest(c, (uint8_t)(t0 >> 8), t2);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void drain_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 57325u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] += c->lane[12] ^ 0xe6d0b26au;
    t2 = (t2 ^ c->sum) * 0x9fc3430fu;
    c->lane[14] += c->lane[11] ^ 0x96be59fbu;
    load_lease(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t push_head(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 17321u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[14] + 0xe7e4fde8u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59158u) % (uint32_t)c->rln)] << 24;
    t2 += defer_field(c, c->rlo, c->rln);
    t2 += (uint32_t)settle_frame(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x34e6fc19u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2a3ce961u;
    c->hash ^= c->lane[10] + 0x33f4c727u;
    c->raw[c->slo + (int)((t0 + 62922u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[29] = c->hash ^ sx_rl(c->lane[14], 9);
    c->sched[2] = c->hash ^ sx_rl(c->lane[7], 31);
    c->lane[7] ^= sx_rl(c->lane[7], 3);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int swap_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->sched[19] = c->hash ^ sx_rl(c->lane[2], 20);
    c->hash ^= c->lane[8] + 0xb7cf0cd6u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa3abb67fu;
    trace_track(c, t0, t1);
    c->hash ^= c->lane[0] + 0x690ebe65u;
    c->lane[14] += c->lane[7]; c->lane[3] ^= c->lane[14]; c->lane[3] = sx_rl(c->lane[3], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12048u) % (uint32_t)c->rln)] << 0;
    merge_part(c, t0, t1);
    c->hash ^= c->lane[15] + 0xa3d2debfu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t1 ^= (uint32_t)trace_stack(c, (uint8_t)(t0 >> 0), t2);
    t2 += store_stream(c, c->rlo, c->rln);
    pick_cell(c, &c->lane[1], 1);
    t1 ^= (uint32_t)chain_pairing(c, (uint8_t)(t0 >> 0), t2);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t queue_path(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x593aaf3du;
    c->raw[c->slo + (int)((t0 + 52019u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    move_frame(c, &c->lane[6], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 19);
    map_view(c, t0, t1);
    c->hash ^= c->lane[13] + 0x324f2f7eu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t queue_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 17);
    c->lane[15] += c->lane[0]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5a) << 8;
    t1 ^= (uint32_t)trace_stack(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16114u) % (uint32_t)c->rln)] << 24;
    c->lane[8] += c->lane[5]; c->lane[6] ^= c->lane[8]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9a) << 16;
    c->sched[18] = c->hash ^ sx_rl(c->lane[11], 27);
    t2 += (uint32_t)settle_field(c);
    c->raw[c->slo + (int)((t0 + 55853u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[10] ^= sx_rl(c->lane[11], 22);
    t1 ^= (uint32_t)join_unit(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[10] = c->hash ^ sx_rl(c->lane[9], 13);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void trace_value(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[11] += c->lane[4]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x1e) << 16;
    t2 += latch_stream(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xeb) << 0;
    tune_state(c, &c->lane[4], 2);
    c->lane[10] += c->lane[4] ^ 0xbe1d4b24u;
    c->hash = (c->hash * 0xc94e4c9du) ^ sx_rr(c->hash, 29);
    c->hash ^= c->lane[7] + 0x9fd87f0bu;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 4);
    t2 += (uint32_t)sort_scope(c);
    t0 ^= stage_node(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcc575b6fu;
    rotate_seat(c, &c->lane[1], 4);
    t2 += rotate_tuple(c, c->slo, c->sln);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void defer_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[9] ^= sx_rl(c->lane[5], 24);
    t2 += merge_unit(c, c->slo, c->sln);
    c->hash = (c->hash * 0x58756c53u) ^ sx_rr(c->hash, 3);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 11);
    t1 ^= (uint32_t)coal_lease(c, (uint8_t)(t0 >> 0), t2);
    c->lane[15] ^= sx_rl(c->lane[8], 2);
    c->hash ^= c->lane[0] + 0x0bc71f96u;
    c->raw[c->slo + (int)((t0 + 53110u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash = (c->hash * 0x91984b9fu) ^ sx_rr(c->hash, 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void store_queue(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21973u) % (uint32_t)c->rln)] << 24;
    t0 ^= pack_band(c, t1);
    c->raw[c->slo + (int)((t0 + 484u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[8] += c->lane[10]; c->lane[12] ^= c->lane[8]; c->lane[12] = sx_rl(c->lane[12], 10);
    c->lane[1] ^= sx_rl(c->lane[5], 11);
    c->lane[12] ^= sx_rl(c->lane[12], 10);
    c->lane[15] ^= sx_rl(c->lane[4], 16);
    c->lane[1] ^= sx_rl(c->lane[7], 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x49) << 16;
    t2 += merge_unit(c, c->slo, c->sln);
    t2 += store_stream(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sched[12] = c->hash ^ sx_rl(c->lane[12], 26);
    c->sched[13] = c->hash ^ sx_rl(c->lane[2], 21);
    t2 += store_unit(c, c->rlo, c->rln);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t defer_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    wrap_tail(c, &c->lane[5], 2);
    c->hash = (c->hash * 0x18a6860du) ^ sx_rr(c->hash, 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17705u) % (uint32_t)c->rln)] << 24;
    emit_field(c, &c->lane[3], 4);
    c->hash = (c->hash * 0x29581b23u) ^ sx_rr(c->hash, 8);
    map_unit(c, t0, t1);
    t2 += mark_chunk(c, c->rlo, c->rln);
    c->lane[7] += c->lane[15]; c->lane[9] ^= c->lane[7]; c->lane[9] = sx_rl(c->lane[9], 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8225u) % (uint32_t)c->rln)] << 24;
    c->sched[15] = c->hash ^ sx_rl(c->lane[6], 13);
    t2 += (uint32_t)latch_record(c);
    c->lane[8] += c->lane[11] ^ 0x24525076u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcd131137u;
    c->sum += t1;
    return t0 + t2;
}

static void map_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[5] + 0x3bd0cea8u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 25);
    c->raw[c->slo + (int)((t0 + 6604u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= grow_label(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xee092993u;
    t1 ^= (uint32_t)push_track(c, (uint8_t)(t0 >> 8), t2);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t rotate_tuple(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += seek_head(c, c->rlo, c->rln);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9aaac943u;
    pick_cell(c, &c->lane[5], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33294u) % (uint32_t)c->rln)] << 24;
    c->sum += t1;
    return t0 + t2;
}

static int sort_scope(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[12] ^= sx_rl(c->lane[3], 25);
    t2 = (t2 ^ c->sum) * 0xadf7c0dfu;
    link_label(c, &c->lane[9], 1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x98fb214du;
    c->lane[15] += c->lane[4]; c->lane[13] ^= c->lane[15]; c->lane[13] = sx_rl(c->lane[13], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe7d73f3du;
    c->raw[c->slo + (int)((t0 + 64219u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    move_frame(c, &c->lane[5], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf5) << 8;
    c->raw[c->slo + (int)((t0 + 6023u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[20] = c->hash ^ sx_rl(c->lane[8], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x0c2511f7u;
    t2 = (t2 ^ c->sum) * 0x07952f85u;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t join_unit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[6] = c->hash ^ sx_rl(c->lane[9], 26);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 6);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 6985u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x2da28429u;
    c->lane[2] += c->lane[5] ^ 0x5d1dad0fu;
    c->lane[10] ^= sx_rl(c->lane[12], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t chain_pairing(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 18);
    c->hash ^= c->lane[7] + 0x8401a374u;
    t2 = (t2 ^ c->sum) * 0xe7b13275u;
    c->hash = (c->hash * 0x2e53a2efu) ^ sx_rr(c->hash, 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[1] += c->lane[14] ^ 0xd09064c7u;
    t0 ^= step_record(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t purge_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    peek_group(c, t0, t1);
    c->lane[8] ^= sx_rl(c->lane[10], 14);
    c->hash ^= c->lane[6] + 0x23bbb162u;
    t2 += stage_segment(c, c->slo, c->sln);
    t2 += (uint32_t)settle_frame(c);
    c->hash = (c->hash * 0x101f3765u) ^ sx_rr(c->hash, 16);
    c->sched[21] = c->hash ^ sx_rl(c->lane[11], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0xa9992e51u) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x05908419u;
    c->hash = (c->hash * 0x06be937du) ^ sx_rr(c->hash, 29);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6a7f0dc5u;
    t2 += relay_span(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x46a276e7u) ^ sx_rr(c->hash, 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t latch_stream(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)drain_view(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xef) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8b) << 16;
    c->hash = (c->hash * 0xa587fe09u) ^ sx_rr(c->hash, 25);
    t2 += (uint32_t)fetch_range(c);
    c->lane[3] += c->lane[12] ^ 0x05b88d7eu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf1fcf9ffu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t store_unit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] += c->lane[2]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44314u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0xa0846c5du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7bb9dc69u;
    t0 ^= grow_label(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x42) << 16;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t trace_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[15] ^= sx_rl(c->lane[14], 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x63fbc2a9u;
    t2 += (uint32_t)reset_gap(c);
    c->lane[11] ^= sx_rl(c->lane[5], 29);
    c->hash ^= c->lane[1] + 0x54569db7u;
    c->hash ^= c->lane[3] + 0x555d7252u;
    c->lane[11] += c->lane[4] ^ 0xd41c3cd7u;
    t1 ^= (uint32_t)queue_unit(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] += c->lane[1] ^ 0x368d0bb4u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void tune_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sched[20] = c->hash ^ sx_rl(c->lane[2], 1);
    c->lane[11] += c->lane[3]; c->lane[1] ^= c->lane[11]; c->lane[1] = sx_rl(c->lane[1], 21);
    t0 ^= prime_index(c, t1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 12);
    c->lane[7] += c->lane[15]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 9);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xaebc8b75u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 9);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 2);
    t2 = (t2 ^ c->sum) * 0xb70f9201u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t merge_unit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[3] ^= sx_rl(c->lane[7], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x075c3dfbu;
    t1 ^= (uint32_t)close_record(c, (uint8_t)(t0 >> 8), t2);
    latch_offset(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfa57d79fu;
    c->lane[15] ^= sx_rl(c->lane[14], 23);
    c->lane[5] ^= sx_rl(c->lane[5], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x71) << 0;
    c->lane[5] += c->lane[6]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 15);
    c->hash = (c->hash * 0xe6a220c5u) ^ sx_rr(c->hash, 25);
    c->hash ^= c->lane[8] + 0xe3689541u;
    t2 = (t2 ^ c->sum) * 0xbc135e21u;
    c->hash = (c->hash * 0x4bb322c3u) ^ sx_rr(c->hash, 9);
    t2 += (uint32_t)push_head_53(c);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t stage_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[11] = c->hash ^ sx_rl(c->lane[12], 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t2 += pair_port(c, c->slo, c->sln);
    prime_table(c, t0, t1);
    c->hash ^= c->lane[1] + 0xcfb949dfu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x44afe50bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0a) << 0;
    t2 += (uint32_t)relay_layer(c);
    c->hash = (c->hash * 0x3c5147d7u) ^ sx_rr(c->hash, 21);
    c->raw[c->slo + (int)((t0 + 11994u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)mix_tail(c);
    c->raw[c->slo + (int)((t0 + 17510u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += (uint32_t)settle_field(c);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void rotate_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] ^= sx_rl(c->lane[13], 16);
    c->raw[c->slo + (int)((t0 + 56635u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[2] ^= sx_rl(c->lane[13], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15600u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t place_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54069u) % (uint32_t)c->rln)] << 8;
    c->lane[11] += c->lane[2] ^ 0xb4fb419du;
    fold_slot(c, &c->lane[0], 1);
    c->hash ^= c->lane[13] + 0x688d1127u;
    c->raw[c->slo + (int)((t0 + 59417u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= swap_count(c, t1);
    c->hash = (c->hash * 0x8c9d812du) ^ sx_rr(c->hash, 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[0] + 0x8c937adeu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 16465u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void trace_track(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18269u) % (uint32_t)c->rln)] << 24;
    c->lane[14] += c->lane[11] ^ 0x6939a44fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[4] += c->lane[3] ^ 0x4211b154u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xca26753du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x1d) << 8;
    c->lane[13] += c->lane[8]; c->lane[4] ^= c->lane[13]; c->lane[4] = sx_rl(c->lane[4], 24);
    c->raw[c->slo + (int)((t0 + 19229u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += stage_limit(c, c->slo, c->sln);
    c->hash ^= c->lane[10] + 0xc6ba64e6u;
    c->hash = (c->hash * 0x762a1c8bu) ^ sx_rr(c->hash, 20);
    c->hash ^= c->lane[3] + 0xd9faeca0u;
    emit_store(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint8_t coal_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[6] + 0x54d00eb8u;
    t1 ^= (uint32_t)tally_window(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7bc6fe15u;
    c->hash ^= c->lane[12] + 0x549d07eau;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x74) << 0;
    c->lane[15] ^= sx_rl(c->lane[3], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44174u) % (uint32_t)c->rln)] << 24;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 4);
    c->lane[10] += c->lane[5]; c->lane[4] ^= c->lane[10]; c->lane[4] = sx_rl(c->lane[4], 24);
    c->hash ^= c->lane[14] + 0x83c9339cu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int poll_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[4] += c->lane[4] ^ 0xa806ef4au;
    c->sched[6] = c->hash ^ sx_rl(c->lane[1], 15);
    c->lane[3] ^= sx_rl(c->lane[14], 15);
    c->lane[9] += c->lane[6] ^ 0xc7f1fa66u;
    t2 += split_delta(c, c->rlo, c->rln);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 25);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 26);
    t1 ^= (uint32_t)flush_arena(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)seek_offset(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[28] = c->hash ^ sx_rl(c->lane[14], 13);
    c->lane[1] += c->lane[13]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 18);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void merge_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xfabfadcfu) ^ sx_rr(c->hash, 21);
    c->hash ^= c->lane[13] + 0x5ef2a4b1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x09e3210du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[2] += c->lane[9] ^ 0xa68061b6u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x07) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[10] += c->lane[14] ^ 0xfe6873abu;
    c->sched[17] = c->hash ^ sx_rl(c->lane[9], 27);
    c->lane[15] ^= sx_rl(c->lane[6], 9);
    t1 ^= (uint32_t)sort_band(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t1 ^= (uint32_t)pick_level(c, (uint8_t)(t0 >> 8), t2);
    t2 += stage_segment(c, c->rlo, c->rln);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t store_stream(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += chain_token(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46148u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31238u) % (uint32_t)c->rln)] << 16;
    t2 += (uint32_t)reset_gap(c);
    c->lane[5] ^= sx_rl(c->lane[7], 18);
    c->hash ^= c->lane[9] + 0xdec73371u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    fold_chunk(c, t0, t1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t grow_label(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[14] + 0x3dbbaf5au;
    pin_label(c, &c->lane[3], 2);
    c->lane[15] += c->lane[12] ^ 0x2fc9dc6au;
    t2 += (uint32_t)peek_stream(c);
    c->lane[11] += c->lane[4] ^ 0x93e4ebb4u;
    c->raw[c->slo + (int)((t0 + 63623u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void peek_group(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 10853u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0xed56fd69u;
    c->lane[2] += c->lane[11] ^ 0x10ed807fu;
    t2 += (uint32_t)sift_head(c);
    c->hash = (c->hash * 0x0f75ba53u) ^ sx_rr(c->hash, 19);
    c->sched[5] = c->hash ^ sx_rl(c->lane[4], 8);
    c->raw[c->slo + (int)((t0 + 35330u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 31941u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += sift_queue(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x9667e923u) ^ sx_rr(c->hash, 23);
    c->hash = (c->hash * 0xbf68e5dfu) ^ sx_rr(c->hash, 25);
    t0 ^= emit_marker(c, t1);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t prime_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 18);
    t2 = (t2 ^ c->sum) * 0xad1e0fc3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49438u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x6c18ead9u;
    t2 += (uint32_t)reset_label(c);
    c->lane[5] += c->lane[2]; c->lane[3] ^= c->lane[5]; c->lane[3] = sx_rl(c->lane[3], 22);
    resize_frame(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8893u) % (uint32_t)c->rln)] << 16;
    t2 += (uint32_t)settle_layer(c);
    c->lane[13] ^= sx_rl(c->lane[3], 10);
    seek_node(c, &c->lane[2], 4);
    c->lane[3] ^= sx_rl(c->lane[15], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x33) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t tally_window(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 13);
    c->raw[c->slo + (int)((t0 + 17283u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49820u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0xfd566721u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    latch_tuple(c, t0, t1);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 63513u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x4a351807u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void emit_store(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x24) << 16;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 26);
    c->sched[11] = c->hash ^ sx_rl(c->lane[12], 30);
    t2 = (t2 ^ c->sum) * 0x8de0a675u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 15);
    t2 += (uint32_t)slice_bound(c);
    c->raw[c->slo + (int)((t0 + 45301u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[13] + 0xa90e4e53u;
    c->hash ^= c->lane[1] + 0xf88d0095u;
    t0 ^= swap_entry(c, t1);
    c->lane[0] += c->lane[1]; c->lane[14] ^= c->lane[0]; c->lane[14] = sx_rl(c->lane[14], 8);
    c->hash = (c->hash * 0x7cea5541u) ^ sx_rr(c->hash, 28);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static int drain_view(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    t2 = (t2 ^ c->sum) * 0x5b2e2117u;
    c->hash = (c->hash * 0xad012d23u) ^ sx_rr(c->hash, 19);
    c->lane[15] ^= sx_rl(c->lane[6], 4);
    t2 = (t2 ^ c->sum) * 0xdff552e9u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd7fbabf7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53961u) % (uint32_t)c->rln)] << 0;
    c->lane[6] += c->lane[1]; c->lane[10] ^= c->lane[6]; c->lane[10] = sx_rl(c->lane[10], 7);
    t1 ^= (uint32_t)fold_stack(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[14] + 0x00b236b8u;
    c->hash ^= c->lane[7] + 0xe4b67d74u;
    c->raw[c->slo + (int)((t0 + 11887u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int settle_frame(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56152u) % (uint32_t)c->rln)] << 24;
    c->lane[14] ^= sx_rl(c->lane[2], 30);
    c->lane[15] ^= sx_rl(c->lane[5], 15);
    c->lane[6] += c->lane[0] ^ 0x38b1a9a9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0x8d3f234bu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x57) << 16;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void prime_table(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 20177u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6b1504f5u;
    c->sched[10] = c->hash ^ sx_rl(c->lane[7], 31);
    c->lane[11] ^= sx_rl(c->lane[12], 4);
    close_mask(c, t0, t1);
    c->sched[13] = c->hash ^ sx_rl(c->lane[10], 18);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint32_t stage_segment(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa292a15fu;
    pin_label(c, &c->lane[7], 2);
    t2 += (uint32_t)merge_token(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58560u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x4834fc89u;
    t1 ^= (uint32_t)purge_ring(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)pack_frame(c);
    c->hash ^= c->lane[7] + 0xe41c127du;
    c->raw[c->slo + (int)((t0 + 29575u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] += c->lane[2] ^ 0x9d1c9d9fu;
    c->lane[15] ^= sx_rl(c->lane[10], 8);
    c->raw[c->slo + (int)((t0 + 55019u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += step_block(c, c->slo, c->sln);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 11);
    c->hash ^= c->lane[5] + 0x1894b919u;
    c->lane[13] ^= sx_rl(c->lane[4], 12);
    c->sum += t1;
    return t0 + t2;
}

static void fold_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] += c->lane[0]; c->lane[12] ^= c->lane[15]; c->lane[12] = sx_rl(c->lane[12], 7);
    t2 = (t2 ^ c->sum) * 0x804f995du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 55609u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[13] + 0xaa6a26e5u;
    c->lane[1] ^= sx_rl(c->lane[10], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf4) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 60301u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t swap_count(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    trace_seat(c, &c->lane[9], 3);
    sift_region(c, &c->lane[8], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37454u) % (uint32_t)c->rln)] << 0;
    wrap_tail(c, &c->lane[1], 4);
    c->lane[14] += c->lane[9]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[7] += c->lane[5] ^ 0xaa31bf53u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void link_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x528cde0bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x76) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[6] += c->lane[1] ^ 0x66a0d667u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void emit_field(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42046u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x4d82e60fu;
    t2 += relay_span(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 52954u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x73dcbb29u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t step_record(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[6] + 0x555860adu;
    c->hash = (c->hash * 0xa731581bu) ^ sx_rr(c->hash, 14);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xd35189abu;
    reset_store(c, &c->lane[11], 1);
    t2 += pack_window(c, c->slo, c->sln);
    t0 ^= swap_entry(c, t1);
    t2 = (t2 ^ c->sum) * 0xe94924cdu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x924412a1u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int mix_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    t2 += mix_index(c, c->rlo, c->rln);
    c->hash ^= c->lane[4] + 0xa16900f9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 = (t2 ^ c->sum) * 0x281a1e1fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x25040355u;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)flush_arena(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x30) << 8;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[7] + 0x2ddc3e29u;
    load_lease(c, t0, t1);
    c->hash ^= c->lane[11] + 0x93fa300fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t1 ^= (uint32_t)place_rate(c, (uint8_t)(t0 >> 16), t2);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 15);
    c->hash ^= c->lane[14] + 0x4829c897u;
    t1 ^= (uint32_t)prime_index_101(c, (uint8_t)(t0 >> 0), t2);
    c->lane[9] ^= sx_rl(c->lane[5], 20);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t stage_limit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 29);
    c->sched[2] = c->hash ^ sx_rl(c->lane[6], 19);
    t2 = (t2 ^ c->sum) * 0xdcb100e1u;
    c->hash = (c->hash * 0xb70e87abu) ^ sx_rr(c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= align_index(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t mark_chunk(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 63862u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43504u) % (uint32_t)c->rln)] << 0;
    c->lane[6] ^= sx_rl(c->lane[0], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1f) << 0;
    c->raw[c->slo + (int)((t0 + 18664u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xa1e07bdbu;
    c->sum += t1;
    return t0 + t2;
}

static int fetch_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] += c->lane[2]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 28);
    c->lane[10] += c->lane[14]; c->lane[12] ^= c->lane[10]; c->lane[12] = sx_rl(c->lane[12], 18);
    c->sched[13] = c->hash ^ sx_rl(c->lane[13], 25);
    t1 ^= (uint32_t)coal_span(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[15] + 0xc270bf3cu;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int push_head_53(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t2 += (uint32_t)probe_span(c);
    c->sched[25] = c->hash ^ sx_rl(c->lane[3], 11);
    c->raw[c->slo + (int)((t0 + 55041u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 38995u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[2] += c->lane[8]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 27);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t seek_head(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 2970u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0xe86a5901u;
    c->lane[0] += c->lane[13]; c->lane[12] ^= c->lane[0]; c->lane[12] = sx_rl(c->lane[12], 9);
    probe_segment(c, &c->lane[6], 4);
    t0 ^= yield_chunk(c, t1);
    t2 += (uint32_t)scan_value(c);
    store_stream_113(c, &c->lane[7], 2);
    t0 ^= merge_path(c, t1);
    t2 += settle_frame_94(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27338u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0xa9402f85u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t split_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xac) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2a4878ddu;
    t0 ^= store_state(c, t1);
    c->raw[c->slo + (int)((t0 + 42375u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4150u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x7e169869u) ^ sx_rr(c->hash, 26);
    t2 += (uint32_t)sift_digest(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x890ae343u;
    c->hash ^= c->lane[15] + 0xfe885cceu;
    c->raw[c->slo + (int)((t0 + 11746u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t pair_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    peek_line(c, &c->lane[2], 2);
    c->lane[5] += c->lane[9]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8ee379d7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 8916u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x12) << 0;
    c->hash = (c->hash * 0xd1ab2337u) ^ sx_rr(c->hash, 30);
    t2 = (t2 ^ c->sum) * 0x6f059d13u;
    c->sum += t1;
    return t0 + t2;
}

static int relay_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash = (c->hash * 0x5c85dfc7u) ^ sx_rr(c->hash, 1);
    c->lane[6] += c->lane[4]; c->lane[14] ^= c->lane[6]; c->lane[14] = sx_rl(c->lane[14], 2);
    t2 += (uint32_t)fold_view(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sched[4] = c->hash ^ sx_rl(c->lane[15], 27);
    t2 += (uint32_t)reset_marker(c);
    c->raw[c->slo + (int)((t0 + 25331u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[2] + 0x14170fe2u;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void latch_offset(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5394u) % (uint32_t)c->rln)] << 24;
    c->lane[1] += c->lane[12]; c->lane[14] ^= c->lane[1]; c->lane[14] = sx_rl(c->lane[14], 17);
    c->sched[20] = c->hash ^ sx_rl(c->lane[5], 31);
    t0 ^= tap_part(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x62091aa3u;
    c->sched[6] = c->hash ^ sx_rl(c->lane[4], 28);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t queue_unit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[14] ^= sx_rl(c->lane[1], 3);
    t2 = (t2 ^ c->sum) * 0xdcda7bb7u;
    t2 = (t2 ^ c->sum) * 0xa7bdc1fbu;
    c->hash ^= c->lane[5] + 0xca38ba78u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += (uint32_t)patch_segment(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe1) << 0;
    c->hash = (c->hash * 0xc464975du) ^ sx_rr(c->hash, 4);
    c->hash = (c->hash * 0x5e5718e1u) ^ sx_rr(c->hash, 18);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int settle_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd7) << 16;
    c->hash ^= c->lane[13] + 0x0b0adc3cu;
    c->hash ^= c->lane[2] + 0xdec5f7a1u;
    c->hash ^= c->lane[4] + 0x1006959du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x00f2604bu;
    t2 += tally_group(c, c->slo, c->sln);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 28);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    c->sched[15] = c->hash ^ sx_rl(c->lane[14], 29);
    c->lane[3] += c->lane[12]; c->lane[2] ^= c->lane[3]; c->lane[2] = sx_rl(c->lane[2], 2);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void map_unit(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x0dc174b7u) ^ sx_rr(c->hash, 21);
    t1 ^= (uint32_t)push_track(c, (uint8_t)(t0 >> 8), t2);
    c->raw[c->slo + (int)((t0 + 19052u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0xf7af5927u;
    t2 = (t2 ^ c->sum) * 0x403820f5u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37577u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x56) << 16;
    c->hash = (c->hash * 0xabe0cf21u) ^ sx_rr(c->hash, 18);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26669u) % (uint32_t)c->rln)] << 24;
    c->sched[1] = c->hash ^ sx_rl(c->lane[2], 18);
    c->hash ^= c->lane[2] + 0x17d424dau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42155u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void move_frame(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] += c->lane[8] ^ 0x858bc3f0u;
    c->lane[8] += c->lane[13]; c->lane[10] ^= c->lane[8]; c->lane[10] = sx_rl(c->lane[10], 8);
    t2 += (uint32_t)fill_seat(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    pair_part(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33471u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 15203u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x29) << 8;
    t2 = (t2 ^ c->sum) * 0x82942a85u;
    seek_node(c, &c->lane[11], 3);
    c->hash ^= c->lane[8] + 0x73d7a024u;
    t0 ^= pack_band(c, t1);
    c->raw[c->slo + (int)((t0 + 37957u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= yield_chunk(c, t1);
    t0 ^= align_index(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t close_record(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)reset_marker(c);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x931827afu;
    c->hash ^= c->lane[0] + 0xc78fc9c3u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    c->hash ^= c->lane[6] + 0x3f320155u;
    resize_ring(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t sort_band(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x0d8018c3u) ^ sx_rr(c->hash, 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x632937c3u) ^ sx_rr(c->hash, 3);
    t2 = (t2 ^ c->sum) * 0x8eddad63u;
    c->lane[6] ^= sx_rl(c->lane[15], 13);
    c->sched[30] = c->hash ^ sx_rl(c->lane[7], 7);
    c->hash = (c->hash * 0x1420c4efu) ^ sx_rr(c->hash, 25);
    t2 = (t2 ^ c->sum) * 0xd117e1dbu;
    c->raw[c->slo + (int)((t0 + 30347u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 18);
    t2 += (uint32_t)latch_record(c);
    chain_table(c, &c->lane[3], 2);
    c->lane[3] ^= sx_rl(c->lane[12], 23);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int reset_gap(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38268u) % (uint32_t)c->rln)] << 0;
    emit_layer(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56733u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0xef941e9bu;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    emit_batch(c, &c->lane[4], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3273b3f7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd0) << 16;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void pick_cell(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    mix_layer(c, &c->lane[8], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48262u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 20234u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[16] = c->hash ^ sx_rl(c->lane[9], 23);
    c->hash ^= c->lane[13] + 0x8ac7af79u;
    c->sched[11] = c->hash ^ sx_rl(c->lane[3], 28);
    c->lane[13] += c->lane[4] ^ 0xedaee8ccu;
    t2 += resize_chunk(c, c->slo, c->sln);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t seek_offset(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 29);
    wrap_group(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x750ec87fu;
    t2 += chain_token(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t pick_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 7);
    c->lane[9] += c->lane[1] ^ 0x28d2017au;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44969u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x53) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[7] += c->lane[2] ^ 0xb1b75125u;
    c->lane[2] += c->lane[4]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 6);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int sift_head(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    t2 = (t2 ^ c->sum) * 0x45cd64afu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60885u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0xb8cee16du) ^ sx_rr(c->hash, 2);
    c->raw[c->slo + (int)((t0 + 2125u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 4252u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[15] = c->hash ^ sx_rl(c->lane[1], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb9326eedu;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int reset_marker(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash = (c->hash * 0x3c1f906du) ^ sx_rr(c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= c->lane[6] + 0x4c59bc46u;
    c->raw[c->slo + (int)((t0 + 64155u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 46688u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[0] += c->lane[15]; c->lane[4] ^= c->lane[0]; c->lane[4] = sx_rl(c->lane[4], 10);
    c->hash = (c->hash * 0x437cd2c9u) ^ sx_rr(c->hash, 23);
    c->lane[13] += c->lane[14] ^ 0xf1364aa1u;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int scan_value(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[15] += c->lane[14] ^ 0xe0b6d77cu;
    c->raw[c->slo + (int)((t0 + 13560u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34605u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 23225u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7bc8b945u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 35192u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x94ed5185u;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void probe_segment(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xeb178319u;
    c->lane[2] += c->lane[0]; c->lane[4] ^= c->lane[2]; c->lane[4] = sx_rl(c->lane[4], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x62) << 0;
    c->lane[13] += c->lane[7] ^ 0x9de92704u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void seek_node(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] ^= sx_rl(c->lane[2], 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25489u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xd9dabfbfu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void resize_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] += c->lane[6]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] ^= sx_rl(c->lane[14], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 726u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45352u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void wrap_tail(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5ff1f281u;
    c->raw[c->slo + (int)((t0 + 13350u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[0] + 0xd4a37915u;
    c->lane[2] += c->lane[12]; c->lane[4] ^= c->lane[2]; c->lane[4] = sx_rl(c->lane[4], 3);
    c->raw[c->slo + (int)((t0 + 16797u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[8] += c->lane[12] ^ 0xa23c0f7fu;
    c->raw[c->slo + (int)((t0 + 60394u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[2] = c->hash ^ sx_rl(c->lane[5], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xea) << 0;
    c->lane[6] ^= sx_rl(c->lane[14], 25);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void mix_layer(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf6a4fe51u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 24);
    c->lane[0] += c->lane[6]; c->lane[13] ^= c->lane[0]; c->lane[13] = sx_rl(c->lane[13], 11);
    c->sched[30] = c->hash ^ sx_rl(c->lane[12], 7);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int probe_span(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->lane[5] += c->lane[8]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 10);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 24);
    c->lane[1] ^= sx_rl(c->lane[13], 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16862u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdb) << 0;
    c->hash ^= c->lane[1] + 0x688996a4u;
    c->sched[31] = c->hash ^ sx_rl(c->lane[7], 21);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t store_state(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x3af4df3fu) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xa5) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0xf8e0b645u) ^ sx_rr(c->hash, 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59980u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void wrap_group(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x34b075f7u) ^ sx_rr(c->hash, 18);
    c->hash = (c->hash * 0x8ca4f82fu) ^ sx_rr(c->hash, 9);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    c->raw[c->slo + (int)((t0 + 28160u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[13] + 0xdf56907du;
    t2 = (t2 ^ c->sum) * 0x25946101u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 27);
    c->lane[4] += c->lane[10]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 21);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t tally_group(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 25538u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[4] + 0xfea9040bu;
    c->raw[c->slo + (int)((t0 + 35667u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[20] = c->hash ^ sx_rl(c->lane[1], 18);
    c->lane[7] += c->lane[12]; c->lane[14] ^= c->lane[7]; c->lane[14] = sx_rl(c->lane[14], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe20343f9u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t push_track(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->hash ^= c->lane[12] + 0x0e137d2fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)grow_stream(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9a) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 28);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t step_block(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[10] ^= sx_rl(c->lane[14], 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3a) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 59939u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0x096d5607u;
    c->lane[5] += c->lane[0]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 4);
    c->raw[c->slo + (int)((t0 + 46992u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb2) << 0;
    c->lane[2] += c->lane[12]; c->lane[4] ^= c->lane[2]; c->lane[4] = sx_rl(c->lane[4], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum += t1;
    return t0 + t2;
}

static int peek_stream(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t2 = (t2 ^ c->sum) * 0xc091adfbu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] ^= sx_rl(c->lane[12], 20);
    c->lane[14] ^= sx_rl(c->lane[6], 5);
    c->hash = (c->hash * 0x1f2e7b2du) ^ sx_rr(c->hash, 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void peek_line(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4e970a0bu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x0792bc89u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[0] += c->lane[7]; c->lane[4] ^= c->lane[0]; c->lane[4] = sx_rl(c->lane[4], 12);
    c->hash ^= c->lane[7] + 0xbd08617fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbe32dea7u;
    c->hash = (c->hash * 0xa8ba0837u) ^ sx_rr(c->hash, 18);
    c->lane[15] += c->lane[8]; c->lane[9] ^= c->lane[15]; c->lane[9] = sx_rl(c->lane[9], 29);
    c->raw[c->slo + (int)((t0 + 43156u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int merge_token(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1504821du;
    c->sched[1] = c->hash ^ sx_rl(c->lane[7], 3);
    c->lane[10] += c->lane[7] ^ 0x587105b3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65051u) % (uint32_t)c->rln)] << 16;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t fold_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] += c->lane[15]; c->lane[6] ^= c->lane[12]; c->lane[6] = sx_rl(c->lane[6], 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5d) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x050f60bbu;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 9);
    c->hash = (c->hash * 0x24f137d3u) ^ sx_rr(c->hash, 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash = (c->hash * 0xb7e4ec63u) ^ sx_rr(c->hash, 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2411u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[14] + 0x6f887889u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t sift_queue(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x021139f5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[9] += c->lane[13]; c->lane[12] ^= c->lane[9]; c->lane[12] = sx_rl(c->lane[12], 22);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t emit_marker(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xf0c5fa57u;
    c->lane[9] += c->lane[12] ^ 0xd1ca3da2u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x22) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x69) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15192u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[22] = c->hash ^ sx_rl(c->lane[10], 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void pair_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] += c->lane[4]; c->lane[13] ^= c->lane[6]; c->lane[13] = sx_rl(c->lane[13], 17);
    c->sched[0] = c->hash ^ sx_rl(c->lane[0], 17);
    c->hash ^= c->lane[15] + 0x11cdfbb9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58618u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 4036u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 27);
    c->sched[14] = c->hash ^ sx_rl(c->lane[5], 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc6) << 0;
    c->lane[4] += c->lane[4] ^ 0x9eb849a2u;
    c->sched[17] = c->hash ^ sx_rl(c->lane[2], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static int patch_segment(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->hash ^= c->lane[5] + 0xc38129e3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x3c) << 16;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 19);
    c->hash ^= c->lane[3] + 0xf2e4a806u;
    c->hash = (c->hash * 0xc4ed8357u) ^ sx_rr(c->hash, 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16662u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 65176u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 33947u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x450daac7u;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t chain_token(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39967u) % (uint32_t)c->rln)] << 24;
    c->lane[9] += c->lane[11] ^ 0xd6943122u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 14);
    c->lane[14] ^= sx_rl(c->lane[3], 30);
    c->sum += t1;
    return t0 + t2;
}

static void close_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x4f34f391u) ^ sx_rr(c->hash, 5);
    c->hash = (c->hash * 0xe4116097u) ^ sx_rr(c->hash, 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46460u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6bb6b3f3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11473u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x588e85c7u) ^ sx_rr(c->hash, 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[4] += c->lane[7]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa8d50525u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe7) << 16;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t tap_part(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] += c->lane[0]; c->lane[2] ^= c->lane[11]; c->lane[2] = sx_rl(c->lane[2], 7);
    c->lane[15] += c->lane[11] ^ 0xab1f99feu;
    c->hash ^= c->lane[5] + 0x76a90effu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21913u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[12] ^= sx_rl(c->lane[7], 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t settle_frame_94(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4839391du;
    c->raw[c->slo + (int)((t0 + 38556u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xf6e4236bu;
    c->lane[13] ^= sx_rl(c->lane[8], 15);
    c->sum += t1;
    return t0 + t2;
}

static void resize_frame(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[12] = c->hash ^ sx_rl(c->lane[7], 10);
    c->lane[3] += c->lane[12]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 9);
    c->hash ^= c->lane[9] + 0xd19ec6b3u;
    c->hash = (c->hash * 0x0b757939u) ^ sx_rr(c->hash, 1);
    c->lane[4] += c->lane[1]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 29);
    c->hash ^= c->lane[4] + 0xba99305du;
    c->hash ^= c->lane[0] + 0x686939fau;
    c->lane[8] += c->lane[3]; c->lane[12] ^= c->lane[8]; c->lane[12] = sx_rl(c->lane[12], 15);
    c->sched[15] = c->hash ^ sx_rl(c->lane[3], 14);
    c->hash = (c->hash * 0x748e46a1u) ^ sx_rr(c->hash, 22);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static void pin_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x6409960bu;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 21);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 8);
    t2 = (t2 ^ c->sum) * 0x9772b2c5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[3] += c->lane[12]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 12);
    c->hash = (c->hash * 0x4b944197u) ^ sx_rr(c->hash, 11);
    c->hash = (c->hash * 0xc242c805u) ^ sx_rr(c->hash, 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] += c->lane[10]; c->lane[4] ^= c->lane[3]; c->lane[4] = sx_rl(c->lane[4], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void sift_region(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x7276a17bu) ^ sx_rr(c->hash, 26);
    c->hash = (c->hash * 0x2b1281afu) ^ sx_rr(c->hash, 23);
    c->lane[14] ^= sx_rl(c->lane[9], 22);
    c->lane[10] += c->lane[8] ^ 0x30be45beu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5e) << 0;
    c->lane[6] ^= sx_rl(c->lane[1], 29);
    c->lane[1] += c->lane[8]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 12);
    t2 = (t2 ^ c->sum) * 0x933629b5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t purge_ring(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[14] += c->lane[4]; c->lane[12] ^= c->lane[14]; c->lane[12] = sx_rl(c->lane[12], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[12] + 0xd1f824efu;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 12);
    c->lane[1] += c->lane[4]; c->lane[14] ^= c->lane[1]; c->lane[14] = sx_rl(c->lane[14], 29);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t align_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdbda28a5u;
    c->raw[c->slo + (int)((t0 + 15335u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[14] += c->lane[3] ^ 0xb22cb5b7u;
    c->hash = (c->hash * 0xe178c715u) ^ sx_rr(c->hash, 30);
    c->lane[13] += c->lane[6] ^ 0x758058a3u;
    c->lane[11] += c->lane[10]; c->lane[4] ^= c->lane[11]; c->lane[4] = sx_rl(c->lane[4], 25);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void trace_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xaa) << 16;
    t2 = (t2 ^ c->sum) * 0x21ba8cf5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[11] += c->lane[10]; c->lane[1] ^= c->lane[11]; c->lane[1] = sx_rl(c->lane[1], 6);
    t2 = (t2 ^ c->sum) * 0x96aa84ebu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 1);
    c->lane[14] += c->lane[12]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5a) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x0a967eb7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x08272be9u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t prime_index_101(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[11] + 0x3b9f6d37u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x11222eb1u;
    c->lane[13] ^= sx_rl(c->lane[5], 9);
    c->lane[10] ^= sx_rl(c->lane[9], 19);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xaf695a0bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x62875f5fu;
    t2 = (t2 ^ c->sum) * 0x0ea8fa51u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6e) << 8;
    c->hash ^= c->lane[1] + 0x3629830eu;
    c->lane[8] += c->lane[5] ^ 0x3d951b55u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t relay_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[13] += c->lane[15] ^ 0x118b6d85u;
    c->raw[c->slo + (int)((t0 + 42344u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[1] ^= sx_rl(c->lane[13], 28);
    c->lane[14] += c->lane[2] ^ 0x6c1ae71fu;
    t2 = (t2 ^ c->sum) * 0x8d1bdf15u;
    c->lane[1] += c->lane[7] ^ 0x769781a1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x8c) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5400f16fu;
    c->lane[3] += c->lane[11]; c->lane[12] ^= c->lane[3]; c->lane[12] = sx_rl(c->lane[12], 24);
    c->hash ^= c->lane[13] + 0x04e2affcu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf2290705u;
    c->sum += t1;
    return t0 + t2;
}

static int fold_view(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3ad021f3u;
    c->hash = (c->hash * 0xe803c5ffu) ^ sx_rr(c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf4bcfb03u;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t place_rate(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37590u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf9010951u;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 29);
    c->sched[3] = c->hash ^ sx_rl(c->lane[9], 9);
    fold_bucket(c, &c->lane[8], 1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void load_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 31039u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 6211u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 9);
    c->hash ^= c->lane[5] + 0xf2369ea0u;
    t2 = (t2 ^ c->sum) * 0xa19e9fa1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x2fbcbd7du) ^ sx_rr(c->hash, 3);
    c->lane[14] += c->lane[8]; c->lane[5] ^= c->lane[14]; c->lane[5] = sx_rl(c->lane[5], 2);
    c->hash ^= c->lane[13] + 0x188cfe92u;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t flush_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x48) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd1) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x981089bfu;
    c->raw[c->slo + (int)((t0 + 617u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x79) << 0;
    c->lane[10] ^= sx_rl(c->lane[11], 25);
    c->lane[3] += c->lane[5] ^ 0x92805892u;
    c->raw[c->slo + (int)((t0 + 40644u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61616u) % (uint32_t)c->rln)] << 8;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 31);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int pack_frame(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->hash ^= c->lane[15] + 0xab41061fu;
    c->hash = (c->hash * 0xa273e069u) ^ sx_rr(c->hash, 31);
    t2 = (t2 ^ c->sum) * 0xa9904977u;
    c->hash = (c->hash * 0xa34a3b31u) ^ sx_rr(c->hash, 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x37) << 0;
    t2 = (t2 ^ c->sum) * 0xe1670a09u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= c->lane[2] + 0x8911a47au;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void chain_table(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xa05a7ac7u) ^ sx_rr(c->hash, 28);
    t2 = (t2 ^ c->sum) * 0x0171979du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[7] += c->lane[0]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 13);
    c->lane[5] += c->lane[14] ^ 0x257ea3a8u;
    t2 = (t2 ^ c->sum) * 0x2864e319u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x33) << 0;
    c->raw[c->slo + (int)((t0 + 1129u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int slice_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->hash ^= c->lane[8] + 0x05961699u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62784u) % (uint32_t)c->rln)] << 8;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 26);
    c->sched[27] = c->hash ^ sx_rl(c->lane[9], 19);
    c->sched[8] = c->hash ^ sx_rl(c->lane[7], 12);
    c->raw[c->slo + (int)((t0 + 29773u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 63906u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int fill_seat(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    t2 = (t2 ^ c->sum) * 0x72b0ad57u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 2);
    c->hash = (c->hash * 0xcdd479fbu) ^ sx_rr(c->hash, 11);
    c->raw[c->slo + (int)((t0 + 56639u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[13] += c->lane[11] ^ 0xee28bc3au;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xad) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10388u) % (uint32_t)c->rln)] << 24;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t merge_path(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x36) << 16;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xee) << 16;
    c->hash = (c->hash * 0x391aeb4bu) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xfa) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t yield_chunk(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[7] += c->lane[11]; c->lane[6] ^= c->lane[7]; c->lane[6] = sx_rl(c->lane[6], 4);
    c->lane[1] += c->lane[11]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 11);
    t2 = (t2 ^ c->sum) * 0x7176fdddu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57003u) % (uint32_t)c->rln)] << 8;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7595u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x23b8b977u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 30);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void store_stream_113(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[2] += c->lane[13] ^ 0x7f3e3f7cu;
    c->lane[6] += c->lane[3] ^ 0xaf8f8282u;
    c->sched[1] = c->hash ^ sx_rl(c->lane[15], 31);
    c->sched[31] = c->hash ^ sx_rl(c->lane[12], 17);
    t2 = (t2 ^ c->sum) * 0x22ba14a1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 40141u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xb6f74233u) ^ sx_rr(c->hash, 21);
    c->sched[20] = c->hash ^ sx_rl(c->lane[6], 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sched[26] = c->hash ^ sx_rl(c->lane[9], 19);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t swap_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[3] + 0x8448774eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe6) << 8;
    c->lane[0] += c->lane[7] ^ 0x94068cd7u;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    c->raw[c->slo + (int)((t0 + 47170u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x05) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pack_band(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[15] ^= sx_rl(c->lane[14], 15);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6518u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x06) << 0;
    c->sched[5] = c->hash ^ sx_rl(c->lane[12], 19);
    c->raw[c->slo + (int)((t0 + 60290u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[7] + 0x35d6210bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int sift_digest(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    t2 = (t2 ^ c->sum) * 0x994b2321u;
    c->sched[16] = c->hash ^ sx_rl(c->lane[13], 22);
    c->hash ^= c->lane[1] + 0xe3f28bbeu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x90e0da83u;
    c->hash ^= c->lane[14] + 0xb47c9cc6u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbd5cf0edu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5397abdbu;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 30);
    c->lane[2] ^= sx_rl(c->lane[13], 28);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mix_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x12fe622fu) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[15] += c->lane[14] ^ 0x0ceb2ef7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1353ab37u;
    t2 = (t2 ^ c->sum) * 0xa3a12b6du;
    t2 = (t2 ^ c->sum) * 0x888f0c5du;
    c->sum += t1;
    return t0 + t2;
}

static void emit_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55330u) % (uint32_t)c->rln)] << 0;
    c->lane[1] ^= sx_rl(c->lane[5], 11);
    c->hash = (c->hash * 0x6dc6025fu) ^ sx_rr(c->hash, 14);
    c->raw[c->slo + (int)((t0 + 57296u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 28);
    c->sched[28] = c->hash ^ sx_rl(c->lane[8], 3);
    c->hash ^= c->lane[2] + 0xb79ef748u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void reset_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[9] + 0x7db20877u;
    c->lane[10] += c->lane[9] ^ 0xec861994u;
    c->lane[12] ^= sx_rl(c->lane[3], 11);
    c->hash = (c->hash * 0xed001db3u) ^ sx_rr(c->hash, 6);
    t2 = (t2 ^ c->sum) * 0x8fab6649u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void emit_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    move_value(c, &c->lane[6], 3);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26024u) % (uint32_t)c->rln)] << 0;
    c->lane[8] += c->lane[14]; c->lane[4] ^= c->lane[8]; c->lane[4] = sx_rl(c->lane[4], 8);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 25);
    c->raw[c->slo + (int)((t0 + 19250u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf5) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t resize_chunk(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x39325779u) ^ sx_rr(c->hash, 4);
    c->lane[4] += c->lane[0] ^ 0xb6eb3ed3u;
    c->lane[1] ^= sx_rl(c->lane[14], 10);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 4);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 28);
    c->lane[4] += c->lane[13] ^ 0x1269b1f0u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 19);
    c->hash = (c->hash * 0x4d587457u) ^ sx_rr(c->hash, 17);
    c->sum += t1;
    return t0 + t2;
}

static int settle_layer(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash = (c->hash * 0x3e022a35u) ^ sx_rr(c->hash, 28);
    c->lane[12] += c->lane[7] ^ 0x3cc365bcu;
    c->raw[c->slo + (int)((t0 + 59411u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x78) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x20) << 16;
    c->hash = (c->hash * 0x3cf729a3u) ^ sx_rr(c->hash, 23);
    c->lane[9] ^= sx_rl(c->lane[4], 9);
    c->hash ^= c->lane[3] + 0xa82a6772u;
    t2 = (t2 ^ c->sum) * 0x5045f327u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int latch_record(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->lane[3] += c->lane[11]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19236u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8e) << 16;
    c->lane[2] += c->lane[1] ^ 0x09d26e7du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63578u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 63900u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x6f9808edu) ^ sx_rr(c->hash, 31);
    c->lane[3] ^= sx_rl(c->lane[13], 15);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t coal_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xa8cebba3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x02471b47u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x3b99ededu;
    c->hash = (c->hash * 0x407d0bc3u) ^ sx_rr(c->hash, 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x918fb1e1u) ^ sx_rr(c->hash, 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41334u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2013u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void latch_tuple(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 21851u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 2);
    c->lane[5] ^= sx_rl(c->lane[4], 22);
    c->sched[21] = c->hash ^ sx_rl(c->lane[2], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xba) << 8;
    c->lane[3] ^= sx_rl(c->lane[10], 28);
    c->lane[5] += c->lane[10] ^ 0xcd36b2c0u;
    c->lane[11] += c->lane[9]; c->lane[1] ^= c->lane[11]; c->lane[1] = sx_rl(c->lane[1], 13);
    c->lane[8] += c->lane[4] ^ 0x7a0a4a90u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static int reset_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->lane[12] ^= sx_rl(c->lane[1], 22);
    c->raw[c->slo + (int)((t0 + 43637u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32164u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x62) << 8;
    c->lane[10] ^= sx_rl(c->lane[0], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe31097c9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd2) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56269u) % (uint32_t)c->rln)] << 8;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t pack_window(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] ^= sx_rl(c->lane[9], 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x14f6974du;
    t2 = (t2 ^ c->sum) * 0x171d6ac3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x67) << 0;
    c->lane[8] += c->lane[2] ^ 0xd75bbad9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe8) << 8;
    c->sum += t1;
    return t0 + t2;
}

static void peek_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x743efcc1u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += (uint32_t)sync_index(c);
    t2 += sift_level(c, c->rlo, c->rln);
    map_gap(c, t0, t1);
    t1 ^= (uint32_t)seek_span(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x8ffdea5bu) ^ sx_rr(c->hash, 11);
    t2 += peek_record(c, c->slo, c->sln);
    patch_delta(c, &c->lane[7], 2);
    t2 += fold_label(c, c->slo, c->sln);
    t2 += (uint32_t)prime_stream(c);
    patch_item(c, t0, t1);
    c->lane[5] ^= sx_rl(c->lane[5], 21);
    t1 ^= (uint32_t)hold_queue(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)rotate_slot(c);
    t1 ^= (uint32_t)probe_index(c, (uint8_t)(t0 >> 16), t2);
    t1 ^= (uint32_t)blend_batch(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)defer_part_176(c, (uint8_t)(t0 >> 8), t2);
    link_bucket(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= swap_pool(c, t1);
    t1 ^= (uint32_t)fetch_line(c, (uint8_t)(t0 >> 16), t2);
    c->lane[3] += c->lane[6] ^ 0xb97f037eu;
    c->hash = (c->hash * 0xd9d586adu) ^ sx_rr(c->hash, 6);
    t2 += (uint32_t)purge_pool(c);
    t1 ^= (uint32_t)align_item(c, (uint8_t)(t0 >> 16), t2);
    trim_block(c, &c->lane[8], 1);
    poll_rate(c, &c->lane[5], 1);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 23);
    c->lane[0] += c->lane[9] ^ 0x7d732d99u;
    t1 ^= (uint32_t)place_rate_251(c, (uint8_t)(t0 >> 8), t2);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void trim_block(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    store_scope(c, t0, t1);
    t2 += (uint32_t)align_count(c);
    t2 += (uint32_t)emit_marker_199(c);
    c->lane[8] += c->lane[0]; c->lane[1] ^= c->lane[8]; c->lane[1] = sx_rl(c->lane[1], 11);
    pair_field(c, &c->lane[7], 1);
    fold_mask(c, &c->lane[4], 1);
    t2 += (uint32_t)merge_port(c);
    fetch_window(c, &c->lane[3], 1);
    c->hash ^= c->lane[10] + 0x401cb534u;
    t2 += (uint32_t)close_window(c);
    c->lane[15] += c->lane[10] ^ 0xff609866u;
    c->hash ^= c->lane[8] + 0x686661e3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1675u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 14217u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5358u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int prime_stream(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->hash ^= c->lane[15] + 0xf962a558u;
    hold_bound(c, &c->lane[8], 2);
    coal_limit(c, &c->lane[1], 3);
    t2 += (uint32_t)trim_queue(c);
    t2 += drain_region(c, c->slo, c->sln);
    t2 += (uint32_t)coal_offset(c);
    t2 += parse_run(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4618u) % (uint32_t)c->rln)] << 24;
    pair_ring(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x640eac03u;
    coal_port(c, t0, t1);
    c->sched[7] = c->hash ^ sx_rl(c->lane[15], 7);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t align_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= trim_ring(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += (uint32_t)fold_seat(c);
    load_lease_134(c, &c->lane[4], 2);
    t0 ^= step_entry(c, t1);
    c->sched[24] = c->hash ^ sx_rl(c->lane[10], 14);
    t1 ^= (uint32_t)relay_gap(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3d) << 0;
    c->lane[3] += c->lane[0] ^ 0x1f36ef4bu;
    t2 += (uint32_t)sort_region(c);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 1);
    t2 += close_pool(c, c->rlo, c->rln);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 4);
    t2 = (t2 ^ c->sum) * 0x5f492befu;
    t2 += (uint32_t)queue_view(c);
    c->hash = (c->hash * 0x6c10ef83u) ^ sx_rr(c->hash, 15);
    c->lane[5] += c->lane[1]; c->lane[2] ^= c->lane[5]; c->lane[2] = sx_rl(c->lane[2], 16);
    t2 += (uint32_t)sift_chunk(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void store_scope(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += resize_table(c, c->slo, c->sln);
    c->sched[7] = c->hash ^ sx_rl(c->lane[3], 2);
    c->hash = (c->hash * 0xefb56637u) ^ sx_rr(c->hash, 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xac283cbfu;
    c->raw[c->slo + (int)((t0 + 11865u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 54633u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    map_gap(c, t0, t1);
    c->lane[6] += c->lane[4] ^ 0xdcb6958eu;
    patch_delta(c, &c->lane[9], 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe3) << 0;
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t parse_run(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x97d7ae6fu;
    link_label_149(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 62380u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xdd3cd105u;
    t1 ^= (uint32_t)coal_line(c, (uint8_t)(t0 >> 0), t2);
    c->lane[7] ^= sx_rl(c->lane[12], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x88) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)join_stack(c, (uint8_t)(t0 >> 16), t2);
    c->sched[6] = c->hash ^ sx_rl(c->lane[1], 13);
    c->sum += t1;
    return t0 + t2;
}

static void load_lease_134(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += yield_bound(c, c->slo, c->sln);
    mix_page(c, t0, t1);
    t2 += fold_label(c, c->rlo, c->rln);
    link_label_149(c, t0, t1);
    t0 ^= swap_pool(c, t1);
    t2 += (uint32_t)emit_lease(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0xc7c33e61u) ^ sx_rr(c->hash, 30);
    pair_field(c, &c->lane[9], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash = (c->hash * 0x50f1054du) ^ sx_rr(c->hash, 10);
    t2 += prime_node(c, c->rlo, c->rln);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int queue_view(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa7) << 16;
    c->hash ^= c->lane[15] + 0xf2a34a3du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x06) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 28);
    fold_mask(c, &c->lane[10], 3);
    t2 = (t2 ^ c->sum) * 0xd6808ce7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58596u) % (uint32_t)c->rln)] << 0;
    trim_lease(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xd38457c7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2ccfaf9fu;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int align_count(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x52128b4fu) ^ sx_rr(c->hash, 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 8048u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 33841u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += (uint32_t)merge_port(c);
    t2 = (t2 ^ c->sum) * 0xb0d6ca91u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[0], 1);
    t1 ^= (uint32_t)probe_index(c, (uint8_t)(t0 >> 16), t2);
    c->lane[2] += c->lane[10]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 27);
    t2 = (t2 ^ c->sum) * 0x9b828b8bu;
    t2 += (uint32_t)tally_arena(c);
    t1 ^= (uint32_t)join_stack(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x57) << 0;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int close_window(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    patch_item(c, t0, t1);
    c->hash ^= c->lane[13] + 0x632782c5u;
    t1 ^= (uint32_t)map_band(c, (uint8_t)(t0 >> 8), t2);
    c->hash = (c->hash * 0x74f77de5u) ^ sx_rr(c->hash, 3);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 13);
    c->hash = (c->hash * 0x83bcc5a7u) ^ sx_rr(c->hash, 30);
    c->raw[c->slo + (int)((t0 + 49148u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[13] += c->lane[3]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void hold_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t1 ^= (uint32_t)seek_span(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51173u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0x90d554e7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x83) << 8;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 8);
    t2 += (uint32_t)fold_range(c);
    c->hash ^= c->lane[9] + 0xbf64bffau;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa3) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void coal_port(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[11] += c->lane[3] ^ 0x105061adu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)relay_field(c, (uint8_t)(t0 >> 8), t2);
    c->lane[12] += c->lane[7]; c->lane[13] ^= c->lane[12]; c->lane[13] = sx_rl(c->lane[13], 14);
    c->hash = (c->hash * 0x881b5e43u) ^ sx_rr(c->hash, 24);
    yield_pairing(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x08819e6fu;
    t2 += yield_bound(c, c->slo, c->sln);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static int trim_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 11);
    c->sched[8] = c->hash ^ sx_rl(c->lane[15], 14);
    t2 += slice_level_226(c, c->slo, c->sln);
    c->hash ^= c->lane[5] + 0x7337109cu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63616u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37358u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1e) << 8;
    t0 ^= stage_block(c, t1);
    wrap_batch(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34613u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x43) << 8;
    t2 += fetch_page(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t trim_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    slice_level(c, &c->lane[7], 1);
    poll_ring(c, t0, t1);
    c->hash = (c->hash * 0x50062281u) ^ sx_rr(c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x63) << 8;
    t2 += resize_table(c, c->rlo, c->rln);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 18);
    t1 ^= (uint32_t)split_scope(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9479u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8f) << 0;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 24);
    c->lane[6] += c->lane[3]; c->lane[4] ^= c->lane[6]; c->lane[4] = sx_rl(c->lane[4], 8);
    c->lane[2] ^= sx_rl(c->lane[12], 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x65) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void wrap_batch(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[9] + 0xc158ad0cu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50569u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8a) << 8;
    c->hash ^= c->lane[15] + 0x5a28310cu;
    t2 = (t2 ^ c->sum) * 0x7243720bu;
    t2 = (t2 ^ c->sum) * 0x3df04adbu;
    c->raw[c->slo + (int)((t0 + 22320u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30273u) % (uint32_t)c->rln)] << 8;
    load_store(c, &c->lane[9], 4);
    c->hash = (c->hash * 0xa74e38cfu) ^ sx_rr(c->hash, 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5ab6c823u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 460u) % (uint32_t)c->rln)] << 16;
    t0 ^= drain_mask(c, t1);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t yield_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 18);
    c->lane[3] += c->lane[5] ^ 0xff968988u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x682a25cbu;
    t2 += latch_entry(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xd6d391c7u;
    c->sched[20] = c->hash ^ sx_rl(c->lane[6], 23);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34844u) % (uint32_t)c->rln)] << 8;
    t0 ^= sync_tail(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49864u) % (uint32_t)c->rln)] << 16;
    t2 += queue_layer(c, c->rlo, c->rln);
    t0 ^= tune_rate(c, t1);
    c->raw[c->slo + (int)((t0 + 24525u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t resize_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[0] ^ 0x15e6ce42u;
    t1 ^= (uint32_t)hold_queue(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe95c2f39u;
    c->lane[15] += c->lane[6]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 27);
    c->sched[5] = c->hash ^ sx_rl(c->lane[7], 24);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 11);
    c->raw[c->slo + (int)((t0 + 5679u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)sync_index(c);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t prime_node(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xc5d7a3d1u) ^ sx_rr(c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x6393ffbdu) ^ sx_rr(c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x59de2c49u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)coal_line(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x67) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63411u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x0cab70b5u;
    c->hash ^= c->lane[15] + 0x0e369995u;
    cache_slot(c, &c->lane[4], 4);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t swap_pool(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 59562u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57027u) % (uint32_t)c->rln)] << 16;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 3);
    t2 += sift_level(c, c->slo, c->sln);
    c->lane[9] += c->lane[6]; c->lane[5] ^= c->lane[9]; c->lane[5] = sx_rl(c->lane[5], 20);
    t2 += rotate_pairing(c, c->slo, c->sln);
    c->lane[0] ^= sx_rl(c->lane[5], 10);
    t1 ^= (uint32_t)relay_field(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9301u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22814u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48492u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xda40736bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void trim_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[27] = c->hash ^ sx_rl(c->lane[4], 28);
    c->lane[0] += c->lane[10]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x21) << 16;
    t1 ^= (uint32_t)place_token(c, (uint8_t)(t0 >> 16), t2);
    c->lane[5] ^= sx_rl(c->lane[15], 1);
    c->lane[13] ^= sx_rl(c->lane[14], 22);
    c->lane[6] ^= sx_rl(c->lane[8], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50235u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x99a8f6a5u;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t probe_index(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[5] = c->hash ^ sx_rl(c->lane[10], 26);
    c->raw[c->slo + (int)((t0 + 8940u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x1bc0b2d7u;
    c->hash ^= c->lane[12] + 0x998b63f3u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x25400987u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[19] = c->hash ^ sx_rl(c->lane[8], 25);
    link_bucket(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x9e74032bu;
    c->lane[11] += c->lane[6] ^ 0x0fa781afu;
    t0 ^= probe_delta(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void link_label_149(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= sort_unit(c, t1);
    t2 += purge_marker(c, c->slo, c->sln);
    c->lane[11] += c->lane[1] ^ 0x5b7b4186u;
    merge_part_162(c, &c->lane[9], 1);
    t2 += peek_segment(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 50715u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    scan_label(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xe9032443u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t join_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)emit_lease(c);
    c->hash ^= c->lane[3] + 0x51e509f0u;
    t2 += (uint32_t)reap_index(c);
    parse_run_164(c, &c->lane[1], 2);
    c->lane[7] += c->lane[0] ^ 0xf5923c00u;
    c->lane[4] += c->lane[3] ^ 0x7b7f8237u;
    c->hash = (c->hash * 0x318c5d07u) ^ sx_rr(c->hash, 23);
    t2 += close_pool(c, c->slo, c->sln);
    c->lane[1] ^= sx_rl(c->lane[1], 14);
    c->lane[9] ^= sx_rl(c->lane[12], 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void pair_field(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x975256f5u) ^ sx_rr(c->hash, 15);
    c->hash ^= c->lane[13] + 0x300c2eafu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)settle_list(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t2 += (uint32_t)store_value(c);
    c->raw[c->slo + (int)((t0 + 44938u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[1] = c->hash ^ sx_rl(c->lane[13], 29);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcb) << 0;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 22);
    coal_limit(c, &c->lane[10], 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void patch_item(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x7f25c28fu;
    c->hash = (c->hash * 0xfe6bf301u) ^ sx_rr(c->hash, 24);
    c->lane[1] += c->lane[3]; c->lane[0] ^= c->lane[1]; c->lane[0] = sx_rl(c->lane[0], 3);
    t2 += sync_cell(c, c->rlo, c->rln);
    t2 += (uint32_t)rotate_item(c);
    t1 ^= (uint32_t)tally_entry(c, (uint8_t)(t0 >> 0), t2);
    c->lane[6] += c->lane[9] ^ 0x507811a7u;
    t2 += (uint32_t)purge_scope(c);
    c->hash ^= c->lane[9] + 0x9d4f9543u;
    c->hash ^= c->lane[15] + 0x51650667u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xfe) << 16;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t fold_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7b3224fdu;
    c->lane[5] += c->lane[6]; c->lane[2] ^= c->lane[5]; c->lane[2] = sx_rl(c->lane[2], 22);
    t2 += (uint32_t)pair_mask(c);
    c->lane[13] ^= sx_rl(c->lane[13], 19);
    t0 ^= probe_window(c, t1);
    t2 += queue_layer(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x8108184du) ^ sx_rr(c->hash, 11);
    c->sum += t1;
    return t0 + t2;
}

static int merge_port(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 6);
    t2 += sync_cell(c, c->slo, c->sln);
    c->lane[2] += c->lane[15] ^ 0xc259051eu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x164bb74du;
    t0 ^= align_key(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x9b39fa25u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t1 ^= (uint32_t)tally_entry(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    mix_page(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x01473e87u;
    c->hash ^= c->lane[2] + 0x381b95efu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 27);
    c->hash ^= c->lane[14] + 0xd1eebb31u;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void patch_delta(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd1) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    merge_part_162(c, &c->lane[6], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x10c0ba9du;
    t2 += (uint32_t)coal_offset(c);
    c->hash ^= c->lane[4] + 0x24b32976u;
    t1 ^= (uint32_t)defer_part_176(c, (uint8_t)(t0 >> 8), t2);
    c->lane[0] += c->lane[1] ^ 0x5e76fe75u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int fold_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    t2 += (uint32_t)swap_bucket(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += grow_scope(c, c->slo, c->sln);
    t2 += latch_entry(c, c->slo, c->sln);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 31);
    t2 += (uint32_t)reap_index(c);
    c->raw[c->slo + (int)((t0 + 55417u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x95af6c0du;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_mask(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9d) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52749u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 35864u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] += c->lane[7]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 23);
    c->hash = (c->hash * 0x2bca6109u) ^ sx_rr(c->hash, 10);
    c->sched[2] = c->hash ^ sx_rl(c->lane[3], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64456u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int tally_arena(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x58c837b5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[12] += c->lane[5] ^ 0x5d3b7eb8u;
    t2 = (t2 ^ c->sum) * 0x89bb2f03u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xf5984ad9u;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void map_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    c->lane[15] += c->lane[12]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 5);
    t0 ^= sync_tail(c, t1);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 24);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 31);
    c->lane[1] += c->lane[14] ^ 0x2343147eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] += c->lane[14]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x49) << 8;
    c->lane[1] += c->lane[6] ^ 0x816455e6u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22975u) % (uint32_t)c->rln)] << 16;
    c->lane[13] += c->lane[7] ^ 0xb939f725u;
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static uint8_t seek_span(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x859bcd2bu;
    c->sched[0] = c->hash ^ sx_rl(c->lane[6], 13);
    c->hash = (c->hash * 0x38d9cd77u) ^ sx_rr(c->hash, 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5392u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[0] + 0x7a26f71fu;
    t1 ^= (uint32_t)map_band(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9224u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 43383u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25537u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0xfa8088afu) ^ sx_rr(c->hash, 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50946u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t stage_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35559u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 16763u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[10] ^= sx_rl(c->lane[6], 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_part_162(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[4]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 18);
    c->hash ^= c->lane[15] + 0x1f2336c3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0e5169cdu;
    t2 = (t2 ^ c->sum) * 0x10a314bbu;
    t2 = (t2 ^ c->sum) * 0xeca9ffedu;
    t2 += (uint32_t)map_node(c);
    load_store(c, &c->lane[5], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int reap_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    t1 ^= (uint32_t)close_line(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x6df015a7u) ^ sx_rr(c->hash, 7);
    c->hash ^= c->lane[14] + 0x59db04f6u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x75) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    fetch_window(c, &c->lane[3], 1);
    t1 ^= (uint32_t)relay_gap(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0x5245c075u) ^ sx_rr(c->hash, 23);
    c->raw[c->slo + (int)((t0 + 14335u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] ^= sx_rl(c->lane[2], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 1);
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void parse_run_164(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc0) << 16;
    t2 = (t2 ^ c->sum) * 0x5b2fef1du;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 10);
    c->lane[3] += c->lane[2] ^ 0x87575f7eu;
    c->hash = (c->hash * 0xf95ea0b3u) ^ sx_rr(c->hash, 23);
    t2 = (t2 ^ c->sum) * 0x58431a55u;
    c->sched[24] = c->hash ^ sx_rl(c->lane[5], 8);
    c->raw[c->slo + (int)((t0 + 40727u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int swap_bucket(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15254u) % (uint32_t)c->rln)] << 24;
    c->lane[11] += c->lane[10]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29244u) % (uint32_t)c->rln)] << 16;
    t0 ^= purge_pairing(c, t1);
    c->sched[24] = c->hash ^ sx_rl(c->lane[11], 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4f5a88f9u;
    c->sched[11] = c->hash ^ sx_rl(c->lane[13], 25);
    pair_ring(c, t0, t1);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t tally_entry(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1f) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2980c30du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x96b147b5u;
    c->raw[c->slo + (int)((t0 + 57924u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t grow_scope(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[8] + 0xbc6bf875u;
    c->raw[c->slo + (int)((t0 + 64407u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x9f430ef9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57031u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xdebb97abu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22691u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1c) << 16;
    c->lane[5] += c->lane[13] ^ 0xdb5da6a6u;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 18);
    c->lane[0] += c->lane[5]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    slice_level(c, &c->lane[6], 1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t peek_segment(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[2] = c->hash ^ sx_rl(c->lane[0], 26);
    c->lane[13] += c->lane[11]; c->lane[3] ^= c->lane[13]; c->lane[3] = sx_rl(c->lane[3], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x90893ab5u;
    c->hash = (c->hash * 0x5e296043u) ^ sx_rr(c->hash, 20);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t sift_level(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t1 ^= (uint32_t)fetch_line(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0xaf2f4c77u;
    t2 += (uint32_t)purge_pool(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8a) << 16;
    c->hash ^= c->lane[3] + 0x330bb7f4u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x67) << 0;
    c->hash ^= c->lane[15] + 0xe4bf4cebu;
    c->lane[15] += c->lane[4]; c->lane[8] ^= c->lane[15]; c->lane[8] = sx_rl(c->lane[8], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36758u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static int rotate_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[9] += c->lane[7]; c->lane[0] ^= c->lane[9]; c->lane[0] = sx_rl(c->lane[0], 24);
    c->raw[c->slo + (int)((t0 + 47275u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x838b9409u) ^ sx_rr(c->hash, 17);
    c->sched[26] = c->hash ^ sx_rl(c->lane[2], 23);
    fold_bucket(c, &c->lane[4], 4);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t coal_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t1 ^= (uint32_t)tally_line(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0x828395e9u) ^ sx_rr(c->hash, 31);
    c->hash ^= c->lane[2] + 0x6cde9fefu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[14] += c->lane[15]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 10);
    t1 ^= (uint32_t)blend_batch(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += peek_record(c, c->slo, c->sln);
    c->lane[13] += c->lane[11]; c->lane[15] ^= c->lane[13]; c->lane[15] = sx_rl(c->lane[15], 17);
    c->sched[3] = c->hash ^ sx_rl(c->lane[4], 6);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t tune_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[8] += c->lane[8] ^ 0xf25c4b79u;
    c->lane[12] += c->lane[15] ^ 0x22d1ef72u;
    c->lane[7] ^= sx_rl(c->lane[6], 22);
    t2 += close_pairing(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 52634u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4748u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7737u) % (uint32_t)c->rln)] << 24;
    t1 ^= (uint32_t)split_scope(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9f2fbedfu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t sync_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 56343u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += drain_region(c, c->slo, c->sln);
    t2 += (uint32_t)align_page(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37855u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 += step_port(c, c->slo, c->sln);
    c->hash ^= c->lane[9] + 0xb29ce4a0u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sched[23] = c->hash ^ sx_rl(c->lane[1], 7);
    c->sum += t1;
    return t0 + t2;
}

static void mix_page(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[0] + 0xc336b2f8u;
    c->lane[9] ^= sx_rl(c->lane[12], 4);
    t2 += pair_pool(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x2dd745adu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10858u) % (uint32_t)c->rln)] << 16;
    c->lane[11] ^= sx_rl(c->lane[11], 8);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static int pair_mask(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->lane[12] += c->lane[15]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 17);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52581u) % (uint32_t)c->rln)] << 8;
    c->lane[4] += c->lane[11]; c->lane[6] ^= c->lane[4]; c->lane[6] = sx_rl(c->lane[6], 6);
    poll_ring(c, t0, t1);
    c->hash ^= c->lane[13] + 0xca7d0839u;
    c->lane[5] += c->lane[11] ^ 0x64a75061u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x22e24b43u;
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t defer_part_176(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[0] + 0x52c1e406u;
    c->hash ^= c->lane[15] + 0x8c949e71u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x97da43e5u;
    t2 = (t2 ^ c->sum) * 0x506cbf87u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45384u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += (uint32_t)settle_tail(c);
    t2 = (t2 ^ c->sum) * 0x589fa3c9u;
    c->lane[7] ^= sx_rl(c->lane[1], 11);
    poll_rate(c, &c->lane[0], 1);
    t2 = (t2 ^ c->sum) * 0x0e97bdf5u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int sync_index(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->sched[8] = c->hash ^ sx_rl(c->lane[8], 2);
    c->lane[12] += c->lane[1] ^ 0xaa17a4d1u;
    c->raw[c->slo + (int)((t0 + 4254u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x963bb1b9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdc) << 8;
    c->hash = (c->hash * 0x39fb1a05u) ^ sx_rr(c->hash, 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= join_pairing(c, t1);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t place_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t1 ^= (uint32_t)wrap_cursor(c, (uint8_t)(t0 >> 0), t2);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x43f91d83u;
    c->lane[15] ^= sx_rl(c->lane[15], 22);
    t0 ^= probe_delta(c, t1);
    c->lane[14] += c->lane[11]; c->lane[5] ^= c->lane[14]; c->lane[5] = sx_rl(c->lane[5], 9);
    c->lane[3] ^= sx_rl(c->lane[14], 15);
    t1 ^= (uint32_t)place_rate_251(c, (uint8_t)(t0 >> 8), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t sort_unit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[14] + 0x4295953bu;
    t1 ^= (uint32_t)yield_marker(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[8] + 0x3a9ba9fcu;
    c->sched[5] = c->hash ^ sx_rl(c->lane[7], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb831ff5fu;
    t1 ^= (uint32_t)scan_digest(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)emit_marker_199(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x30) << 0;
    t2 = (t2 ^ c->sum) * 0x0a419977u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2b396963u;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 27);
    c->lane[1] += c->lane[4]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0d) << 16;
    t2 += (uint32_t)sift_chunk(c);
    t1 ^= (uint32_t)drain_tuple(c, (uint8_t)(t0 >> 16), t2);
    c->lane[7] += c->lane[1] ^ 0x26ef7e61u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t sync_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x08c030edu) ^ sx_rr(c->hash, 16);
    t2 = (t2 ^ c->sum) * 0xa162115fu;
    c->hash ^= c->lane[14] + 0x12c26e2au;
    t2 = (t2 ^ c->sum) * 0xe3977f07u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void coal_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[7] + 0xfb4e49b0u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    probe_block(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 13575u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22059u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x83abf173u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t align_key(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x20) << 8;
    t0 ^= sift_stream(c, t1);
    c->sched[1] = c->hash ^ sx_rl(c->lane[3], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x0db53a2du;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24214u) % (uint32_t)c->rln)] << 8;
    c->sched[18] = c->hash ^ sx_rl(c->lane[10], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[10] += c->lane[11]; c->lane[8] ^= c->lane[10]; c->lane[8] = sx_rl(c->lane[8], 31);
    c->lane[2] += c->lane[14]; c->lane[1] ^= c->lane[2]; c->lane[1] = sx_rl(c->lane[1], 14);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t settle_list(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[0] ^= sx_rl(c->lane[9], 21);
    t2 = (t2 ^ c->sum) * 0x28ee0b19u;
    t2 = (t2 ^ c->sum) * 0x3efa5a33u;
    t2 = (t2 ^ c->sum) * 0xd001c4a1u;
    c->lane[15] += c->lane[7]; c->lane[9] ^= c->lane[15]; c->lane[9] = sx_rl(c->lane[9], 5);
    t2 += fetch_page(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x02675435u) ^ sx_rr(c->hash, 6);
    c->hash = (c->hash * 0x39a363c5u) ^ sx_rr(c->hash, 7);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t queue_layer(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] += c->lane[10] ^ 0xdd91a403u;
    merge_seat(c, t0, t1);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 21);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 31);
    t2 += trim_mask(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 4704u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 ^= drain_mask(c, t1);
    c->raw[c->slo + (int)((t0 + 27249u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t1 ^= (uint32_t)scan_digest(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x16075c6du;
    mark_table(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x19) << 0;
    c->sum += t1;
    return t0 + t2;
}

static int coal_offset(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x77) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36070u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 6398u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63503u) % (uint32_t)c->rln)] << 24;
    c->lane[1] ^= sx_rl(c->lane[5], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32484u) % (uint32_t)c->rln)] << 8;
    c->lane[12] ^= sx_rl(c->lane[3], 24);
    c->lane[3] += c->lane[5]; c->lane[7] ^= c->lane[3]; c->lane[7] = sx_rl(c->lane[7], 10);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t rotate_pairing(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[2] + 0x8064433eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    seek_slot(c, &c->lane[0], 4);
    fold_bucket(c, &c->lane[11], 1);
    c->hash ^= c->lane[8] + 0x0c14bf0eu;
    t2 += (uint32_t)clamp_bound(c);
    c->lane[15] ^= sx_rl(c->lane[13], 27);
    c->hash = (c->hash * 0xd1a15f21u) ^ sx_rr(c->hash, 15);
    t2 += (uint32_t)map_node(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15868u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static void link_bucket(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] += c->lane[10] ^ 0x293997c8u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44783u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)drain_tuple(c, (uint8_t)(t0 >> 16), t2);
    t2 += split_batch(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 ^= resize_pool(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5a) << 16;
    c->raw[c->slo + (int)((t0 + 893u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint32_t probe_window(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[6] ^= sx_rl(c->lane[11], 7);
    c->lane[8] += c->lane[14]; c->lane[11] ^= c->lane[8]; c->lane[11] = sx_rl(c->lane[11], 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x61) << 0;
    t2 += (uint32_t)grow_stream(c);
    c->raw[c->slo + (int)((t0 + 50353u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += purge_marker(c, c->slo, c->sln);
    t2 += (uint32_t)sort_region(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x19) << 16;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 9);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t map_band(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[14] ^= sx_rl(c->lane[14], 1);
    c->lane[0] ^= sx_rl(c->lane[3], 24);
    c->lane[2] += c->lane[14] ^ 0xd4458b6cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 23);
    c->lane[9] ^= sx_rl(c->lane[8], 10);
    t2 = (t2 ^ c->sum) * 0xb370cda3u;
    c->hash = (c->hash * 0xf606fd95u) ^ sx_rr(c->hash, 5);
    t1 ^= (uint32_t)settle_lease(c, (uint8_t)(t0 >> 8), t2);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 13);
    t2 += close_pairing(c, c->slo, c->sln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t latch_entry(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += (uint32_t)purge_scope(c);
    c->hash ^= c->lane[5] + 0x9f749fd5u;
    c->lane[5] += c->lane[7]; c->lane[12] ^= c->lane[5]; c->lane[12] = sx_rl(c->lane[12], 28);
    c->lane[1] += c->lane[5] ^ 0x135060f6u;
    move_value(c, &c->lane[9], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0xce558055u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void cache_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 17918u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[3] += c->lane[15] ^ 0x76382032u;
    t2 = (t2 ^ c->sum) * 0x3bed87f1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe90237bbu;
    t2 += (uint32_t)stage_marker(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9a) << 8;
    c->hash = (c->hash * 0x25d5e999u) ^ sx_rr(c->hash, 13);
    c->lane[14] += c->lane[13] ^ 0x020d298bu;
    c->hash = (c->hash * 0xe070ee79u) ^ sx_rr(c->hash, 15);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t close_pool(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 10);
    t2 += (uint32_t)rotate_slot(c);
    t2 = (t2 ^ c->sum) * 0x465a1445u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20194u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0xadcff11bu) ^ sx_rr(c->hash, 13);
    t0 ^= fetch_bucket(c, t1);
    c->raw[c->slo + (int)((t0 + 49405u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0xef4f820bu) ^ sx_rr(c->hash, 27);
    c->lane[5] ^= sx_rl(c->lane[14], 1);
    c->sum += t1;
    return t0 + t2;
}

static int emit_lease(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    t2 += slice_level_226(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x28) << 8;
    t0 ^= wrap_rate(c, t1);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33454u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)store_value(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x6a) << 0;
    c->lane[11] += c->lane[14]; c->lane[2] ^= c->lane[11]; c->lane[2] = sx_rl(c->lane[2], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t relay_field(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += pair_pool(c, c->slo, c->sln);
    c->lane[10] ^= sx_rl(c->lane[7], 27);
    c->raw[c->slo + (int)((t0 + 27142u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash = (c->hash * 0xe35eed71u) ^ sx_rr(c->hash, 25);
    c->lane[8] ^= sx_rl(c->lane[13], 23);
    c->lane[4] ^= sx_rl(c->lane[13], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 6586u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 15);
    t0 ^= step_entry(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t hold_queue(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 9);
    t2 = (t2 ^ c->sum) * 0x320c9349u;
    c->lane[9] ^= sx_rl(c->lane[7], 2);
    t2 = (t2 ^ c->sum) * 0x8c147371u;
    c->sched[31] = c->hash ^ sx_rl(c->lane[15], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[13] += c->lane[7] ^ 0x98c010acu;
    t2 += (uint32_t)fold_seat(c);
    yield_pairing(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x26646027u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 += queue_digest(c, c->rlo, c->rln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void scan_label(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] += c->lane[1] ^ 0x14e1eb35u;
    c->lane[5] += c->lane[7] ^ 0xd6596a1du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57853u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[5] += c->lane[15] ^ 0x3b09eab6u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd4) << 0;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void pair_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1338u) % (uint32_t)c->rln)] << 8;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 6);
    c->lane[5] += c->lane[4]; c->lane[2] ^= c->lane[5]; c->lane[2] = sx_rl(c->lane[2], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0x47a12ed7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 981u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[5] ^= sx_rl(c->lane[11], 23);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint8_t wrap_cursor(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[9] ^= sx_rl(c->lane[2], 23);
    c->raw[c->slo + (int)((t0 + 15144u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x33fcbe5du;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 19);
    c->lane[2] += c->lane[5]; c->lane[0] ^= c->lane[2]; c->lane[0] = sx_rl(c->lane[0], 8);
    c->raw[c->slo + (int)((t0 + 10471u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 8);
    c->lane[10] += c->lane[14]; c->lane[9] ^= c->lane[10]; c->lane[9] = sx_rl(c->lane[9], 15);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int emit_marker_199(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    mark_region(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 36102u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[9] = c->hash ^ sx_rl(c->lane[4], 2);
    c->raw[c->slo + (int)((t0 + 24094u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[1] += c->lane[0]; c->lane[14] ^= c->lane[1]; c->lane[14] = sx_rl(c->lane[14], 7);
    t2 = (t2 ^ c->sum) * 0x14dbc389u;
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int clamp_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash ^= c->lane[0] + 0x685546a4u;
    c->hash = (c->hash * 0xc43d2639u) ^ sx_rr(c->hash, 12);
    c->lane[12] += c->lane[10] ^ 0xe03b8f76u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44805u) % (uint32_t)c->rln)] << 24;
    c->lane[0] ^= sx_rl(c->lane[10], 17);
    c->hash = (c->hash * 0xcc592c5du) ^ sx_rr(c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xed52f91du;
    c->sched[28] = c->hash ^ sx_rl(c->lane[3], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61719u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[5] + 0xcf6e6023u;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int sift_chunk(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->hash ^= c->lane[7] + 0xcd90731au;
    c->lane[5] += c->lane[13] ^ 0x966a0d22u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xe4a8c955u;
    c->hash ^= c->lane[5] + 0x434a9279u;
    c->hash = (c->hash * 0x29f3ec85u) ^ sx_rr(c->hash, 12);
    c->hash ^= c->lane[0] + 0x09e72cebu;
    c->raw[c->slo + (int)((t0 + 31343u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x4756471fu) ^ sx_rr(c->hash, 18);
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t split_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 999u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x14) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30518u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void poll_rate(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[12] += c->lane[1] ^ 0x977b2c3cu;
    c->lane[3] ^= sx_rl(c->lane[7], 13);
    c->raw[c->slo + (int)((t0 + 39799u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[13] ^= sx_rl(c->lane[4], 2);
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 22);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t drain_tuple(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 5);
    c->lane[9] ^= sx_rl(c->lane[6], 7);
    t2 = (t2 ^ c->sum) * 0x3195e255u;
    c->lane[14] += c->lane[15]; c->lane[6] ^= c->lane[14]; c->lane[6] = sx_rl(c->lane[6], 14);
    t2 = (t2 ^ c->sum) * 0xcc3e75cdu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xcd24f255u;
    c->hash = (c->hash * 0xe8b829e3u) ^ sx_rr(c->hash, 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void slice_level(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x18) << 8;
    c->raw[c->slo + (int)((t0 + 15565u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10992u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37571u) % (uint32_t)c->rln)] << 24;
    c->lane[9] += c->lane[0]; c->lane[10] ^= c->lane[9]; c->lane[10] = sx_rl(c->lane[10], 26);
    c->lane[11] += c->lane[14]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x15) << 8;
    c->lane[11] ^= sx_rl(c->lane[14], 14);
    c->raw[c->slo + (int)((t0 + 53195u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void seek_slot(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x2d3db723u) ^ sx_rr(c->hash, 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6bc0f4f5u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc5) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[0] += c->lane[8]; c->lane[12] ^= c->lane[0]; c->lane[12] = sx_rl(c->lane[12], 5);
    t2 = (t2 ^ c->sum) * 0x10c263c9u;
    c->raw[c->slo + (int)((t0 + 6645u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[13] + 0x9fa87947u;
    c->lane[11] ^= sx_rl(c->lane[3], 24);
    c->lane[1] += c->lane[10] ^ 0xcea3d582u;
    c->raw[c->slo + (int)((t0 + 30786u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void fetch_window(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] += c->lane[11] ^ 0xbd7c4428u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9a) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[12] ^= sx_rl(c->lane[10], 18);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int map_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xde) << 16;
    c->lane[8] += c->lane[8] ^ 0x918fbcabu;
    c->raw[c->slo + (int)((t0 + 6025u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] += c->lane[4] ^ 0xbb81979au;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14045u) % (uint32_t)c->rln)] << 16;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t close_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[13] += c->lane[11]; c->lane[3] ^= c->lane[13]; c->lane[3] = sx_rl(c->lane[3], 5);
    c->hash ^= c->lane[3] + 0x2480d964u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe3355da1u;
    c->hash = (c->hash * 0xf4980fe3u) ^ sx_rr(c->hash, 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t step_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[2] = c->hash ^ sx_rl(c->lane[4], 17);
    c->raw[c->slo + (int)((t0 + 30667u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3c) << 0;
    c->hash ^= c->lane[9] + 0xc24147f9u;
    c->lane[6] += c->lane[0]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 31);
    c->sched[28] = c->hash ^ sx_rl(c->lane[12], 31);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 6);
    c->hash ^= c->lane[3] + 0xc9b34700u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t split_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x2010aa07u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[8], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61809u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x33c3300bu) ^ sx_rr(c->hash, 19);
    c->hash ^= c->lane[7] + 0x2151f504u;
    c->hash ^= c->lane[6] + 0x87db49dau;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x414007cdu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x91) << 16;
    c->lane[8] += c->lane[7] ^ 0xbd98e8d3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static uint32_t fetch_bucket(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[13], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[12] ^= sx_rl(c->lane[3], 26);
    c->lane[3] += c->lane[8]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 4);
    c->sched[2] = c->hash ^ sx_rl(c->lane[11], 31);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t tally_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] += c->lane[7] ^ 0x32ff2ab3u;
    c->hash ^= c->lane[14] + 0x2f473f7du;
    c->sched[11] = c->hash ^ sx_rl(c->lane[15], 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= c->lane[3] + 0x4079afbau;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int purge_scope(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[19] = c->hash ^ sx_rl(c->lane[12], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sched[14] = c->hash ^ sx_rl(c->lane[7], 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[12] + 0x0a4676d5u;
    c->lane[2] += c->lane[3] ^ 0x8931e90fu;
    c->lane[1] += c->lane[8] ^ 0xbf31e853u;
    c->lane[14] += c->lane[13] ^ 0x7386340eu;
    c->sched[4] = c->hash ^ sx_rl(c->lane[8], 9);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_bucket(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[4] + 0xaec83594u;
    c->raw[c->slo + (int)((t0 + 2668u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 3404u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x67b551c7u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 30);
    c->lane[12] ^= sx_rl(c->lane[6], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] += c->lane[3]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 22);
    t2 = (t2 ^ c->sum) * 0xc1a5ab15u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13013u) % (uint32_t)c->rln)] << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void merge_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21696u) % (uint32_t)c->rln)] << 0;
    c->lane[8] += c->lane[10] ^ 0x8bc82ed6u;
    c->raw[c->slo + (int)((t0 + 58693u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 40292u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xce) << 0;
    c->sched[17] = c->hash ^ sx_rl(c->lane[10], 2);
    c->hash = (c->hash * 0x2f66271du) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xdb497cf3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t resize_pool(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9a) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34635u) % (uint32_t)c->rln)] << 8;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 23);
    t2 = (t2 ^ c->sum) * 0x3183cbabu;
    t2 = (t2 ^ c->sum) * 0x9c19d777u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[6] += c->lane[9]; c->lane[4] ^= c->lane[6]; c->lane[4] = sx_rl(c->lane[4], 24);
    t2 = (t2 ^ c->sum) * 0x457f1ac9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static int sort_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= c->lane[14] + 0xaf1768a7u;
    c->lane[9] += c->lane[8]; c->lane[7] ^= c->lane[9]; c->lane[7] = sx_rl(c->lane[7], 2);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[11] += c->lane[7] ^ 0xc45143f6u;
    c->sched[15] = c->hash ^ sx_rl(c->lane[11], 17);
    c->lane[5] += c->lane[9] ^ 0x5afe1c43u;
    c->lane[15] += c->lane[14] ^ 0x622d14d8u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4e67f2e7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5302ebdbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14536u) % (uint32_t)c->rln)] << 0;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void move_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sched[29] = c->hash ^ sx_rl(c->lane[4], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31718u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x97) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55349u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11371u) % (uint32_t)c->rln)] << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t close_pairing(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 1272u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[15] + 0x41138f34u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0xac198135u) ^ sx_rr(c->hash, 6);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 4);
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 15);
    c->sched[23] = c->hash ^ sx_rl(c->lane[0], 18);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 22);
    c->lane[13] += c->lane[12]; c->lane[7] ^= c->lane[13]; c->lane[7] = sx_rl(c->lane[7], 10);
    c->sum += t1;
    return t0 + t2;
}

static void load_store(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x7b) << 16;
    c->hash ^= c->lane[3] + 0xa198888fu;
    c->raw[c->slo + (int)((t0 + 32875u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[5] ^= sx_rl(c->lane[0], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xfd8a2fe9u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 9383u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 26789u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void probe_block(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[11] += c->lane[7]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[13] + 0x014bf1f7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x4a3f195fu;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 24);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64214u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint32_t purge_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] ^= sx_rl(c->lane[9], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe0) << 0;
    t2 = (t2 ^ c->sum) * 0xc6ebcb19u;
    c->sched[20] = c->hash ^ sx_rl(c->lane[10], 24);
    c->hash ^= c->lane[9] + 0xc8b17f04u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2117u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x64bc6015u;
    c->lane[4] += c->lane[8] ^ 0x2e4afaedu;
    c->hash = (c->hash * 0x1709aac3u) ^ sx_rr(c->hash, 3);
    c->raw[c->slo + (int)((t0 + 11589u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int grow_stream(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[1] += c->lane[0]; c->lane[12] ^= c->lane[1]; c->lane[12] = sx_rl(c->lane[12], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1b) << 8;
    t2 = (t2 ^ c->sum) * 0x0d3c43b1u;
    c->raw[c->slo + (int)((t0 + 25152u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xaa116f7du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x34725541u;
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t step_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[3] + 0x99203a03u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= c->lane[14] + 0x63b05fc8u;
    c->lane[2] ^= sx_rl(c->lane[11], 15);
    c->lane[6] ^= sx_rl(c->lane[3], 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 3);
    c->lane[11] += c->lane[14] ^ 0xcaee103cu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t slice_level_226(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] += c->lane[13] ^ 0xaaa18463u;
    c->lane[9] ^= sx_rl(c->lane[6], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5e23f41du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x57c31a59u;
    c->hash = (c->hash * 0x39f96651u) ^ sx_rr(c->hash, 6);
    c->hash = (c->hash * 0xd287eaf1u) ^ sx_rr(c->hash, 8);
    c->lane[5] += c->lane[0] ^ 0x6a93aa7fu;
    c->sum += t1;
    return t0 + t2;
}

static void mark_table(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[5] = c->hash ^ sx_rl(c->lane[7], 9);
    c->lane[14] += c->lane[10]; c->lane[11] ^= c->lane[14]; c->lane[11] = sx_rl(c->lane[11], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe67f14edu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4269200fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x73) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xbc) << 8;
    c->hash = (c->hash * 0x43297569u) ^ sx_rr(c->hash, 18);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t blend_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    c->lane[7] += c->lane[6] ^ 0xbfab2b6au;
    c->lane[10] ^= sx_rl(c->lane[13], 23);
    c->hash = (c->hash * 0x07fa9a19u) ^ sx_rr(c->hash, 15);
    c->lane[11] += c->lane[5] ^ 0x576c2d52u;
    c->hash ^= c->lane[10] + 0x115f4153u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30748u) % (uint32_t)c->rln)] << 16;
    c->lane[5] += c->lane[4]; c->lane[10] ^= c->lane[5]; c->lane[10] = sx_rl(c->lane[10], 21);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t probe_delta(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[2] ^ 0x259f8bceu;
    t2 = (t2 ^ c->sum) * 0xe93099f5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x178ce0b7u;
    c->lane[10] += c->lane[4]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 4);
    c->lane[6] ^= sx_rl(c->lane[4], 26);
    c->lane[6] ^= sx_rl(c->lane[5], 15);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 25);
    c->lane[4] += c->lane[12]; c->lane[6] ^= c->lane[4]; c->lane[6] = sx_rl(c->lane[6], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58287u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t relay_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[12] + 0xc2e71474u;
    c->sched[10] = c->hash ^ sx_rl(c->lane[7], 2);
    c->lane[15] += c->lane[4]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa2) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->raw[c->slo + (int)((t0 + 21140u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[0] + 0x68c4da72u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xfb) << 0;
    c->hash ^= c->lane[13] + 0x5625352bu;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 4);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t trim_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x226d0aedu;
    c->lane[0] ^= sx_rl(c->lane[4], 9);
    c->lane[0] += c->lane[12]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 17);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xfc6b4009u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sched[8] = c->hash ^ sx_rl(c->lane[0], 9);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t scan_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[6] + 0xaf8d2551u;
    c->lane[12] ^= sx_rl(c->lane[12], 28);
    c->sched[22] = c->hash ^ sx_rl(c->lane[14], 24);
    c->hash ^= c->lane[8] + 0xfa7a92c7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int purge_pool(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash = (c->hash * 0xc1fb1c79u) ^ sx_rr(c->hash, 2);
    c->hash = (c->hash * 0xfb0ba363u) ^ sx_rr(c->hash, 19);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x045d6735u;
    c->raw[c->slo + (int)((t0 + 48559u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xab) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3869u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 16895u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t fetch_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 6);
    c->lane[3] ^= sx_rl(c->lane[7], 29);
    c->lane[2] += c->lane[12]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 11);
    c->lane[1] += c->lane[0] ^ 0x291bb4d8u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void poll_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] += c->lane[8] ^ 0xf9e6a68eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xca) << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x9b175879u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19734u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 11620u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[29] = c->hash ^ sx_rl(c->lane[13], 5);
    c->lane[9] += c->lane[15]; c->lane[12] ^= c->lane[9]; c->lane[12] = sx_rl(c->lane[12], 3);
    t2 += mark_unit(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t settle_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 2);
    c->lane[6] += c->lane[5]; c->lane[12] ^= c->lane[6]; c->lane[12] = sx_rl(c->lane[12], 22);
    t2 = (t2 ^ c->sum) * 0xaacd235fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t fetch_page(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[3] + 0xb7931eedu;
    c->lane[4] ^= sx_rl(c->lane[6], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5c83b287u;
    c->lane[9] ^= sx_rl(c->lane[9], 11);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t drain_region(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xb87051f3u) ^ sx_rr(c->hash, 31);
    c->raw[c->slo + (int)((t0 + 21978u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[1] = c->hash ^ sx_rl(c->lane[9], 28);
    c->lane[7] ^= sx_rl(c->lane[7], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4394a47du;
    t2 = (t2 ^ c->sum) * 0xe7d41fe5u;
    c->sum += t1;
    return t0 + t2;
}

static int store_value(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->raw[c->slo + (int)((t0 + 62713u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[2] ^= sx_rl(c->lane[5], 28);
    c->hash ^= c->lane[0] + 0x6afaa03bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[3] + 0xf6936eafu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcc) << 0;
    c->sched[31] = c->hash ^ sx_rl(c->lane[0], 9);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 3);
    t2 = (t2 ^ c->sum) * 0x6177776bu;
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void yield_pairing(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] += c->lane[14] ^ 0x01bcc1e2u;
    c->hash ^= c->lane[11] + 0x94111843u;
    c->hash = (c->hash * 0x180c1949u) ^ sx_rr(c->hash, 19);
    c->lane[2] ^= sx_rl(c->lane[15], 8);
    t2 = (t2 ^ c->sum) * 0x2dc121d7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xab06087bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[5] += c->lane[9] ^ 0xd04d02a3u;
    c->lane[15] += c->lane[0] ^ 0xa27c9318u;
    c->lane[12] += c->lane[1]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 7);
    c->lane[11] ^= sx_rl(c->lane[6], 30);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t sift_stream(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xf4) << 0;
    c->hash ^= c->lane[13] + 0x48d08514u;
    c->raw[c->slo + (int)((t0 + 12139u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcb) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9c) << 8;
    c->hash ^= c->lane[6] + 0xdd70053fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x0ee50f55u;
    t2 = (t2 ^ c->sum) * 0xa189bc01u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t purge_marker(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x55) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x38d2b68fu;
    t0 ^= pair_tail(c, t1);
    t2 = (t2 ^ c->sum) * 0x3b013219u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33823u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 13985u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 12119u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[10] + 0xff22b563u;
    c->hash = (c->hash * 0x4e8b3dedu) ^ sx_rr(c->hash, 28);
    t2 = (t2 ^ c->sum) * 0x41f62353u;
    c->lane[4] += c->lane[12] ^ 0x4b3d993eu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t drain_mask(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] += c->lane[13] ^ 0x11e163b4u;
    t2 = (t2 ^ c->sum) * 0xc6a799ffu;
    c->lane[0] += c->lane[10]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pair_pool(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sched[25] = c->hash ^ sx_rl(c->lane[8], 29);
    c->lane[15] ^= sx_rl(c->lane[9], 2);
    t2 = (t2 ^ c->sum) * 0x89542b75u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[11], 21);
    c->lane[3] += c->lane[7]; c->lane[13] ^= c->lane[3]; c->lane[13] = sx_rl(c->lane[13], 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa6) << 8;
    c->sched[6] = c->hash ^ sx_rl(c->lane[3], 26);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t queue_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3977c22fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[9] + 0xc6a1283bu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 11);
    c->hash = (c->hash * 0x1f9e2199u) ^ sx_rr(c->hash, 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[7] += c->lane[15] ^ 0x9195d5c7u;
    c->hash = (c->hash * 0xa19aefd1u) ^ sx_rr(c->hash, 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t peek_record(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x582821afu;
    c->hash = (c->hash * 0x9be21037u) ^ sx_rr(c->hash, 28);
    c->raw[c->slo + (int)((t0 + 43724u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= pin_entry(c, t1);
    c->sched[19] = c->hash ^ sx_rl(c->lane[0], 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xdda89583u;
    c->sched[11] = c->hash ^ sx_rl(c->lane[14], 22);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum += t1;
    return t0 + t2;
}

static int rotate_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->hash ^= c->lane[1] + 0x230b0b4eu;
    c->lane[11] += c->lane[12] ^ 0x2678c4f3u;
    c->lane[13] += c->lane[6]; c->lane[11] ^= c->lane[13]; c->lane[11] = sx_rl(c->lane[11], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa4) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65244u) % (uint32_t)c->rln)] << 8;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int fold_seat(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[16] = c->hash ^ sx_rl(c->lane[12], 27);
    c->sched[8] = c->hash ^ sx_rl(c->lane[4], 3);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int settle_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->raw[c->slo + (int)((t0 + 34561u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56880u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x4ab49145u) ^ sx_rr(c->hash, 25);
    c->lane[6] ^= sx_rl(c->lane[13], 1);
    t2 = (t2 ^ c->sum) * 0x76002fb1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x756aae3bu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 20);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int stage_marker(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    c->lane[7] += c->lane[3]; c->lane[5] ^= c->lane[7]; c->lane[5] = sx_rl(c->lane[5], 8);
    c->lane[12] += c->lane[13]; c->lane[2] ^= c->lane[12]; c->lane[2] = sx_rl(c->lane[2], 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0xca6deaf3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x8b) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17663u) % (uint32_t)c->rln)] << 8;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 18);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t place_rate_251(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[14] = c->hash ^ sx_rl(c->lane[1], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[9] += c->lane[3] ^ 0xebc93f5bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0xf95a1c41u) ^ sx_rr(c->hash, 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10099u) % (uint32_t)c->rln)] << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t join_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] += c->lane[2] ^ 0x32188c13u;
    c->raw[c->slo + (int)((t0 + 55819u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[8] += c->lane[2]; c->lane[11] ^= c->lane[8]; c->lane[11] = sx_rl(c->lane[11], 30);
    c->lane[5] += c->lane[14] ^ 0x254b4e3fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2b6c90bfu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47680u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 32277u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[0] ^= sx_rl(c->lane[7], 2);
    c->lane[4] ^= sx_rl(c->lane[14], 1);
    c->lane[11] ^= sx_rl(c->lane[8], 28);
    c->hash = (c->hash * 0x9b47eaa5u) ^ sx_rr(c->hash, 29);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t wrap_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[2] ^= sx_rl(c->lane[12], 31);
    c->raw[c->slo + (int)((t0 + 55260u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x47c541b3u;
    t2 = (t2 ^ c->sum) * 0xcee75b21u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44208u) % (uint32_t)c->rln)] << 0;
    c->sched[9] = c->hash ^ sx_rl(c->lane[13], 10);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t yield_marker(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[12] + 0x3820d3b8u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[16] = c->hash ^ sx_rl(c->lane[15], 17);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 20);
    c->sched[12] = c->hash ^ sx_rl(c->lane[6], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int align_page(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x94) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[7] += c->lane[14]; c->lane[12] ^= c->lane[7]; c->lane[12] = sx_rl(c->lane[12], 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xf24399bfu;
    c->lane[0] ^= sx_rl(c->lane[3], 14);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void reset_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += (uint32_t)link_run(c);
    t2 += (uint32_t)reap_tail(c);
    pick_layer(c, t0, t1);
    t2 += tally_slot(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0xfaa95de1u;
    t2 += wrap_rate_258(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)split_layer(c, (uint8_t)(t0 >> 0), t2);
    coal_port_373(c, t0, t1);
    c->sched[5] = c->hash ^ sx_rl(c->lane[9], 9);
    t2 += tap_stack(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x2171085bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1fda7213u;
    t1 ^= (uint32_t)parse_part(c, (uint8_t)(t0 >> 8), t2);
    map_window(c, &c->lane[9], 2);
    t0 ^= chain_layer(c, t1);
    t1 ^= (uint32_t)shift_ring(c, (uint8_t)(t0 >> 16), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57281u) % (uint32_t)c->rln)] << 24;
    t1 ^= (uint32_t)pick_digest(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc7) << 0;
    t1 ^= (uint32_t)step_cell(c, (uint8_t)(t0 >> 0), t2);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static int reap_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    t1 ^= (uint32_t)prime_ring(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= load_track(c, t1);
    c->lane[2] += c->lane[13]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 = (t2 ^ c->sum) * 0xa07f7f77u;
    resize_marker(c, t0, t1);
    c->sched[27] = c->hash ^ sx_rl(c->lane[4], 16);
    join_mask(c, t0, t1);
    peek_level(c, &c->lane[8], 1);
    t0 ^= fetch_batch(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcb1c72b7u;
    t2 += prime_arena(c, c->rlo, c->rln);
    t1 ^= (uint32_t)peek_cell(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)move_frame_278(c, (uint8_t)(t0 >> 8), t2);
    c->hash = (c->hash * 0xe7009c27u) ^ sx_rr(c->hash, 18);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 20);
    t0 ^= pair_tail(c, t1);
    c->lane[8] += c->lane[0] ^ 0x5a5900ddu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0e) << 8;
    rotate_range(c, &c->lane[9], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x93) << 16;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t wrap_rate_258(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x99) << 8;
    t2 = (t2 ^ c->sum) * 0xf836292fu;
    t2 += (uint32_t)parse_token(c);
    t2 += (uint32_t)pick_tail(c);
    t2 += tap_marker(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17848u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)blend_mask(c, (uint8_t)(t0 >> 0), t2);
    emit_record(c, t0, t1);
    emit_port(c, t0, t1);
    t0 ^= pin_entry(c, t1);
    c->hash = (c->hash * 0x393fefd3u) ^ sx_rr(c->hash, 10);
    t0 ^= tap_path(c, t1);
    t0 ^= fetch_window_344(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tap_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)push_head_314(c);
    t0 ^= sort_pairing(c, t1);
    t2 += mark_unit(c, c->rlo, c->rln);
    t0 ^= clamp_marker(c, t1);
    c->lane[7] ^= sx_rl(c->lane[13], 3);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45155u) % (uint32_t)c->rln)] << 24;
    poll_mask(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xfa) << 8;
    purge_count(c, &c->lane[7], 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbba02befu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x81) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x62) << 0;
    t2 += drain_queue(c, c->slo, c->sln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x0f08f529u;
    c->hash = (c->hash * 0x9656d77du) ^ sx_rr(c->hash, 9);
    yield_pairing_322(c, &c->lane[3], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5c) << 8;
    t1 ^= (uint32_t)pack_batch(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= load_unit(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t prime_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3d) << 16;
    c->lane[12] += c->lane[13]; c->lane[3] ^= c->lane[12]; c->lane[3] = sx_rl(c->lane[3], 8);
    c->hash = (c->hash * 0xe4bafb15u) ^ sx_rr(c->hash, 26);
    t1 ^= (uint32_t)scan_bound(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] += c->lane[12]; c->lane[1] ^= c->lane[2]; c->lane[1] = sx_rl(c->lane[1], 1);
    shift_offset(c, t0, t1);
    c->sched[9] = c->hash ^ sx_rl(c->lane[0], 17);
    t2 = (t2 ^ c->sum) * 0xa9e4ff0bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45494u) % (uint32_t)c->rln)] << 0;
    t0 ^= fetch_batch(c, t1);
    t1 ^= (uint32_t)shift_ring(c, (uint8_t)(t0 >> 0), t2);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t sort_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd0) << 16;
    c->lane[13] += c->lane[4] ^ 0x903fa727u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xbeb5fedfu;
    c->sched[16] = c->hash ^ sx_rl(c->lane[5], 16);
    pack_entry(c, &c->lane[0], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1724u) % (uint32_t)c->rln)] << 24;
    c->lane[12] += c->lane[12] ^ 0xa2b914b0u;
    c->lane[4] += c->lane[7]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 21);
    purge_entry(c, t0, t1);
    c->lane[1] += c->lane[15]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 7);
    c->hash = (c->hash * 0xa00ee501u) ^ sx_rr(c->hash, 4);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void peek_level(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += reap_pool(c, c->slo, c->sln);
    c->lane[9] ^= sx_rl(c->lane[0], 11);
    tune_arena(c, t0, t1);
    t2 += queue_list(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x68) << 16;
    t1 ^= (uint32_t)pick_digest(c, (uint8_t)(t0 >> 8), t2);
    c->sched[11] = c->hash ^ sx_rl(c->lane[11], 17);
    c->raw[c->slo + (int)((t0 + 56349u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x84a54c09u;
    c->lane[13] += c->lane[15] ^ 0xb7be6656u;
    c->hash = (c->hash * 0xc1e62747u) ^ sx_rr(c->hash, 16);
    c->lane[7] += c->lane[12] ^ 0x3f8c658fu;
    t2 += (uint32_t)align_table(c);
    t2 += (uint32_t)cache_window(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t load_unit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 56482u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x613a13b5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[1] = c->hash ^ sx_rl(c->lane[10], 25);
    c->lane[15] += c->lane[2] ^ 0x084db800u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6891u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x73) << 8;
    c->lane[7] ^= sx_rl(c->lane[11], 7);
    c->lane[3] += c->lane[10]; c->lane[8] ^= c->lane[3]; c->lane[8] = sx_rl(c->lane[8], 7);
    c->lane[15] ^= sx_rl(c->lane[8], 3);
    c->raw[c->slo + (int)((t0 + 17604u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void rotate_range(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x13) << 8;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 7);
    c->lane[1] += c->lane[8] ^ 0x507814e8u;
    purge_group(c, &c->lane[0], 4);
    c->sched[20] = c->hash ^ sx_rl(c->lane[4], 16);
    t2 = (t2 ^ c->sum) * 0x5d085839u;
    poll_group(c, t0, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void poll_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    mark_region(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22608u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[13] + 0x191f48c4u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    emit_port(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9c) << 0;
    t1 ^= (uint32_t)split_layer(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x18d5e9bfu;
    resize_lease(c, t0, t1);
    t0 ^= move_tail(c, t1);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void join_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[9] += c->lane[5]; c->lane[11] ^= c->lane[9]; c->lane[11] = sx_rl(c->lane[11], 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xa5) << 8;
    c->raw[c->slo + (int)((t0 + 49172u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x64) << 8;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 11);
    c->hash = (c->hash * 0x68f9d353u) ^ sx_rr(c->hash, 25);
    t2 = (t2 ^ c->sum) * 0xddf23ef3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16370u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)stage_key(c);
    coal_batch(c, &c->lane[5], 1);
    emit_record(c, t0, t1);
    c->lane[5] += c->lane[13]; c->lane[15] ^= c->lane[5]; c->lane[15] = sx_rl(c->lane[15], 23);
    t1 ^= (uint32_t)align_level(c, (uint8_t)(t0 >> 0), t2);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 5);
    c->sched[14] = c->hash ^ sx_rl(c->lane[6], 29);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static void resize_marker(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t1 ^= (uint32_t)fill_row(c, (uint8_t)(t0 >> 0), t2);
    tune_arena(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x74d0ac61u) ^ sx_rr(c->hash, 21);
    c->raw[c->slo + (int)((t0 + 13047u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += prime_stream_283(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[9] = c->hash ^ sx_rl(c->lane[14], 18);
    peek_record_302(c, t0, t1);
    t2 += queue_list(c, c->rlo, c->rln);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t tap_marker(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5644u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[4] + 0xc969cd09u;
    c->raw[c->slo + (int)((t0 + 47773u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x20e5dc75u;
    t0 ^= cache_window_331(c, t1);
    c->lane[13] += c->lane[0]; c->lane[15] ^= c->lane[13]; c->lane[15] = sx_rl(c->lane[15], 14);
    t1 ^= (uint32_t)move_frame_278(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)parse_part(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x6225c0bfu) ^ sx_rr(c->hash, 30);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tap_path(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[13] += c->lane[2]; c->lane[0] ^= c->lane[13]; c->lane[0] = sx_rl(c->lane[0], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63711u) % (uint32_t)c->rln)] << 16;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 9);
    c->hash = (c->hash * 0xfe0f3d85u) ^ sx_rr(c->hash, 3);
    t2 += latch_port(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= c->lane[8] + 0x5b6b9cb5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe9219ce5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xfe) << 8;
    c->sched[15] = c->hash ^ sx_rl(c->lane[9], 5);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int stage_key(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    t0 ^= close_bucket(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc4) << 8;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[4] += c->lane[12]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 2);
    c->lane[14] ^= sx_rl(c->lane[2], 6);
    c->lane[9] ^= sx_rl(c->lane[5], 7);
    coal_batch(c, &c->lane[3], 2);
    t2 = (t2 ^ c->sum) * 0x7fc1a131u;
    c->hash = (c->hash * 0x30cccb55u) ^ sx_rr(c->hash, 13);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void purge_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe7997e73u;
    c->hash = (c->hash * 0x4bf520b9u) ^ sx_rr(c->hash, 26);
    c->lane[13] ^= sx_rl(c->lane[7], 18);
    place_mask(c, t0, t1);
    t2 += (uint32_t)push_head_314(c);
    c->hash ^= c->lane[10] + 0x0100aa3du;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t split_layer(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] ^= sx_rl(c->lane[7], 5);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 10);
    c->lane[5] ^= sx_rl(c->lane[14], 31);
    c->lane[12] += c->lane[1]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[5] = c->hash ^ sx_rl(c->lane[8], 1);
    c->hash ^= c->lane[13] + 0xd9c5b27du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5616554bu;
    c->lane[6] += c->lane[8]; c->lane[11] ^= c->lane[6]; c->lane[11] = sx_rl(c->lane[11], 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t reap_pool(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50251u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)align_cursor(c, (uint8_t)(t0 >> 16), t2);
    c->sched[26] = c->hash ^ sx_rl(c->lane[10], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= c->lane[1] + 0x6bc453c7u;
    c->sum += t1;
    return t0 + t2;
}

static void emit_port(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x029684cbu) ^ sx_rr(c->hash, 28);
    c->hash = (c->hash * 0x8c91ba4du) ^ sx_rr(c->hash, 26);
    split_scope_317(c, t0, t1);
    c->sched[26] = c->hash ^ sx_rl(c->lane[9], 18);
    c->raw[c->slo + (int)((t0 + 49617u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0xa6f78075u) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0x17c32f7fu) ^ sx_rr(c->hash, 1);
    c->lane[10] ^= sx_rl(c->lane[6], 26);
    c->sum ^= t0 + t1;
    c->lane[6] ^= t2;
}

static void tune_arena(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] += c->lane[5] ^ 0xca1bd73cu;
    t1 ^= (uint32_t)align_cursor(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= load_track(c, t1);
    c->lane[14] += c->lane[4] ^ 0x8c2d0a51u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[12], 8);
    c->sched[28] = c->hash ^ sx_rl(c->lane[0], 10);
    t0 ^= chain_layer(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23358u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x6fccb633u;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void emit_record(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 17054u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[9] = c->hash ^ sx_rl(c->lane[12], 30);
    t2 = (t2 ^ c->sum) * 0x50ad9251u;
    t1 ^= (uint32_t)wrap_cell(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x44b2cb5bu;
    c->sched[13] = c->hash ^ sx_rl(c->lane[9], 22);
    c->raw[c->slo + (int)((t0 + 2115u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x4f10076eu;
    c->sched[28] = c->hash ^ sx_rl(c->lane[13], 12);
    t2 = (t2 ^ c->sum) * 0x924ea9bfu;
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint8_t align_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] ^= sx_rl(c->lane[5], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63959u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[11] += c->lane[2] ^ 0xc5ad7531u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf5977c87u;
    c->lane[11] ^= sx_rl(c->lane[6], 3);
    c->hash = (c->hash * 0xb6b7eeddu) ^ sx_rr(c->hash, 7);
    t0 ^= map_item(c, t1);
    c->lane[10] += c->lane[9] ^ 0x6a33fb59u;
    c->hash ^= c->lane[2] + 0xee9c9ff7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t move_frame_278(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6e) << 0;
    c->hash ^= c->lane[4] + 0x67bf1815u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9646u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[0] + 0xc5eea195u;
    c->lane[0] ^= sx_rl(c->lane[15], 3);
    c->lane[5] ^= sx_rl(c->lane[5], 23);
    t2 += (uint32_t)emit_rate(c);
    t1 ^= (uint32_t)slice_layer(c, (uint8_t)(t0 >> 8), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t fill_row(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[0] ^= sx_rl(c->lane[3], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbb0dd903u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= c->lane[2] + 0x2f0545d0u;
    t2 = (t2 ^ c->sum) * 0x794347b5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x4ff07e47u;
    c->lane[10] += c->lane[8]; c->lane[13] ^= c->lane[10]; c->lane[13] = sx_rl(c->lane[13], 5);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t scan_bound(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= mark_item(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc5f5e7b3u;
    yield_pairing_322(c, &c->lane[6], 3);
    c->hash = (c->hash * 0x16cdbca5u) ^ sx_rr(c->hash, 18);
    c->hash ^= c->lane[13] + 0xc88dfc41u;
    c->lane[1] ^= sx_rl(c->lane[10], 19);
    peek_record_302(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t shift_ring(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[6], 14);
    c->hash = (c->hash * 0x9883ccd3u) ^ sx_rr(c->hash, 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7e) << 8;
    c->lane[10] ^= sx_rl(c->lane[0], 8);
    t1 ^= (uint32_t)tap_segment(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= load_line(c, t1);
    t2 += tally_slot(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x57529d9du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 23);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t pick_digest(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[0] += c->lane[0] ^ 0xd061268fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= close_bucket(c, t1);
    t2 = (t2 ^ c->sum) * 0x17b07dc3u;
    c->lane[13] ^= sx_rl(c->lane[14], 30);
    c->hash ^= c->lane[10] + 0x38a03e13u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x88) << 16;
    t2 += resize_lease_324(c, c->rlo, c->rln);
    t2 += (uint32_t)patch_line(c);
    t2 += (uint32_t)pin_region(c);
    c->hash ^= c->lane[13] + 0x68f27376u;
    c->hash ^= c->lane[12] + 0xb6c133a7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t prime_stream_283(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 24882u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= purge_ring_291(c, t1);
    c->sched[17] = c->hash ^ sx_rl(c->lane[0], 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t1 ^= (uint32_t)prime_ring(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 27175u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 30);
    c->lane[1] += c->lane[13]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 11);
    c->hash = (c->hash * 0x425cb8b5u) ^ sx_rr(c->hash, 1);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t parse_part(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x74) << 0;
    c->raw[c->slo + (int)((t0 + 13181u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24972u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)emit_rate(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf15b405du;
    c->lane[13] ^= sx_rl(c->lane[3], 16);
    c->sched[21] = c->hash ^ sx_rl(c->lane[5], 28);
    t2 = (t2 ^ c->sum) * 0xcbdba575u;
    c->lane[7] += c->lane[10]; c->lane[8] ^= c->lane[7]; c->lane[8] = sx_rl(c->lane[8], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x98f663c3u;
    c->lane[12] += c->lane[3]; c->lane[9] ^= c->lane[12]; c->lane[9] = sx_rl(c->lane[9], 14);
    c->lane[12] += c->lane[8]; c->lane[6] ^= c->lane[12]; c->lane[6] = sx_rl(c->lane[6], 31);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t fetch_batch(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 ^= fetch_slot(c, t1);
    mark_node(c, &c->lane[10], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28578u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34152u) % (uint32_t)c->rln)] << 24;
    split_window(c, &c->lane[0], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 7861u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)merge_region(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void resize_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x6d863c5bu) ^ sx_rr(c->hash, 4);
    c->lane[12] += c->lane[15]; c->lane[4] ^= c->lane[12]; c->lane[4] = sx_rl(c->lane[4], 4);
    shift_offset(c, t0, t1);
    t2 += defer_table(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[10] += c->lane[12]; c->lane[3] ^= c->lane[10]; c->lane[3] = sx_rl(c->lane[3], 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x4a) << 0;
    c->hash = (c->hash * 0x7347f4c1u) ^ sx_rr(c->hash, 10);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t latch_port(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 20);
    c->hash = (c->hash * 0xa1f023d9u) ^ sx_rr(c->hash, 1);
    t1 ^= (uint32_t)peek_cell(c, (uint8_t)(t0 >> 0), t2);
    c->lane[0] += c->lane[4] ^ 0x87c7ee33u;
    c->raw[c->slo + (int)((t0 + 64693u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    map_window(c, &c->lane[7], 4);
    seek_gap(c, &c->lane[3], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3724u) % (uint32_t)c->rln)] << 16;
    c->lane[11] ^= sx_rl(c->lane[1], 3);
    c->sum += t1;
    return t0 + t2;
}

static void poll_group(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[7] += c->lane[5]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 3);
    c->lane[1] += c->lane[13]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 6);
    t2 = (t2 ^ c->sum) * 0x76eee8dbu;
    t2 += (uint32_t)cache_window(c);
    t2 += (uint32_t)link_run(c);
    t1 ^= (uint32_t)stage_range(c, (uint8_t)(t0 >> 16), t2);
    t2 += wrap_marker(c, c->slo, c->sln);
    c->lane[1] += c->lane[8] ^ 0x04c947c1u;
    c->hash ^= c->lane[3] + 0x50fd8eafu;
    c->lane[6] += c->lane[5]; c->lane[15] ^= c->lane[6]; c->lane[15] = sx_rl(c->lane[15], 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 44299u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    coal_layer(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x2363637bu;
    t1 ^= (uint32_t)step_cell(c, (uint8_t)(t0 >> 8), t2);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t queue_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t1 ^= (uint32_t)pack_batch(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] += c->lane[2]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 17);
    c->raw[c->slo + (int)((t0 + 39911u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd6) << 0;
    c->sched[19] = c->hash ^ sx_rl(c->lane[2], 20);
    t2 += store_token(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    swap_tail(c, &c->lane[11], 2);
    c->lane[2] ^= sx_rl(c->lane[10], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[3] += c->lane[4] ^ 0xfef8313du;
    defer_field_303(c, &c->lane[7], 3);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t pack_batch(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 6);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xed214de5u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += (uint32_t)sift_block(c);
    c->hash = (c->hash * 0xb6f8b1e1u) ^ sx_rr(c->hash, 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[5] = c->hash ^ sx_rl(c->lane[10], 16);
    c->hash = (c->hash * 0x1fde346du) ^ sx_rr(c->hash, 14);
    c->sched[12] = c->hash ^ sx_rl(c->lane[11], 24);
    t2 += (uint32_t)fold_list(c);
    c->hash ^= c->lane[13] + 0x9b2afadcu;
    c->lane[14] += c->lane[3]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 10);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t purge_ring_291(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[1] = c->hash ^ sx_rl(c->lane[7], 19);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sched[5] = c->hash ^ sx_rl(c->lane[3], 5);
    t0 ^= mix_ring(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49919u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0xdeaba1a7u) ^ sx_rr(c->hash, 24);
    c->lane[5] += c->lane[14]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 15);
    c->raw[c->slo + (int)((t0 + 24852u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[9] + 0x2a87275eu;
    c->lane[13] += c->lane[11]; c->lane[3] ^= c->lane[13]; c->lane[3] = sx_rl(c->lane[3], 23);
    c->hash ^= c->lane[7] + 0xa126e072u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int emit_rate(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] ^= sx_rl(c->lane[14], 25);
    c->lane[11] ^= sx_rl(c->lane[0], 30);
    c->sched[17] = c->hash ^ sx_rl(c->lane[0], 24);
    c->lane[3] += c->lane[4]; c->lane[9] ^= c->lane[3]; c->lane[9] = sx_rl(c->lane[9], 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t step_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6624u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x399c4c1du) ^ sx_rr(c->hash, 28);
    sync_record(c, &c->lane[7], 3);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 23);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 15);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x16274c3bu;
    t0 ^= move_tail(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63858u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void coal_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[11] ^= sx_rl(c->lane[12], 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3190u) % (uint32_t)c->rln)] << 16;
    merge_limit(c, &c->lane[4], 2);
    c->lane[2] += c->lane[11]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 24);
    c->lane[9] += c->lane[6]; c->lane[3] ^= c->lane[9]; c->lane[3] = sx_rl(c->lane[3], 20);
    t2 += (uint32_t)relay_batch(c);
    t2 = (t2 ^ c->sum) * 0xbcefcbffu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[14] = c->hash ^ sx_rl(c->lane[15], 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16793u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x07) << 16;
    c->sched[20] = c->hash ^ sx_rl(c->lane[13], 16);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t map_item(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= slice_list(c, t1);
    c->hash ^= c->lane[8] + 0x5b21b7b3u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x34d3f60fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 14);
    t2 += relay_span_340(c, c->rlo, c->rln);
    step_tail(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t slice_layer(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] += c->lane[9] ^ 0x43655224u;
    t2 += (uint32_t)slice_item(c);
    t0 ^= fold_unit(c, t1);
    c->sched[20] = c->hash ^ sx_rl(c->lane[15], 2);
    c->lane[1] ^= sx_rl(c->lane[7], 20);
    c->raw[c->slo + (int)((t0 + 42929u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    sift_range(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t load_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] += c->lane[5]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 ^= defer_limit(c, t1);
    c->hash = (c->hash * 0x4d287c1bu) ^ sx_rr(c->hash, 1);
    t2 = (t2 ^ c->sum) * 0x574bf6d7u;
    c->lane[14] += c->lane[15] ^ 0x1c15aa78u;
    c->sched[25] = c->hash ^ sx_rl(c->lane[5], 16);
    c->hash ^= c->lane[3] + 0x96f36339u;
    t2 += (uint32_t)merge_region(c);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t wrap_marker(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[13] += c->lane[9]; c->lane[12] ^= c->lane[13]; c->lane[12] = sx_rl(c->lane[12], 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xeb) << 16;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 13);
    t0 ^= fetch_slot(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x993eab31u;
    c->lane[14] += c->lane[0] ^ 0x3709cf84u;
    place_mask(c, t0, t1);
    load_pairing(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[7] ^= sx_rl(c->lane[6], 22);
    c->raw[c->slo + (int)((t0 + 20167u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3cfc120fu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tally_slot(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x2d16ffc7u) ^ sx_rr(c->hash, 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash = (c->hash * 0xceeea227u) ^ sx_rr(c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5e61d153u;
    purge_list(c, t0, t1);
    t2 += step_span(c, c->rlo, c->rln);
    purge_region(c, &c->lane[6], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x63) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0a) << 0;
    c->sum += t1;
    return t0 + t2;
}

static void split_window(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    purge_group(c, &c->lane[3], 2);
    t2 += (uint32_t)latch_node(c);
    c->hash = (c->hash * 0xba03d437u) ^ sx_rr(c->hash, 18);
    t0 ^= pair_tail(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9462u) % (uint32_t)c->rln)] << 24;
    coal_port_373(c, t0, t1);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 12);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 28);
    t0 ^= tap_lease(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void coal_batch(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 44999u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x4edf793bu;
    c->sched[11] = c->hash ^ sx_rl(c->lane[14], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x79) << 0;
    purge_list(c, t0, t1);
    c->lane[15] ^= sx_rl(c->lane[11], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5d) << 8;
    t2 = (t2 ^ c->sum) * 0xbc1b68ddu;
    pick_layer(c, t0, t1);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 14);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void peek_record_302(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 40547u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2c7f5e15u;
    load_pairing(c, t0, t1);
    c->hash ^= c->lane[15] + 0x56b71aebu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14807u) % (uint32_t)c->rln)] << 0;
    c->sched[22] = c->hash ^ sx_rl(c->lane[13], 28);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 7);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void defer_field_303(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6a) << 8;
    t2 += pick_unit(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[15] ^= sx_rl(c->lane[9], 13);
    c->lane[6] ^= sx_rl(c->lane[5], 13);
    pack_entry(c, &c->lane[8], 1);
    t2 = (t2 ^ c->sum) * 0xcfc50275u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t wrap_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)patch_line(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[2] += c->lane[3]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb54c03d1u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8e) << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t peek_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7e0902a7u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    coal_window(c, t0, t1);
    c->lane[6] += c->lane[0]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 21);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 31);
    c->raw[c->slo + (int)((t0 + 14517u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += (uint32_t)tune_tuple(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48031u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t stage_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32148u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0xbd922241u) ^ sx_rr(c->hash, 24);
    c->sched[31] = c->hash ^ sx_rl(c->lane[4], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf605e0edu;
    t2 = (t2 ^ c->sum) * 0x2b19ad1bu;
    c->hash = (c->hash * 0xee054a55u) ^ sx_rr(c->hash, 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb0) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23485u) % (uint32_t)c->rln)] << 0;
    c->lane[10] += c->lane[15]; c->lane[13] ^= c->lane[10]; c->lane[13] = sx_rl(c->lane[13], 27);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6347u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t align_cursor(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 29);
    t2 += probe_mask(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 268u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[8] += c->lane[5] ^ 0x6f4d278fu;
    t2 += drain_queue(c, c->slo, c->sln);
    c->lane[9] ^= sx_rl(c->lane[11], 25);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void swap_tail(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xa7cd7243u) ^ sx_rr(c->hash, 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x85741cbdu;
    c->lane[7] ^= sx_rl(c->lane[3], 28);
    settle_index(c, &c->lane[7], 1);
    c->hash = (c->hash * 0xbb7078c3u) ^ sx_rr(c->hash, 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t close_bucket(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[1] + 0x8b44e641u;
    c->lane[4] += c->lane[3] ^ 0xe447b622u;
    c->lane[5] ^= sx_rl(c->lane[4], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 52247u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 ^= pin_entry(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t defer_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x35) << 16;
    c->raw[c->slo + (int)((t0 + 3169u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 10);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t mark_item(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x50ebe6a7u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5f) << 16;
    blend_head(c, &c->lane[0], 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[15] ^= sx_rl(c->lane[12], 1);
    t2 += mark_unit(c, c->slo, c->sln);
    t2 += (uint32_t)pin_region(c);
    mark_region(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t load_line(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa9) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x07) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x70) << 8;
    c->lane[13] ^= sx_rl(c->lane[3], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42872u) % (uint32_t)c->rln)] << 16;
    t0 ^= fetch_window_344(c, t1);
    c->lane[9] ^= sx_rl(c->lane[7], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34174u) % (uint32_t)c->rln)] << 8;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 7);
    chain_lease(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t tap_segment(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash ^= c->lane[11] + 0x22644f1fu;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 23);
    mark_lease(c, t0, t1);
    c->hash = (c->hash * 0x1e5a4e75u) ^ sx_rr(c->hash, 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x42) << 16;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 20);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 65323u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int push_head_314(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->lane[0] += c->lane[3]; c->lane[11] ^= c->lane[0]; c->lane[11] = sx_rl(c->lane[11], 22);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd0) << 16;
    c->raw[c->slo + (int)((t0 + 50016u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] += c->lane[11] ^ 0xa2d68b21u;
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void map_window(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[15] += c->lane[10] ^ 0xc0c2dcd6u;
    t2 += (uint32_t)align_table(c);
    t2 = (t2 ^ c->sum) * 0x6e946175u;
    t2 += store_token(c, c->rlo, c->rln);
    c->hash = (c->hash * 0xf9a21b07u) ^ sx_rr(c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 8);
    c->lane[7] += c->lane[3]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 2);
    t2 = (t2 ^ c->sum) * 0xd427604bu;
    c->raw[c->slo + (int)((t0 + 42806u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void shift_offset(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbdf10085u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x53cf3115u;
    t2 += probe_mask(c, c->slo, c->sln);
    t2 += (uint32_t)stage_cursor(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38205u) % (uint32_t)c->rln)] << 16;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    t2 += (uint32_t)queue_span(c);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static void split_scope_317(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 49685u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[27] = c->hash ^ sx_rl(c->lane[4], 20);
    t2 = (t2 ^ c->sum) * 0xc58692bbu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xdd) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28617u) % (uint32_t)c->rln)] << 0;
    t0 ^= fold_unit(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[3] += c->lane[3] ^ 0x992e4f65u;
    c->raw[c->slo + (int)((t0 + 34233u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[22] = c->hash ^ sx_rl(c->lane[6], 24);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void mark_node(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[9] + 0x14730105u;
    shift_seat(c, t0, t1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 9);
    c->lane[12] += c->lane[13] ^ 0x26ddd1c0u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[11], 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf91cc99fu;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 28);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int cache_window(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 15);
    c->lane[5] += c->lane[4] ^ 0xe2ecdde3u;
    shift_chunk(c, t0, t1);
    c->lane[8] += c->lane[2] ^ 0x4b0f8f2fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xe89978f7u;
    c->sched[23] = c->hash ^ sx_rl(c->lane[6], 27);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int link_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= c->lane[4] + 0xb74ce7eeu;
    c->hash ^= c->lane[15] + 0x6010943cu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28598u) % (uint32_t)c->rln)] << 16;
    c->lane[8] += c->lane[15]; c->lane[12] ^= c->lane[8]; c->lane[12] = sx_rl(c->lane[12], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t1 ^= (uint32_t)blend_mask(c, (uint8_t)(t0 >> 0), t2);
    purge_gap(c, t0, t1);
    seek_gap(c, &c->lane[2], 1);
    t0 ^= cache_window_331(c, t1);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t prime_ring(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3686a43fu;
    c->lane[10] += c->lane[7] ^ 0x2e67e02fu;
    c->hash = (c->hash * 0x9bbc3d45u) ^ sx_rr(c->hash, 29);
    c->lane[12] += c->lane[10]; c->lane[1] ^= c->lane[12]; c->lane[1] = sx_rl(c->lane[1], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7d) << 8;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 2);
    c->lane[12] ^= sx_rl(c->lane[10], 21);
    c->hash ^= c->lane[12] + 0x75418179u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1709u) % (uint32_t)c->rln)] << 16;
    t1 ^= (uint32_t)sync_cell_335(c, (uint8_t)(t0 >> 0), t2);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 12);
    c->raw[c->slo + (int)((t0 + 63840u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void yield_pairing_322(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += (uint32_t)yield_track(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa023eacfu;
    c->hash = (c->hash * 0x0041ea73u) ^ sx_rr(c->hash, 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe3) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x73c6592du;
    t2 = (t2 ^ c->sum) * 0xa8341043u;
    t2 = (t2 ^ c->sum) * 0x2bec0fd9u;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 31);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 11);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t chain_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x72ee3c53u;
    t2 += (uint32_t)parse_token(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)pick_tail(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26775u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 6);
    c->lane[15] += c->lane[7]; c->lane[12] ^= c->lane[15]; c->lane[12] = sx_rl(c->lane[12], 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t resize_lease_324(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x804f8a81u;
    c->lane[9] += c->lane[13] ^ 0x53ff80b2u;
    c->lane[9] += c->lane[2] ^ 0x51ad6257u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21107u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 7246u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= clamp_marker(c, t1);
    c->lane[0] ^= sx_rl(c->lane[13], 12);
    purge_count(c, &c->lane[5], 4);
    c->raw[c->slo + (int)((t0 + 65208u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum += t1;
    return t0 + t2;
}

static int yield_track(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[9];
    c->raw[c->slo + (int)((t0 + 56393u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[9] ^= sx_rl(c->lane[11], 19);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 12);
    c->raw[c->slo + (int)((t0 + 42600u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x2d751b89u) ^ sx_rr(c->hash, 11);
    c->hash = (c->hash * 0x1cc09db9u) ^ sx_rr(c->hash, 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] += c->lane[3] ^ 0xdac24598u;
    c->lane[4] += c->lane[15]; c->lane[14] ^= c->lane[4]; c->lane[14] = sx_rl(c->lane[14], 12);
    c->sched[10] = c->hash ^ sx_rl(c->lane[14], 14);
    c->lane[9] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sync_record(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[30] = c->hash ^ sx_rl(c->lane[9], 14);
    c->raw[c->slo + (int)((t0 + 26159u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[13] + 0x42ae5889u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[6] ^= sx_rl(c->lane[13], 20);
    c->hash = (c->hash * 0x91ff0ac7u) ^ sx_rr(c->hash, 28);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int merge_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->raw[c->slo + (int)((t0 + 43325u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6b0a7267u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35452u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3e) << 16;
    t2 = (t2 ^ c->sum) * 0xda6d4049u;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t probe_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 41395u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3194eb95u;
    t2 = (t2 ^ c->sum) * 0xc25362afu;
    t2 = (t2 ^ c->sum) * 0xad072f8fu;
    c->sched[29] = c->hash ^ sx_rl(c->lane[3], 9);
    c->sched[10] = c->hash ^ sx_rl(c->lane[1], 21);
    c->lane[12] += c->lane[8]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 14);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x15) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb4bdf123u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t defer_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 16);
    c->hash = (c->hash * 0x65c2c521u) ^ sx_rr(c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x7c6a951bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10556u) % (uint32_t)c->rln)] << 0;
    c->lane[5] += c->lane[1] ^ 0x5bbee658u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void step_tail(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4c) << 0;
    c->lane[13] ^= sx_rl(c->lane[10], 28);
    c->hash = (c->hash * 0xee44fd5du) ^ sx_rr(c->hash, 5);
    c->raw[c->slo + (int)((t0 + 46307u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52492u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[6] + 0x5c7becb1u;
    c->sched[3] = c->hash ^ sx_rl(c->lane[2], 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x49d708f5u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t cache_window_331(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[4] += c->lane[14] ^ 0xfed24adau;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x05e2a185u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[14]; c->lane[6] ^= c->lane[5]; c->lane[6] = sx_rl(c->lane[6], 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa9da812bu;
    c->raw[c->slo + (int)((t0 + 65297u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[5] += c->lane[7] ^ 0xd3a14cd8u;
    c->lane[4] ^= sx_rl(c->lane[1], 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xba841b6bu;
    c->sched[26] = c->hash ^ sx_rl(c->lane[12], 15);
    c->sched[3] = c->hash ^ sx_rl(c->lane[2], 1);
    c->lane[0] += c->lane[14]; c->lane[6] ^= c->lane[0]; c->lane[6] = sx_rl(c->lane[6], 1);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 30);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void blend_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[3]; c->lane[9] ^= c->lane[5]; c->lane[9] = sx_rl(c->lane[9], 28);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4086u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[0] + 0x2e057f10u;
    c->lane[1] += c->lane[10]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 6);
    t2 = (t2 ^ c->sum) * 0xac532121u;
    c->hash ^= c->lane[0] + 0x4b681102u;
    c->lane[14] += c->lane[4]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t mix_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[12] + 0x660f8b4cu;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 15);
    c->lane[8] ^= sx_rl(c->lane[12], 4);
    c->lane[1] += c->lane[0] ^ 0x044e9e30u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[13] += c->lane[10] ^ 0xa638c09au;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5b) << 0;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 17);
    c->raw[c->slo + (int)((t0 + 33421u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t sync_cell_335(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xaf2f8421u;
    c->lane[8] += c->lane[3]; c->lane[1] ^= c->lane[8]; c->lane[1] = sx_rl(c->lane[1], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44966u) % (uint32_t)c->rln)] << 24;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcc) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x7d4e9669u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xff) << 0;
    c->lane[11] ^= sx_rl(c->lane[11], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int queue_span(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->lane[5] ^= sx_rl(c->lane[7], 30);
    c->hash ^= c->lane[4] + 0xf4b2d864u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 10);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 31);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void load_pairing(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[14] = c->hash ^ sx_rl(c->lane[10], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x133db5cbu;
    t2 = (t2 ^ c->sum) * 0x3779d299u;
    c->sched[4] = c->hash ^ sx_rl(c->lane[2], 10);
    c->raw[c->slo + (int)((t0 + 19056u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x6eb1ab5fu) ^ sx_rr(c->hash, 17);
    c->lane[10] += c->lane[14] ^ 0x498310c9u;
    c->sched[5] = c->hash ^ sx_rl(c->lane[12], 4);
    c->hash = (c->hash * 0x3dff35ffu) ^ sx_rr(c->hash, 29);
    c->lane[7] += c->lane[15] ^ 0x9a438f7bu;
    c->lane[12] ^= sx_rl(c->lane[7], 4);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t pick_unit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[7] = c->hash ^ sx_rl(c->lane[5], 16);
    c->lane[12] += c->lane[13] ^ 0x7e1d73bbu;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[6] += c->lane[8]; c->lane[13] ^= c->lane[6]; c->lane[13] = sx_rl(c->lane[13], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37541u) % (uint32_t)c->rln)] << 16;
    c->lane[8] += c->lane[9]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 22);
    c->hash = (c->hash * 0xa185d92du) ^ sx_rr(c->hash, 30);
    c->hash ^= c->lane[14] + 0x34876b88u;
    c->raw[c->slo + (int)((t0 + 47061u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum += t1;
    return t0 + t2;
}

static void chain_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] += c->lane[11] ^ 0xb6608019u;
    c->hash ^= c->lane[13] + 0xbbc87115u;
    c->lane[14] += c->lane[11] ^ 0x6223bb02u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x83d58dfbu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[4] ^= sx_rl(c->lane[1], 7);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t relay_span_340(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[10] ^= sx_rl(c->lane[14], 7);
    c->lane[6] += c->lane[13] ^ 0x3d1b3befu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t move_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[3] + 0x061b63a4u;
    c->raw[c->slo + (int)((t0 + 42752u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[3] ^= sx_rl(c->lane[0], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfaf7c5edu;
    c->lane[7] ^= sx_rl(c->lane[2], 14);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void purge_group(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x53) << 8;
    c->raw[c->slo + (int)((t0 + 60556u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x3a298eb1u) ^ sx_rr(c->hash, 6);
    c->sched[29] = c->hash ^ sx_rl(c->lane[0], 2);
    c->lane[4] ^= sx_rl(c->lane[1], 27);
    c->lane[15] += c->lane[11] ^ 0x98f66016u;
    c->lane[10] += c->lane[1] ^ 0xc3dba975u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd2) << 0;
    t2 = (t2 ^ c->sum) * 0xa35a9857u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pin_entry(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 26080u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x946cd5bbu) ^ sx_rr(c->hash, 26);
    c->lane[0] ^= sx_rl(c->lane[12], 8);
    c->raw[c->slo + (int)((t0 + 21971u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41345u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fetch_window_344(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[5] + 0x02b8bb3bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x81) << 8;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 10);
    t2 = (t2 ^ c->sum) * 0xa768310bu;
    c->lane[13] += c->lane[6]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 7);
    c->lane[11] += c->lane[10] ^ 0x376b8d4au;
    c->lane[12] += c->lane[8]; c->lane[4] ^= c->lane[12]; c->lane[4] = sx_rl(c->lane[4], 18);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fold_unit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 47040u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[5] ^ 0x5db98841u;
    c->lane[13] ^= sx_rl(c->lane[14], 31);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3b059627u;
    c->hash = (c->hash * 0xffccf77du) ^ sx_rr(c->hash, 23);
    c->sched[30] = c->hash ^ sx_rl(c->lane[12], 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x9e) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void seek_gap(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x87c945c3u;
    c->hash = (c->hash * 0x73980b67u) ^ sx_rr(c->hash, 29);
    c->hash ^= c->lane[7] + 0x0d32f639u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x641f5b9fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t store_token(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x83c8ccb7u;
    c->sched[23] = c->hash ^ sx_rl(c->lane[3], 19);
    c->lane[5] += c->lane[6] ^ 0xff930480u;
    t2 = (t2 ^ c->sum) * 0x768da629u;
    c->sum += t1;
    return t0 + t2;
}

static void pack_entry(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[6] + 0x3cfa4055u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44151u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe0a97b8bu;
    c->raw[c->slo + (int)((t0 + 45261u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t tap_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x25) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10535u) % (uint32_t)c->rln)] << 16;
    c->lane[7] ^= sx_rl(c->lane[3], 8);
    c->lane[8] += c->lane[10]; c->lane[7] ^= c->lane[8]; c->lane[7] = sx_rl(c->lane[7], 22);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45030u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x5fb87289u;
    c->raw[c->slo + (int)((t0 + 47097u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[14] ^= sx_rl(c->lane[10], 23);
    c->lane[14] += c->lane[0]; c->lane[11] ^= c->lane[14]; c->lane[11] = sx_rl(c->lane[11], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xbd) << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int sift_block(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[8] ^= sx_rl(c->lane[2], 30);
    c->lane[0] += c->lane[4] ^ 0xf90fc69fu;
    c->lane[7] += c->lane[1] ^ 0xd3736c62u;
    c->hash ^= c->lane[6] + 0xf9e09d87u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x303a46b7u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 16);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t fetch_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[17] = c->hash ^ sx_rl(c->lane[6], 11);
    c->lane[2] += c->lane[10]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 2);
    c->lane[15] += c->lane[14]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29098u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0xb8ddf069u) ^ sx_rr(c->hash, 15);
    c->lane[2] ^= sx_rl(c->lane[11], 21);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int slice_item(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x49) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xfb) << 16;
    c->lane[1] += c->lane[7]; c->lane[5] ^= c->lane[1]; c->lane[5] = sx_rl(c->lane[5], 29);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void shift_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[0] += c->lane[10]; c->lane[3] ^= c->lane[0]; c->lane[3] = sx_rl(c->lane[3], 10);
    t2 = (t2 ^ c->sum) * 0xe61b0645u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 23);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int latch_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x9791f691u) ^ sx_rr(c->hash, 4);
    c->sched[25] = c->hash ^ sx_rl(c->lane[4], 3);
    c->hash = (c->hash * 0x35f5c9e7u) ^ sx_rr(c->hash, 1);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 24);
    c->lane[10] += c->lane[4]; c->lane[7] ^= c->lane[10]; c->lane[7] = sx_rl(c->lane[7], 1);
    c->lane[7] ^= sx_rl(c->lane[4], 29);
    t2 = (t2 ^ c->sum) * 0x33b288adu;
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void pick_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x4d4188a9u) ^ sx_rr(c->hash, 5);
    c->sched[2] = c->hash ^ sx_rl(c->lane[14], 28);
    c->lane[12] += c->lane[10]; c->lane[4] ^= c->lane[12]; c->lane[4] = sx_rl(c->lane[4], 16);
    t2 += yield_queue(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 46012u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[17] = c->hash ^ sx_rl(c->lane[14], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47841u) % (uint32_t)c->rln)] << 0;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static int parse_token(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->lane[8] ^= sx_rl(c->lane[13], 2);
    c->sched[0] = c->hash ^ sx_rl(c->lane[1], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[13] += c->lane[14]; c->lane[1] ^= c->lane[13]; c->lane[1] = sx_rl(c->lane[1], 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54467u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x752d1133u) ^ sx_rr(c->hash, 20);
    c->lane[9] ^= sx_rl(c->lane[10], 17);
    c->lane[2] += c->lane[14]; c->lane[9] ^= c->lane[2]; c->lane[9] = sx_rl(c->lane[9], 23);
    c->lane[2] += c->lane[9] ^ 0x2ad78ea2u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3bb81965u;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int patch_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] += c->lane[2]; c->lane[10] ^= c->lane[14]; c->lane[10] = sx_rl(c->lane[10], 22);
    t2 = (t2 ^ c->sum) * 0x380a4d1fu;
    c->hash ^= c->lane[6] + 0x706e3e76u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sched[22] = c->hash ^ sx_rl(c->lane[6], 24);
    c->lane[15] += c->lane[9] ^ 0xdf5b3c82u;
    c->hash = (c->hash * 0x86fcb257u) ^ sx_rr(c->hash, 27);
    c->lane[3] += c->lane[11] ^ 0xb8691388u;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int tune_tuple(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcdb52931u;
    c->raw[c->slo + (int)((t0 + 25141u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[12] += c->lane[4]; c->lane[11] ^= c->lane[12]; c->lane[11] = sx_rl(c->lane[11], 6);
    c->sched[21] = c->hash ^ sx_rl(c->lane[11], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[10] += c->lane[0]; c->lane[11] ^= c->lane[10]; c->lane[11] = sx_rl(c->lane[11], 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[1] + 0x7e898f34u;
    c->sched[30] = c->hash ^ sx_rl(c->lane[15], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int relay_batch(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= c->lane[14] + 0x0f0a540cu;
    c->raw[c->slo + (int)((t0 + 3575u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x090eb45du;
    c->lane[14] += c->lane[8] ^ 0x4ed240a4u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void sift_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x008bcc65u;
    c->lane[7] ^= sx_rl(c->lane[7], 24);
    c->lane[11] += c->lane[1]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 12);
    c->hash ^= c->lane[11] + 0xe8f417b1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static int align_table(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->lane[13] ^= sx_rl(c->lane[5], 13);
    c->lane[8] += c->lane[6] ^ 0xc35c9dc8u;
    c->hash = (c->hash * 0xe526a043u) ^ sx_rr(c->hash, 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52377u) % (uint32_t)c->rln)] << 0;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void place_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[8] += c->lane[9] ^ 0xbb4def10u;
    c->lane[7] ^= sx_rl(c->lane[4], 28);
    c->lane[4] ^= sx_rl(c->lane[2], 3);
    c->lane[1] ^= sx_rl(c->lane[5], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0f) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x8a) << 8;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static void purge_list(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[8] += c->lane[9] ^ 0x009031f4u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0xf868ddf7u) ^ sx_rr(c->hash, 7);
    c->lane[14] += c->lane[9]; c->lane[13] ^= c->lane[14]; c->lane[13] = sx_rl(c->lane[13], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xfa549e2du) ^ sx_rr(c->hash, 12);
    c->lane[2] ^= sx_rl(c->lane[11], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static int stage_cursor(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->sched[1] = c->hash ^ sx_rl(c->lane[12], 24);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 8);
    c->sched[18] = c->hash ^ sx_rl(c->lane[1], 19);
    c->hash ^= c->lane[4] + 0x0b585f8bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf1364527u;
    c->lane[15] += c->lane[13]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xa87b7717u;
    c->hash = (c->hash * 0x1fd865a5u) ^ sx_rr(c->hash, 26);
    c->hash = (c->hash * 0xe192f4e1u) ^ sx_rr(c->hash, 10);
    c->lane[2] += c->lane[15] ^ 0xf98d6879u;
    c->lane[1] += c->lane[13] ^ 0xf386b4dau;
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t drain_queue(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[2] ^ 0x45497320u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x44) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x21) << 8;
    c->lane[7] += c->lane[10] ^ 0x03b15b17u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 28);
    c->raw[c->slo + (int)((t0 + 59502u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 3);
    c->lane[14] ^= sx_rl(c->lane[5], 10);
    c->sum += t1;
    return t0 + t2;
}

static void settle_index(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xbbfbb6a9u) ^ sx_rr(c->hash, 25);
    c->lane[0] ^= sx_rl(c->lane[14], 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34967u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0xe64a9e57u) ^ sx_rr(c->hash, 7);
    c->lane[11] += c->lane[7] ^ 0x4bc8bc9fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60968u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x917f839fu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void purge_region(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd7) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 32610u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 46275u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[3] + 0x5a5a3615u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 25);
    c->lane[10] ^= sx_rl(c->lane[14], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t step_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 13647u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xeda9c66du;
    c->lane[14] += c->lane[15]; c->lane[8] ^= c->lane[14]; c->lane[8] = sx_rl(c->lane[8], 9);
    c->hash = (c->hash * 0xa04316cbu) ^ sx_rr(c->hash, 1);
    c->hash ^= c->lane[14] + 0xf9fbd397u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t slice_list(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29222u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x0875dee7u;
    c->raw[c->slo + (int)((t0 + 4321u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[24] = c->hash ^ sx_rl(c->lane[11], 28);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void mark_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[10], 28);
    c->lane[3] ^= sx_rl(c->lane[2], 28);
    c->hash = (c->hash * 0x2573af1du) ^ sx_rr(c->hash, 23);
    c->lane[13] ^= sx_rl(c->lane[7], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[15] + 0x16611b67u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[14] ^= sx_rl(c->lane[9], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48033u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x25) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x022c4149u;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static int pin_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->lane[6] += c->lane[10]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 14);
    c->hash ^= c->lane[15] + 0xc992db86u;
    c->lane[8] += c->lane[11]; c->lane[1] ^= c->lane[8]; c->lane[1] = sx_rl(c->lane[1], 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3d) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12848u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 37245u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[5] += c->lane[13] ^ 0x657f0ea6u;
    c->hash ^= c->lane[3] + 0x3a4f0972u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 28);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int fold_list(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->lane[9] += c->lane[15] ^ 0xaf9197cbu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->lane[14] ^= sx_rl(c->lane[2], 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x6afa6c29u) ^ sx_rr(c->hash, 11);
    c->hash ^= c->lane[4] + 0xa31ec8b2u;
    c->hash = (c->hash * 0x77172683u) ^ sx_rr(c->hash, 22);
    c->raw[c->slo + (int)((t0 + 24185u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void coal_port_373(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 30822u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 28);
    c->lane[2] += c->lane[0]; c->lane[13] ^= c->lane[2]; c->lane[13] = sx_rl(c->lane[13], 29);
    c->hash ^= c->lane[13] + 0xdefb7e52u;
    c->lane[12] += c->lane[12] ^ 0xd805e00fu;
    c->lane[10] ^= sx_rl(c->lane[8], 8);
    t2 = (t2 ^ c->sum) * 0xcc7a9ac7u;
    t2 = (t2 ^ c->sum) * 0x456dad77u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void shift_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[5] ^= sx_rl(c->lane[3], 24);
    c->hash ^= c->lane[13] + 0x58960f96u;
    close_value(c, &c->lane[4], 3);
    t2 = (t2 ^ c->sum) * 0x83d22b79u;
    c->lane[7] ^= sx_rl(c->lane[9], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xae) << 0;
    c->sched[10] = c->hash ^ sx_rl(c->lane[5], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x680048ffu;
    c->lane[3] += c->lane[15]; c->lane[7] ^= c->lane[3]; c->lane[7] = sx_rl(c->lane[7], 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0f) << 16;
    c->raw[c->slo + (int)((t0 + 18541u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void purge_gap(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6855u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd37513e7u;
    c->lane[2] += c->lane[3]; c->lane[1] ^= c->lane[2]; c->lane[1] = sx_rl(c->lane[1], 25);
    c->hash ^= c->lane[10] + 0x14cfd7b9u;
    t2 = (t2 ^ c->sum) * 0x2232b0e3u;
    c->lane[8] += c->lane[9] ^ 0xa23c49acu;
    t2 = (t2 ^ c->sum) * 0x203c5f09u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x8f) << 0;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int pick_tail(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x05) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xb8) << 16;
    t2 = (t2 ^ c->sum) * 0x8f5c6ac3u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 11);
    c->raw[c->slo + (int)((t0 + 927u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x0d09a87du;
    t2 = (t2 ^ c->sum) * 0xc8c45e43u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x1fbc8e5fu) ^ sx_rr(c->hash, 6);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t clamp_marker(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xe3558a4bu) ^ sx_rr(c->hash, 31);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x0f) << 8;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 23);
    c->raw[c->slo + (int)((t0 + 48728u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void mark_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x96994fbbu;
    t2 = (t2 ^ c->sum) * 0x0402dd99u;
    c->lane[10] += c->lane[15] ^ 0xb71e8755u;
    c->lane[4] += c->lane[10]; c->lane[9] ^= c->lane[4]; c->lane[9] = sx_rl(c->lane[9], 23);
    c->sched[25] = c->hash ^ sx_rl(c->lane[11], 31);
    c->lane[2] ^= sx_rl(c->lane[11], 6);
    c->sched[18] = c->hash ^ sx_rl(c->lane[10], 14);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t pair_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xd6) << 8;
    c->sched[23] = c->hash ^ sx_rl(c->lane[5], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x58a7c02fu;
    c->sched[30] = c->hash ^ sx_rl(c->lane[0], 13);
    c->hash = (c->hash * 0x58de26e5u) ^ sx_rr(c->hash, 25);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t mark_unit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39623u) % (uint32_t)c->rln)] << 8;
    c->lane[2] ^= sx_rl(c->lane[13], 29);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe5edd219u;
    c->hash ^= c->lane[9] + 0xa83117d7u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x14f0066bu;
    c->sum += t1;
    return t0 + t2;
}

static void purge_count(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0xbf76ded1u) ^ sx_rr(c->hash, 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18582u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[8], 13);
    c->lane[1] += c->lane[15]; c->lane[10] ^= c->lane[1]; c->lane[10] = sx_rl(c->lane[10], 9);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x07) << 8;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x72) << 0;
    c->raw[c->slo + (int)((t0 + 55579u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[0] += c->lane[7]; c->lane[14] ^= c->lane[0]; c->lane[14] = sx_rl(c->lane[14], 22);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t blend_mask(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xd3201f17u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x09) << 0;
    c->hash ^= c->lane[12] + 0xbc0af7b1u;
    c->lane[1] ^= sx_rl(c->lane[7], 12);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void coal_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[7] + 0xa10cde54u;
    c->hash ^= c->lane[4] + 0x04fac1c0u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfc8dee19u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x4f) << 0;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static void pack_index(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    parse_count(c, t0, t1);
    purge_count_386(c, &c->lane[0], 4);
    t2 += clamp_stack(c, c->slo, c->sln);
    t0 ^= join_unit_494(c, t1);
    t1 ^= (uint32_t)grow_field(c, (uint8_t)(t0 >> 0), t2);
    t0 ^= resize_delta(c, t1);
    c->lane[7] += c->lane[6] ^ 0x240e69d8u;
    t2 += grow_delta(c, c->rlo, c->rln);
    c->hash ^= c->lane[2] + 0x35185f57u;
    pick_ring(c, t0, t1);
    t1 ^= (uint32_t)trim_item(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[1] + 0x5c5705d7u;
    move_offset(c, t0, t1);
    t2 += clamp_state(c, c->slo, c->sln);
    mark_lease_422(c, &c->lane[1], 4);
    t0 ^= fetch_batch_428(c, t1);
    c->lane[11] += c->lane[1]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 28);
    c->lane[3] ^= sx_rl(c->lane[9], 9);
    t0 ^= move_ring(c, t1);
    t2 += hold_part(c, c->slo, c->sln);
    t0 ^= swap_offset(c, t1);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint8_t grow_field(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[7] ^ 0xc391590fu;
    mark_track(c, t0, t1);
    relay_stack(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 18074u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t1 ^= (uint32_t)tune_store(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= sync_node(c, t1);
    c->sched[14] = c->hash ^ sx_rl(c->lane[11], 20);
    c->lane[14] += c->lane[13]; c->lane[1] ^= c->lane[14]; c->lane[1] = sx_rl(c->lane[1], 5);
    c->lane[7] += c->lane[15] ^ 0x0b9489c4u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[9] + 0x6484418eu;
    c->lane[4] += c->lane[11]; c->lane[7] ^= c->lane[4]; c->lane[7] = sx_rl(c->lane[7], 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x41e80733u;
    c->hash ^= c->lane[0] + 0x6245ee6du;
    t1 ^= (uint32_t)store_stream_391(c, (uint8_t)(t0 >> 8), t2);
    peek_run(c, t0, t1);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 22);
    close_seat(c, &c->lane[6], 2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void purge_count_386(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[18] = c->hash ^ sx_rl(c->lane[5], 11);
    settle_region(c, &c->lane[5], 4);
    stage_region(c, t0, t1);
    t0 ^= split_frame(c, t1);
    map_seat(c, &c->lane[9], 3);
    sync_node_410(c, &c->lane[8], 4);
    c->lane[0] ^= sx_rl(c->lane[1], 24);
    t0 ^= hold_page(c, t1);
    t0 ^= pin_label_395(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[4] += c->lane[4] ^ 0x58b7fc86u;
    t0 ^= chain_batch(c, t1);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 27);
    close_value(c, &c->lane[4], 3);
    c->hash ^= c->lane[12] + 0xc7d30296u;
    settle_cursor(c, &c->lane[10], 3);
    drain_track(c, &c->lane[6], 3);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t resize_delta(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    tune_mask(c, t0, t1);
    slice_entry(c, t0, t1);
    t1 ^= (uint32_t)close_slot(c, (uint8_t)(t0 >> 16), t2);
    t2 += (uint32_t)settle_range(c);
    c->hash ^= c->lane[11] + 0x1fa2d279u;
    c->raw[c->slo + (int)((t0 + 59797u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x472d78adu;
    t1 ^= (uint32_t)settle_head(c, (uint8_t)(t0 >> 16), t2);
    join_marker(c, t0, t1);
    t0 ^= fill_slot(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= c->lane[4] + 0x6a71621cu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void stage_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 15);
    latch_token(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 48453u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += trim_mask_439(c, c->rlo, c->rln);
    load_key(c, t0, t1);
    c->lane[1] ^= sx_rl(c->lane[4], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x62) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x50) << 16;
    t1 ^= (uint32_t)mark_scope(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xaef5198fu;
    c->lane[9] ^= sx_rl(c->lane[6], 13);
    t2 += (uint32_t)push_digest(c);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static void peek_run(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= move_ring(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)settle_range(c);
    c->raw[c->slo + (int)((t0 + 60851u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[6] += c->lane[10] ^ 0x9b57f990u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x77fb846bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29487u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x6f8d80adu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t1 ^= (uint32_t)shift_line(c, (uint8_t)(t0 >> 0), t2);
    c->lane[9] += c->lane[12]; c->lane[10] ^= c->lane[9]; c->lane[10] = sx_rl(c->lane[10], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc463e851u;
    t0 ^= align_scope(c, t1);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static void relay_stack(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 5);
    t1 ^= (uint32_t)place_bound(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)mark_scope(c, (uint8_t)(t0 >> 16), t2);
    t0 ^= map_block(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x61) << 8;
    c->lane[13] ^= sx_rl(c->lane[9], 24);
    map_seat(c, &c->lane[4], 4);
    c->lane[14] += c->lane[3] ^ 0x04441210u;
    t2 += purge_run(c, c->rlo, c->rln);
    c->lane[13] ^= sx_rl(c->lane[15], 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63404u) % (uint32_t)c->rln)] << 16;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t store_stream_391(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] += c->lane[13]; c->lane[9] ^= c->lane[10]; c->lane[9] = sx_rl(c->lane[9], 22);
    c->hash ^= c->lane[8] + 0x861e8c36u;
    c->lane[4] += c->lane[11]; c->lane[14] ^= c->lane[4]; c->lane[14] = sx_rl(c->lane[14], 14);
    t2 += trim_table(c, c->rlo, c->rln);
    t2 += (uint32_t)chain_line(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 += (uint32_t)place_store(c);
    t2 += hold_part(c, c->rlo, c->rln);
    c->hash ^= c->lane[12] + 0x3f162890u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void settle_cursor(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    load_key(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61401u) % (uint32_t)c->rln)] << 24;
    c->lane[15] ^= sx_rl(c->lane[9], 2);
    c->lane[12] ^= sx_rl(c->lane[15], 21);
    t1 ^= (uint32_t)drain_table(c, (uint8_t)(t0 >> 16), t2);
    c->lane[13] ^= sx_rl(c->lane[10], 23);
    c->lane[3] += c->lane[5] ^ 0x92aa5fd2u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    slice_view(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t settle_head(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= hold_page(c, t1);
    c->lane[5] += c->lane[7]; c->lane[12] ^= c->lane[5]; c->lane[12] = sx_rl(c->lane[12], 22);
    yield_region(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 64108u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= flush_gap(c, t1);
    c->lane[12] += c->lane[9] ^ 0xbc8fb07au;
    c->lane[15] ^= sx_rl(c->lane[10], 28);
    t2 += (uint32_t)push_digest(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t sync_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xeb) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6483a26fu;
    t0 ^= rotate_store(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2e1eff69u;
    t1 ^= (uint32_t)shift_line(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x2243a1c3u;
    t0 ^= patch_rate(c, t1);
    c->lane[13] += c->lane[0] ^ 0x7efb3f7eu;
    t2 = (t2 ^ c->sum) * 0x7849ac85u;
    t1 ^= (uint32_t)stage_list(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pin_label_395(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x47c51973u;
    trim_segment(c, &c->lane[9], 1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63850u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x6929e743u) ^ sx_rr(c->hash, 6);
    t0 ^= align_scope(c, t1);
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void slice_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    sync_node_410(c, &c->lane[6], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x23f01c7bu;
    t2 += stage_offset(c, c->rlo, c->rln);
    t2 += purge_run(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xd1aed2abu;
    c->hash ^= c->lane[11] + 0x1f6815b4u;
    c->lane[12] ^= sx_rl(c->lane[4], 30);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static void tune_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 39804u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 23560u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x4167523bu) ^ sx_rr(c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 3);
    yield_region(c, t0, t1);
    t2 += grow_delta(c, c->rlo, c->rln);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint8_t stage_list(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 27);
    c->sched[29] = c->hash ^ sx_rl(c->lane[14], 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe2) << 16;
    c->lane[7] += c->lane[5]; c->lane[14] ^= c->lane[7]; c->lane[14] = sx_rl(c->lane[14], 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= c->lane[13] + 0x28106c4bu;
    c->lane[6] += c->lane[11]; c->lane[2] ^= c->lane[6]; c->lane[2] = sx_rl(c->lane[2], 27);
    t2 = (t2 ^ c->sum) * 0xb24d9f15u;
    t2 += patch_record(c, c->slo, c->sln);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 19);
    c->sched[19] = c->hash ^ sx_rl(c->lane[14], 12);
    t0 ^= split_frame(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1865487du;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t move_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 51758u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[4] += c->lane[11]; c->lane[5] ^= c->lane[4]; c->lane[5] = sx_rl(c->lane[5], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 55293u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 ^= fill_slot(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6fb0fa83u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 17);
    c->hash ^= c->lane[5] + 0x20ace1ddu;
    c->sched[5] = c->hash ^ sx_rl(c->lane[4], 12);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void load_key(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] += c->lane[13]; c->lane[5] ^= c->lane[6]; c->lane[5] = sx_rl(c->lane[5], 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50487u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x9cf734ebu;
    c->hash ^= c->lane[6] + 0xf9cc41e9u;
    t1 ^= (uint32_t)trim_pool(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7946e5cbu;
    c->raw[c->slo + (int)((t0 + 14142u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] += c->lane[1]; c->lane[10] ^= c->lane[7]; c->lane[10] = sx_rl(c->lane[10], 29);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint32_t purge_run(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[12] + 0xe6482be0u;
    t0 ^= stage_block_497(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xab) << 0;
    c->lane[11] += c->lane[13] ^ 0x16d226f9u;
    c->lane[4] ^= sx_rl(c->lane[0], 27);
    c->sched[4] = c->hash ^ sx_rl(c->lane[15], 25);
    c->lane[9] ^= sx_rl(c->lane[4], 31);
    t1 ^= (uint32_t)place_lease(c, (uint8_t)(t0 >> 8), t2);
    t2 += map_range(c, c->slo, c->sln);
    t2 += trim_table(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x3f8d841du;
    c->lane[15] += c->lane[9]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 27);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t map_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 19686u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7c) << 16;
    c->lane[7] ^= sx_rl(c->lane[7], 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x155fd745u) ^ sx_rr(c->hash, 19);
    c->lane[13] += c->lane[14] ^ 0x100652c2u;
    c->lane[8] += c->lane[0]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 15);
    c->hash ^= c->lane[9] + 0x12d60e5cu;
    t1 ^= (uint32_t)fold_segment(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t grow_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41245u) % (uint32_t)c->rln)] << 0;
    mark_track(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54690u) % (uint32_t)c->rln)] << 8;
    c->lane[8] ^= sx_rl(c->lane[1], 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22503u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[10] += c->lane[15]; c->lane[1] ^= c->lane[10]; c->lane[1] = sx_rl(c->lane[1], 29);
    c->sum += t1;
    return t0 + t2;
}

static int settle_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xeb) << 16;
    c->hash ^= c->lane[10] + 0xa93a003cu;
    c->raw[c->slo + (int)((t0 + 61664u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 ^= swap_offset(c, t1);
    c->hash ^= c->lane[10] + 0x0a2f6c67u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[3] + 0xe05da523u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xcd) << 16;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t mark_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    cache_layer(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xac) << 16;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 1);
    drain_cell(c, &c->lane[6], 4);
    t2 = (t2 ^ c->sum) * 0x762b2153u;
    c->hash ^= c->lane[2] + 0xa1489ef5u;
    wrap_pool(c, t0, t1);
    c->lane[14] += c->lane[8]; c->lane[1] ^= c->lane[14]; c->lane[1] = sx_rl(c->lane[1], 31);
    c->raw[c->slo + (int)((t0 + 61807u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t align_scope(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[14] + 0x26c05157u;
    chain_scope(c, &c->lane[5], 4);
    mark_lease_422(c, &c->lane[5], 4);
    t2 += (uint32_t)cache_list(c);
    t0 ^= chain_batch_446(c, t1);
    c->hash = (c->hash * 0x9aa501edu) ^ sx_rr(c->hash, 2);
    c->lane[1] += c->lane[13]; c->lane[10] ^= c->lane[1]; c->lane[10] = sx_rl(c->lane[10], 24);
    c->lane[7] ^= sx_rl(c->lane[13], 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t hold_page(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[7] = c->hash ^ sx_rl(c->lane[14], 2);
    c->hash ^= c->lane[12] + 0x56691098u;
    t2 += store_path(c, c->rlo, c->rln);
    t1 ^= (uint32_t)trim_pool(c, (uint8_t)(t0 >> 8), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xacf414b3u;
    c->hash = (c->hash * 0x44aeea9bu) ^ sx_rr(c->hash, 1);
    c->lane[3] += c->lane[5]; c->lane[6] ^= c->lane[3]; c->lane[6] = sx_rl(c->lane[6], 21);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t place_bound(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    cache_layer(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61384u) % (uint32_t)c->rln)] << 16;
    c->lane[2] += c->lane[10] ^ 0x1e3b3897u;
    c->lane[8] += c->lane[12]; c->lane[9] ^= c->lane[8]; c->lane[9] = sx_rl(c->lane[9], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x92) << 16;
    t2 = (t2 ^ c->sum) * 0x2e740f51u;
    wrap_pool(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t1 ^= (uint32_t)tally_stack(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= c->lane[0] + 0xdcee608au;
    t2 = (t2 ^ c->sum) * 0x55b67d31u;
    c->lane[5] ^= sx_rl(c->lane[6], 3);
    c->hash = (c->hash * 0x6a4d7821u) ^ sx_rr(c->hash, 3);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t shift_line(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[16] = c->hash ^ sx_rl(c->lane[3], 13);
    c->lane[12] ^= sx_rl(c->lane[8], 7);
    t1 ^= (uint32_t)swap_token(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x82) << 0;
    c->lane[3] += c->lane[13] ^ 0x28be8c78u;
    c->hash ^= c->lane[14] + 0xe68e3b04u;
    c->lane[1] += c->lane[2] ^ 0x96730fc5u;
    t2 += join_digest(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x059f1631u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc67e534du;
    c->lane[10] += c->lane[3]; c->lane[9] ^= c->lane[10]; c->lane[9] = sx_rl(c->lane[9], 30);
    drain_cell(c, &c->lane[4], 2);
    c->lane[0] ^= sx_rl(c->lane[1], 28);
    t2 += (uint32_t)blend_run(c);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void sync_node_410(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += tune_marker(c, c->slo, c->sln);
    c->lane[0] += c->lane[13] ^ 0x245f2535u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x879a1273u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    t1 ^= (uint32_t)peek_block_420(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t2 += poll_field(c, c->slo, c->sln);
    c->lane[3] ^= sx_rl(c->lane[1], 28);
    c->lane[14] += c->lane[5]; c->lane[15] ^= c->lane[14]; c->lane[15] = sx_rl(c->lane[15], 5);
    c->lane[0] ^= sx_rl(c->lane[5], 22);
    parse_count(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xcd) << 0;
    drain_track(c, &c->lane[0], 4);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void slice_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x41d7697bu) ^ sx_rr(c->hash, 22);
    c->hash ^= c->lane[1] + 0x4c79a571u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62039u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x38) << 16;
    c->sched[16] = c->hash ^ sx_rl(c->lane[10], 17);
    c->hash = (c->hash * 0x5ef7e19du) ^ sx_rr(c->hash, 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[1] += c->lane[6] ^ 0x65d2324bu;
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void yield_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x0c) << 0;
    c->lane[13] ^= sx_rl(c->lane[1], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t1 ^= (uint32_t)pin_rate(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)chain_line(c);
    t2 += patch_label(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 13301u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xeccf4459u;
    c->lane[11] += c->lane[8] ^ 0x14e4d8e2u;
    t2 = (t2 ^ c->sum) * 0xc6616d9bu;
    t2 = (t2 ^ c->sum) * 0x0b4bdeb5u;
    c->hash ^= c->lane[12] + 0x582f8102u;
    c->lane[2] ^= sx_rl(c->lane[13], 2);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static int push_digest(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t1 ^= (uint32_t)drain_table(c, (uint8_t)(t0 >> 16), t2);
    c->lane[15] += c->lane[2]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 24);
    t2 = (t2 ^ c->sum) * 0x2299bf21u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[2], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa3) << 16;
    t0 ^= chain_batch(c, t1);
    c->raw[c->slo + (int)((t0 + 61087u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[8] = c->hash ^ sx_rl(c->lane[14], 26);
    t2 = (t2 ^ c->sum) * 0x4f0dd3b7u;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void trim_segment(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] += c->lane[5] ^ 0x1cec8f76u;
    c->hash = (c->hash * 0xc7f6dc87u) ^ sx_rr(c->hash, 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc7) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52086u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 17);
    t1 ^= (uint32_t)close_slot(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[23] = c->hash ^ sx_rl(c->lane[13], 11);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t hold_part(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] ^= sx_rl(c->lane[14], 14);
    blend_span(c, t0, t1);
    t2 += clamp_stack(c, c->slo, c->sln);
    c->lane[4] += c->lane[15]; c->lane[1] ^= c->lane[4]; c->lane[1] = sx_rl(c->lane[1], 6);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x917d3c83u) ^ sx_rr(c->hash, 29);
    c->sum += t1;
    return t0 + t2;
}

static void map_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= slice_record(c, t1);
    c->sched[6] = c->hash ^ sx_rl(c->lane[10], 7);
    t1 ^= (uint32_t)tune_row(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x404a348bu;
    t2 = (t2 ^ c->sum) * 0x54c5b7cdu;
    c->lane[11] ^= sx_rl(c->lane[3], 16);
    t2 += trim_mask_439(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6558u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x56fe518fu;
    c->lane[9] ^= sx_rl(c->lane[11], 10);
    c->lane[9] += c->lane[15]; c->lane[1] ^= c->lane[9]; c->lane[1] = sx_rl(c->lane[1], 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xbe) << 0;
    t2 += (uint32_t)pair_line(c);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t rotate_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += tune_marker(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 ^= fetch_batch_428(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xaa) << 0;
    c->hash ^= c->lane[1] + 0xcd87fdd2u;
    c->hash ^= c->lane[3] + 0xfa8fef8au;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3091u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x6f421461u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa54d457bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t close_slot(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 1);
    c->raw[c->slo + (int)((t0 + 14951u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 15180u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4bf6dc8fu;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 31);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t clamp_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35840u) % (uint32_t)c->rln)] << 8;
    c->lane[11] ^= sx_rl(c->lane[12], 18);
    t2 += stage_offset(c, c->rlo, c->rln);
    stage_page(c, &c->lane[6], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xc9fe9261u;
    c->hash ^= c->lane[9] + 0x39277ae8u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    pick_range(c, t0, t1);
    c->hash = (c->hash * 0x7d770971u) ^ sx_rr(c->hash, 10);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t peek_block_420(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[7] + 0xac64260cu;
    c->lane[9] += c->lane[2] ^ 0x4d05120eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x47) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37019u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)place_store(c);
    c->lane[12] += c->lane[13]; c->lane[10] ^= c->lane[12]; c->lane[10] = sx_rl(c->lane[10], 1);
    t2 = (t2 ^ c->sum) * 0x224c04fdu;
    c->lane[5] += c->lane[0]; c->lane[2] ^= c->lane[5]; c->lane[2] = sx_rl(c->lane[2], 4);
    c->lane[4] += c->lane[8] ^ 0x783fddcdu;
    t2 += (uint32_t)mix_slot(c);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd7) << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t join_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += (uint32_t)pick_delta(c);
    c->lane[6] += c->lane[2] ^ 0x9d0af4aeu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xaad5b099u;
    t0 ^= seek_port(c, t1);
    c->lane[8] += c->lane[13]; c->lane[7] ^= c->lane[8]; c->lane[7] = sx_rl(c->lane[7], 16);
    c->lane[0] ^= sx_rl(c->lane[14], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x5ee419bfu;
    c->hash ^= c->lane[6] + 0xd209edc9u;
    c->lane[10] += c->lane[9] ^ 0xb6c69ca3u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 += (uint32_t)swap_pool_493(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void mark_lease_422(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[15] ^ 0xdd280219u;
    t2 += (uint32_t)reap_token(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t1 ^= (uint32_t)drain_table_459(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0x8a86a42bu) ^ sx_rr(c->hash, 24);
    t0 ^= yield_index(c, t1);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[12] += c->lane[8]; c->lane[14] ^= c->lane[12]; c->lane[14] = sx_rl(c->lane[14], 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t split_frame(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    hold_digest(c, &c->lane[8], 3);
    t2 = (t2 ^ c->sum) * 0xec293717u;
    t2 = (t2 ^ c->sum) * 0x06c1724bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6df0c0a7u;
    t2 = (t2 ^ c->sum) * 0xc22837e3u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 11);
    reset_seat(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbbfbe977u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t fold_segment(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 32391u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 += (uint32_t)sync_range(c);
    t2 = (t2 ^ c->sum) * 0x63b6795du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7e) << 0;
    c->sched[2] = c->hash ^ sx_rl(c->lane[3], 16);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 31);
    t2 = (t2 ^ c->sum) * 0x6efb1f01u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 ^= flush_gap(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x9f4caba1u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void parse_count(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 13698u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa14ea623u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd4) << 0;
    c->lane[14] ^= sx_rl(c->lane[9], 16);
    t1 ^= (uint32_t)chain_stream(c, (uint8_t)(t0 >> 0), t2);
    c->lane[8] += c->lane[2] ^ 0x8558bf4au;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 ^= join_unit_494(c, t1);
    c->sched[29] = c->hash ^ sx_rl(c->lane[9], 30);
    c->lane[2] ^= sx_rl(c->lane[11], 10);
    c->lane[5] ^= sx_rl(c->lane[7], 8);
    probe_stream(c, &c->lane[3], 4);
    c->lane[12] ^= sx_rl(c->lane[15], 8);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint32_t tune_marker(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 15631u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0xa1af95d5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x4ef4d0b1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 43128u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x9cf939dfu) ^ sx_rr(c->hash, 29);
    close_value(c, &c->lane[0], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->hash = (c->hash * 0xc2573439u) ^ sx_rr(c->hash, 4);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xd4fb7b4du;
    c->hash ^= c->lane[8] + 0x3bf0a3abu;
    c->sum += t1;
    return t0 + t2;
}

static int chain_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33453u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x78890aefu;
    c->sched[26] = c->hash ^ sx_rl(c->lane[11], 15);
    c->lane[10] ^= sx_rl(c->lane[6], 8);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t fetch_batch_428(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x88b42fddu) ^ sx_rr(c->hash, 16);
    c->lane[2] += c->lane[15] ^ 0xc3081f93u;
    settle_region(c, &c->lane[9], 3);
    c->lane[10] += c->lane[9] ^ 0x4432237eu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x97) << 16;
    t2 += (uint32_t)mix_key(c);
    t2 += defer_rate(c, c->rlo, c->rln);
    latch_token(c, t0, t1);
    c->hash = (c->hash * 0x08be246fu) ^ sx_rr(c->hash, 8);
    t0 ^= pair_store(c, t1);
    pick_range(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static int cache_list(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe2) << 8;
    c->sched[23] = c->hash ^ sx_rl(c->lane[0], 14);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 += (uint32_t)pick_queue(c);
    t2 = (t2 ^ c->sum) * 0x5999346bu;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t swap_offset(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[7] += c->lane[5]; c->lane[13] ^= c->lane[7]; c->lane[13] = sx_rl(c->lane[13], 31);
    c->hash = (c->hash * 0xc9009931u) ^ sx_rr(c->hash, 20);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] += c->lane[3] ^ 0xae810090u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fill_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x61b09329u;
    c->hash ^= c->lane[7] + 0x70cb39cdu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[11] ^= sx_rl(c->lane[8], 7);
    c->hash = (c->hash * 0x482cb709u) ^ sx_rr(c->hash, 15);
    queue_lease_484(c, &c->lane[7], 3);
    c->lane[7] += c->lane[3] ^ 0xa04a2d32u;
    c->sched[6] = c->hash ^ sx_rl(c->lane[10], 12);
    c->sched[4] = c->hash ^ sx_rl(c->lane[2], 18);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t patch_label(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[15] + 0xc77f0300u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xcb282365u;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 30);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash = (c->hash * 0x07e91117u) ^ sx_rr(c->hash, 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 40664u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41269u) % (uint32_t)c->rln)] << 8;
    t0 ^= push_track_480(c, t1);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb5) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    shift_slot(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum += t1;
    return t0 + t2;
}

static uint8_t tune_row(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[14] += c->lane[7] ^ 0x44252567u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33191u) % (uint32_t)c->rln)] << 16;
    c->sched[3] = c->hash ^ sx_rl(c->lane[13], 30);
    c->sched[21] = c->hash ^ sx_rl(c->lane[15], 6);
    c->lane[2] += c->lane[7] ^ 0xf03d6ef0u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x1e) << 8;
    c->hash ^= c->lane[1] + 0x3df2bc1du;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t trim_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sched[8] = c->hash ^ sx_rl(c->lane[12], 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x232c3b0du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27034u) % (uint32_t)c->rln)] << 16;
    t2 += pin_lease(c, c->slo, c->sln);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 += clamp_state(c, c->rlo, c->rln);
    t0 ^= stage_range_467(c, t1);
    c->hash ^= c->lane[11] + 0xbd96898au;
    c->raw[c->slo + (int)((t0 + 15360u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[10] + 0xbac86993u;
    c->hash = (c->hash * 0x01b4608bu) ^ sx_rr(c->hash, 30);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t map_range(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf1e56e35u;
    join_marker(c, t0, t1);
    c->lane[15] += c->lane[5]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 7);
    t0 ^= stage_span(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3817u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17863u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x9b3fb05du;
    c->sched[23] = c->hash ^ sx_rl(c->lane[13], 18);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t store_path(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe32845ebu;
    c->sched[29] = c->hash ^ sx_rl(c->lane[6], 24);
    c->lane[13] += c->lane[15]; c->lane[5] ^= c->lane[13]; c->lane[5] = sx_rl(c->lane[5], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x43) << 16;
    place_range(c, &c->lane[6], 3);
    c->raw[c->slo + (int)((t0 + 35660u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    sift_stack(c, t0, t1);
    c->hash = (c->hash * 0x88a54b4bu) ^ sx_rr(c->hash, 28);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x6a) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x3cfd8001u) ^ sx_rr(c->hash, 26);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t slice_record(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t1 ^= (uint32_t)pin_rate(c, (uint8_t)(t0 >> 0), t2);
    chain_scope(c, &c->lane[11], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9889u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[5], 22);
    c->raw[c->slo + (int)((t0 + 56730u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0x88475a7bu) ^ sx_rr(c->hash, 20);
    t2 = (t2 ^ c->sum) * 0xc553db5bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += (uint32_t)pick_delta(c);
    c->lane[6] += c->lane[1] ^ 0x78fb279cu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t trim_pool(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xf38e0293u) ^ sx_rr(c->hash, 5);
    c->hash ^= c->lane[9] + 0x29b294eau;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16805u) % (uint32_t)c->rln)] << 0;
    c->sched[2] = c->hash ^ sx_rl(c->lane[1], 23);
    c->lane[7] += c->lane[10] ^ 0x244ce516u;
    c->lane[12] += c->lane[14]; c->lane[4] ^= c->lane[12]; c->lane[4] = sx_rl(c->lane[4], 1);
    c->lane[7] += c->lane[10]; c->lane[6] ^= c->lane[7]; c->lane[6] = sx_rl(c->lane[6], 30);
    c->lane[4] += c->lane[15]; c->lane[12] ^= c->lane[4]; c->lane[12] = sx_rl(c->lane[12], 24);
    c->lane[3] += c->lane[8]; c->lane[10] ^= c->lane[3]; c->lane[10] = sx_rl(c->lane[10], 5);
    t2 = (t2 ^ c->sum) * 0x5311435bu;
    t0 ^= stage_block_497(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t trim_mask_439(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 2);
    t2 += mark_frame(c, c->slo, c->sln);
    t0 ^= cache_chunk(c, t1);
    c->sched[17] = c->hash ^ sx_rl(c->lane[4], 8);
    c->hash ^= c->lane[5] + 0xab695430u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[12] = c->hash ^ sx_rl(c->lane[5], 16);
    c->lane[2] += c->lane[9]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 2);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 12);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xcdf8e363u;
    c->sum += t1;
    return t0 + t2;
}

static void mark_track(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[4] += c->lane[6]; c->lane[7] ^= c->lane[4]; c->lane[7] = sx_rl(c->lane[7], 27);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 9);
    t2 = (t2 ^ c->sum) * 0x9c71bfd5u;
    c->hash ^= c->lane[2] + 0x0dc5b07bu;
    c->sched[22] = c->hash ^ sx_rl(c->lane[0], 14);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= c->lane[1] + 0xe6f74425u;
    t2 = (t2 ^ c->sum) * 0x9aa8d7abu;
    t2 += yield_queue(c, c->rlo, c->rln);
    parse_bound(c, &c->lane[2], 3);
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static uint8_t swap_token(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    close_seat(c, &c->lane[10], 3);
    c->hash ^= c->lane[15] + 0x74e8f776u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x37) << 0;
    c->sched[13] = c->hash ^ sx_rl(c->lane[4], 2);
    t1 ^= (uint32_t)trim_item(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[7] ^= sx_rl(c->lane[6], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6e0d565bu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void cache_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += poll_field(c, c->rlo, c->rln);
    c->lane[1] += c->lane[10] ^ 0x018fc94eu;
    c->hash = (c->hash * 0xe13cccefu) ^ sx_rr(c->hash, 14);
    t0 ^= pack_path(c, t1);
    c->raw[c->slo + (int)((t0 + 35560u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sched[16] = c->hash ^ sx_rl(c->lane[6], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc2fe0197u;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7e22325du;
    c->lane[1] ^= sx_rl(c->lane[14], 23);
    c->lane[9] += c->lane[6] ^ 0xce131393u;
    t2 = (t2 ^ c->sum) * 0xbacf7b93u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static void blend_span(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[9] = c->hash ^ sx_rl(c->lane[10], 20);
    c->hash = (c->hash * 0xf231a0bbu) ^ sx_rr(c->hash, 10);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x67415433u;
    c->raw[c->slo + (int)((t0 + 15916u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x023212e1u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x88f66f15u;
    c->lane[6] ^= sx_rl(c->lane[6], 8);
    c->lane[7] ^= sx_rl(c->lane[0], 8);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t chain_batch(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x56dddeb1u;
    c->hash ^= c->lane[9] + 0x203f9893u;
    c->hash = (c->hash * 0xe28b4f65u) ^ sx_rr(c->hash, 4);
    c->lane[12] += c->lane[5] ^ 0x7691086au;
    c->lane[4] += c->lane[9] ^ 0x83063ee8u;
    t2 = (t2 ^ c->sum) * 0xee09dc7bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t tally_stack(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[14] += c->lane[11]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 19);
    c->hash ^= c->lane[1] + 0xd580bc6bu;
    c->sched[7] = c->hash ^ sx_rl(c->lane[2], 29);
    c->lane[11] += c->lane[0] ^ 0x8caef160u;
    c->lane[15] += c->lane[10]; c->lane[8] ^= c->lane[15]; c->lane[8] = sx_rl(c->lane[8], 28);
    c->lane[4] += c->lane[4] ^ 0x46397d1bu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x37) << 16;
    t0 ^= patch_rate(c, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t chain_batch_446(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[9] = c->hash ^ sx_rl(c->lane[11], 12);
    c->lane[5] += c->lane[9]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 10);
    c->hash ^= c->lane[12] + 0x991d8ba2u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 54299u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2887u) % (uint32_t)c->rln)] << 0;
    purge_chunk(c, t0, t1);
    c->hash = (c->hash * 0x10f1adbdu) ^ sx_rr(c->hash, 8);
    pick_ring(c, t0, t1);
    c->sched[21] = c->hash ^ sx_rl(c->lane[13], 6);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void drain_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[14] + 0x80a754edu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42276u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x7f18aa47u) ^ sx_rr(c->hash, 12);
    c->hash ^= c->lane[0] + 0x05ba599au;
    c->hash ^= c->lane[1] + 0x8aad0e77u;
    move_offset(c, t0, t1);
    c->hash = (c->hash * 0x12233b9du) ^ sx_rr(c->hash, 15);
    t2 = (t2 ^ c->sum) * 0xc1591c21u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void drain_cell(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xc6add46du;
    c->lane[8] += c->lane[8] ^ 0x947e77ffu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    t2 += (uint32_t)swap_pool_493(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15814u) % (uint32_t)c->rln)] << 8;
    c->sched[2] = c->hash ^ sx_rl(c->lane[10], 28);
    c->lane[1] ^= sx_rl(c->lane[3], 28);
    c->lane[12] += c->lane[11]; c->lane[2] ^= c->lane[12]; c->lane[2] = sx_rl(c->lane[2], 12);
    c->lane[3] ^= sx_rl(c->lane[8], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x3247b8b9u;
    t2 = (t2 ^ c->sum) * 0xcfc42279u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t drain_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[7] ^= sx_rl(c->lane[15], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53310u) % (uint32_t)c->rln)] << 8;
    t2 += yield_queue(c, c->rlo, c->rln);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 24);
    c->raw[c->slo + (int)((t0 + 45614u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += merge_band(c, c->rlo, c->rln);
    c->hash = (c->hash * 0xcd87002fu) ^ sx_rr(c->hash, 22);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void wrap_pool(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[12] + 0x9071e545u;
    c->hash = (c->hash * 0x5a49a907u) ^ sx_rr(c->hash, 16);
    c->lane[11] += c->lane[1]; c->lane[12] ^= c->lane[11]; c->lane[12] = sx_rl(c->lane[12], 7);
    c->lane[14] ^= sx_rl(c->lane[10], 15);
    c->hash = (c->hash * 0x8d199f4du) ^ sx_rr(c->hash, 6);
    c->lane[14] += c->lane[1] ^ 0x116a13d0u;
    t0 ^= chain_tail(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48905u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash = (c->hash * 0xed6fdf9fu) ^ sx_rr(c->hash, 20);
    t2 += patch_record(c, c->slo, c->sln);
    t2 += (uint32_t)pair_line(c);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint8_t place_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x1678b6bfu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x8e) << 16;
    c->hash ^= c->lane[14] + 0xaf4ff25au;
    c->sched[7] = c->hash ^ sx_rl(c->lane[14], 3);
    t1 ^= (uint32_t)tune_store(c, (uint8_t)(t0 >> 8), t2);
    t2 = (t2 ^ c->sum) * 0xdec89e05u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    mix_bucket(c, &c->lane[7], 4);
    c->raw[c->slo + (int)((t0 + 30563u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x938b0c07u;
    c->hash ^= c->lane[6] + 0xe5beaa5eu;
    c->lane[15] += c->lane[5]; c->lane[4] ^= c->lane[15]; c->lane[4] = sx_rl(c->lane[4], 11);
    c->lane[13] += c->lane[3]; c->lane[12] ^= c->lane[13]; c->lane[12] = sx_rl(c->lane[12], 5);
    sync_value(c, &c->lane[8], 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int blend_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58089u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 19);
    c->lane[6] += c->lane[12] ^ 0x6ecb9f22u;
    c->lane[15] += c->lane[9]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 13);
    t2 += join_group(c, c->rlo, c->rln);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 8);
    t0 ^= align_band(c, t1);
    t1 ^= (uint32_t)drain_table_459(c, (uint8_t)(t0 >> 0), t2);
    c->lane[1] += c->lane[10]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 31);
    c->lane[10] ^= sx_rl(c->lane[14], 3);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int reap_token(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash = (c->hash * 0xea4cc731u) ^ sx_rr(c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] += c->lane[5] ^ 0x36d7aa69u;
    c->lane[1] += c->lane[8]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 26);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t poll_field(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 24);
    c->raw[c->slo + (int)((t0 + 646u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x8a12fa8bu;
    t2 = (t2 ^ c->sum) * 0x6a916473u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 15);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t merge_band(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x10bcdbbdu;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 24);
    t2 = (t2 ^ c->sum) * 0x6ba501a9u;
    c->lane[3] ^= sx_rl(c->lane[3], 19);
    c->hash = (c->hash * 0xea5f7c85u) ^ sx_rr(c->hash, 14);
    c->hash ^= c->lane[6] + 0x873db889u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xb2719a9bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62857u) % (uint32_t)c->rln)] << 8;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t mark_frame(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 19);
    c->lane[11] ^= sx_rl(c->lane[0], 31);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 7);
    c->sched[18] = c->hash ^ sx_rl(c->lane[12], 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8355u) % (uint32_t)c->rln)] << 0;
    c->sum += t1;
    return t0 + t2;
}

static void probe_stream(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 56771u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[4] + 0xf08c2048u;
    c->hash = (c->hash * 0xc4af7f95u) ^ sx_rr(c->hash, 14);
    c->raw[c->slo + (int)((t0 + 24612u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] += c->lane[12] ^ 0x94ed2b13u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x4e) << 16;
    c->lane[6] ^= sx_rl(c->lane[12], 18);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] += c->lane[6]; c->lane[15] ^= c->lane[10]; c->lane[15] = sx_rl(c->lane[15], 9);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void pick_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[15] ^= sx_rl(c->lane[9], 17);
    c->lane[7] += c->lane[8]; c->lane[12] ^= c->lane[7]; c->lane[12] = sx_rl(c->lane[12], 28);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 16);
    c->sched[31] = c->hash ^ sx_rl(c->lane[3], 21);
    c->lane[7] += c->lane[10]; c->lane[12] ^= c->lane[7]; c->lane[12] = sx_rl(c->lane[12], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x70319329u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x78d0bc41u;
    c->lane[11] ^= sx_rl(c->lane[5], 23);
    c->hash = (c->hash * 0x10dbb433u) ^ sx_rr(c->hash, 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 13405u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static uint8_t drain_table_459(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[2] + 0xca7d328du;
    c->hash = (c->hash * 0x5ea7ad79u) ^ sx_rr(c->hash, 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47182u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0xc6684dc3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44435u) % (uint32_t)c->rln)] << 8;
    c->lane[7] ^= sx_rl(c->lane[12], 27);
    c->lane[7] += c->lane[9] ^ 0xe078b97au;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void latch_token(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xd773b4b3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x7c) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf3b6e5dbu;
    c->hash ^= c->lane[14] + 0x931e2b38u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[5] + 0x17363c44u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int mix_key(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    c->lane[7] += c->lane[7] ^ 0x3b2b0629u;
    t2 = (t2 ^ c->sum) * 0x2f961e29u;
    c->hash ^= c->lane[0] + 0x2f40d1ccu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[13] ^= sx_rl(c->lane[12], 11);
    c->lane[6] ^= sx_rl(c->lane[9], 24);
    c->lane[5] += c->lane[2] ^ 0x3efb8becu;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t stage_span(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x9e228163u;
    c->lane[2] += c->lane[9]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 30);
    t2 = (t2 ^ c->sum) * 0x7484c8c3u;
    c->lane[7] += c->lane[10]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void mix_bucket(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x7bc9020du;
    c->lane[5] ^= sx_rl(c->lane[11], 23);
    t2 = (t2 ^ c->sum) * 0xf3c35ed9u;
    c->hash ^= c->lane[7] + 0xf129e292u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t yield_queue(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[11] + 0x6b9868beu;
    c->lane[14] += c->lane[5] ^ 0x8d1bde53u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57429u) % (uint32_t)c->rln)] << 24;
    c->lane[2] ^= sx_rl(c->lane[5], 28);
    t2 = (t2 ^ c->sum) * 0xd032f303u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xdb324c43u;
    c->hash = (c->hash * 0x49412615u) ^ sx_rr(c->hash, 6);
    c->lane[0] += c->lane[8]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 21);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 10);
    c->lane[12] += c->lane[9] ^ 0x63d7464au;
    c->sum += t1;
    return t0 + t2;
}

static void sync_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[2] += c->lane[3] ^ 0x2de16b7cu;
    c->lane[0] += c->lane[11] ^ 0x92a600a2u;
    t2 = (t2 ^ c->sum) * 0x2a24f839u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xfd) << 8;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int pair_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash = (c->hash * 0x7a6621edu) ^ sx_rr(c->hash, 12);
    c->sched[23] = c->hash ^ sx_rl(c->lane[11], 23);
    c->hash ^= c->lane[15] + 0xf2cdc114u;
    c->lane[15] ^= sx_rl(c->lane[12], 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x203afebfu;
    c->raw[c->slo + (int)((t0 + 13863u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xdd) << 8;
    c->raw[c->slo + (int)((t0 + 42947u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t stage_range_467(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x6c29bd4bu) ^ sx_rr(c->hash, 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x1e0cba6bu;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 4);
    t2 = (t2 ^ c->sum) * 0xdba6b65du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void join_marker(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xec217d9du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0x925253e3u;
    c->raw[c->slo + (int)((t0 + 38730u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 34869u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static void settle_region(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 1057u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42423u) % (uint32_t)c->rln)] << 24;
    c->sched[8] = c->hash ^ sx_rl(c->lane[12], 11);
    c->lane[10] += c->lane[5] ^ 0xe59a087cu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x87) << 8;
    c->hash = (c->hash * 0xfc76a2ffu) ^ sx_rr(c->hash, 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x2b) << 0;
    c->lane[0] += c->lane[7]; c->lane[15] ^= c->lane[0]; c->lane[15] = sx_rl(c->lane[15], 14);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49547u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 33087u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void chain_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    t2 = (t2 ^ c->sum) * 0xe714b70du;
    c->lane[5] += c->lane[2] ^ 0xcadb0a25u;
    c->lane[0] ^= sx_rl(c->lane[13], 9);
    c->hash ^= c->lane[10] + 0xb1184725u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4cfdc0fdu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2881u) % (uint32_t)c->rln)] << 16;
    c->lane[1] ^= sx_rl(c->lane[14], 11);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void place_range(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[10] ^= sx_rl(c->lane[3], 13);
    c->raw[c->slo + (int)((t0 + 15572u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 20779u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x638c8f23u;
    c->lane[11] ^= sx_rl(c->lane[0], 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[18] = c->hash ^ sx_rl(c->lane[10], 12);
    c->lane[4] += c->lane[3] ^ 0xeb33c6cfu;
    c->lane[0] += c->lane[14] ^ 0xbcdfb4b9u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 26);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t yield_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[8] ^= sx_rl(c->lane[7], 26);
    c->lane[9] ^= sx_rl(c->lane[5], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[0] ^= sx_rl(c->lane[1], 23);
    c->hash ^= c->lane[15] + 0x9acf65e1u;
    c->hash ^= c->lane[0] + 0xd7d809beu;
    c->lane[8] += c->lane[14] ^ 0xbaf1c1acu;
    c->lane[10] += c->lane[5] ^ 0x17c9ad08u;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 6);
    c->lane[2] += c->lane[1] ^ 0x84b03a1au;
    c->lane[7] ^= sx_rl(c->lane[6], 29);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void close_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x3a25dc13u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20343u) % (uint32_t)c->rln)] << 24;
    c->lane[10] += c->lane[2]; c->lane[7] ^= c->lane[10]; c->lane[7] = sx_rl(c->lane[7], 26);
    c->raw[c->slo + (int)((t0 + 4645u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[1] += c->lane[6] ^ 0x16a45788u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15247u) % (uint32_t)c->rln)] << 24;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 3);
    c->hash ^= c->lane[11] + 0x41e0dbe6u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31468u) % (uint32_t)c->rln)] << 8;
    c->sched[20] = c->hash ^ sx_rl(c->lane[8], 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void parse_bound(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc5) << 8;
    c->lane[7] ^= sx_rl(c->lane[6], 7);
    c->raw[c->slo + (int)((t0 + 49061u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 22365u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] ^= sx_rl(c->lane[5], 14);
    c->lane[3] += c->lane[11] ^ 0xed54ad2du;
    t2 = (t2 ^ c->sum) * 0x646730c5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xb35fe4c3u;
    c->raw[c->slo + (int)((t0 + 42962u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t stage_offset(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[11] = c->hash ^ sx_rl(c->lane[4], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash = (c->hash * 0x0e3c42bbu) ^ sx_rr(c->hash, 4);
    c->hash = (c->hash * 0x74581771u) ^ sx_rr(c->hash, 24);
    c->lane[10] += c->lane[6]; c->lane[8] ^= c->lane[10]; c->lane[8] = sx_rl(c->lane[8], 1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 16);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4b1ad393u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28593u) % (uint32_t)c->rln)] << 0;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t patch_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[7] + 0x18f5df16u;
    c->raw[c->slo + (int)((t0 + 21050u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[8] + 0x694c3d46u;
    c->hash ^= c->lane[0] + 0x8662e8ceu;
    c->lane[15] += c->lane[13] ^ 0x5557b3f8u;
    c->raw[c->slo + (int)((t0 + 57280u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t seek_port(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5e24dfd3u;
    c->lane[0] ^= sx_rl(c->lane[7], 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x461df821u;
    c->hash ^= c->lane[5] + 0xc6c7cee1u;
    c->hash = (c->hash * 0x49e283fbu) ^ sx_rr(c->hash, 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x9b) << 16;
    c->hash ^= c->lane[10] + 0xd6176948u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int pick_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[7] ^= sx_rl(c->lane[7], 16);
    c->lane[14] ^= sx_rl(c->lane[4], 5);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 15);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16617u) % (uint32_t)c->rln)] << 8;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23058u) % (uint32_t)c->rln)] << 24;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void stage_page(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x0cf65e77u) ^ sx_rr(c->hash, 14);
    c->hash ^= c->lane[13] + 0x7c2f1aaau;
    c->lane[8] += c->lane[3]; c->lane[14] ^= c->lane[8]; c->lane[14] = sx_rl(c->lane[14], 3);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf4) << 8;
    c->hash = (c->hash * 0xfe9c226du) ^ sx_rr(c->hash, 5);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t push_track_480(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[2] += c->lane[3]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 7);
    c->lane[11] ^= sx_rl(c->lane[13], 2);
    c->raw[c->slo + (int)((t0 + 10194u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[11] + 0x13c1ddf4u;
    c->raw[c->slo + (int)((t0 + 48930u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void hold_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xef) << 16;
    c->hash ^= c->lane[1] + 0x8e69d613u;
    c->lane[1] ^= sx_rl(c->lane[1], 26);
    c->hash = (c->hash * 0x73ea2721u) ^ sx_rr(c->hash, 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pin_lease(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[6], 8);
    c->raw[c->slo + (int)((t0 + 32799u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6b) << 0;
    c->sched[16] = c->hash ^ sx_rl(c->lane[2], 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28166u) % (uint32_t)c->rln)] << 0;
    c->lane[0] ^= sx_rl(c->lane[14], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc51e58adu;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t cache_chunk(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12988u) % (uint32_t)c->rln)] << 16;
    c->hash ^= c->lane[15] + 0x64056cafu;
    c->hash = (c->hash * 0x17027559u) ^ sx_rr(c->hash, 5);
    t0 ^= probe_arena(c, t1);
    c->hash ^= c->lane[3] + 0xaf6c870du;
    t2 = (t2 ^ c->sum) * 0x07264927u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 15349u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[0] + 0x43072dfeu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void queue_lease_484(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 24218u) % (uint32_t)c->rln)] << 24;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x6b) << 16;
    c->hash ^= c->lane[2] + 0x89a261ecu;
    c->lane[10] += c->lane[3] ^ 0x2eecd7e3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 5987u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t trim_item(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x2fc7bb57u) ^ sx_rr(c->hash, 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51123u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xcef28bd1u;
    c->lane[3] += c->lane[2] ^ 0x1d5b373cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[9] += c->lane[13] ^ 0xf01e84aeu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x30) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7de3a721u;
    c->lane[4] += c->lane[2] ^ 0x19b7345au;
    c->sched[24] = c->hash ^ sx_rl(c->lane[7], 13);
    c->hash ^= c->lane[3] + 0xbc1587b9u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void reset_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 28);
    c->lane[3] += c->lane[2]; c->lane[4] ^= c->lane[3]; c->lane[4] = sx_rl(c->lane[4], 3);
    c->lane[4] += c->lane[10] ^ 0xa1d39587u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53822u) % (uint32_t)c->rln)] << 0;
    c->lane[15] += c->lane[11]; c->lane[13] ^= c->lane[15]; c->lane[13] = sx_rl(c->lane[13], 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3ed93117u;
    c->raw[c->slo + (int)((t0 + 38715u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xf2) << 8;
    c->lane[6] += c->lane[1] ^ 0x269d149fu;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t patch_record(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb0) << 8;
    t2 = (t2 ^ c->sum) * 0x8301511fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32155u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x29e14f99u) ^ sx_rr(c->hash, 4);
    c->lane[15] += c->lane[0]; c->lane[2] ^= c->lane[15]; c->lane[2] = sx_rl(c->lane[2], 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xec) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe4) << 8;
    c->raw[c->slo + (int)((t0 + 30098u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[26] = c->hash ^ sx_rl(c->lane[12], 18);
    c->lane[5] += c->lane[14] ^ 0x050faa89u;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t pack_path(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xa93b2d7du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x24) << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6892c173u;
    c->hash = (c->hash * 0xad8ad101u) ^ sx_rr(c->hash, 29);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x63) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x3e318f4du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xd85f0589u) ^ sx_rr(c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33151u) % (uint32_t)c->rln)] << 24;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t chain_tail(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 30);
    c->hash ^= c->lane[13] + 0xd58f14eeu;
    c->sched[5] = c->hash ^ sx_rl(c->lane[8], 25);
    c->lane[14] ^= sx_rl(c->lane[9], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34005u) % (uint32_t)c->rln)] << 24;
    pair_pairing(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pair_store(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[14] += c->lane[8] ^ 0x7f63b3afu;
    c->raw[c->slo + (int)((t0 + 58986u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26314u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xcf) << 16;
    c->raw[c->slo + (int)((t0 + 21871u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t flush_gap(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31053u) % (uint32_t)c->rln)] << 0;
    c->lane[6] += c->lane[11] ^ 0x1b495175u;
    c->hash ^= c->lane[8] + 0xaf495c13u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 13);
    t2 = (t2 ^ c->sum) * 0x95b032ddu;
    c->sched[18] = c->hash ^ sx_rl(c->lane[12], 31);
    c->hash = (c->hash * 0x5dc9a257u) ^ sx_rr(c->hash, 14);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4ea3e9d5u;
    c->lane[0] += c->lane[5]; c->lane[2] ^= c->lane[0]; c->lane[2] = sx_rl(c->lane[2], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x65) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53133u) % (uint32_t)c->rln)] << 16;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void shift_slot(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 7);
    c->sched[11] = c->hash ^ sx_rl(c->lane[14], 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x389a0d4du;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14960u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x0b691af5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x81fc5185u;
    c->raw[c->slo + (int)((t0 + 55258u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 28);
    c->lane[10] ^= sx_rl(c->lane[8], 26);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static int swap_pool_493(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 13);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 5);
    c->hash ^= c->lane[8] + 0x6ca33215u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x00195dbfu;
    c->raw[c->slo + (int)((t0 + 6248u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t join_unit_494(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[13] + 0xeef21be5u;
    t2 = (t2 ^ c->sum) * 0xbb55cc95u;
    c->raw[c->slo + (int)((t0 + 31718u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->sched[0] = c->hash ^ sx_rl(c->lane[15], 1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa8) << 0;
    c->lane[15] += c->lane[14]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 5);
    c->lane[3] += c->lane[13]; c->lane[1] ^= c->lane[3]; c->lane[1] = sx_rl(c->lane[1], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[3] + 0xc067ac86u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t tune_store(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] += c->lane[6]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= c->lane[7] + 0x863d9f08u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 17);
    t2 = (t2 ^ c->sum) * 0xad536dddu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void pick_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37532u) % (uint32_t)c->rln)] << 16;
    t2 = (t2 ^ c->sum) * 0xf0d36bd5u;
    c->raw[c->slo + (int)((t0 + 20351u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    reset_range(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x83) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x4431a75du) ^ sx_rr(c->hash, 5);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 18);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t stage_block_497(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 11);
    c->sched[14] = c->hash ^ sx_rl(c->lane[9], 24);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 22);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x74fcfbf1u;
    c->raw[c->slo + (int)((t0 + 15167u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t clamp_state(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[11] + 0x49ef508au;
    c->sched[17] = c->hash ^ sx_rl(c->lane[13], 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 15670u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 12);
    reap_marker(c, &c->lane[1], 4);
    c->hash = (c->hash * 0x0011a94fu) ^ sx_rr(c->hash, 12);
    c->sum += t1;
    return t0 + t2;
}

static void purge_chunk(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 42284u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe1) << 8;
    c->lane[12] ^= sx_rl(c->lane[12], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x7fa47ba7u;
    c->lane[15] ^= sx_rl(c->lane[7], 25);
    c->lane[2] += c->lane[15]; c->lane[12] ^= c->lane[2]; c->lane[12] = sx_rl(c->lane[12], 11);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 2);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 13);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int mix_slot(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t2 = (t2 ^ c->sum) * 0x809ce737u;
    c->raw[c->slo + (int)((t0 + 25719u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= c->lane[13] + 0x8f8f3082u;
    c->lane[14] += c->lane[4] ^ 0x4dde1ae1u;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t align_band(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[4] + 0xb4997b65u;
    c->hash ^= c->lane[10] + 0x509b2704u;
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 28);
    c->lane[5] ^= sx_rl(c->lane[2], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 49356u) % (uint32_t)c->rln)] << 0;
    c->lane[13] ^= sx_rl(c->lane[14], 30);
    t2 = (t2 ^ c->sum) * 0xddc271edu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void close_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2973524fu;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 28);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 31);
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 3);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x7019e463u;
    c->hash = (c->hash * 0x8240b9e1u) ^ sx_rr(c->hash, 28);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int sync_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14169u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0xad982a6du) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash = (c->hash * 0x9c0f2979u) ^ sx_rr(c->hash, 10);
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t pin_rate(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 37767u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[31] = c->hash ^ sx_rl(c->lane[0], 5);
    c->raw[c->slo + (int)((t0 + 5023u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[1] += c->lane[2] ^ 0xa2ee84dfu;
    c->lane[12] += c->lane[0] ^ 0x74d5e0cfu;
    c->lane[9] += c->lane[11]; c->lane[8] ^= c->lane[9]; c->lane[8] = sx_rl(c->lane[8], 30);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void move_offset(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[2] + 0xaf2556bdu;
    t2 = (t2 ^ c->sum) * 0xd0d35f05u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sched[19] = c->hash ^ sx_rl(c->lane[13], 18);
    c->raw[c->slo + (int)((t0 + 45159u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x51b0b8e9u) ^ sx_rr(c->hash, 10);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 22);
    c->lane[15] += c->lane[9]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 4);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x1ac30423u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 10599u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static int place_store(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x44d9a2adu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5a36f9a1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29507u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[5] + 0xcc1912c3u;
    c->hash = (c->hash * 0x3b32822fu) ^ sx_rr(c->hash, 2);
    c->raw[c->slo + (int)((t0 + 33649u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x5f4a19b9u) ^ sx_rr(c->hash, 12);
    t2 = (t2 ^ c->sum) * 0x6f0b1749u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4c4b3f65u;
    c->lane[6] ^= sx_rl(c->lane[5], 29);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int pick_delta(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->lane[2] += c->lane[5] ^ 0xb989d623u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9bc66fefu;
    c->lane[6] += c->lane[4]; c->lane[0] ^= c->lane[6]; c->lane[0] = sx_rl(c->lane[0], 14);
    c->hash ^= c->lane[2] + 0x96fcf2fbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59808u) % (uint32_t)c->rln)] << 24;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t chain_stream(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x7545b1bfu) ^ sx_rr(c->hash, 1);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 7);
    c->lane[15] ^= sx_rl(c->lane[9], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x304ce019u;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 26);
    c->lane[4] ^= sx_rl(c->lane[0], 22);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 43706u) % (uint32_t)c->rln)] << 8;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t join_group(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] += c->lane[9]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd3596e91u;
    t2 += (uint32_t)split_bucket_618(c);
    c->lane[6] += c->lane[11]; c->lane[13] ^= c->lane[6]; c->lane[13] = sx_rl(c->lane[13], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 16942u) % (uint32_t)c->rln)] << 24;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 26);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t defer_rate(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[15] += c->lane[12]; c->lane[8] ^= c->lane[15]; c->lane[8] = sx_rl(c->lane[8], 29);
    c->lane[15] += c->lane[10] ^ 0xda2796dfu;
    c->lane[3] += c->lane[10] ^ 0x52ef2f19u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x44f3fce3u;
    c->hash = (c->hash * 0x7060fb29u) ^ sx_rr(c->hash, 5);
    c->lane[1] += c->lane[7]; c->lane[2] ^= c->lane[1]; c->lane[2] = sx_rl(c->lane[2], 14);
    c->sum += t1;
    return t0 + t2;
}

static void sift_stack(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[24] = c->hash ^ sx_rl(c->lane[4], 29);
    c->sched[0] = c->hash ^ sx_rl(c->lane[15], 5);
    c->hash = (c->hash * 0x7ca7e51fu) ^ sx_rr(c->hash, 3);
    c->sched[15] = c->hash ^ sx_rl(c->lane[0], 21);
    c->raw[c->slo + (int)((t0 + 47046u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[15] += c->lane[0]; c->lane[10] ^= c->lane[15]; c->lane[10] = sx_rl(c->lane[10], 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static void peek_digest(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    defer_slot(c, t0, t1);
    tap_entry(c, &c->lane[9], 2);
    c->hash = (c->hash * 0x3f9c2c27u) ^ sx_rr(c->hash, 7);
    t0 ^= step_ring(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23237u) % (uint32_t)c->rln)] << 24;
    merge_layer(c, &c->lane[8], 4);
    c->hash ^= c->lane[13] + 0x58a97566u;
    t2 = (t2 ^ c->sum) * 0x1f0c122fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xbe) << 0;
    t2 += (uint32_t)drain_offset(c);
    tap_count(c, &c->lane[11], 2);
    t2 += grow_index(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x6d) << 8;
    t2 += (uint32_t)grow_stream_579(c);
    split_path(c, &c->lane[11], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45001u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x31) << 0;
    reap_ring(c, &c->lane[3], 1);
    c->hash = (c->hash * 0x773dc003u) ^ sx_rr(c->hash, 15);
    tune_head_589(c, t0, t1);
    t2 += yield_count(c, c->rlo, c->rln);
    rotate_track(c, t0, t1);
    clamp_value(c, &c->lane[7], 1);
    t2 += (uint32_t)store_label(c);
    t0 ^= close_view(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xda) << 8;
    tune_head(c, &c->lane[5], 3);
    move_table(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void rotate_track(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)fetch_bound(c, (uint8_t)(t0 >> 0), t2);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 25);
    t2 += parse_node(c, c->rlo, c->rln);
    t0 ^= settle_segment(c, t1);
    c->raw[c->slo + (int)((t0 + 44694u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= c->lane[12] + 0xd5585d02u;
    c->hash = (c->hash * 0xe34a1a43u) ^ sx_rr(c->hash, 29);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9e119f53u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[2], 10);
    c->sched[31] = c->hash ^ sx_rl(c->lane[15], 10);
    c->raw[c->slo + (int)((t0 + 59051u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += sort_view(c, c->rlo, c->rln);
    t0 ^= sort_pairing_525(c, t1);
    fill_head(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t grow_index(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 25757u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x95) << 8;
    c->lane[15] += c->lane[9]; c->lane[13] ^= c->lane[15]; c->lane[13] = sx_rl(c->lane[13], 15);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 10);
    t2 += (uint32_t)split_window_519(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7a) << 0;
    t1 ^= (uint32_t)trim_node(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x73) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29145u) % (uint32_t)c->rln)] << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void tune_head(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8311u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 5558u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 43492u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[2] + 0xbe7c9ec2u;
    load_ring(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xdd) << 0;
    t0 ^= align_span(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xa4) << 8;
    t1 ^= (uint32_t)hold_page_631(c, (uint8_t)(t0 >> 0), t2);
    c->raw[c->slo + (int)((t0 + 1302u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    peek_scope(c, t0, t1);
    t2 += (uint32_t)hold_gap(c);
    t2 += fold_limit(c, c->slo, c->sln);
    emit_limit(c, &c->lane[4], 2);
    t0 ^= split_bucket(c, t1);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 31);
    c->lane[15] += c->lane[5]; c->lane[0] ^= c->lane[15]; c->lane[0] = sx_rl(c->lane[0], 26);
    link_queue(c, &c->lane[0], 3);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t settle_segment(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x90aefb1du;
    t0 ^= split_bucket(c, t1);
    c->sched[14] = c->hash ^ sx_rl(c->lane[4], 5);
    load_ring(c, t0, t1);
    t0 ^= slice_tuple(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xae) << 8;
    t2 = (t2 ^ c->sum) * 0x7821daa3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2897u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfbe7a7d3u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t parse_node(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[5] += c->lane[8]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 17);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xd3cf7ec9u;
    c->hash ^= c->lane[2] + 0xee6ed73au;
    c->hash = (c->hash * 0xb65b0587u) ^ sx_rr(c->hash, 17);
    t0 ^= sift_node(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5c) << 0;
    t0 ^= fold_count(c, t1);
    c->lane[13] += c->lane[3]; c->lane[6] ^= c->lane[13]; c->lane[6] = sx_rl(c->lane[6], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 ^= load_rate(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void emit_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[14] + 0x6596e3e2u;
    tune_state_539(c, &c->lane[8], 4);
    c->sched[12] = c->hash ^ sx_rl(c->lane[13], 7);
    c->lane[8] += c->lane[11]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 6);
    c->lane[13] += c->lane[8] ^ 0x91bd807cu;
    t0 ^= fill_rate(c, t1);
    c->lane[6] ^= sx_rl(c->lane[0], 22);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int split_window_519(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x62) << 8;
    c->lane[5] += c->lane[10]; c->lane[7] ^= c->lane[5]; c->lane[7] = sx_rl(c->lane[7], 30);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xc5) << 8;
    c->lane[11] ^= sx_rl(c->lane[7], 6);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xdc8c044fu;
    t0 ^= fold_arena(c, t1);
    t2 += merge_range(c, c->rlo, c->rln);
    c->lane[10] += c->lane[5] ^ 0x3848643fu;
    c->hash = (c->hash * 0xd68f8fa1u) ^ sx_rr(c->hash, 31);
    defer_slot(c, t0, t1);
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t sort_view(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[7] + 0x7b9ea2e1u;
    t2 += (uint32_t)drain_gap(c);
    c->lane[9] += c->lane[2]; c->lane[14] ^= c->lane[9]; c->lane[14] = sx_rl(c->lane[14], 29);
    t2 = (t2 ^ c->sum) * 0x1abf89ebu;
    c->lane[10] += c->lane[3]; c->lane[8] ^= c->lane[10]; c->lane[8] = sx_rl(c->lane[8], 31);
    t0 ^= fold_arena(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25239u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x3f86108du) ^ sx_rr(c->hash, 1);
    c->hash = (c->hash * 0xa342da01u) ^ sx_rr(c->hash, 31);
    t0 ^= blend_head_620(c, t1);
    t1 ^= (uint32_t)load_offset_611(c, (uint8_t)(t0 >> 0), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52234u) % (uint32_t)c->rln)] << 0;
    t0 ^= tune_store_529(c, t1);
    c->hash ^= c->lane[9] + 0xec74a1a9u;
    c->lane[0] += c->lane[13]; c->lane[6] ^= c->lane[0]; c->lane[6] = sx_rl(c->lane[6], 24);
    c->sum += t1;
    return t0 + t2;
}

static uint8_t fetch_bound(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[13] += c->lane[3]; c->lane[14] ^= c->lane[13]; c->lane[14] = sx_rl(c->lane[14], 29);
    c->lane[13] += c->lane[14] ^ 0x852ba522u;
    c->lane[1] ^= sx_rl(c->lane[7], 25);
    t2 = (t2 ^ c->sum) * 0x571d2c8du;
    t0 ^= poll_track(c, t1);
    c->lane[3] ^= sx_rl(c->lane[4], 23);
    sift_segment(c, t0, t1);
    merge_layer(c, &c->lane[5], 3);
    t0 ^= merge_stack(c, t1);
    t2 += load_offset(c, c->rlo, c->rln);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t align_span(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xac8400a7u) ^ sx_rr(c->hash, 16);
    c->hash = (c->hash * 0x2272cebbu) ^ sx_rr(c->hash, 20);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25859u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x41405979u;
    settle_region_625(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xf1055819u;
    poll_arena(c, t0, t1);
    c->hash = (c->hash * 0x81e54ca3u) ^ sx_rr(c->hash, 17);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 10);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 14);
    t2 = (t2 ^ c->sum) * 0x9c4db5a3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa2) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void fill_head(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 ^= close_view(c, t1);
    tune_queue(c, t0, t1);
    c->sched[23] = c->hash ^ sx_rl(c->lane[14], 22);
    c->hash = (c->hash * 0x22f4dc49u) ^ sx_rr(c->hash, 26);
    c->hash ^= c->lane[13] + 0x8ab14d9eu;
    t0 ^= tune_store_529(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static int hold_gap(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 8);
    t2 += (uint32_t)store_label(c);
    c->sched[11] = c->hash ^ sx_rl(c->lane[10], 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15804u) % (uint32_t)c->rln)] << 0;
    chain_digest(c, &c->lane[10], 2);
    c->hash ^= c->lane[15] + 0x23397480u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x15773771u;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t sort_pairing_525(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[9] += c->lane[4] ^ 0xfad26e6bu;
    pair_item(c, &c->lane[7], 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x13) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 ^= pin_level(c, t1);
    t2 = (t2 ^ c->sum) * 0xb0bc0b17u;
    c->hash = (c->hash * 0xa4cdac2fu) ^ sx_rr(c->hash, 16);
    t0 ^= merge_stack(c, t1);
    c->raw[c->slo + (int)((t0 + 22881u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    split_path(c, &c->lane[8], 1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 19);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x48ee5319u;
    t2 += (uint32_t)parse_rate(c);
    c->hash ^= c->lane[12] + 0x91010f2du;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void chain_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 52335u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[9] += c->lane[4]; c->lane[3] ^= c->lane[9]; c->lane[3] = sx_rl(c->lane[3], 25);
    c->lane[3] += c->lane[6]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 8);
    t2 += (uint32_t)drain_offset(c);
    t2 = (t2 ^ c->sum) * 0x58c313cbu;
    c->sched[3] = c->hash ^ sx_rl(c->lane[7], 17);
    c->lane[15] ^= sx_rl(c->lane[4], 3);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void poll_arena(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 62334u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 61969u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60491u) % (uint32_t)c->rln)] << 24;
    c->lane[14] += c->lane[8] ^ 0x2a74f9f9u;
    t2 += peek_table(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe3c9a09fu;
    c->lane[1] += c->lane[5]; c->lane[13] ^= c->lane[1]; c->lane[13] = sx_rl(c->lane[13], 11);
    c->raw[c->slo + (int)((t0 + 56183u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    move_table(c, t0, t1);
    c->sched[17] = c->hash ^ sx_rl(c->lane[9], 22);
    c->lane[5] += c->lane[4]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 9);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint32_t fold_count(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd6) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 13);
    clamp_lease(c, &c->lane[1], 3);
    c->sched[15] = c->hash ^ sx_rl(c->lane[10], 25);
    c->lane[9] ^= sx_rl(c->lane[3], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xc64e50ddu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45801u) % (uint32_t)c->rln)] << 16;
    flush_limit(c, &c->lane[10], 1);
    c->lane[9] += c->lane[3]; c->lane[7] ^= c->lane[9]; c->lane[7] = sx_rl(c->lane[7], 16);
    c->sched[30] = c->hash ^ sx_rl(c->lane[7], 4);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tune_store_529(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[6] += c->lane[3] ^ 0xd306cb8eu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 2);
    c->lane[6] ^= sx_rl(c->lane[9], 12);
    c->lane[11] += c->lane[10]; c->lane[4] ^= c->lane[11]; c->lane[4] = sx_rl(c->lane[4], 31);
    c->raw[c->slo + (int)((t0 + 34818u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5f) << 16;
    t2 += load_offset(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54160u) % (uint32_t)c->rln)] << 24;
    clamp_value(c, &c->lane[2], 3);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t merge_range(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    pair_pairing(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8778u) % (uint32_t)c->rln)] << 0;
    yield_digest(c, &c->lane[5], 1);
    t2 = (t2 ^ c->sum) * 0x57e4f82fu;
    t2 = (t2 ^ c->sum) * 0xbcb005d5u;
    c->hash = (c->hash * 0xa7e859a3u) ^ sx_rr(c->hash, 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void load_ring(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[8] + 0xb1f4c443u;
    c->lane[9] += c->lane[1]; c->lane[4] ^= c->lane[9]; c->lane[4] = sx_rl(c->lane[4], 28);
    c->sched[16] = c->hash ^ sx_rl(c->lane[4], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53525u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xf66a12fdu;
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 7);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t split_bucket(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x6e24d671u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35534u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0xd1181939u) ^ sx_rr(c->hash, 18);
    c->lane[15] += c->lane[7] ^ 0x5049820fu;
    c->raw[c->slo + (int)((t0 + 47816u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[7] = c->hash ^ sx_rl(c->lane[13], 7);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x9d8b5033u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19453u) % (uint32_t)c->rln)] << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int store_label(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sched[9] = c->hash ^ sx_rl(c->lane[7], 11);
    c->lane[15] += c->lane[10] ^ 0xb8a8aa58u;
    sift_segment(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xa798bd4bu;
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 9);
    t1 ^= (uint32_t)resize_cell(c, (uint8_t)(t0 >> 0), t2);
    t2 += (uint32_t)settle_stack(c);
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void defer_slot(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 2);
    c->lane[0] ^= sx_rl(c->lane[2], 17);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 14);
    c->lane[2] ^= sx_rl(c->lane[5], 3);
    c->lane[1] += c->lane[1] ^ 0x4d731881u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x133d678fu;
    c->lane[15] += c->lane[7] ^ 0x2f86ac08u;
    c->hash ^= c->lane[10] + 0x572cf758u;
    c->raw[c->slo + (int)((t0 + 52572u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 25470u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xe9b862c9u;
    move_ring_566(c, &c->lane[1], 4);
    swap_value(c, &c->lane[11], 1);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t load_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] ^= sx_rl(c->lane[5], 31);
    t2 = (t2 ^ c->sum) * 0x792afd75u;
    t2 += (uint32_t)store_run(c);
    t2 += (uint32_t)sync_bound(c);
    c->lane[14] ^= sx_rl(c->lane[11], 21);
    c->sched[9] = c->hash ^ sx_rl(c->lane[10], 4);
    t0 ^= patch_stack(c, t1);
    t2 = (t2 ^ c->sum) * 0x7bf47ca5u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t close_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 ^= swap_block(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x0d) << 8;
    c->lane[11] ^= sx_rl(c->lane[2], 11);
    t1 ^= (uint32_t)push_lease(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc4) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x49) << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int drain_gap(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    prime_count(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 24371u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 ^= step_ring(c, t1);
    t2 += probe_track(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 780u) % (uint32_t)c->rln)] << 8;
    c->lane[8] ^= sx_rl(c->lane[4], 12);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 1);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t merge_stack(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa81b52fdu;
    t2 += probe_track(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[1] = c->hash ^ sx_rl(c->lane[7], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tune_state_539(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[13] += c->lane[7] ^ 0x80745b26u;
    c->hash ^= c->lane[3] + 0xda774604u;
    t2 += resize_seat(c, c->rlo, c->rln);
    c->lane[13] += c->lane[15]; c->lane[14] ^= c->lane[13]; c->lane[14] = sx_rl(c->lane[14], 12);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t1 ^= (uint32_t)pin_range(c, (uint8_t)(t0 >> 8), t2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fill_rate(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 6);
    sync_gap_570(c, t0, t1);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xde) << 0;
    t2 += (uint32_t)probe_region(c);
    c->lane[4] += c->lane[2]; c->lane[14] ^= c->lane[4]; c->lane[14] = sx_rl(c->lane[14], 12);
    t2 += fold_limit(c, c->slo, c->sln);
    t2 += wrap_stack(c, c->slo, c->sln);
    c->lane[2] += c->lane[1]; c->lane[4] ^= c->lane[2]; c->lane[4] = sx_rl(c->lane[4], 14);
    c->lane[14] += c->lane[3]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 3);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    drain_seat(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void merge_layer(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x28c83fc7u) ^ sx_rr(c->hash, 27);
    c->lane[5] += c->lane[6]; c->lane[0] ^= c->lane[5]; c->lane[0] = sx_rl(c->lane[0], 20);
    c->raw[c->slo + (int)((t0 + 46283u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xdd16e0e3u) ^ sx_rr(c->hash, 10);
    peek_scope(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb4) << 8;
    c->lane[11] += c->lane[0] ^ 0x24496782u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x7cb044e1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46694u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 ^= poll_block(c, t1);
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 20);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t pin_level(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += (uint32_t)tap_cursor(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 21);
    c->hash ^= c->lane[6] + 0xf5c7637eu;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 16);
    c->lane[7] ^= sx_rl(c->lane[13], 29);
    c->hash = (c->hash * 0x125c43c7u) ^ sx_rr(c->hash, 25);
    c->hash = (c->hash * 0xec1ed3cfu) ^ sx_rr(c->hash, 12);
    probe_value(c, &c->lane[1], 4);
    t0 ^= slice_tuple(c, t1);
    t1 ^= (uint32_t)sync_gap(c, (uint8_t)(t0 >> 0), t2);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 17);
    c->lane[13] += c->lane[15]; c->lane[10] ^= c->lane[13]; c->lane[10] = sx_rl(c->lane[10], 8);
    c->sched[20] = c->hash ^ sx_rl(c->lane[12], 18);
    c->lane[7] += c->lane[2]; c->lane[6] ^= c->lane[7]; c->lane[6] = sx_rl(c->lane[6], 13);
    t0 ^= swap_block(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void split_path(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[6] += c->lane[15] ^ 0xfead8233u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= c->lane[7] + 0xffd0e828u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd2) << 16;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 7);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb3) << 0;
    c->lane[2] += c->lane[14]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fold_arena(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0xde8f4c4fu) ^ sx_rr(c->hash, 27);
    tap_count(c, &c->lane[8], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x38) << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3756u) % (uint32_t)c->rln)] << 0;
    fold_value(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0a) << 8;
    t2 += sift_slot(c, c->rlo, c->rln);
    c->sched[30] = c->hash ^ sx_rl(c->lane[0], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    drain_lease(c, t0, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int parse_rate(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    t2 += (uint32_t)grow_stream_579(c);
    c->lane[5] += c->lane[6] ^ 0xb41ae3f5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x49d824c9u;
    c->raw[c->slo + (int)((t0 + 58270u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9975u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x30673adfu) ^ sx_rr(c->hash, 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t1 ^= (uint32_t)seek_group(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)scan_key(c, (uint8_t)(t0 >> 0), t2);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void flush_limit(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 ^= rotate_mask(c, t1);
    c->hash = (c->hash * 0x21cdc0b3u) ^ sx_rr(c->hash, 4);
    c->hash ^= c->lane[6] + 0x93c6d9deu;
    c->raw[c->slo + (int)((t0 + 25066u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 60635u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0xad5bd72bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void swap_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdd) << 0;
    c->hash ^= c->lane[9] + 0x27db9748u;
    t1 ^= (uint32_t)hold_page_631(c, (uint8_t)(t0 >> 16), t2);
    c->lane[2] += c->lane[15]; c->lane[11] ^= c->lane[2]; c->lane[11] = sx_rl(c->lane[11], 25);
    tune_head_589(c, t0, t1);
    t1 ^= (uint32_t)load_offset_611(c, (uint8_t)(t0 >> 16), t2);
    c->raw[c->slo + (int)((t0 + 3890u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void probe_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[27] = c->hash ^ sx_rl(c->lane[3], 19);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 26);
    t2 = (t2 ^ c->sum) * 0x2fa76b3du;
    c->hash ^= c->lane[4] + 0x86fc63eau;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void yield_digest(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[9] + 0x8b482b94u;
    c->lane[1] += c->lane[13]; c->lane[9] ^= c->lane[1]; c->lane[9] = sx_rl(c->lane[9], 29);
    c->lane[14] ^= sx_rl(c->lane[12], 10);
    purge_node(c, t0, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x3f) << 8;
    c->raw[c->slo + (int)((t0 + 35131u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[11] += c->lane[3]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 25);
    c->raw[c->slo + (int)((t0 + 5216u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0x87340223u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t sync_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= blend_head_620(c, t1);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 27);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xa1) << 16;
    c->lane[1] += c->lane[14]; c->lane[12] ^= c->lane[1]; c->lane[12] = sx_rl(c->lane[12], 20);
    tune_queue(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->sched[20] = c->hash ^ sx_rl(c->lane[10], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 8);
    t1 ^= (uint32_t)prime_label(c, (uint8_t)(t0 >> 8), t2);
    settle_region_625(c, t0, t1);
    c->hash ^= c->lane[14] + 0xe6dd40c3u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void fold_value(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[10] + 0xbcec992au;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    probe_pairing(c, &c->lane[2], 3);
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 29);
    blend_token(c, t0, t1);
    t2 += (uint32_t)drain_delta(c);
    c->lane[7] ^= sx_rl(c->lane[0], 23);
    c->raw[c->slo + (int)((t0 + 2817u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[15] ^= sx_rl(c->lane[6], 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x96) << 16;
    drain_seat(c, t0, t1);
    t2 += align_table_609(c, c->slo, c->sln);
    c->hash = (c->hash * 0x875767f9u) ^ sx_rr(c->hash, 17);
    c->lane[8] += c->lane[11]; c->lane[15] ^= c->lane[8]; c->lane[15] = sx_rl(c->lane[15], 5);
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint8_t push_lease(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 ^= sift_node(c, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x3fc007bbu;
    c->raw[c->slo + (int)((t0 + 46882u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0x48c7bea9u;
    c->lane[0] += c->lane[6] ^ 0x20380955u;
    c->lane[8] += c->lane[3]; c->lane[7] ^= c->lane[8]; c->lane[7] = sx_rl(c->lane[7], 27);
    c->raw[c->slo + (int)((t0 + 2757u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa9) << 16;
    c->raw[c->slo + (int)((t0 + 59886u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64754u) % (uint32_t)c->rln)] << 0;
    c->lane[11] += c->lane[13] ^ 0x76d7ebafu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t resize_seat(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[12] ^= sx_rl(c->lane[8], 30);
    c->lane[6] ^= sx_rl(c->lane[9], 1);
    t2 = (t2 ^ c->sum) * 0x99db77afu;
    t1 ^= (uint32_t)prime_label(c, (uint8_t)(t0 >> 16), t2);
    c->lane[1] ^= sx_rl(c->lane[14], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xae) << 0;
    c->sched[12] = c->hash ^ sx_rl(c->lane[13], 17);
    c->hash = (c->hash * 0xe5d1f4c5u) ^ sx_rr(c->hash, 6);
    c->sum += t1;
    return t0 + t2;
}

static int drain_offset(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33948u) % (uint32_t)c->rln)] << 16;
    c->sched[27] = c->hash ^ sx_rl(c->lane[9], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61950u) % (uint32_t)c->rln)] << 16;
    c->lane[10] += c->lane[15]; c->lane[12] ^= c->lane[10]; c->lane[12] = sx_rl(c->lane[12], 9);
    t2 = (t2 ^ c->sum) * 0xc8ecafbdu;
    t1 ^= (uint32_t)close_layer(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= c->lane[4] + 0xc19b6f2bu;
    t2 += fetch_gap(c, c->slo, c->sln);
    c->sched[14] = c->hash ^ sx_rl(c->lane[7], 7);
    t2 = (t2 ^ c->sum) * 0x35cda113u;
    c->sched[2] = c->hash ^ sx_rl(c->lane[1], 2);
    c->lane[5] ^= sx_rl(c->lane[5], 10);
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t wrap_stack(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 21395u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[10] ^= sx_rl(c->lane[15], 12);
    c->raw[c->slo + (int)((t0 + 35867u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 ^= tally_pairing(c, t1);
    c->hash = (c->hash * 0x7ac59035u) ^ sx_rr(c->hash, 3);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t peek_table(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[9] ^= sx_rl(c->lane[14], 3);
    t2 = (t2 ^ c->sum) * 0x958278e9u;
    t0 ^= probe_arena(c, t1);
    c->sched[16] = c->hash ^ sx_rl(c->lane[13], 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53401u) % (uint32_t)c->rln)] << 16;
    c->sched[26] = c->hash ^ sx_rl(c->lane[3], 24);
    t2 += trace_delta(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0x7a5c82c3u;
    c->hash ^= c->lane[1] + 0xe5a81e85u;
    push_seat(c, &c->lane[1], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void clamp_lease(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 22);
    c->hash = (c->hash * 0xbffb4fa5u) ^ sx_rr(c->hash, 1);
    join_pool(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0xaeae9783u;
    reap_ring(c, &c->lane[4], 1);
    c->sched[29] = c->hash ^ sx_rl(c->lane[2], 8);
    c->lane[1] ^= sx_rl(c->lane[5], 5);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 12);
    link_queue(c, &c->lane[11], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void drain_lease(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[8] ^= sx_rl(c->lane[8], 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54379u) % (uint32_t)c->rln)] << 8;
    t1 ^= (uint32_t)seek_table(c, (uint8_t)(t0 >> 16), t2);
    c->lane[7] += c->lane[8]; c->lane[9] ^= c->lane[7]; c->lane[9] = sx_rl(c->lane[9], 29);
    c->lane[11] += c->lane[13] ^ 0x2dd02e5cu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47497u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x7d0d8c65u;
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t swap_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 18);
    c->lane[14] += c->lane[15]; c->lane[4] ^= c->lane[14]; c->lane[4] = sx_rl(c->lane[4], 15);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x09) << 16;
    c->hash ^= c->lane[15] + 0x44e825d6u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int settle_stack(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xf741f5e5u;
    t1 ^= (uint32_t)seek_group(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0x5e11057du) ^ sx_rr(c->hash, 30);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xe8) << 0;
    c->hash ^= c->lane[1] + 0xb89f84adu;
    drain_span(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xc067d6e3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20954u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x7b) << 0;
    c->sched[7] = c->hash ^ sx_rl(c->lane[0], 4);
    c->lane[2] += c->lane[11] ^ 0x78390bf9u;
    t2 = (t2 ^ c->sum) * 0x997cdb17u;
    t1 ^= (uint32_t)scan_key(c, (uint8_t)(t0 >> 8), t2);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t slice_tuple(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 27);
    t2 = (t2 ^ c->sum) * 0xec3ae823u;
    t0 ^= poll_track(c, t1);
    t2 = (t2 ^ c->sum) * 0x6f5e25adu;
    t1 ^= (uint32_t)trim_node(c, (uint8_t)(t0 >> 8), t2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t fold_limit(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4458a6adu;
    c->lane[11] ^= sx_rl(c->lane[8], 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xfc) << 0;
    push_field(c, &c->lane[5], 4);
    reap_marker(c, &c->lane[4], 3);
    c->hash = (c->hash * 0xe7d60dbbu) ^ sx_rr(c->hash, 12);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t poll_block(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[1] ^= sx_rl(c->lane[8], 6);
    t2 += tap_gap(c, c->slo, c->sln);
    c->hash = (c->hash * 0x062ceebbu) ^ sx_rr(c->hash, 10);
    c->hash = (c->hash * 0x6bcf9c21u) ^ sx_rr(c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 19971u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0x2e181479u) ^ sx_rr(c->hash, 24);
    t1 ^= (uint32_t)pin_range(c, (uint8_t)(t0 >> 16), t2);
    drain_span(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= c->lane[4] + 0x4ab78a21u;
    c->raw[c->slo + (int)((t0 + 61728u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void sift_segment(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[2] ^= sx_rl(c->lane[0], 7);
    c->hash ^= c->lane[14] + 0x43d1be04u;
    place_ring(c, &c->lane[5], 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x19) << 8;
    hold_scope(c, t0, t1);
    t0 ^= patch_stack(c, t1);
    c->sched[11] = c->hash ^ sx_rl(c->lane[10], 16);
    c->hash = (c->hash * 0x90d2d3ebu) ^ sx_rr(c->hash, 1);
    c->sched[13] = c->hash ^ sx_rl(c->lane[15], 8);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t load_offset(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += fetch_gap(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x2014c18bu;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 13);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xf74b19efu;
    t2 += trace_delta(c, c->slo, c->sln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 12860u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xdd) << 8;
    c->sum += t1;
    return t0 + t2;
}

static void move_ring_566(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 += map_cursor(c, c->slo, c->sln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->lane[2] += c->lane[5] ^ 0xa9cc636eu;
    c->raw[c->slo + (int)((t0 + 41307u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[26] = c->hash ^ sx_rl(c->lane[3], 26);
    peek_entry(c, t0, t1);
    c->hash = (c->hash * 0x3162c50du) ^ sx_rr(c->hash, 17);
    c->hash ^= c->lane[0] + 0x5680097au;
    hold_layer(c, &c->lane[4], 2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x780c595fu;
    c->hash ^= c->lane[11] + 0x53794a31u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void move_table(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[4], 21);
    t2 += fold_span(c, c->rlo, c->rln);
    shift_line_600(c, &c->lane[7], 1);
    t0 ^= emit_track(c, t1);
    c->raw[c->slo + (int)((t0 + 19316u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += (uint32_t)split_bucket_618(c);
    t2 = (t2 ^ c->sum) * 0x0ca61ee3u;
    c->hash = (c->hash * 0x3befac6du) ^ sx_rr(c->hash, 15);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static void peek_scope(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] += c->lane[5]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 16);
    reset_range(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 4954u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[23] = c->hash ^ sx_rl(c->lane[2], 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42275u) % (uint32_t)c->rln)] << 8;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 5);
    c->lane[7] ^= sx_rl(c->lane[1], 28);
    c->sched[9] = c->hash ^ sx_rl(c->lane[12], 30);
    t2 = (t2 ^ c->sum) * 0x6431c7e5u;
    c->hash ^= c->lane[14] + 0xcdd9493du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb2) << 16;
    c->lane[6] += c->lane[1] ^ 0xb77ce5e4u;
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t probe_track(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 6010u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x95f4ddd3u) ^ sx_rr(c->hash, 15);
    c->lane[3] += c->lane[12]; c->lane[0] ^= c->lane[3]; c->lane[0] = sx_rl(c->lane[0], 25);
    c->hash = (c->hash * 0x4ad6e47du) ^ sx_rr(c->hash, 28);
    c->sum += t1;
    return t0 + t2;
}

static void sync_gap_570(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    peek_port(c, &c->lane[0], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44472u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[15] + 0x23175077u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xec) << 16;
    c->lane[11] += c->lane[7]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 18);
    t2 += yield_count(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x6d) << 0;
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static int sync_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[13];
    t2 += (uint32_t)poll_token(c);
    c->lane[5] += c->lane[1]; c->lane[14] ^= c->lane[5]; c->lane[14] = sx_rl(c->lane[14], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    c->lane[5] ^= sx_rl(c->lane[15], 28);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)stage_value(c, (uint8_t)(t0 >> 8), t2);
    c->lane[13] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void clamp_value(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[1] + 0x363fa5adu;
    t2 = (t2 ^ c->sum) * 0xbf17fb99u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xcbf73f21u;
    c->raw[c->slo + (int)((t0 + 26975u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] ^= sx_rl(c->lane[0], 20);
    t0 ^= relay_bucket(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int store_run(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[14];
    tap_entry(c, &c->lane[1], 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47167u) % (uint32_t)c->rln)] << 8;
    c->hash = (c->hash * 0x9f467001u) ^ sx_rr(c->hash, 14);
    t0 ^= swap_queue(c, t1);
    c->lane[15] += c->lane[10]; c->lane[5] ^= c->lane[15]; c->lane[5] = sx_rl(c->lane[5], 2);
    t2 = (t2 ^ c->sum) * 0x590969c5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 23);
    pair_pairing(c, t0, t1);
    c->hash ^= c->lane[5] + 0x0d3bb82fu;
    c->lane[14] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t step_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9e) << 0;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 31);
    t1 ^= (uint32_t)close_layer(c, (uint8_t)(t0 >> 8), t2);
    c->lane[11] += c->lane[1]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 21);
    t2 += (uint32_t)shift_record(c);
    c->hash = (c->hash * 0xd94ac25bu) ^ sx_rr(c->hash, 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tap_count(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 31548u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 15544u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x02b11b45u;
    reset_range(c, t0, t1);
    t2 += drain_tail(c, c->rlo, c->rln);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 25);
    reap_marker(c, &c->lane[1], 1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t resize_cell(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] += c->lane[6]; c->lane[4] ^= c->lane[11]; c->lane[4] = sx_rl(c->lane[4], 2);
    c->sched[25] = c->hash ^ sx_rl(c->lane[1], 23);
    c->sched[12] = c->hash ^ sx_rl(c->lane[15], 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[4] = c->hash ^ sx_rl(c->lane[12], 30);
    t2 += (uint32_t)drain_delta(c);
    c->hash = (c->hash * 0x2aa3b8d1u) ^ sx_rr(c->hash, 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int tap_cursor(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    t1 ^= (uint32_t)reap_scope(c, (uint8_t)(t0 >> 16), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x22) << 8;
    c->lane[8] += c->lane[3]; c->lane[2] ^= c->lane[8]; c->lane[2] = sx_rl(c->lane[2], 30);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4472u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[12] + 0xe005fd96u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe6) << 0;
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int probe_region(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4c4dae53u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42352u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0xe4318f95u;
    c->hash ^= c->lane[8] + 0xe2ae9620u;
    t2 += wrap_chunk(c, c->rlo, c->rln);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 8);
    c->raw[c->slo + (int)((t0 + 46645u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x9c) << 0;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int grow_stream_579(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9e) << 0;
    t2 = (t2 ^ c->sum) * 0x86dd2d77u;
    c->hash = (c->hash * 0xee143427u) ^ sx_rr(c->hash, 13);
    c->raw[c->slo + (int)((t0 + 32100u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    pair_item(c, &c->lane[6], 1);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void prime_count(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8a2bf5bbu;
    t2 = (t2 ^ c->sum) * 0x2b03fe1bu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22901u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[7] + 0x6c4d56beu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x568efb27u;
    t2 += sift_slot(c, c->slo, c->sln);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 2);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t trace_delta(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 3727u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 14780u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sched[5] = c->hash ^ sx_rl(c->lane[13], 27);
    c->raw[c->slo + (int)((t0 + 24591u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x1c) << 16;
    t2 = (t2 ^ c->sum) * 0x332f7367u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51193u) % (uint32_t)c->rln)] << 16;
    c->sched[12] = c->hash ^ sx_rl(c->lane[5], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7c) << 8;
    c->sum += t1;
    return t0 + t2;
}

static void drain_seat(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 32162u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[11] += c->lane[3]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 11);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x88a3e78du;
    c->hash ^= c->lane[2] + 0x802e65b4u;
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void hold_layer(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x49145cedu) ^ sx_rr(c->hash, 29);
    c->lane[8] += c->lane[5] ^ 0xd1e9ad3au;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[12] + 0x8106d59au;
    c->sched[9] = c->hash ^ sx_rl(c->lane[2], 30);
    c->lane[4] ^= sx_rl(c->lane[4], 12);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t relay_bucket(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x875bdbb5u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbf3accd1u;
    c->lane[13] ^= sx_rl(c->lane[7], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x16) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xbe0f8171u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x19d65723u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= c->lane[1] + 0xe1fd07e9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sched[12] = c->hash ^ sx_rl(c->lane[15], 17);
    c->hash = (c->hash * 0xaa0bd36du) ^ sx_rr(c->hash, 7);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t swap_queue(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[11] += c->lane[10]; c->lane[6] ^= c->lane[11]; c->lane[6] = sx_rl(c->lane[6], 29);
    t2 = (t2 ^ c->sum) * 0xadf2f82du;
    c->raw[c->slo + (int)((t0 + 29796u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[7] += c->lane[15]; c->lane[2] ^= c->lane[7]; c->lane[2] = sx_rl(c->lane[2], 23);
    c->lane[15] += c->lane[2] ^ 0xb3e29932u;
    c->lane[11] += c->lane[1]; c->lane[4] ^= c->lane[11]; c->lane[4] = sx_rl(c->lane[4], 15);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void drain_span(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35346u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 9999u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[1] += c->lane[9] ^ 0x4f86d945u;
    c->lane[7] += c->lane[5] ^ 0x73ed35a5u;
    c->hash = (c->hash * 0x7ebe0243u) ^ sx_rr(c->hash, 21);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static void reset_range(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 53530u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[13] += c->lane[10]; c->lane[0] ^= c->lane[13]; c->lane[0] = sx_rl(c->lane[0], 31);
    c->raw[c->slo + (int)((t0 + 3049u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xe2ee357bu;
    c->raw[c->slo + (int)((t0 + 31851u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22645u) % (uint32_t)c->rln)] << 16;
    c->hash = (c->hash * 0x2f8dc0fdu) ^ sx_rr(c->hash, 4);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static uint32_t sift_node(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x995e8f81u;
    c->lane[1] += c->lane[0]; c->lane[15] ^= c->lane[1]; c->lane[15] = sx_rl(c->lane[15], 5);
    c->lane[2] += c->lane[0]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 30);
    c->hash = (c->hash * 0x2022edd7u) ^ sx_rr(c->hash, 29);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 18);
    c->raw[c->slo + (int)((t0 + 12153u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sched[29] = c->hash ^ sx_rl(c->lane[3], 22);
    c->sched[14] = c->hash ^ sx_rl(c->lane[3], 27);
    c->lane[8] += c->lane[2] ^ 0x1f035df7u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void tune_head_589(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51323u) % (uint32_t)c->rln)] << 16;
    c->sched[26] = c->hash ^ sx_rl(c->lane[14], 23);
    c->raw[c->slo + (int)((t0 + 65522u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->lane[1] ^= sx_rl(c->lane[3], 26);
    c->sum ^= t0 + t1;
    c->lane[0] ^= t2;
}

static int shift_record(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 42156u) % (uint32_t)c->rln)] << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t2 = (t2 ^ c->sum) * 0xf46f560du;
    c->raw[c->slo + (int)((t0 + 21939u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[2] += c->lane[11]; c->lane[1] ^= c->lane[2]; c->lane[1] = sx_rl(c->lane[1], 23);
    c->hash = (c->hash * 0x369a1699u) ^ sx_rr(c->hash, 5);
    c->hash ^= c->lane[14] + 0xfca74911u;
    c->lane[8] += c->lane[14] ^ 0x0269a487u;
    c->lane[1] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t wrap_chunk(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0xdeb22143u) ^ sx_rr(c->hash, 19);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xde) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x706b9c31u;
    c->sched[11] = c->hash ^ sx_rl(c->lane[8], 5);
    c->hash = (c->hash * 0xfc64cec5u) ^ sx_rr(c->hash, 22);
    c->hash ^= c->lane[3] + 0x35c408b5u;
    c->hash = (c->hash * 0xcfa3038fu) ^ sx_rr(c->hash, 16);
    t2 = (t2 ^ c->sum) * 0x7469c4a3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void pair_pairing(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[17] = c->hash ^ sx_rl(c->lane[11], 17);
    t2 = (t2 ^ c->sum) * 0x6b842dc3u;
    c->hash ^= c->lane[1] + 0x432fe86eu;
    c->raw[c->slo + (int)((t0 + 42733u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 18823u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57442u) % (uint32_t)c->rln)] << 24;
    c->lane[1] += c->lane[2] ^ 0x81c11a5cu;
    c->sched[21] = c->hash ^ sx_rl(c->lane[6], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash ^= c->lane[14] + 0xb148149cu;
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void reap_marker(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sched[30] = c->hash ^ sx_rl(c->lane[0], 3);
    c->sched[7] = c->hash ^ sx_rl(c->lane[3], 20);
    t2 = (t2 ^ c->sum) * 0xd95c9509u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xfd06a795u;
    t2 = (t2 ^ c->sum) * 0x903fb24bu;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 20);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 19);
    c->raw[c->slo + (int)((t0 + 50509u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0xe63271bdu) ^ sx_rr(c->hash, 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void push_seat(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x4cb8f38du;
    c->lane[2] += c->lane[1]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 23);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 36809u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x4d0b6ebdu;
    c->lane[10] ^= sx_rl(c->lane[5], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sched[25] = c->hash ^ sx_rl(c->lane[6], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8607u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x16f74ae5u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t seek_group(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[4] += c->lane[4] ^ 0xe39ef6b5u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 25);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 13);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32437u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27366u) % (uint32_t)c->rln)] << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t fetch_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x7bf73321u) ^ sx_rr(c->hash, 4);
    c->raw[c->slo + (int)((t0 + 10713u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0xd858000du) ^ sx_rr(c->hash, 21);
    c->hash = (c->hash * 0xf50ca791u) ^ sx_rr(c->hash, 15);
    c->lane[6] ^= sx_rl(c->lane[8], 27);
    c->lane[7] += c->lane[6] ^ 0x4e01d9d0u;
    c->raw[c->slo + (int)((t0 + 13343u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xeb) << 16;
    c->lane[8] ^= sx_rl(c->lane[7], 24);
    c->lane[0] += c->lane[8]; c->lane[5] ^= c->lane[0]; c->lane[5] = sx_rl(c->lane[5], 27);
    c->sum += t1;
    return t0 + t2;
}

static void probe_pairing(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] ^= sx_rl(c->lane[7], 5);
    c->lane[6] += c->lane[12] ^ 0xef53a89bu;
    c->raw[c->slo + (int)((t0 + 3676u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x3a) << 0;
    t2 = (t2 ^ c->sum) * 0x0717a72fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void reap_ring(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x715aa2c5u;
    c->lane[9] ^= sx_rl(c->lane[11], 5);
    c->sched[4] = c->hash ^ sx_rl(c->lane[8], 25);
    c->lane[9] += c->lane[11] ^ 0xca5a25dau;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15653u) % (uint32_t)c->rln)] << 24;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[13] ^= sx_rl(c->lane[1], 4);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xec) << 16;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int poll_token(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->hash = (c->hash * 0xbc449bd7u) ^ sx_rr(c->hash, 18);
    c->raw[c->slo + (int)((t0 + 35273u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[13] += c->lane[13] ^ 0xa8642721u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xdacbaa87u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xe7) << 8;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 14);
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void shift_line_600(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash = (c->hash * 0x2de79efbu) ^ sx_rr(c->hash, 24);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 19);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 26);
    c->hash ^= c->lane[4] + 0x1ad43767u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44222u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[3] + 0x408deda5u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t patch_stack(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[15] ^= sx_rl(c->lane[14], 24);
    c->raw[c->slo + (int)((t0 + 41845u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[31] = c->hash ^ sx_rl(c->lane[7], 14);
    c->sched[0] = c->hash ^ sx_rl(c->lane[5], 23);
    c->sched[26] = c->hash ^ sx_rl(c->lane[13], 13);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t tap_gap(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[12] += c->lane[8]; c->lane[15] ^= c->lane[12]; c->lane[15] = sx_rl(c->lane[15], 24);
    c->sched[2] = c->hash ^ sx_rl(c->lane[7], 29);
    c->hash ^= c->lane[2] + 0x7d59ff1eu;
    c->lane[4] += c->lane[1] ^ 0x39dc97b5u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t reap_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x29) << 0;
    c->hash = (c->hash * 0xaa09e551u) ^ sx_rr(c->hash, 27);
    c->lane[4] += c->lane[0]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 4);
    c->lane[11] += c->lane[7] ^ 0x531df179u;
    c->hash = (c->hash * 0xa1b2630bu) ^ sx_rr(c->hash, 12);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5f) << 16;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 16);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 15);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t scan_key(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[6] = c->hash ^ sx_rl(c->lane[9], 20);
    c->hash = (c->hash * 0xd8558757u) ^ sx_rr(c->hash, 17);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 7);
    c->sched[28] = c->hash ^ sx_rl(c->lane[2], 27);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void tune_queue(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x980fd17bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x2ab4629bu;
    c->hash = (c->hash * 0x6ca1d453u) ^ sx_rr(c->hash, 8);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 29);
    c->hash = (c->hash * 0xb6083a65u) ^ sx_rr(c->hash, 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x7e) << 16;
    c->sched[25] = c->hash ^ sx_rl(c->lane[12], 2);
    c->sched[1] = c->hash ^ sx_rl(c->lane[9], 24);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t yield_count(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->lane[10] += c->lane[3] ^ 0x01150784u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 22);
    t2 = (t2 ^ c->sum) * 0x6a0b55c7u;
    c->lane[3] += c->lane[12] ^ 0x646ec668u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xcc) << 8;
    c->hash = (c->hash * 0xb9fb4c71u) ^ sx_rr(c->hash, 9);
    c->hash = (c->hash * 0x0fd39203u) ^ sx_rr(c->hash, 20);
    c->lane[14] ^= sx_rl(c->lane[0], 8);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t rotate_mask(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sched[22] = c->hash ^ sx_rl(c->lane[6], 17);
    c->hash = (c->hash * 0x02351f8du) ^ sx_rr(c->hash, 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33445u) % (uint32_t)c->rln)] << 24;
    c->lane[3] ^= sx_rl(c->lane[14], 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xb2) << 8;
    c->lane[10] += c->lane[2]; c->lane[9] ^= c->lane[10]; c->lane[9] = sx_rl(c->lane[9], 13);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2b86773bu;
    c->lane[8] += c->lane[6]; c->lane[12] ^= c->lane[8]; c->lane[12] = sx_rl(c->lane[12], 4);
    c->raw[c->slo + (int)((t0 + 352u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0xd64b6021u) ^ sx_rr(c->hash, 30);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t seek_table(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x8dd4842bu;
    c->sched[27] = c->hash ^ sx_rl(c->lane[2], 8);
    t2 = (t2 ^ c->sum) * 0xb021a2e5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[10] += c->lane[3] ^ 0xbd462668u;
    c->hash ^= c->lane[6] + 0x1062c881u;
    c->raw[c->slo + (int)((t0 + 59156u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t align_table_609(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xf2aa6bf3u;
    t2 = (t2 ^ c->sum) * 0x1a29a9cbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32238u) % (uint32_t)c->rln)] << 16;
    c->sched[21] = c->hash ^ sx_rl(c->lane[15], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32606u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9816acefu;
    c->lane[0] += c->lane[14] ^ 0x3eca273eu;
    t2 = (t2 ^ c->sum) * 0x5d1d28d7u;
    c->hash = (c->hash * 0xb0cc3423u) ^ sx_rr(c->hash, 30);
    c->lane[8] += c->lane[10] ^ 0xeb05340bu;
    c->sum += t1;
    return t0 + t2;
}

static void tap_entry(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] += c->lane[11]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 19);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 1);
    c->hash ^= c->lane[3] + 0xd6029398u;
    c->hash = (c->hash * 0x9012fdabu) ^ sx_rr(c->hash, 16);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t load_offset_611(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xb8793953u) ^ sx_rr(c->hash, 24);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26280u) % (uint32_t)c->rln)] << 24;
    c->hash ^= c->lane[8] + 0x4399e2edu;
    c->hash = (c->hash * 0x817aa767u) ^ sx_rr(c->hash, 9);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t poll_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 8);
    c->lane[10] += c->lane[3]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 47356u) % (uint32_t)c->rln)] << 24;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x36) << 0;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t drain_tail(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[15] += c->lane[15] ^ 0x02395a66u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37847u) % (uint32_t)c->rln)] << 16;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 12);
    c->hash ^= c->lane[3] + 0x2ebd54f1u;
    c->lane[6] += c->lane[14]; c->lane[13] ^= c->lane[6]; c->lane[13] = sx_rl(c->lane[13], 26);
    c->lane[2] += c->lane[14] ^ 0x5fb72686u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 16);
    c->hash = (c->hash * 0x64db373fu) ^ sx_rr(c->hash, 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void push_field(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[4] += c->lane[15]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 8);
    c->hash ^= c->lane[5] + 0x2282c612u;
    t2 = (t2 ^ c->sum) * 0x3744ee11u;
    c->lane[0] ^= sx_rl(c->lane[4], 9);
    t2 = (t2 ^ c->sum) * 0x3065f743u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t fold_span(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[11] ^= sx_rl(c->lane[1], 9);
    t2 = (t2 ^ c->sum) * 0x77ba42fbu;
    c->lane[13] ^= sx_rl(c->lane[10], 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 51977u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t stage_value(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x47bc212du) ^ sx_rr(c->hash, 8);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x2c22291fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xc0) << 0;
    c->hash ^= c->lane[8] + 0x0bd6828fu;
    c->raw[c->slo + (int)((t0 + 46230u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xc0) << 16;
    c->lane[14] ^= sx_rl(c->lane[11], 13);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 63312u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0xe94c4e65u) ^ sx_rr(c->hash, 4);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void join_pool(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 26);
    c->sched[11] = c->hash ^ sx_rl(c->lane[5], 5);
    c->lane[8] ^= sx_rl(c->lane[8], 18);
    c->lane[5] += c->lane[4] ^ 0xa59ecb14u;
    c->raw[c->slo + (int)((t0 + 65117u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x6e) << 0;
    c->raw[c->slo + (int)((t0 + 21288u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x37e73d93u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xfa) << 8;
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static int split_bucket_618(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    t2 = (t2 ^ c->sum) * 0x7fc7f04du;
    c->hash = (c->hash * 0x03b527d7u) ^ sx_rr(c->hash, 29);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 19);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void peek_port(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash ^= c->lane[4] + 0x418d3aafu;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 23);
    c->lane[8] += c->lane[6] ^ 0xc841cbdbu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14897u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x7b) << 0;
    c->raw[c->slo + (int)((t0 + 37588u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 51758u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->raw[c->slo + (int)((t0 + 50530u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[12] ^= sx_rl(c->lane[13], 19);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t blend_head_620(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[12] + 0x8edd6de1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0xad948dc9u;
    c->lane[11] += c->lane[9]; c->lane[3] ^= c->lane[11]; c->lane[3] = sx_rl(c->lane[3], 17);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t emit_track(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[1] + 0x92f31dc5u;
    c->sched[14] = c->hash ^ sx_rl(c->lane[14], 9);
    t2 = (t2 ^ c->sum) * 0xf12b19b9u;
    c->lane[7] += c->lane[13] ^ 0xc4522926u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 23);
    c->hash = (c->hash * 0xdce74f03u) ^ sx_rr(c->hash, 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void purge_node(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31461u) % (uint32_t)c->rln)] << 8;
    c->hash ^= c->lane[10] + 0xbe8f5272u;
    c->lane[12] += c->lane[10] ^ 0xf997ed74u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x82) << 0;
    t2 += (uint32_t)fold_span_727(c);
    c->sched[10] = c->hash ^ sx_rl(c->lane[1], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53905u) % (uint32_t)c->rln)] << 8;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t trim_node(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0x4dbc655bu) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc8) << 0;
    c->hash = (c->hash * 0xd6aee461u) ^ sx_rr(c->hash, 5);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x32) << 16;
    c->hash ^= c->lane[0] + 0x0a6e074cu;
    c->lane[4] ^= sx_rl(c->lane[7], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x5b) << 8;
    t2 = (t2 ^ c->sum) * 0x36edbc27u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[0] += c->lane[2]; c->lane[1] ^= c->lane[0]; c->lane[1] = sx_rl(c->lane[1], 14);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t map_cursor(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x2dc23929u) ^ sx_rr(c->hash, 25);
    t2 = (t2 ^ c->sum) * 0x3ae05e29u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[8] += c->lane[9]; c->lane[7] ^= c->lane[8]; c->lane[7] = sx_rl(c->lane[7], 5);
    c->sum += t1;
    return t0 + t2;
}

static void settle_region_625(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x8e76220du;
    c->hash = (c->hash * 0x79961aedu) ^ sx_rr(c->hash, 6);
    c->lane[2] ^= sx_rl(c->lane[0], 3);
    c->lane[15] += c->lane[1] ^ 0xd28229e1u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[3], 13);
    c->hash = (c->hash * 0x8fc784a5u) ^ sx_rr(c->hash, 3);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 31);
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static void pair_item(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[24] = c->hash ^ sx_rl(c->lane[7], 16);
    c->lane[2] += c->lane[13] ^ 0xa02732cfu;
    c->hash ^= c->lane[3] + 0xca7165c0u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 37754u) % (uint32_t)c->rln)] << 16;
    c->lane[15] += c->lane[4] ^ 0xe8b057a0u;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void link_queue(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x52) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->hash = (c->hash * 0x4ce62d3bu) ^ sx_rr(c->hash, 22);
    c->lane[11] += c->lane[14]; c->lane[5] ^= c->lane[11]; c->lane[5] = sx_rl(c->lane[5], 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb4320085u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xe6) << 0;
    c->sched[4] = c->hash ^ sx_rl(c->lane[11], 9);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 18);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t pin_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x6da07979u;
    c->hash = (c->hash * 0xec503ec9u) ^ sx_rr(c->hash, 10);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->lane[4] ^= sx_rl(c->lane[2], 27);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int drain_delta(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[15];
    c->lane[0] += c->lane[8] ^ 0xff2d48e7u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[11] ^= sx_rl(c->lane[14], 14);
    c->lane[13] ^= sx_rl(c->lane[1], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x71da12cfu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->lane[2] ^= sx_rl(c->lane[1], 20);
    c->hash = (c->hash * 0xc879f0a9u) ^ sx_rr(c->hash, 23);
    c->lane[15] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void blend_token(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sched[14] = c->hash ^ sx_rl(c->lane[9], 13);
    c->hash = (c->hash * 0x0321a6c9u) ^ sx_rr(c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= c->lane[5] + 0x4992538cu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x81065a8fu;
    c->lane[14] ^= sx_rl(c->lane[2], 30);
    c->lane[7] += c->lane[0] ^ 0x897fe4ffu;
    c->raw[c->slo + (int)((t0 + 19239u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[0] += c->lane[3]; c->lane[7] ^= c->lane[0]; c->lane[7] = sx_rl(c->lane[7], 6);
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint8_t hold_page_631(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[1] ^= sx_rl(c->lane[13], 2);
    c->lane[8] += c->lane[13]; c->lane[5] ^= c->lane[8]; c->lane[5] = sx_rl(c->lane[5], 5);
    t2 = (t2 ^ c->sum) * 0x6d8202a3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x3d) << 0;
    c->hash = (c->hash * 0x853f9413u) ^ sx_rr(c->hash, 22);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t probe_arena(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 4);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30273u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xd7734ee7u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64928u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x28) << 0;
    c->lane[6] += c->lane[1]; c->lane[8] ^= c->lane[6]; c->lane[8] = sx_rl(c->lane[8], 21);
    c->raw[c->slo + (int)((t0 + 49661u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[14] += c->lane[13]; c->lane[7] ^= c->lane[14]; c->lane[7] = sx_rl(c->lane[7], 18);
    c->hash ^= c->lane[13] + 0xbd721f7bu;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void hold_scope(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 59412u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xee6a531fu) ^ sx_rr(c->hash, 16);
    c->lane[5] ^= sx_rl(c->lane[10], 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x40) << 8;
    c->lane[8] ^= sx_rl(c->lane[9], 13);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x18) << 0;
    c->lane[4] ^= sx_rl(c->lane[13], 27);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x39224b27u;
    c->sum ^= t0 + t1;
    c->lane[10] ^= t2;
}

static uint8_t prime_label(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35296u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x956b176fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xdf) << 8;
    c->hash ^= c->lane[13] + 0xec20651fu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 28229u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18530u) % (uint32_t)c->rln)] << 0;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void place_ring(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xb2) << 16;
    c->lane[13] ^= sx_rl(c->lane[12], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x39c05765u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x23) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 58515u) % (uint32_t)c->rln)] << 16;
    c->lane[9] ^= sx_rl(c->lane[6], 26);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 30);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t close_layer(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[4] + 0x6477bef9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x92) << 0;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 7);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 54152u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0x4bfc204bu;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void peek_entry(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[2], 15);
    c->lane[11] ^= sx_rl(c->lane[2], 14);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 20);
    c->hash ^= c->lane[5] + 0x61185948u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 5);
    c->lane[4] += c->lane[13] ^ 0xe4535bb7u;
    t2 = (t2 ^ c->sum) * 0x121660b3u;
    c->sched[26] = c->hash ^ sx_rl(c->lane[6], 4);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint32_t tally_pairing(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 5);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55775u) % (uint32_t)c->rln)] << 8;
    c->lane[2] += c->lane[1]; c->lane[14] ^= c->lane[2]; c->lane[14] = sx_rl(c->lane[14], 27);
    c->hash ^= c->lane[7] + 0x2cb98f3du;
    c->sched[15] = c->hash ^ sx_rl(c->lane[2], 29);
    t2 = (t2 ^ c->sum) * 0x538675d5u;
    c->sched[7] = c->hash ^ sx_rl(c->lane[8], 10);
    c->lane[1] += c->lane[6]; c->lane[11] ^= c->lane[1]; c->lane[11] = sx_rl(c->lane[11], 8);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29331u) % (uint32_t)c->rln)] << 8;
    c->lane[3] ^= sx_rl(c->lane[10], 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t sift_slot(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[10] + 0x1694af36u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[4] ^= sx_rl(c->lane[6], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x71) << 16;
    c->hash = (c->hash * 0x651ab875u) ^ sx_rr(c->hash, 22);
    c->sched[11] = c->hash ^ sx_rl(c->lane[1], 31);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 21);
    c->sum += t1;
    return t0 + t2;
}

static void purge_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x72206887u;
    c->lane[8] += c->lane[7]; c->lane[12] ^= c->lane[8]; c->lane[12] = sx_rl(c->lane[12], 26);
    fill_region(c, t0, t1);
    t1 ^= (uint32_t)reset_arena(c, (uint8_t)(t0 >> 16), t2);
    split_arena(c, t0, t1);
    t2 += (uint32_t)yield_delta(c);
    flush_segment(c, &c->lane[5], 2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[5] + 0x3eb989adu;
    t2 += tap_batch(c, c->rlo, c->rln);
    c->raw[c->slo + (int)((t0 + 44360u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 += split_tail(c, c->rlo, c->rln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 5580u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)parse_marker(c);
    blend_part(c, &c->lane[7], 3);
    merge_list(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63997u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 65130u) % (uint32_t)c->rln)] << 24;
    mix_level(c, &c->lane[9], 1);
    pack_state(c, &c->lane[7], 4);
    t2 += (uint32_t)parse_field(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xb2) << 16;
    split_key(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static void blend_part(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[5] += c->lane[0]; c->lane[1] ^= c->lane[5]; c->lane[1] = sx_rl(c->lane[1], 25);
    t0 ^= close_table(c, t1);
    c->lane[9] ^= sx_rl(c->lane[1], 3);
    pick_marker(c, t0, t1);
    t1 ^= (uint32_t)clamp_queue(c, (uint8_t)(t0 >> 0), t2);
    t1 ^= (uint32_t)cache_page(c, (uint8_t)(t0 >> 8), t2);
    t0 ^= coal_group(c, t1);
    c->hash = (c->hash * 0x800c8385u) ^ sx_rr(c->hash, 16);
    coal_part(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x13e1c8f1u;
    t2 += (uint32_t)defer_gap(c);
    c->raw[c->slo + (int)((t0 + 59971u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[5] + 0xc8433855u;
    t0 ^= reap_slot(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->hash = (c->hash * 0x19dd64f7u) ^ sx_rr(c->hash, 31);
    t1 ^= (uint32_t)sort_region_669(c, (uint8_t)(t0 >> 8), t2);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 18);
    c->raw[c->slo + (int)((t0 + 11018u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 ^= defer_marker(c, t1);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t reset_arena(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t1 ^= (uint32_t)reap_range(c, (uint8_t)(t0 >> 16), t2);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa3643c9bu;
    t2 += (uint32_t)prime_stream_725(c);
    t0 ^= trim_digest(c, t1);
    fetch_tuple(c, t0, t1);
    t2 += (uint32_t)pack_queue(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= grow_ring(c, t1);
    t0 ^= pick_lease(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 61990u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x9031bd0du;
    t0 ^= chain_limit(c, t1);
    store_node(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void split_arena(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    join_batch(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x04966449u;
    c->lane[8] ^= sx_rl(c->lane[13], 28);
    c->hash ^= c->lane[8] + 0xa46b26d2u;
    t2 += (uint32_t)stage_node_651(c);
    t2 += trace_cursor(c, c->slo, c->sln);
    c->raw[c->slo + (int)((t0 + 36041u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x4aed28ebu;
    t2 += parse_line(c, c->rlo, c->rln);
    t1 ^= (uint32_t)store_tail(c, (uint8_t)(t0 >> 0), t2);
    c->lane[2] += c->lane[7] ^ 0x55e3dcbdu;
    c->raw[c->slo + (int)((t0 + 48728u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint8_t clamp_queue(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[13] ^= sx_rl(c->lane[2], 30);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 18);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xed) << 16;
    c->lane[5] += c->lane[9] ^ 0xcc34b84bu;
    c->raw[c->slo + (int)((t0 + 35330u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t1 ^= (uint32_t)hold_port(c, (uint8_t)(t0 >> 8), t2);
    t2 = (t2 ^ c->sum) * 0x4cc3f077u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34123u) % (uint32_t)c->rln)] << 16;
    t2 += drain_digest(c, c->rlo, c->rln);
    c->lane[2] += c->lane[14]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 16);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    t2 += (uint32_t)latch_record_672(c);
    c->raw[c->slo + (int)((t0 + 44184u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t grow_ring(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x845e1f67u;
    patch_page(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x80d29f57u;
    t2 += (uint32_t)parse_marker(c);
    c->hash ^= c->lane[7] + 0x40741298u;
    t1 ^= (uint32_t)flush_level(c, (uint8_t)(t0 >> 16), t2);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 2);
    t2 += probe_token(c, c->slo, c->sln);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 11);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22892u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 51431u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[11] += c->lane[4]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 20);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void fetch_tuple(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 21411u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 = (t2 ^ c->sum) * 0x01d1f9bfu;
    c->hash ^= c->lane[12] + 0x17aa3301u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x53) << 0;
    c->raw[c->slo + (int)((t0 + 7878u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 += (uint32_t)trim_queue_667(c);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 10);
    scan_layer(c, t0, t1);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 20);
    fill_region(c, t0, t1);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 9);
    join_value(c, t0, t1);
    t2 = (t2 ^ c->sum) * 0x888d797fu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= relay_head(c, t1);
    parse_mask(c, t0, t1);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint8_t store_tail(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    parse_mask(c, t0, t1);
    t0 ^= tally_span(c, t1);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 25);
    pick_marker(c, t0, t1);
    c->hash ^= c->lane[11] + 0xa9590ef6u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xab3fd621u;
    c->lane[4] ^= sx_rl(c->lane[11], 24);
    c->raw[c->slo + (int)((t0 + 12002u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t parse_line(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash = (c->hash * 0x682ecf07u) ^ sx_rr(c->hash, 17);
    t2 += seek_span_658(c, c->slo, c->sln);
    c->hash = (c->hash * 0xda969415u) ^ sx_rr(c->hash, 18);
    t0 ^= tally_span(c, t1);
    fill_stream(c, t0, t1);
    c->lane[4] ^= sx_rl(c->lane[14], 30);
    merge_list(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35156u) % (uint32_t)c->rln)] << 8;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t reap_range(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x58d0da61u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 27993u) % (uint32_t)c->rln)] << 8;
    t2 += grow_slot(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa2) << 16;
    c->lane[14] += c->lane[10]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29133u) % (uint32_t)c->rln)] << 16;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 13);
    c->lane[5] += c->lane[3] ^ 0x06912edcu;
    c->hash = (c->hash * 0x956d34f1u) ^ sx_rr(c->hash, 4);
    t2 = (t2 ^ c->sum) * 0xdf3ac49du;
    c->lane[14] += c->lane[2] ^ 0x63ed1002u;
    c->hash = (c->hash * 0x48cd5a15u) ^ sx_rr(c->hash, 24);
    slice_mask(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void coal_part(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t1 ^= (uint32_t)drain_span_663(c, (uint8_t)(t0 >> 8), t2);
    t2 += (uint32_t)sync_line(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x6862586fu;
    c->lane[12] += c->lane[6]; c->lane[11] ^= c->lane[12]; c->lane[11] = sx_rl(c->lane[11], 6);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static int stage_node_651(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->lane[2] += c->lane[9]; c->lane[5] ^= c->lane[2]; c->lane[5] = sx_rl(c->lane[5], 19);
    c->lane[2] = sx_rr(c->lane[2] + c->hash, 16);
    c->raw[c->slo + (int)((t0 + 16646u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[0] = c->hash ^ sx_rl(c->lane[9], 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55221u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x62) << 16;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 1);
    t2 += (uint32_t)blend_range(c);
    t2 += (uint32_t)reap_scope_694(c);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int pack_queue(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    t2 += (uint32_t)trim_queue_667(c);
    t2 = (t2 ^ c->sum) * 0xc20d20b1u;
    t2 += seek_block(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x47e6d48du;
    t2 = (t2 ^ c->sum) * 0x19b03a47u;
    c->raw[c->slo + (int)((t0 + 50616u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[24] = c->hash ^ sx_rl(c->lane[9], 30);
    c->raw[c->slo + (int)((t0 + 56074u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t1 ^= (uint32_t)rotate_pool(c, (uint8_t)(t0 >> 8), t2);
    t1 ^= (uint32_t)rotate_gap(c, (uint8_t)(t0 >> 8), t2);
    c->raw[c->slo + (int)((t0 + 3170u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[4] ^= sx_rl(c->lane[15], 3);
    c->lane[11] += c->lane[13]; c->lane[8] ^= c->lane[11]; c->lane[8] = sx_rl(c->lane[8], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x23) << 0;
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t cache_page(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56909u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 4917u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    flush_segment(c, &c->lane[5], 2);
    c->lane[4] += c->lane[2]; c->lane[10] ^= c->lane[4]; c->lane[10] = sx_rl(c->lane[10], 24);
    pack_state(c, &c->lane[9], 3);
    t1 ^= (uint32_t)sort_region_669(c, (uint8_t)(t0 >> 0), t2);
    c->hash ^= c->lane[6] + 0xaa2bce4du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xcfb18835u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void merge_list(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 51732u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    trim_view(c, t0, t1);
    t2 += (uint32_t)blend_node(c);
    t1 ^= (uint32_t)trace_stack_679(c, (uint8_t)(t0 >> 8), t2);
    c->sched[29] = c->hash ^ sx_rl(c->lane[4], 21);
    c->lane[13] += c->lane[6] ^ 0x8114eea3u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9435u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static void parse_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    fold_scope(c, &c->lane[1], 2);
    c->hash = (c->hash * 0x6163485du) ^ sx_rr(c->hash, 9);
    t2 += (uint32_t)reap_bound(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 20053u) % (uint32_t)c->rln)] << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x64) << 16;
    t2 += pack_node(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xc6) << 0;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 15);
    t2 += (uint32_t)load_stream(c);
    swap_label(c, &c->lane[3], 3);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint8_t rotate_pool(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x20f65e49u;
    t0 ^= align_label(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x10) << 16;
    c->sched[31] = c->hash ^ sx_rl(c->lane[8], 25);
    c->hash = (c->hash * 0xd549eedfu) ^ sx_rr(c->hash, 18);
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 26);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static void pack_state(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xbc20f051u;
    c->hash = (c->hash * 0x7e7c43edu) ^ sx_rr(c->hash, 5);
    scan_layer(c, t0, t1);
    t2 += (uint32_t)parse_field(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53553u) % (uint32_t)c->rln)] << 0;
    c->sched[18] = c->hash ^ sx_rl(c->lane[8], 26);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56674u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t2 += swap_cell(c, c->rlo, c->rln);
    c->sched[16] = c->hash ^ sx_rl(c->lane[8], 15);
    c->hash ^= c->lane[7] + 0x0e2ccc3bu;
    seek_chunk(c, &c->lane[3], 2);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t seek_span_658(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 11266u) % (uint32_t)c->rln)] << 8;
    t0 ^= tune_view(c, t1);
    t2 += sync_key(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x12e8a0bdu) ^ sx_rr(c->hash, 4);
    c->lane[10] += c->lane[3] ^ 0xa677c284u;
    c->lane[10] += c->lane[14]; c->lane[0] ^= c->lane[10]; c->lane[0] = sx_rl(c->lane[0], 30);
    c->sum += t1;
    return t0 + t2;
}

static void fill_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 323u) % (uint32_t)c->rln)] << 24;
    c->hash = (c->hash * 0xe0557fefu) ^ sx_rr(c->hash, 24);
    c->hash = (c->hash * 0x7a6253e7u) ^ sx_rr(c->hash, 26);
    c->sched[30] = c->hash ^ sx_rl(c->lane[9], 30);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t tally_span(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash ^= c->lane[14] + 0x2af56961u;
    c->hash = (c->hash * 0x2cfd96f7u) ^ sx_rr(c->hash, 3);
    t2 += pack_node(c, c->slo, c->sln);
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 8);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 23);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xcd9acf89u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void patch_page(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] += c->lane[5]; c->lane[11] ^= c->lane[3]; c->lane[11] = sx_rl(c->lane[11], 14);
    t2 = (t2 ^ c->sum) * 0xca0dcc0fu;
    t2 += slice_part(c, c->slo, c->sln);
    c->hash ^= c->lane[11] + 0xdeaa4855u;
    t2 += (uint32_t)reap_scope_694(c);
    t2 = (t2 ^ c->sum) * 0x3cf42501u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8782u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[15] ^= sx_rl(c->lane[1], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)mark_page(c, (uint8_t)(t0 >> 0), t2);
    c->sum ^= t0 + t1;
    c->lane[11] ^= t2;
}

static uint8_t hold_port(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sched[19] = c->hash ^ sx_rl(c->lane[4], 21);
    c->lane[15] ^= sx_rl(c->lane[14], 16);
    c->lane[1] ^= sx_rl(c->lane[2], 30);
    c->lane[4] += c->lane[10]; c->lane[0] ^= c->lane[4]; c->lane[0] = sx_rl(c->lane[0], 13);
    c->hash ^= c->lane[10] + 0x7cc46478u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint8_t drain_span_663(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] += c->lane[6] ^ 0xd3206231u;
    t1 ^= (uint32_t)trace_stack_679(c, (uint8_t)(t0 >> 8), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= trace_view(c, t1);
    c->lane[10] += c->lane[5]; c->lane[4] ^= c->lane[10]; c->lane[4] = sx_rl(c->lane[4], 28);
    t2 += (uint32_t)mix_node(c);
    t2 += (uint32_t)emit_field_680(c);
    t0 ^= fold_item(c, t1);
    c->sched[22] = c->hash ^ sx_rl(c->lane[2], 27);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static int blend_range(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->hash ^= c->lane[0] + 0x134a9f02u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    t2 += slice_part(c, c->slo, c->sln);
    t0 ^= prime_limit(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->hash ^= c->lane[8] + 0x69d7f20cu;
    seek_chunk(c, &c->lane[10], 1);
    c->hash ^= c->lane[3] + 0xd4cf4039u;
    t2 = (t2 ^ c->sum) * 0xcb288a03u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void flush_segment(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x28) << 8;
    t2 += pin_value(c, c->rlo, c->rln);
    c->lane[11] += c->lane[10] ^ 0xb812a631u;
    t0 ^= load_group(c, t1);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x73) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t flush_level(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash = (c->hash * 0xeb5035bfu) ^ sx_rr(c->hash, 24);
    mix_row(c, t0, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x20607c0fu) ^ sx_rr(c->hash, 8);
    c->lane[7] += c->lane[1] ^ 0xbf145facu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    join_batch(c, t0, t1);
    t1 ^= (uint32_t)shift_key(c, (uint8_t)(t0 >> 0), t2);
    patch_line_740(c, &c->lane[10], 2);
    tap_window(c, t0, t1);
    c->lane[14] += c->lane[2] ^ 0xb9707850u;
    t1 ^= (uint32_t)clamp_scope(c, (uint8_t)(t0 >> 0), t2);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static int trim_queue_667(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[4];
    t2 = (t2 ^ c->sum) * 0x84f559cbu;
    c->sched[23] = c->hash ^ sx_rl(c->lane[4], 31);
    c->sched[15] = c->hash ^ sx_rl(c->lane[11], 2);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t2 = (t2 ^ c->sum) * 0x2d0804cfu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xb96e5debu;
    c->hash ^= c->lane[10] + 0x4deca0c0u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 8);
    c->hash = (c->hash * 0xef27f495u) ^ sx_rr(c->hash, 5);
    t1 ^= (uint32_t)rotate_gap(c, (uint8_t)(t0 >> 16), t2);
    c->lane[4] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int parse_marker(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash = (c->hash * 0x8a478ebdu) ^ sx_rr(c->hash, 31);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46143u) % (uint32_t)c->rln)] << 0;
    move_page(c, t0, t1);
    t1 ^= (uint32_t)sync_mask(c, (uint8_t)(t0 >> 0), t2);
    c->hash = (c->hash * 0xe8ac44edu) ^ sx_rr(c->hash, 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 3859u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x38a80c99u;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x33) << 0;
    c->hash ^= c->lane[0] + 0xeed069f3u;
    c->lane[4] += c->lane[6] ^ 0x71bde750u;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 28);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x03b4bae1u;
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t sort_region_669(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 += (uint32_t)sync_line(c);
    t0 ^= reap_slot(c, t1);
    store_node(c, t0, t1);
    c->lane[5] ^= sx_rl(c->lane[2], 11);
    t2 += (uint32_t)fold_span_727(c);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 15);
    c->hash ^= c->lane[1] + 0x3af26865u;
    c->hash ^= c->lane[6] + 0x2fe64395u;
    t2 = (t2 ^ c->sum) * 0x6f37f987u;
    c->lane[0] ^= sx_rl(c->lane[15], 1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void pick_marker(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0xbcf74281u) ^ sx_rr(c->hash, 16);
    c->raw[c->slo + (int)((t0 + 21308u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf0) << 16;
    c->sched[8] = c->hash ^ sx_rl(c->lane[5], 14);
    chain_row_767(c, t0, t1);
    t1 ^= (uint32_t)map_stream(c, (uint8_t)(t0 >> 8), t2);
    t2 += probe_token(c, c->slo, c->sln);
    c->lane[2] += c->lane[15] ^ 0x2a1468c8u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[13], 29);
    t2 = (t2 ^ c->sum) * 0x2b642b1du;
    c->hash = (c->hash * 0x1a6efa4bu) ^ sx_rr(c->hash, 5);
    c->hash ^= c->lane[8] + 0xfa791373u;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static void fill_stream(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    t0 ^= defer_marker(c, t1);
    c->lane[8] += c->lane[5] ^ 0xf8747066u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 ^= pair_index(c, t1);
    t0 ^= peek_delta(c, t1);
    c->lane[9] += c->lane[4] ^ 0xfcde3bf8u;
    t0 ^= link_layer(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)sync_mask(c, (uint8_t)(t0 >> 16), t2);
    t2 = (t2 ^ c->sum) * 0x3e49118du;
    c->hash = (c->hash * 0x349c7d05u) ^ sx_rr(c->hash, 5);
    c->lane[8] += c->lane[14]; c->lane[0] ^= c->lane[8]; c->lane[0] = sx_rl(c->lane[0], 29);
    c->hash = (c->hash * 0xd0a79505u) ^ sx_rr(c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}

static int latch_record_672(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[3];
    t2 = (t2 ^ c->sum) * 0x1233ffe1u;
    c->raw[c->slo + (int)((t0 + 10926u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= c->lane[6] + 0xeae3c6e6u;
    t2 = (t2 ^ c->sum) * 0xbe2e30c3u;
    t2 += (uint32_t)pair_part_693(c);
    c->lane[8] += c->lane[5] ^ 0x6be9c1f9u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x94bebfd9u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 38450u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[10] + 0x06ae5544u;
    t2 = (t2 ^ c->sum) * 0xc837cc2bu;
    c->lane[3] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t relay_head(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xa948d3d1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x6b811e3bu;
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 9);
    t1 ^= (uint32_t)peek_scope_705(c, (uint8_t)(t0 >> 8), t2);
    c->hash = (c->hash * 0x3ac339a9u) ^ sx_rr(c->hash, 25);
    c->lane[14] ^= sx_rl(c->lane[9], 16);
    c->hash ^= c->lane[5] + 0x1f12d474u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46323u) % (uint32_t)c->rln)] << 16;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    relay_table(c, &c->lane[10], 4);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t prime_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0xf8875fd1u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48351u) % (uint32_t)c->rln)] << 0;
    t2 += split_tail(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0xcff4d869u;
    c->raw[c->slo + (int)((t0 + 61359u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15122u) % (uint32_t)c->rln)] << 24;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xf43db619u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t1 ^= (uint32_t)stage_tuple(c, (uint8_t)(t0 >> 8), t2);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 44259u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void swap_label(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 42808u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x62b5e827u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 42006u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int parse_field(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[10];
    mix_level(c, &c->lane[1], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[6] += c->lane[10]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 25);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26225u) % (uint32_t)c->rln)] << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 34403u) % (uint32_t)c->rln)] << 8;
    t2 += (uint32_t)fold_span_727(c);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sched[5] = c->hash ^ sx_rl(c->lane[12], 10);
    c->lane[0] += c->lane[6] ^ 0xf4b0f34cu;
    c->lane[10] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void mix_row(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 9);
    c->lane[1] = sx_rr(c->lane[1] + c->hash, 2);
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 2);
    c->hash = (c->hash * 0x7f1a6cd9u) ^ sx_rr(c->hash, 23);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd1) << 0;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->hash = (c->hash * 0x8e208221u) ^ sx_rr(c->hash, 9);
    c->lane[7] += c->lane[0]; c->lane[9] ^= c->lane[7]; c->lane[9] = sx_rl(c->lane[9], 4);
    t2 = (t2 ^ c->sum) * 0xc7ff2e87u;
    c->raw[c->slo + (int)((t0 + 11398u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x97a302d7u) ^ sx_rr(c->hash, 2);
    c->sum ^= t0 + t1;
    c->lane[4] ^= t2;
}

static uint32_t fold_item(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 31);
    c->sched[15] = c->hash ^ sx_rl(c->lane[12], 3);
    c->hash = (c->hash * 0x776056f9u) ^ sx_rr(c->hash, 24);
    t2 += (uint32_t)yield_delta(c);
    patch_line_740(c, &c->lane[10], 1);
    fold_scope(c, &c->lane[11], 1);
    c->raw[c->slo + (int)((t0 + 25510u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x8ed791f5u;
    c->sched[18] = c->hash ^ sx_rl(c->lane[4], 9);
    c->lane[15] += c->lane[11]; c->lane[6] ^= c->lane[15]; c->lane[6] = sx_rl(c->lane[6], 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t trace_stack_679(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    slice_mask(c, t0, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t0 ^= shift_view(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 += grow_slot(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xb7fc6995u;
    fill_path(c, t0, t1);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static int emit_field_680(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[6] += c->lane[14]; c->lane[12] ^= c->lane[6]; c->lane[12] = sx_rl(c->lane[12], 13);
    c->hash ^= c->lane[0] + 0x40eb1dfau;
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int sync_line(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x56beab81u;
    c->lane[7] += c->lane[10]; c->lane[8] ^= c->lane[7]; c->lane[8] = sx_rl(c->lane[8], 26);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xa3c20ae5u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 += swap_token_731(c, c->slo, c->sln);
    pin_stack(c, t0, t1);
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t map_stream(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[2] += c->lane[9]; c->lane[10] ^= c->lane[2]; c->lane[10] = sx_rl(c->lane[10], 4);
    c->hash = (c->hash * 0xa19e1ca5u) ^ sx_rr(c->hash, 7);
    c->hash = (c->hash * 0x77743179u) ^ sx_rr(c->hash, 20);
    t2 = (t2 ^ c->sum) * 0xfe31aa29u;
    c->lane[9] ^= sx_rl(c->lane[11], 19);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void move_page(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash = (c->hash * 0x4402ede1u) ^ sx_rr(c->hash, 17);
    c->lane[9] ^= sx_rl(c->lane[5], 13);
    t2 = (t2 ^ c->sum) * 0x41a35e9bu;
    c->lane[4] ^= sx_rl(c->lane[7], 5);
    c->hash = (c->hash * 0xa23a769du) ^ sx_rr(c->hash, 22);
    c->sched[12] = c->hash ^ sx_rl(c->lane[11], 4);
    c->sched[28] = c->hash ^ sx_rl(c->lane[13], 11);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x8c) << 0;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 15);
    t2 += (uint32_t)prime_stream_725(c);
    c->raw[c->slo + (int)((t0 + 9564u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[6] + 0x9b169c26u;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint32_t align_label(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[4] += c->lane[12] ^ 0xb6957830u;
    t2 = (t2 ^ c->sum) * 0x774f682fu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xca0d51a1u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63908u) % (uint32_t)c->rln)] << 0;
    t2 += (uint32_t)fold_tuple(c);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48879u) % (uint32_t)c->rln)] << 8;
    c->lane[8] ^= sx_rl(c->lane[5], 14);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[6] += c->lane[10]; c->lane[11] ^= c->lane[6]; c->lane[11] = sx_rl(c->lane[11], 2);
    c->hash = (c->hash * 0x6d73b961u) ^ sx_rr(c->hash, 7);
    c->hash ^= c->lane[3] + 0x7edf7887u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static void join_batch(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 5832u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += mark_list(c, c->rlo, c->rln);
    c->lane[3] ^= sx_rl(c->lane[8], 25);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->hash = (c->hash * 0x3fb4968du) ^ sx_rr(c->hash, 5);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[12] = sx_rr(c->lane[12] + c->hash, 16);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x70) << 16;
    c->sum ^= t0 + t1;
    c->lane[1] ^= t2;
}

static uint8_t rotate_gap(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[11] += c->lane[14]; c->lane[1] ^= c->lane[11]; c->lane[1] = sx_rl(c->lane[1], 6);
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 21);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 56601u) % (uint32_t)c->rln)] << 24;
    t2 = (t2 ^ c->sum) * 0x5f3daff7u;
    c->hash = (c->hash * 0x9ca214bfu) ^ sx_rr(c->hash, 20);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void store_node(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[0] ^= sx_rl(c->lane[13], 11);
    t2 = (t2 ^ c->sum) * 0x407e5e77u;
    c->lane[7] += c->lane[9]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 8);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 20);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 2);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x77) << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xaf) << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    t2 += clamp_store(c, c->rlo, c->rln);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x77289d6du;
    c->sum ^= t0 + t1;
    c->lane[12] ^= t2;
}

static void scan_layer(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x9c011afdu;
    t0 ^= trim_digest(c, t1);
    c->hash = (c->hash * 0x5f39f9a9u) ^ sx_rr(c->hash, 11);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash = (c->hash * 0x4a22ec61u) ^ sx_rr(c->hash, 21);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 3);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sched[2] = c->hash ^ sx_rl(c->lane[9], 4);
    c->lane[9] ^= sx_rl(c->lane[9], 18);
    t0 ^= chain_limit(c, t1);
    c->sched[0] = c->hash ^ sx_rl(c->lane[1], 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 9131u) % (uint32_t)c->rln)] << 8;
    c->lane[11] = sx_rr(c->lane[11] + c->hash, 1);
    c->sum ^= t0 + t1;
    c->lane[14] ^= t2;
}

static uint32_t pack_node(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    chain_row(c, &c->lane[2], 1);
    c->lane[13] += c->lane[0]; c->lane[15] ^= c->lane[13]; c->lane[15] = sx_rl(c->lane[15], 22);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x2d32e92du;
    c->sched[21] = c->hash ^ sx_rl(c->lane[1], 21);
    t0 ^= close_key(c, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1956u) % (uint32_t)c->rln)] << 0;
    c->lane[10] += c->lane[1]; c->lane[2] ^= c->lane[10]; c->lane[2] = sx_rl(c->lane[2], 2);
    c->lane[15] += c->lane[13] ^ 0x51e02c62u;
    c->lane[1] += c->lane[11]; c->lane[7] ^= c->lane[1]; c->lane[7] = sx_rl(c->lane[7], 17);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26188u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    t1 ^= (uint32_t)merge_pairing(c, (uint8_t)(t0 >> 0), t2);
    coal_region(c, t0, t1);
    c->sum += t1;
    return t0 + t2;
}

static void tap_window(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 += trace_cursor(c, c->rlo, c->rln);
    c->sched[8] = c->hash ^ sx_rl(c->lane[0], 10);
    t2 = (t2 ^ c->sum) * 0xa59bae89u;
    t2 = (t2 ^ c->sum) * 0xb30c912fu;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf8) << 8;
    c->hash ^= c->lane[0] + 0x7099b2cau;
    c->lane[0] += c->lane[1]; c->lane[8] ^= c->lane[0]; c->lane[8] = sx_rl(c->lane[8], 11);
    c->hash ^= c->lane[5] + 0xf18aac00u;
    scan_cell(c, &c->lane[7], 2);
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 11);
    t1 ^= (uint32_t)stage_tuple(c, (uint8_t)(t0 >> 0), t2);
    c->sum ^= t0 + t1;
    c->lane[3] ^= t2;
}

static uint32_t link_layer(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd2) << 8;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 7);
    t1 ^= (uint32_t)poll_limit(c, (uint8_t)(t0 >> 0), t2);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[10] ^= sx_rl(c->lane[14], 6);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t peek_delta(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[12] += c->lane[2] ^ 0xdc177290u;
    c->lane[11] += c->lane[5] ^ 0x2258f7d8u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[3] ^= sx_rl(c->lane[13], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 25);
    t0 ^= tune_view(c, t1);
    c->hash ^= t2;
    return t0 ^ t1;
}

static int pair_part_693(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x3392cabdu;
    t2 += align_arena(c, c->rlo, c->rln);
    c->lane[15] ^= sx_rl(c->lane[14], 24);
    split_key(c, t0, t1);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31122u) % (uint32_t)c->rln)] << 0;
    t1 ^= (uint32_t)clamp_scope(c, (uint8_t)(t0 >> 16), t2);
    c->hash = (c->hash * 0xcd39282bu) ^ sx_rr(c->hash, 3);
    c->lane[10] ^= sx_rl(c->lane[3], 23);
    c->lane[10] ^= sx_rl(c->lane[7], 10);
    c->hash ^= c->lane[0] + 0x3e248e6cu;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int reap_scope_694(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->hash = (c->hash * 0x92022ecdu) ^ sx_rr(c->hash, 6);
    c->raw[c->slo + (int)((t0 + 12390u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash ^= c->lane[10] + 0x90d9bda1u;
    c->raw[c->slo + (int)((t0 + 59284u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    relay_table(c, &c->lane[7], 3);
    c->hash = (c->hash * 0xf2b5ce1du) ^ sx_rr(c->hash, 23);
    t2 += (uint32_t)blend_node(c);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t2 += mark_window(c, c->rlo, c->rln);
    t2 = (t2 ^ c->sum) * 0xee12a003u;
    c->hash = (c->hash * 0xf5bcc8fbu) ^ sx_rr(c->hash, 23);
    c->lane[15] += c->lane[11] ^ 0xd6d82c99u;
    t2 += swap_cell(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x30c47655u) ^ sx_rr(c->hash, 7);
    hold_track(c, &c->lane[9], 3);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 8);
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int mix_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x3b) << 8;
    t1 ^= (uint32_t)cache_list_713(c, (uint8_t)(t0 >> 0), t2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->lane[1] += c->lane[10]; c->lane[7] ^= c->lane[1]; c->lane[7] = sx_rl(c->lane[7], 14);
    c->lane[11] += c->lane[8]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 19);
    t1 ^= (uint32_t)poll_limit(c, (uint8_t)(t0 >> 16), t2);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t defer_marker(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash = (c->hash * 0x77614c4du) ^ sx_rr(c->hash, 5);
    c->raw[c->slo + (int)((t0 + 10943u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t2 += (uint32_t)fold_tuple(c);
    c->lane[6] = sx_rr(c->lane[6] + c->hash, 12);
    c->hash = (c->hash * 0x7a694e8bu) ^ sx_rr(c->hash, 3);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4248u) % (uint32_t)c->rln)] << 16;
    join_value(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb87f5c2bu;
    t2 += (uint32_t)defer_gap(c);
    c->hash ^= c->lane[12] + 0x6455d546u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t probe_token(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 += seek_block(c, c->slo, c->sln);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xcb) << 8;
    c->lane[1] += c->lane[6]; c->lane[7] ^= c->lane[1]; c->lane[7] = sx_rl(c->lane[7], 21);
    t2 = (t2 ^ c->sum) * 0x8b696db3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x60) << 16;
    c->sum += t1;
    return t0 + t2;
}

static void seek_chunk(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[1] += c->lane[3]; c->lane[4] ^= c->lane[1]; c->lane[4] = sx_rl(c->lane[4], 5);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x9b1ceaadu;
    c->raw[c->slo + (int)((t0 + 62730u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sched[22] = c->hash ^ sx_rl(c->lane[3], 24);
    t2 += split_mask(c, c->rlo, c->rln);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->raw[c->slo + (int)((t0 + 56297u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    chain_row_767(c, t0, t1);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x70e1e665u;
    c->hash ^= c->lane[8] + 0x92f90c5bu;
    c->raw[c->slo + (int)((t0 + 33439u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t reap_slot(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[0] += c->lane[14] ^ 0x83a79989u;
    c->hash ^= c->lane[2] + 0xb3a32a26u;
    c->lane[15] ^= sx_rl(c->lane[1], 25);
    c->raw[c->slo + (int)((t0 + 10486u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 1166u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[6] ^= sx_rl(c->lane[5], 27);
    c->lane[14] += c->lane[4]; c->lane[0] ^= c->lane[14]; c->lane[0] = sx_rl(c->lane[0], 15);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    t0 ^= step_delta(c, t1);
    sift_scope(c, &c->lane[4], 3);
    c->sched[9] = c->hash ^ sx_rl(c->lane[5], 30);
    patch_port(c, &c->lane[8], 2);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t trace_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd4) << 0;
    t0 ^= coal_group(c, t1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x68242005u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
    c->sched[27] = c->hash ^ sx_rl(c->lane[13], 6);
    c->hash ^= c->lane[4] + 0x6a67af5au;
    c->raw[c->slo + (int)((t0 + 18843u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x4ccbf9cbu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x3803a6f5u;
    c->hash ^= t2;
    return t0 ^ t1;
}

static int load_stream(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[8];
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 17632u) % (uint32_t)c->rln)] << 0;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48820u) % (uint32_t)c->rln)] << 8;
    c->lane[14] ^= sx_rl(c->lane[6], 2);
    c->lane[8] += c->lane[12] ^ 0x778c7c7du;
    c->lane[7] += c->lane[14] ^ 0x63cf6c60u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 2000u) % (uint32_t)c->rln)] << 24;
    c->raw[c->slo + (int)((t0 + 17562u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t2 = (t2 ^ c->sum) * 0xf3675831u;
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 23);
    t2 += (uint32_t)split_table(c);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xd0) << 16;
    c->hash ^= c->lane[15] + 0x7f9a1679u;
    t2 += grow_mask(c, c->rlo, c->rln);
    c->lane[8] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t slice_part(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29779u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 24558u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->hash = (c->hash * 0xdf74cbf1u) ^ sx_rr(c->hash, 23);
    t2 += mark_window(c, c->slo, c->sln);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 22923u) % (uint32_t)c->rln)] << 16;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xf7) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash = (c->hash * 0x15cb49ffu) ^ sx_rr(c->hash, 24);
    t0 ^= pick_lease(c, t1);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t sync_key(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    trim_view(c, t0, t1);
    c->raw[c->slo + (int)((t0 + 34470u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->raw[c->slo + (int)((t0 + 61940u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sched[22] = c->hash ^ sx_rl(c->lane[7], 26);
    c->sched[16] = c->hash ^ sx_rl(c->lane[15], 25);
    c->lane[12] += c->lane[8]; c->lane[6] ^= c->lane[12]; c->lane[6] = sx_rl(c->lane[6], 5);
    t0 ^= grow_item(c, t1);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum += t1;
    return t0 + t2;
}

static uint32_t load_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t2 += drain_digest(c, c->slo, c->sln);
    c->lane[4] += c->lane[1]; c->lane[10] ^= c->lane[4]; c->lane[10] = sx_rl(c->lane[10], 5);
    c->hash = (c->hash * 0xfe880613u) ^ sx_rr(c->hash, 9);
    c->sched[21] = c->hash ^ sx_rl(c->lane[0], 5);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->raw[c->slo + (int)((t0 + 4954u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0xaff17f39u) ^ sx_rr(c->hash, 31);
    c->lane[3] ^= sx_rl(c->lane[13], 21);
    t2 += (uint32_t)push_ring(c);
    c->lane[7] += c->lane[12] ^ 0xe55e834au;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t peek_scope_705(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[0] + 0x81f5ec93u;
    c->lane[9] ^= sx_rl(c->lane[2], 23);
    t2 = (t2 ^ c->sum) * 0x3249eb9fu;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 26);
    t2 += tap_batch(c, c->rlo, c->rln);
    t2 += pin_value(c, c->slo, c->sln);
    t2 = (t2 ^ c->sum) * 0x403b2667u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xed) << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint8_t sync_mask(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26099u) % (uint32_t)c->rln)] << 24;
    t2 += (uint32_t)reap_bound(c);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x4bc6edbfu;
    t1 ^= (uint32_t)shift_key(c, (uint8_t)(t0 >> 16), t2);
    c->sched[9] = c->hash ^ sx_rl(c->lane[13], 12);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x08c553fbu;
    c->sched[10] = c->hash ^ sx_rl(c->lane[8], 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint8_t mark_page(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    t2 = (t2 ^ c->sum) * 0x5eb3c0b3u;
    t2 += pin_bound(c, c->slo, c->sln);
    c->hash = (c->hash * 0x2ff503efu) ^ sx_rr(c->hash, 23);
    c->lane[11] += c->lane[12]; c->lane[7] ^= c->lane[11]; c->lane[7] = sx_rl(c->lane[7], 19);
    t0 ^= close_table(c, t1);
    c->raw[c->slo + (int)((t0 + 9472u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[7] ^= sx_rl(c->lane[5], 30);
    t2 = (t2 ^ c->sum) * 0xd804c4a7u;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static uint32_t pair_index(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[4]); t1 += c->step;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 7);
    t2 += tune_level(c, c->rlo, c->rln);
    c->hash = (c->hash * 0x2d28c419u) ^ sx_rr(c->hash, 27);
    c->lane[7] += c->lane[7] ^ 0x7100e84eu;
    c->sched[29] = c->hash ^ sx_rl(c->lane[5], 31);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void fill_path(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= c->lane[3] + 0xa0574d9cu;
    t2 = (t2 ^ c->sum) * 0x3e87a553u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xac7a3af3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[21] = c->hash ^ sx_rl(c->lane[2], 21);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t seek_block(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0xec9189bfu) ^ sx_rr(c->hash, 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x4c) << 8;
    c->sum += t1;
    return t0 + t2;
}

static void chain_row(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 29);
    t2 = (t2 ^ c->sum) * 0x7fdcea47u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[15], 14);
    c->raw[c->slo + (int)((t0 + 8192u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[1] ^= sx_rl(c->lane[11], 10);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[2] += c->lane[4]; c->lane[7] ^= c->lane[2]; c->lane[7] = sx_rl(c->lane[7], 18);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t mark_list(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->raw[c->slo + (int)((t0 + 52108u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->raw[c->slo + (int)((t0 + 9243u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->sched[24] = c->hash ^ sx_rl(c->lane[7], 29);
    c->hash = (c->hash * 0x904c1ff3u) ^ sx_rr(c->hash, 1);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->sched[22] = c->hash ^ sx_rl(c->lane[1], 26);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    t2 = (t2 ^ c->sum) * 0x19f3e709u;
    c->sum += t1;
    return t0 + t2;
}

static uint8_t cache_list_713(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[10] ^= sx_rl(c->lane[11], 12);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 18670u) % (uint32_t)c->rln)] << 16;
    c->lane[4] ^= sx_rl(c->lane[1], 3);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 29);
    c->hash = (c->hash * 0xafc2168fu) ^ sx_rr(c->hash, 3);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x36) << 16;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 16);
}

static void relay_table(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[0] += c->lane[11]; c->lane[13] ^= c->lane[0]; c->lane[13] = sx_rl(c->lane[13], 24);
    c->sched[0] = c->hash ^ sx_rl(c->lane[8], 12);
    c->hash ^= c->lane[13] + 0xc459ab10u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1104u) % (uint32_t)c->rln)] << 16;
    c->lane[14] ^= sx_rl(c->lane[4], 5);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 4);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[0] += c->lane[12] ^ 0x1de7248du;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xd1) << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t close_key(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xa4) << 8;
    t2 = (t2 ^ c->sum) * 0x74d2f74du;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc1a5f1d9u;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x33) << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->sched[10] = c->hash ^ sx_rl(c->lane[12], 9);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26642u) % (uint32_t)c->rln)] << 0;
    t2 = (t2 ^ c->sum) * 0x96700669u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= t2;
    return t0 ^ t1;
}

static void trim_view(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xc0407f75u;
    c->lane[13] += c->lane[4] ^ 0xd97b2367u;
    c->hash = (c->hash * 0xbfad3b11u) ^ sx_rr(c->hash, 8);
    c->hash = (c->hash * 0x43ed6197u) ^ sx_rr(c->hash, 19);
    c->lane[7] = sx_rr(c->lane[7] + c->hash, 22);
    c->lane[11] += c->lane[13]; c->lane[15] ^= c->lane[11]; c->lane[15] = sx_rl(c->lane[15], 14);
    c->sched[15] = c->hash ^ sx_rl(c->lane[11], 27);
    c->sum ^= t0 + t1;
    c->lane[2] ^= t2;
}

static uint32_t pin_value(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41706u) % (uint32_t)c->rln)] << 16;
    c->lane[15] += c->lane[2] ^ 0x6bf624dcu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->raw[c->slo + (int)((t0 + 48448u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 21);
    c->sum += t1;
    return t0 + t2;
}

static void scan_cell(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[12] + 0xbf412fedu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xaa29aa4bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[13] ^= sx_rl(c->lane[3], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52427u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[7] += c->lane[9]; c->lane[15] ^= c->lane[7]; c->lane[15] = sx_rl(c->lane[15], 27);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t align_arena(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[8] += c->lane[7]; c->lane[13] ^= c->lane[8]; c->lane[13] = sx_rl(c->lane[13], 7);
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 14);
    c->hash = (c->hash * 0x121eed45u) ^ sx_rr(c->hash, 26);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x9a8fe01fu) ^ sx_rr(c->hash, 9);
    c->lane[12] += c->lane[7] ^ 0xd6adecc8u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static uint32_t drain_digest(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[19] = c->hash ^ sx_rl(c->lane[13], 7);
    t2 = (t2 ^ c->sum) * 0x0b991ebfu;
    c->hash = (c->hash * 0x0204552fu) ^ sx_rr(c->hash, 9);
    t2 = (t2 ^ c->sum) * 0xf7b08b2bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[10] += c->lane[8]; c->lane[14] ^= c->lane[10]; c->lane[14] = sx_rl(c->lane[14], 21);
    c->sum += t1;
    return t0 + t2;
}

static uint32_t tune_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[6] ^= sx_rl(c->lane[9], 5);
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 8);
    c->hash ^= c->lane[3] + 0xb7edbdbcu;
    c->lane[0] += c->lane[3]; c->lane[10] ^= c->lane[0]; c->lane[10] = sx_rl(c->lane[10], 16);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t poll_limit(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[13] + 0x32a51aedu;
    c->hash = (c->hash * 0x94183809u) ^ sx_rr(c->hash, 20);
    c->hash = (c->hash * 0x213fd625u) ^ sx_rr(c->hash, 21);
    c->lane[1] += c->lane[2] ^ 0xf57b1539u;
    c->hash ^= c->lane[15] + 0x9e8cadf9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t tune_level(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->raw[c->slo + (int)((t0 + 59349u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[7] += c->lane[8]; c->lane[1] ^= c->lane[7]; c->lane[1] = sx_rl(c->lane[1], 4);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xc8) << 0;
    c->raw[c->slo + (int)((t0 + 35805u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 6);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1382u) % (uint32_t)c->rln)] << 0;
    c->raw[c->slo + (int)((t0 + 2734u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->sum += t1;
    return t0 + t2;
}

static void sift_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->lane[8] ^= sx_rl(c->lane[3], 24);
    c->lane[9] += c->lane[15]; c->lane[10] ^= c->lane[9]; c->lane[10] = sx_rl(c->lane[10], 21);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x4b) << 8;
    c->hash = (c->hash * 0x842de56bu) ^ sx_rr(c->hash, 8);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static int prime_stream_725(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[2];
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash ^= c->lane[12] + 0xae112f52u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sched[15] = c->hash ^ sx_rl(c->lane[7], 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x13288223u;
    c->lane[2] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int split_table(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->raw[c->slo + (int)((t0 + 12005u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[0] ^= sx_rl(c->lane[9], 7);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x68b60585u;
    c->lane[12] += c->lane[14]; c->lane[0] ^= c->lane[12]; c->lane[0] = sx_rl(c->lane[0], 12);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int fold_span_727(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[12];
    c->lane[14] = sx_rr(c->lane[14] + c->hash, 1);
    c->lane[13] += c->lane[9] ^ 0xcb06f263u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x5b7b4e55u;
    c->hash ^= c->lane[11] + 0x196921b7u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4921u) % (uint32_t)c->rln)] << 24;
    c->sched[21] = c->hash ^ sx_rl(c->lane[7], 20);
    c->lane[11] ^= sx_rl(c->lane[3], 7);
    c->lane[12] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static int push_ring(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[0];
    c->sched[7] = c->hash ^ sx_rl(c->lane[13], 18);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xed8ae83du;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 29);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 30);
    c->lane[3] += c->lane[14] ^ 0x8c405a41u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
    c->sched[29] = c->hash ^ sx_rl(c->lane[11], 22);
    c->hash = (c->hash * 0xb1e08c55u) ^ sx_rr(c->hash, 20);
    c->hash ^= c->lane[11] + 0x8b06b638u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x5b) << 0;
    c->lane[4] += c->lane[2]; c->lane[10] ^= c->lane[4]; c->lane[10] = sx_rl(c->lane[10], 24);
    c->lane[0] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t trace_cursor(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x74) << 16;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26929u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0xe7a3486fu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 23036u) % (uint32_t)c->rln)] << 16;
    c->lane[5] ^= sx_rl(c->lane[3], 13);
    c->sched[2] = c->hash ^ sx_rl(c->lane[9], 2);
    c->raw[c->slo + (int)((t0 + 21728u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[11] += c->lane[9]; c->lane[10] ^= c->lane[11]; c->lane[10] = sx_rl(c->lane[10], 2);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void slice_mask(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x9cdc3abbu;
    c->lane[11] ^= sx_rl(c->lane[15], 14);
    c->hash = (c->hash * 0xfbe2c5c9u) ^ sx_rr(c->hash, 26);
    c->hash ^= c->lane[15] + 0x43012f67u;
    c->raw[c->slo + (int)((t0 + 34272u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    t2 = (t2 ^ c->sum) * 0xf8cce393u;
    c->raw[c->slo + (int)((t0 + 64525u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum ^= t0 + t1;
    c->lane[5] ^= t2;
}

static uint32_t swap_token_731(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x087a7cf3u;
    c->hash = (c->hash * 0xb434ac63u) ^ sx_rr(c->hash, 25);
    c->lane[14] += c->lane[6]; c->lane[2] ^= c->lane[14]; c->lane[2] = sx_rl(c->lane[2], 27);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x94ae3a17u) ^ sx_rr(c->hash, 24);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55282u) % (uint32_t)c->rln)] << 16;
    c->sum += t1;
    return t0 + t2;
}

static void coal_region(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0x28225783u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x8a9c0dadu;
    c->lane[0] += c->lane[15] ^ 0x4bf2c999u;
    c->raw[c->slo + (int)((t0 + 51193u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 30184u) % (uint32_t)c->rln)] << 16;
    c->raw[c->slo + (int)((t0 + 9425u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[15] += c->lane[5]; c->lane[8] ^= c->lane[15]; c->lane[8] = sx_rl(c->lane[8], 9);
    c->lane[5] += c->lane[4] ^ 0x54a1267fu;
    c->sum ^= t0 + t1;
    c->lane[9] ^= t2;
}

static uint32_t step_delta(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[14] += c->lane[15] ^ 0xf7584b86u;
    c->hash = (c->hash * 0x7fe2744fu) ^ sx_rr(c->hash, 15);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xa5505329u;
    c->sched[0] = c->hash ^ sx_rl(c->lane[12], 29);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t split_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[2] += c->lane[1] ^ 0x2c68f0d9u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x2c) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x30afc99bu;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x7086bca1u;
    c->raw[c->slo + (int)((t0 + 25727u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash = (c->hash * 0x13be1a93u) ^ sx_rr(c->hash, 23);
    c->lane[5] = sx_rr(c->lane[5] + c->hash, 9);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[10] += c->lane[11] ^ 0xf5e23456u;
    c->lane[15] ^= sx_rl(c->lane[14], 30);
    c->sum += t1;
    return t0 + t2;
}

static int fold_tuple(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[6];
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x43) << 0;
    c->sched[5] = c->hash ^ sx_rl(c->lane[5], 31);
    c->lane[8] ^= sx_rl(c->lane[1], 29);
    c->raw[c->slo + (int)((t0 + 27453u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[6] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t mark_window(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->hash ^= c->lane[5] + 0x51749086u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 15913u) % (uint32_t)c->rln)] << 16;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 23);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static void hold_track(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[13] += c->lane[6] ^ 0x18c6d839u;
    c->lane[4] += c->lane[13] ^ 0xc335327au;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 11);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->hash = (c->hash * 0x549e2dddu) ^ sx_rr(c->hash, 11);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t merge_pairing(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= c->lane[8] + 0x4a7b1addu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 59812u) % (uint32_t)c->rln)] << 0;
    c->sched[5] = c->hash ^ sx_rl(c->lane[15], 10);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xe0) << 8;
    c->lane[8] ^= sx_rl(c->lane[13], 12);
    c->hash = (c->hash * 0x7912ae73u) ^ sx_rr(c->hash, 4);
    c->raw[c->slo + (int)((t0 + 29779u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[6] ^= sx_rl(c->lane[12], 9);
    c->lane[0] += c->lane[2] ^ 0xb4c8cf7au;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t chain_limit(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sched[30] = c->hash ^ sx_rl(c->lane[12], 18);
    c->lane[9] ^= sx_rl(c->lane[4], 8);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x43) << 8;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x86) << 16;
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 23);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void patch_line_740(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[12] + 0x5189d93bu;
    t2 = (t2 ^ c->sum) * 0x14c9aa63u;
    c->lane[5] += c->lane[11]; c->lane[14] ^= c->lane[5]; c->lane[14] = sx_rl(c->lane[14], 2);
    c->lane[2] += c->lane[14]; c->lane[6] ^= c->lane[2]; c->lane[6] = sx_rl(c->lane[6], 19);
    c->sched[1] = c->hash ^ sx_rl(c->lane[2], 9);
    c->lane[10] += c->lane[11] ^ 0xed982b9bu;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->lane[4] += c->lane[5] ^ 0xc06b2c83u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
    c->hash ^= c->lane[10] + 0x1140340bu;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t close_table(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 6);
    c->lane[4] += c->lane[11]; c->lane[8] ^= c->lane[4]; c->lane[8] = sx_rl(c->lane[8], 16);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33886u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57617u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 33428u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t pin_bound(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t2 = (t2 ^ c->sum) * 0xeecee2d9u;
    c->lane[0] += c->lane[9]; c->lane[14] ^= c->lane[0]; c->lane[14] = sx_rl(c->lane[14], 8);
    t2 = (t2 ^ c->sum) * 0x69550d8bu;
    c->raw[c->slo + (int)((t0 + 32359u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[8] += c->lane[2] ^ 0x78c4c563u;
    c->hash ^= c->lane[4] + 0x0d6e975au;
    c->sched[15] = c->hash ^ sx_rl(c->lane[14], 6);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xfb) << 0;
    c->sum += t1;
    return t0 + t2;
}

static void pin_stack(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 7);
    c->lane[14] += c->lane[4] ^ 0xa9326319u;
    c->hash ^= c->lane[14] + 0x9c03109bu;
    c->sched[31] = c->hash ^ sx_rl(c->lane[13], 3);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 6);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0xed) << 16;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x19be54a3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sum ^= t0 + t1;
    c->lane[7] ^= t2;
}

static uint32_t coal_group(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[4] + 0x7164c87cu;
    c->hash = (c->hash * 0x0b9b8765u) ^ sx_rr(c->hash, 26);
    c->sched[28] = c->hash ^ sx_rl(c->lane[7], 29);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 63938u) % (uint32_t)c->rln)] << 0;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[14]); t1 += c->step;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[8]); t1 += c->step;
    c->lane[12] ^= sx_rl(c->lane[7], 10);
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t grow_item(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x940ea945u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 41360u) % (uint32_t)c->rln)] << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57454u) % (uint32_t)c->rln)] << 16;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[12]); t1 += c->step;
    c->lane[1] += c->lane[2]; c->lane[8] ^= c->lane[1]; c->lane[8] = sx_rl(c->lane[8], 6);
    c->hash = (c->hash * 0x10778417u) ^ sx_rr(c->hash, 22);
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 14);
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 5);
    t2 = (t2 ^ c->sum) * 0xdb921d47u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint8_t shift_key(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->lane[12] ^= sx_rl(c->lane[9], 25);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xb3b4801du;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[3] += c->lane[14]; c->lane[13] ^= c->lane[3]; c->lane[13] = sx_rl(c->lane[13], 25);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[9]); t1 += c->step;
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static uint32_t clamp_store(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->lane[5] ^= sx_rl(c->lane[1], 26);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 57625u) % (uint32_t)c->rln)] << 0;
    c->hash ^= c->lane[6] + 0x8b37b5adu;
    c->lane[1] += c->lane[12]; c->lane[8] ^= c->lane[1]; c->lane[8] = sx_rl(c->lane[8], 16);
    c->lane[1] += c->lane[13] ^ 0x2e949504u;
    t2 = (t2 ^ c->sum) * 0x39deb9f3u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[3]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static uint32_t swap_cell(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[6] ^= sx_rl(c->lane[0], 10);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26351u) % (uint32_t)c->rln)] << 0;
    c->lane[7] ^= sx_rl(c->lane[1], 27);
    t2 = (t2 ^ c->sum) * 0x9a59b589u;
    c->sum += t1;
    return t0 + t2;
}

static int reap_bound(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[11];
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xc0fea519u;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[5]); t1 += c->step;
    c->hash = (c->hash * 0x214fd935u) ^ sx_rr(c->hash, 9);
    c->hash ^= c->lane[14] + 0xf7e1cee0u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x0a) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[11] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint8_t clamp_scope(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->raw[c->slo + (int)((t0 + 64921u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0x696f0a8bu;
    t2 = (t2 ^ c->sum) * 0x184ca377u;
    c->hash = (c->hash * 0xad70a607u) ^ sx_rr(c->hash, 7);
    c->hash = (c->hash * 0x5c30e307u) ^ sx_rr(c->hash, 30);
    c->lane[13] ^= sx_rl(c->lane[6], 9);
    c->lane[0] ^= sx_rl(c->lane[7], 17);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->lane[12] ^= sx_rl(c->lane[12], 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 8);
}

static uint32_t split_tail(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[7] += c->lane[15] ^ 0xb64ef581u;
    c->lane[0] += c->lane[9]; c->lane[6] ^= c->lane[0]; c->lane[6] = sx_rl(c->lane[6], 18);
    c->lane[12] += c->lane[2] ^ 0xad712e05u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x07) << 8;
    c->lane[5] += c->lane[0]; c->lane[6] ^= c->lane[5]; c->lane[6] = sx_rl(c->lane[6], 14);
    c->sched[0] = c->hash ^ sx_rl(c->lane[14], 4);
    c->sum += t1;
    return t0 + t2;
}

static int defer_gap(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    c->raw[c->slo + (int)((t0 + 2208u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[13] += c->lane[8]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 18);
    c->hash = (c->hash * 0x8bc259f9u) ^ sx_rr(c->hash, 10);
    c->lane[11] += c->lane[13]; c->lane[14] ^= c->lane[11]; c->lane[14] = sx_rl(c->lane[14], 25);
    c->hash ^= c->lane[9] + 0xeb6b2119u;
    t2 = (t2 ^ c->sum) * 0x7a35e031u;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static void fold_scope(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[28] = c->hash ^ sx_rl(c->lane[15], 4);
    c->hash ^= c->lane[2] + 0x373d9a2eu;
    c->raw[c->slo + (int)((t0 + 2952u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x902c2ae9u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 31644u) % (uint32_t)c->rln)] << 0;
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static void mix_level(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 52983u) % (uint32_t)c->rln)] << 8;
    t2 = (t2 ^ c->sum) * 0xccdc08e9u;
    c->hash ^= c->lane[12] + 0x73a97e87u;
    c->sched[21] = c->hash ^ sx_rl(c->lane[5], 21);
    c->raw[c->slo + (int)((t0 + 43688u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->hash = (c->hash * 0x05909c3bu) ^ sx_rr(c->hash, 26);
    c->sched[22] = c->hash ^ sx_rl(c->lane[0], 30);
    c->lane[9] = sx_rr(c->lane[9] + c->hash, 15);
    c->hash = (c->hash * 0xbf5f7cfdu) ^ sx_rr(c->hash, 8);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint32_t trim_digest(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->lane[0] += c->lane[4]; c->lane[10] ^= c->lane[0]; c->lane[10] = sx_rl(c->lane[10], 31);
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0x03d203e5u) ^ sx_rr(c->hash, 30);
    c->raw[c->slo + (int)((t0 + 29885u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    c->raw[c->slo + (int)((t0 + 49707u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
    c->lane[10] += c->lane[10] ^ 0x27b88951u;
    c->lane[15] += c->lane[2]; c->lane[1] ^= c->lane[15]; c->lane[1] = sx_rl(c->lane[1], 23);
    c->sched[27] = c->hash ^ sx_rl(c->lane[14], 22);
    c->lane[1] += c->lane[0]; c->lane[10] ^= c->lane[1]; c->lane[10] = sx_rl(c->lane[10], 26);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void join_value(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash = (c->hash * 0xbfb6f5cfu) ^ sx_rr(c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xb433c50fu;
    c->lane[5] += c->lane[6]; c->lane[4] ^= c->lane[5]; c->lane[4] = sx_rl(c->lane[4], 3);
    c->hash ^= c->lane[15] + 0xf54fc643u;
    c->hash = (c->hash * 0x771491cdu) ^ sx_rr(c->hash, 1);
    t2 = (t2 ^ c->sum) * 0x579aedf5u;
    c->lane[7] += c->lane[0] ^ 0x5c0d8160u;
    c->lane[11] += c->lane[4]; c->lane[9] ^= c->lane[11]; c->lane[9] = sx_rl(c->lane[9], 7);
    c->sum ^= t0 + t1;
    c->lane[8] ^= t2;
}

static uint32_t grow_mask(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[14] += c->lane[2]; c->lane[4] ^= c->lane[14]; c->lane[4] = sx_rl(c->lane[4], 25);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5c) << 8;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 45252u) % (uint32_t)c->rln)] << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xfe50aaebu;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 46132u) % (uint32_t)c->rln)] << 24;
    c->lane[9] ^= sx_rl(c->lane[12], 3);
    c->sum += t1;
    return t0 + t2;
}

static int yield_delta(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[7];
    t2 = (t2 ^ c->sum) * 0x4eefd9bfu;
    c->lane[4] ^= sx_rl(c->lane[4], 23);
    c->lane[14] ^= sx_rl(c->lane[13], 3);
    c->lane[10] += c->lane[1]; c->lane[2] ^= c->lane[10]; c->lane[2] = sx_rl(c->lane[2], 5);
    c->hash = (c->hash * 0x2e67b92du) ^ sx_rr(c->hash, 30);
    c->lane[4] += c->lane[0] ^ 0xf78b3097u;
    t2 = (t2 ^ c->sum) * 0x71a3c053u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 1165u) % (uint32_t)c->rln)] << 0;
    c->hash = (c->hash * 0xd977bb5fu) ^ sx_rr(c->hash, 17);
    t2 = (t2 ^ c->sum) * 0x0ffac01du;
    c->lane[7] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t grow_slot(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->lane[13] += c->lane[6]; c->lane[2] ^= c->lane[13]; c->lane[2] = sx_rl(c->lane[2], 20);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x9b) << 8;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->sum += t1;
    return t0 + t2;
}

static void patch_port(sx_state *c, uint32_t *v, int k) {
    uint32_t t0 = v[0], t1 = (uint32_t)k, t2 = c->hash;
    c->hash ^= c->lane[5] + 0xabcc5f46u;
    c->lane[14] += c->lane[7] ^ 0xf322f86bu;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xec140c81u;
    c->lane[15] ^= sx_rl(c->lane[1], 27);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[1]); t1 += c->step;
    c->lane[0] = sx_rr(c->lane[0] + c->hash, 7);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14239u) % (uint32_t)c->rln)] << 24;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35974u) % (uint32_t)c->rln)] << 16;
    c->lane[15] = sx_rr(c->lane[15] + c->hash, 29);
    v[(int)(t2 % (uint32_t)k)] ^= t0 ^ c->sum;
}

static uint8_t stage_tuple(sx_state *c, uint8_t v, uint32_t a) {
    uint32_t t0 = v, t1 = a, t2 = c->sum;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x68) << 8;
    t2 = (t2 ^ c->sum) * 0xf3d4b097u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 64260u) % (uint32_t)c->rln)] << 8;
    c->raw[c->slo + (int)((t0 + 20208u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->hash += t1;
    return (uint8_t)((t0 ^ t2) >> 0);
}

static void split_key(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xa2) << 8;
    c->lane[8] = sx_rr(c->lane[8] + c->hash, 22);
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 28);
    t2 = (t2 ^ c->sum) * 0x67060e3bu;
    t2 = (t2 ^ c->sum) * 0x7ba41a01u;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 53263u) % (uint32_t)c->rln)] << 8;
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[0]); t1 += c->step;
    c->lane[1] ^= sx_rl(c->lane[11], 17);
    c->lane[15] += c->lane[6] ^ 0x2a42f88bu;
    c->raw[c->slo + (int)((t0 + 54172u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->hash = (c->hash * 0x46cf3075u) ^ sx_rr(c->hash, 7);
    c->sum ^= t0 + t1;
    c->lane[15] ^= t2;
}

static uint32_t tap_batch(sx_state *c, int lo, int ln) {
    uint32_t t0 = (uint32_t)lo, t1 = (uint32_t)ln, t2 = c->hash;
    c->sched[25] = c->hash ^ sx_rl(c->lane[9], 1);
    c->hash = (c->hash * 0x52d8b025u) ^ sx_rr(c->hash, 10);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x543d9a79u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35065u) % (uint32_t)c->rln)] << 8;
    c->lane[9] += c->lane[2]; c->lane[6] ^= c->lane[9]; c->lane[6] = sx_rl(c->lane[6], 25);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
    c->sched[12] = c->hash ^ sx_rl(c->lane[0], 29);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
    c->sum += t1;
    return t0 + t2;
}

static int blend_node(sx_state *c) {
    uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[5];
    c->raw[c->slo + (int)((t0 + 13690u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
    c->lane[14] ^= sx_rl(c->lane[14], 21);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xf989ac6bu;
    c->sched[24] = c->hash ^ sx_rl(c->lane[7], 26);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0x5d) << 0;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x49) << 0;
    c->hash ^= c->lane[7] + 0x81f98e08u;
    c->lane[5] = t2;
    return (int)((t0 ^ t1) & 0x3Fu);
}

static uint32_t pick_lease(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    c->hash ^= c->lane[11] + 0x8eec9885u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
    c->hash ^= c->lane[5] + 0x54968801u;
    c->lane[4] = sx_rr(c->lane[4] + c->hash, 17);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xf0) << 0;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0xe682cc4fu;
    c->hash = (c->hash * 0x7ff5fcd5u) ^ sx_rr(c->hash, 19);
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 48922u) % (uint32_t)c->rln)] << 8;
    c->hash ^= t2;
    return t0 ^ t1;
}

static uint32_t shift_view(sx_state *c, uint32_t a) {
    uint32_t t0 = a, t1 = c->hash, t2 = c->sum;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 8506u) % (uint32_t)c->rln)] << 16;
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 20);
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x64b30f75u;
    c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
    c->lane[3] = sx_rr(c->lane[3] + c->hash, 22);
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x70) << 8;
    c->lane[6] += c->lane[4] ^ 0x069b5fc4u;
    c->raw[c->slo + (int)((t0 + 18183u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
    c->lane[11] ^= sx_rl(c->lane[0], 31);
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
    c->lane[10] = sx_rr(c->lane[10] + c->hash, 19);
    c->hash ^= t2;
    return t0 ^ t1;
}

static void chain_row_767(sx_state *c, uint32_t a, uint32_t b) {
    uint32_t t0 = a, t1 = b, t2 = c->hash;
    t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 32848u) % (uint32_t)c->rln)] << 24;
    c->sched[21] = c->hash ^ sx_rl(c->lane[8], 16);
    c->lane[4] += c->lane[11] ^ 0x812fb588u;
    c->raw[c->slo + (int)((t0 + 60337u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 0);
    t2 = (t2 ^ c->sum) * 0x630f9a8du;
    c->lane[2] += c->lane[8] ^ 0xe9228327u;
    c->hash ^= (uint32_t)c->fwd[(c->hash >> 0) & 0xFFu] * 0xe03a8de3u;
    c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0xa3) << 16;
    c->lane[13] = sx_rr(c->lane[13] + c->hash, 20);
    c->sum ^= t0 + t1;
    c->lane[13] ^= t2;
}


static const uint8_t sx_polys[16] = { 0x1d, 0x2b, 0x2d, 0x39, 0x3f, 0x4d, 0x5f, 0x63, 0x65, 0x69, 0x71, 0x77, 0x7b, 0x87, 0x8b, 0x8d };

static uint32_t stage_limit_768(uint32_t x) {
    return (x * 0x7fb5d329u) ^ sx_rl(x + 0x4e1d9a2bu, 11);
}

static int fill_gap(sx_state *c) {
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
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x490e0e39u;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 26423u) % (uint32_t)c->rln)] << 16;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 35202u) % (uint32_t)c->rln)] << 24;
        c->lane[6] += c->lane[4] ^ 0xb60470bbu;
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
        t0 ^= slice_record(c, t1);
        t0 ^= pack_path(c, t1);
        t2 += (uint32_t)sync_bound(c);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    return c->nlen;
}

static uint32_t rotate_list(uint32_t prime, uint32_t h, uint8_t x) {
    return (h ^ (uint32_t)x) * prime;
}

static uint32_t stage_ring(sx_state *c, uint32_t nonce, int size) {
    uint32_t m = stage_limit_768((uint32_t)c->nlen);
    uint32_t basis = 0x16de7a0cu ^ m;
    uint32_t prime = 0x96c2e65au ^ m;
    uint32_t h = basis;
    int i;
    for (i = 0; i < c->nlen; i++) h = rotate_list(prime, h, c->name[i]);
    for (i = 0; i < 4; i++) h = rotate_list(prime, h, (uint8_t)(nonce >> (i * 8)));
    h = rotate_list(prime, h, (uint8_t)size);
#ifdef SX_TRACE
    fprintf(stderr, "T fnv_pre h=%08x m=%08x nlen=%d hash=%08x sum=%08x\n", h, m, c->nlen, c->hash, c->sum);
#endif
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t0 ^= slice_tuple(c, t1);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 55158u) % (uint32_t)c->rln)] << 0;
        c->sched[8] = c->hash ^ sx_rl(c->lane[8], 26);
        c->raw[c->slo + (int)((t0 + 13784u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
        c->sched[3] = c->hash ^ sx_rl(c->lane[12], 11);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[7]); t1 += c->step;
        c->hash ^= t0 ^ t1 ^ t2;
    }
#ifdef SX_TRACE
    fprintf(stderr, "T fnv_post h=%08x hash=%08x sum=%08x lane0=%08x\n", h, c->hash, c->sum, c->lane[0]);
#endif
    return h ^ c->hash;
}

static void push_stream(sx_state *c, uint32_t seed) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t0 ^= fill_slot(c, t1);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0xbc0dbe9du;
        c->sched[21] = c->hash ^ sx_rl(c->lane[12], 14);
        cache_slot(c, &c->lane[11], 4);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x99) << 0;
        c->lane[6] += c->lane[1]; c->lane[7] ^= c->lane[6]; c->lane[7] = sx_rl(c->lane[7], 14);
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[13]); t1 += c->step;
        c->sched[1] = c->hash ^ sx_rl(c->lane[6], 8);
        c->raw[c->slo + (int)((t0 + 9671u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
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

static void emit_row(sx_state *c, uint32_t master) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[15] ^= sx_rl(c->lane[11], 15);
        c->lane[0] += c->lane[9] ^ 0x7805937au;
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 14632u) % (uint32_t)c->rln)] << 24;
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[2]); t1 += c->step;
        split_scope_317(c, t0, t1);
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
    push_stream(c, x);
}

static void probe_tail(sx_state *c) {
    int i;
    c->shash = c->hash;
    c->ssum = c->sum;
    for (i = 0; i < 16; i++) c->slane[i] = c->lane[i];
    memcpy(c->sscr, c->raw + c->slo, (size_t)c->sln);
}

static void pin_store(sx_state *c) {
    int i;
    c->hash = c->shash;
    c->sum = c->ssum;
    for (i = 0; i < 16; i++) c->lane[i] = c->slane[i];
    memcpy(c->raw + c->slo, c->sscr, (size_t)c->sln);
}

static void pick_ring_776(sx_state *c, int lo, int ln) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
        probe_stream(c, &c->lane[4], 2);
        c->hash ^= c->lane[1] + 0x16e2298fu;
        c->lane[14] += c->lane[4] ^ 0xfab0c7d2u;
        c->lane[9] += c->lane[12]; c->lane[2] ^= c->lane[9]; c->lane[2] = sx_rl(c->lane[2], 13);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 0));
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0xed) << 16;
        reset_store(c, &c->lane[2], 1);
        c->lane[7] = sx_rr(c->lane[7] + c->hash, 31);
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

static void drain_port(sx_state *c, uint32_t *out) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->lane[7] += c->lane[3]; c->lane[4] ^= c->lane[7]; c->lane[4] = sx_rl(c->lane[4], 27);
        t0 ^= pack_path(c, t1);
        c->hash ^= c->lane[7] + 0xecf57abeu;
        t2 += stage_limit(c, c->slo, c->sln);
        c->raw[c->slo + (int)((t0 + 61149u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 8);
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

static void tap_port(sx_state *c, uint32_t master, int rnd, int lo, int ln, uint32_t *out) {
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
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[6]); t1 += c->step;
        c->lane[2] += c->lane[6]; c->lane[8] ^= c->lane[2]; c->lane[8] = sx_rl(c->lane[8], 18);
        t1 ^= (uint32_t)place_token(c, (uint8_t)(t0 >> 16), t2);
        coal_port_373(c, t0, t1);
        c->hash = (c->hash * 0xbe5d8fadu) ^ sx_rr(c->hash, 5);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    pick_ring_776(c, lo, ln);
    switch (rnd) {
    case 0:
        prime_record(c, c->hash, c->sum);
        peek_block(c, c->hash, c->sum);
        break;
    case 1:
        reset_region(c, c->hash, c->sum);
        pack_index(c, c->hash, c->sum);
        break;
    default:
        peek_digest(c, c->hash, c->sum);
        purge_mask(c, c->hash, c->sum);
        break;
    }
    drain_port(c, out);
#ifdef SX_TRACE
    fprintf(stderr, "T rk%d %08x %08x %08x %08x %08x %08x %08x %08x\n", rnd,
            out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
#endif
}

static void prime_marker(sx_state *c, const uint32_t *kk, uint32_t *out) {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash = (c->hash * 0x46b63197u) ^ sx_rr(c->hash, 15);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 8), 0xd9) << 8;
        c->sched[0] = c->hash ^ sx_rl(c->lane[1], 12);
        c->sched[25] = c->hash ^ sx_rl(c->lane[7], 31);
        pin_stack(c, t0, t1);
        c->lane[0] = sx_rr(c->lane[0] + c->hash, 28);
        c->hash = (c->hash * 0x3361ba83u) ^ sx_rr(c->hash, 22);
        t2 += fold_label(c, c->slo, c->sln);
        c->raw[c->slo + (int)((t0 + 10271u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 16);
        c->lane[15] += c->lane[8] ^ 0xbcd60330u;
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

static void load_node(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    pin_store(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 0), 0x5a) << 0;
        hold_scope(c, t0, t1);
        c->lane[2] += c->lane[10] ^ 0x253672b7u;
        c->hash = (c->hash * 0x9e411a6fu) ^ sx_rr(c->hash, 21);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint8_t tab[SX_PAY];
    uint8_t src[SX_PAY];
    uint32_t x;
    int i;
    prime_marker(c, kk, w);
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

static void emit_bucket(sx_state *c, const uint32_t *w, uint32_t *v) {
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

static void split_value(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    pin_store(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += (uint32_t)mix_slot(c);
        reset_store(c, &c->lane[1], 1);
        patch_page(c, t0, t1);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 8) & 0xFFu] * 0x10cfd7c1u;
        c->hash ^= c->lane[12] + 0xf96e7c2bu;
        c->lane[13] += c->lane[0]; c->lane[11] ^= c->lane[13]; c->lane[11] = sx_rl(c->lane[11], 29);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t v[2];
    uint8_t ks[8];
    int pos = 0, blk = 0, i;
    (void)fwd;
    prime_marker(c, kk, w);
    while (pos < ln) {
        int rot = blk & 31;
        if (rot == 0) rot = 1;
        v[0] = (uint32_t)blk ^ w[0];
        v[1] = sx_rl(w[1], rot) ^ w[3];
        emit_bucket(c, w, v);
        for (i = 0; i < 4; i++) ks[i] = (uint8_t)(v[0] >> (i * 8));
        for (i = 0; i < 4; i++) ks[4 + i] = (uint8_t)(v[1] >> (i * 8));
        for (i = 0; i < 8 && pos + i < ln; i++) c->raw[lo + pos + i] ^= ks[i];
        pos += 8;
        blk++;
    }
}

static void settle_track(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    pin_store(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t2 += (uint32_t)fold_seat(c);
        c->hash ^= c->lane[1] + 0x8dc66a30u;
        c->hash ^= c->lane[13] + 0xc338e16du;
        wrap_tail(c, &c->lane[4], 4);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 60341u) % (uint32_t)c->rln)] << 16;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint8_t m;
    int i;
    prime_marker(c, kk, w);
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

static void swap_queue_784(uint32_t *s, int a, int b, int d, int e) {
    s[a] += s[b]; s[e] = sx_rl(s[e] ^ s[a], 13);
    s[d] += s[e]; s[b] = sx_rl(s[b] ^ s[d], 9);
    s[a] += s[b]; s[e] = sx_rl(s[e] ^ s[a], 11);
    s[d] += s[e]; s[b] = sx_rl(s[b] ^ s[d], 6);
}

static void pair_lease(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    pin_store(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t1 ^= (uint32_t)trim_item(c, (uint8_t)(t0 >> 8), t2);
        c->sched[16] = c->hash ^ sx_rl(c->lane[1], 9);
        c->lane[9] += c->lane[14]; c->lane[13] ^= c->lane[9]; c->lane[13] = sx_rl(c->lane[13], 15);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 50973u) % (uint32_t)c->rln)] << 8;
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0xd4bf5bb7u;
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[15]); t1 += c->step;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t base[16], s[16];
    uint8_t ks[64];
    int pos = 0, counter = 0, i, r;
    (void)fwd;
    prime_marker(c, kk, w);
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
            swap_queue_784(s, 0, 4, 8, 12);
            swap_queue_784(s, 1, 5, 9, 13);
            swap_queue_784(s, 2, 6, 10, 14);
            swap_queue_784(s, 3, 7, 11, 15);
            swap_queue_784(s, 0, 5, 10, 15);
            swap_queue_784(s, 1, 6, 11, 12);
            swap_queue_784(s, 2, 7, 8, 13);
            swap_queue_784(s, 3, 4, 9, 14);
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

static void prime_token(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    pin_store(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->sched[20] = c->hash ^ sx_rl(c->lane[8], 4);
        c->lane[1] ^= sx_rl(c->lane[8], 26);
        c->hash ^= c->lane[8] + 0xe9b13e74u;
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[10]); t1 += c->step;
        t0 ^= resize_pool(c, t1);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 29958u) % (uint32_t)c->rln)] << 0;
        t1 ^= (uint32_t)close_slot(c, (uint8_t)(t0 >> 0), t2);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    int i;
    prime_marker(c, kk, w);
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

static void shift_queue(sx_state *c, int lo, int ln, const uint32_t *kk, int fwd) {
    pin_store(c);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (c->hash + c->lane[11]); t1 += c->step;
        c->sched[14] = c->hash ^ sx_rl(c->lane[11], 31);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 8));
        t0 ^= grow_label(c, t1);
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 24), 0x13) << 0;
        c->lane[3] += c->lane[0]; c->lane[14] ^= c->lane[3]; c->lane[14] = sx_rl(c->lane[14], 31);
        t2 = (t2 ^ c->sum) * 0xe4430a05u;
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x78) << 0;
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 16) & 0xFFu] * 0x5a68ae73u;
        c->hash ^= t0 ^ t1 ^ t2;
    }
    uint32_t w[4];
    uint32_t x;
    int i;
    (void)fwd;
    prime_marker(c, kk, w);
    x = w[0] | 1u;
    for (i = 0; i < ln; i++) {
        x = sx_step(x);
        c->raw[lo + i] ^= (uint8_t)x;
    }
}

static void mix_limit(sx_state *c, int idx, int lo, int ln, const uint32_t *kk, int fwd) {
    switch (idx) {
    case 0: load_node(c, lo, ln, kk, fwd); break;
    case 1: split_value(c, lo, ln, kk, fwd); break;
    case 2: settle_track(c, lo, ln, kk, fwd); break;
    case 3: pair_lease(c, lo, ln, kk, fwd); break;
    case 4: prime_token(c, lo, ln, kk, fwd); break;
    default: shift_queue(c, lo, ln, kk, fwd); break;
    }
}

static void probe_item(sx_state *c, uint32_t master, int rnd, int alo, int aln, int tlo, int tln, int s0, int s1, int fwd) {
    uint32_t kk[8];
    tap_port(c, master, rnd, alo, aln, kk);
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        swap_label(c, &c->lane[0], 2);
        tap_entry(c, &c->lane[0], 2);
        c->sched[23] = c->hash ^ sx_rl(c->lane[8], 31);
        c->lane[5] += c->lane[4]; c->lane[0] ^= c->lane[5]; c->lane[0] = sx_rl(c->lane[0], 24);
        split_window(c, &c->lane[3], 3);
        c->lane[10] += c->lane[13] ^ 0x23a8b4dbu;
        c->hash = (c->hash * 0x6671cabfu) ^ sx_rr(c->hash, 5);
        c->lane[2] ^= sx_rl(c->lane[1], 26);
        c->hash ^= (uint32_t)c->fwd[(c->hash >> 24) & 0xFFu] * 0x33b4e5c5u;
        c->lane[14] ^= sx_rl(c->lane[4], 28);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    probe_tail(c);
    if (fwd) {
        mix_limit(c, s0, tlo, tln, &kk[0], 1);
        mix_limit(c, s1, tlo, tln, &kk[4], 1);
    } else {
        mix_limit(c, s1, tlo, tln, &kk[4], 0);
        mix_limit(c, s0, tlo, tln, &kk[0], 0);
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

    fill_gap(c);
    base = stage_ring(c, nonce, len);
    {
    {
        uint32_t t0 = c->hash, t1 = c->sum, t2 = c->lane[1];
        c->raw[c->slo + (int)((t0 + 18592u) % (uint32_t)c->sln)] ^= (uint8_t)(c->hash >> 24);
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 28659u) % (uint32_t)c->rln)] << 16;
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 16));
        t2 += sync_key(c, c->rlo, c->rln);
        c->sum = sx_crc(c, c->sum, (uint8_t)(c->hash >> 24));
        c->lane[12] += c->lane[12] ^ 0x6750495au;
        c->hash ^= (uint32_t)sx_gf(c, (uint8_t)(c->hash >> 16), 0x93) << 0;
        t0 += (uint32_t)c->raw[c->rlo + (int)((t1 + 4669u) % (uint32_t)c->rln)] << 16;
        c->lane[7] = sx_rr(c->lane[7] + c->hash, 31);
        t1 ^= (uint32_t)blend_batch(c, (uint8_t)(t0 >> 16), t2);
        t2 += fold_label(c, c->rlo, c->rln);
        c->hash ^= t0 ^ t1 ^ t2;
    }
    }
    master = base ^ c->hash ^ c->sum ^ c->lane[3];
#ifdef SX_TRACE
    fprintf(stderr, "T master=%08x base=%08x\n", master, base);
#endif
    emit_row(c, master);
#ifdef SX_TRACE
    fprintf(stderr, "T poly=%02x cpoly=%08x step=%08x vec=%08x %08x %08x %08x fwd7=%02x\n",
            c->poly, c->cpoly, c->step, c->vec[0], c->vec[1], c->vec[2], c->vec[3], c->fwd[7]);
#endif

    h = len / 2;
    lows[0] = 0; lens[0] = h;
    lows[1] = h; lens[1] = len - h;

    for (k = 0; k < 3; k++) {
        i = mode ? k : (2 - k);
        probe_item(c, master, PL[i][0],
                     lows[PL[i][1]], lens[PL[i][1]],
                     lows[PL[i][2]], lens[PL[i][2]],
                     PL[i][3], PL[i][4], mode);
    }

    memcpy(out, c->raw, (size_t)len);
    memset(&st, 0, sizeof(st));
}

