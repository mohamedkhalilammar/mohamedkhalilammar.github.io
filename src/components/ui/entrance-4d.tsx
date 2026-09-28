"use client";

import { motion, AnimatePresence, useReducedMotion } from "framer-motion";
import { useCallback, useEffect, useRef, useState } from "react";

const BG_PHOTOS = [
  "/media/cybersphere.jpeg",
  "/media/finals.jpg",
  "/media/photo.jpg",
];

const LABELS = ["Cyber Lab", "CTF Finals", "Portrait"]; // a11y only (indicator labels)

const AUTO_MS = 5200;

export function CreativeEntrance4D() {
  const reducedMotion = useReducedMotion();
  // slide index + travel direction so transitions can wipe from the right side
  const [slide, setSlide] = useState<{ idx: number; dir: 1 | -1 }>({ idx: 0, dir: 1 });
  const current = slide.idx;
  const [playing, setPlaying] = useState(false);

  const advance = useCallback((dir: 1 | -1) => {
    setSlide((s) => ({ idx: (s.idx + dir + BG_PHOTOS.length) % BG_PHOTOS.length, dir }));
  }, []);

  const goTo = useCallback(
    (idx: number) => setSlide((s) => ({ idx, dir: idx >= s.idx ? 1 : -1 })),
    []
  );

  // Autoplay — resets on any slide change so the progress bar stays in sync
  useEffect(() => {
    if (!playing || reducedMotion) return;
    const t = setInterval(() => advance(1), AUTO_MS);
    return () => clearInterval(t);
  }, [advance, current, playing, reducedMotion]);

  // Touch swipe
  const touchStartX = useRef(0);
  const onTouchStart = (e: React.TouchEvent) => {
    touchStartX.current = e.touches[0].clientX;
  };
  const onTouchEnd = (e: React.TouchEvent) => {
    const delta = touchStartX.current - e.changedTouches[0].clientX;
    if (Math.abs(delta) > 50) advance(delta > 0 ? 1 : -1);
  };

  return (
    <section
      id="entrance"
      aria-label="Introduction"
      onTouchStart={onTouchStart}
      onTouchEnd={onTouchEnd}
      className="entrance-cover relative w-full overflow-hidden flex items-center justify-start"
    >
      {/* ── Full-bleed photo background ── */}
      <div className="absolute inset-0 z-0" aria-hidden>
        <AnimatePresence initial={false} mode="sync" custom={slide.dir}>
          <BgPhoto
            key={current}
            src={BG_PHOTOS[current]}
            idx={current}
            dir={slide.dir}
            reducedMotion={!!reducedMotion || !playing}
          />
        </AnimatePresence>

        {/* aurora wash + legibility scrims */}
        <div className="entrance-aurora absolute inset-0 bg-[radial-gradient(ellipse_at_70%_12%,rgba(129,140,248,0.18),transparent_55%)]" />
        <div className="entrance-veil absolute inset-0 bg-gradient-to-b from-[var(--photo-veil-strong)] via-[var(--photo-veil-mid)] to-[var(--surface-page)]" />
        <div className="entrance-vignette absolute inset-0 bg-[radial-gradient(ellipse_at_center,transparent_30%,var(--photo-veil-mid)_100%)]" />
        {/* bottom fade into page background */}
        <div className="absolute inset-x-0 bottom-0 h-32 bg-gradient-to-b from-transparent to-[var(--surface-page)]" />
      </div>

      {/* ── Content (left-aligned so the photo stays visible) ── */}
      <div className="relative z-10 w-full max-w-[640px] mr-auto px-6 md:px-10 lg:px-16 text-left">
        <h1
          className="font-display font-extrabold uppercase leading-[0.9] tracking-[-0.02em] text-[var(--text-ink)]"
          style={{ fontSize: "clamp(2.8rem, 5.8vw, 5rem)" }}
        >
          <span className="block overflow-hidden pb-[0.04em]">
            <motion.span
              className="block"
              initial={false}
              animate={{ y: "0%" }}
              transition={{ duration: 0.9, ease: [0.16, 1, 0.3, 1], delay: 0.15 }}
              style={{ textShadow: "0 4px 50px rgba(3,7,18,0.7)" }}
            >
              Khalil
            </motion.span>
          </span>
          <span className="block overflow-hidden pb-[0.06em]">
            <motion.span
              className="block"
              initial={false}
              animate={{ y: "0%" }}
              transition={{ duration: 0.9, ease: [0.16, 1, 0.3, 1], delay: 0.28 }}
              style={{
                background: "linear-gradient(100deg, var(--brand-grad-a) 0%, var(--brand-grad-b) 50%, var(--brand-grad-c) 100%)",
                WebkitBackgroundClip: "text",
                backgroundClip: "text",
                WebkitTextFillColor: "transparent",
                filter: "drop-shadow(0 0 44px var(--accent-surface))",
              }}
            >
              Ammar
            </motion.span>
          </span>
        </h1>

        <motion.p
          initial={false}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, ease: [0.16, 1, 0.3, 1], delay: 0.55 }}
          className="mt-6 max-w-md font-mono text-[13px] md:text-sm uppercase tracking-[0.14em] leading-relaxed text-[var(--text-body)]"
          style={{ textShadow: "0 1px 14px rgba(3,7,18,0.9)" }}
        >
          Cybersecurity Enthusiast{" "}
          <br className="hidden sm:block" />
          &amp; Engineering Student at INSAT.
        </motion.p>

        {/* ── Scroll cue ──
               This is a title card, not a conversion surface: the buttons that
               used to sit here pointed at #projects and #contact, the exact two
               anchors the introduction below repeats. A visitor who has seen
               nothing yet has no reason to press them, so the stage now hands
               off to the introduction instead. ── */}
        <motion.a
          href="#projects"
          initial={false}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, ease: [0.16, 1, 0.3, 1], delay: 0.7 }}
          className="entrance-scroll-cue group mt-10 inline-flex items-center gap-3 font-mono text-[11px] uppercase tracking-[0.24em] text-[var(--text-muted)] transition-colors hover:text-[var(--text-ink)] focus-visible:outline-2 focus-visible:outline-offset-4 focus-visible:outline-primary-400"
          style={{ textShadow: "0 1px 14px rgba(3,7,18,0.9)" }}
        >
          <span>Explore my work</span>
          <motion.svg
            width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor"
            strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round" aria-hidden
            animate={reducedMotion ? undefined : { y: [0, 4, 0] }}
            transition={{ duration: 1.8, repeat: Infinity, ease: "easeInOut" }}
          >
            <path d="M8 3v10M4 9l4 4 4-4" />
          </motion.svg>
        </motion.a>
      </div>

      {/* ── Story progress bar (no labels) ── */}
      <div className="absolute left-6 md:left-10 lg:left-16 bottom-7 z-20 flex w-[min(260px,55vw)] items-center gap-2">
        {BG_PHOTOS.map((_, i) => (
          <button
            key={i}
            onClick={() => goTo(i)}
            type="button"
            aria-pressed={current === i}
            aria-label={`Show ${LABELS[i]}`}
            className="entrance-photo-tab relative h-11 flex-1"
          >
            <span className="absolute inset-x-0 top-1/2 h-[2px] bg-[var(--line-border)]" />
            {i === current && (
              <motion.span
                key={current}
                className="absolute top-1/2 h-[2px] left-0"
                style={{ background: "linear-gradient(90deg,var(--brand-grad-a),var(--brand-grad-b))" }}
                initial={{ width: playing && !reducedMotion ? "0%" : "100%" }}
                animate={{ width: "100%" }}
                transition={{ duration: playing && !reducedMotion ? AUTO_MS / 1000 : 0, ease: "linear" }}
              />
            )}
          </button>
        ))}
      </div>

      <button type="button" className="entrance-play" aria-pressed={playing} onClick={() => setPlaying(p => !p)}>{playing ? "Pause slideshow" : "Play slideshow"}</button>

      {/* ── Arrows ── */}
      <button onClick={() => advance(-1)} aria-label="Previous photo" className="entrance-arrow left-4 md:left-7">
        <svg width="14" height="14" viewBox="0 0 14 14" fill="none" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round"><polyline points="9,2 4,7 9,12" /></svg>
      </button>
      <button onClick={() => advance(1)} aria-label="Next photo" className="entrance-arrow right-4 md:right-7">
        <svg width="14" height="14" viewBox="0 0 14 14" fill="none" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round"><polyline points="5,2 10,7 5,12" /></svg>
      </button>
    </section>
  );
}

type BgPhotoProps = {
  src: string;
  idx: number;
  dir: 1 | -1;
  reducedMotion: boolean;
};

/**
 * BgPhoto — cinematic slide: the incoming photo wipes in from the travel
 * direction (clip-path) while the outgoing one pans away underneath.
 * Ken Burns alternates per slide (even → slow zoom-in, odd → slow zoom-out)
 * so consecutive photos never move the same way.
 */
function BgPhoto({ src, idx, dir, reducedMotion }: BgPhotoProps) {
  const zoomIn = idx % 2 === 0;

  const variants = {
    enter: (d: 1 | -1) =>
      reducedMotion
        ? { opacity: 0 }
        : {
            clipPath: d === 1 ? "inset(0 0 0 100%)" : "inset(0 100% 0 0)",
            scale: 1.06,
            filter: "blur(6px)",
            opacity: 1,
          },
    center: {
      clipPath: "inset(0 0 0 0)",
      scale: 1,
      x: 0,
      filter: "blur(0px)",
      opacity: 1,
    },
    exit: (d: 1 | -1) =>
      reducedMotion
        ? { opacity: 0 }
        : {
            opacity: 0,
            scale: 1.07,
            x: d === 1 ? -60 : 60,
            filter: "blur(5px)",
          },
  };

  return (
    <motion.div
      className="absolute inset-0"
      custom={dir}
      variants={variants}
      initial="enter"
      animate="center"
      exit="exit"
      transition={{ duration: 1.15, ease: [0.22, 1, 0.36, 1] }}
    >
      <motion.img
        src={src}
        alt=""
        draggable={false}
        initial={reducedMotion ? { scale: 1 } : { scale: zoomIn ? 1 : 1.14, x: 0 }}
        animate={
          reducedMotion
            ? { scale: 1 }
            : { scale: zoomIn ? 1.12 : 1.03, x: zoomIn ? dir * -14 : dir * 10 }
        }
        transition={{ duration: AUTO_MS / 1000 + 1.5, ease: "easeOut" }}
        className="photo-plate--bare h-full w-full object-cover select-none"
        style={{ objectPosition: "center 25%", filter: "contrast(1.04) saturate(0.95) brightness(0.92)" }}
      />
    </motion.div>
  );
}
