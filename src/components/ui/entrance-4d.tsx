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

  const advance = useCallback((dir: 1 | -1) => {
    setSlide((s) => ({ idx: (s.idx + dir + BG_PHOTOS.length) % BG_PHOTOS.length, dir }));
  }, []);

  const goTo = useCallback(
    (idx: number) => setSlide((s) => ({ idx, dir: idx >= s.idx ? 1 : -1 })),
    []
  );

  // Autoplay — resets on any slide change so the progress bar stays in sync
  useEffect(() => {
    const t = setInterval(() => advance(1), AUTO_MS);
    return () => clearInterval(t);
  }, [advance, current]);

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
      className="entrance-cover relative w-full overflow-hidden min-h-[70vh] md:min-h-[88vh] flex items-center justify-start"
    >
      {/* ── Full-bleed photo background ── */}
      <div className="absolute inset-0 z-0" aria-hidden>
        <AnimatePresence initial={false} mode="sync" custom={slide.dir}>
          <BgPhoto
            key={current}
            src={BG_PHOTOS[current]}
            idx={current}
            dir={slide.dir}
            reducedMotion={!!reducedMotion}
          />
        </AnimatePresence>

        {/* aurora wash + legibility scrims */}
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_70%_12%,rgba(129,140,248,0.18),transparent_55%)]" />
        <div className="absolute inset-0 bg-gradient-to-b from-[#080a14]/75 via-[#080a14]/45 to-[#080a14]" />
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_center,transparent_30%,rgba(8,10,20,0.6)_100%)]" />
        {/* bottom fade into page background */}
        <div className="absolute inset-x-0 bottom-0 h-32 bg-gradient-to-b from-transparent to-[#080a14]" />
      </div>

      {/* ── Content (left-aligned so the photo stays visible) ── */}
      <div className="relative z-10 w-full max-w-[640px] mr-auto px-6 md:px-10 lg:px-16 text-left">
        <h1
          className="font-display font-extrabold uppercase leading-[0.9] tracking-[-0.02em] text-white"
          style={{ fontSize: "clamp(3.2rem, 10vw, 8rem)" }}
        >
          <span className="block overflow-hidden pb-[0.04em]">
            <motion.span
              className="block"
              initial={reducedMotion ? false : { y: "110%" }}
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
              initial={reducedMotion ? false : { y: "110%" }}
              animate={{ y: "0%" }}
              transition={{ duration: 0.9, ease: [0.16, 1, 0.3, 1], delay: 0.28 }}
              style={{
                background: "linear-gradient(100deg, #818cf8 0%, #c4b5fd 50%, #ffffff 100%)",
                WebkitBackgroundClip: "text",
                backgroundClip: "text",
                WebkitTextFillColor: "transparent",
                filter: "drop-shadow(0 0 44px rgba(129,140,248,0.32))",
              }}
            >
              Ammar
            </motion.span>
          </span>
        </h1>

        <motion.p
          initial={reducedMotion ? false : { opacity: 0, y: 12 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, ease: [0.16, 1, 0.3, 1], delay: 0.55 }}
          className="mt-6 max-w-md font-mono text-[13px] md:text-sm uppercase tracking-[0.14em] leading-relaxed text-white/75"
          style={{ textShadow: "0 1px 14px rgba(3,7,18,0.9)" }}
        >
          Cybersecurity Enthusiast{" "}
          <br className="hidden sm:block" />
          &amp; Engineering Student at INSAT.
        </motion.p>

        <motion.div
          initial={reducedMotion ? false : { opacity: 0, y: 12 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, ease: [0.16, 1, 0.3, 1], delay: 0.7 }}
          className="mt-9 flex flex-wrap items-center justify-start gap-4"
        >
          <a href="#projects" className="entrance-cta-primary" data-cursor="View">
            View Work
            <svg width="15" height="15" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
              <path d="M3 8h10M9 4l4 4-4 4" />
            </svg>
          </a>
          <a href="#contact" className="entrance-cta-ghost">
            Contact
          </a>
        </motion.div>
      </div>

      {/* ── Story progress bar (no labels) ── */}
      <div className="absolute left-6 md:left-10 lg:left-16 bottom-7 z-20 flex w-[min(260px,55vw)] items-center gap-2">
        {BG_PHOTOS.map((_, i) => (
          <button
            key={i}
            onClick={() => goTo(i)}
            aria-label={`Show ${LABELS[i]}`}
            className="relative h-[3px] flex-1 overflow-hidden rounded-full bg-white/18"
          >
            {i < current && <span className="absolute inset-0 bg-primary-400/80" />}
            {i === current && (
              <motion.span
                key={current}
                className="absolute inset-y-0 left-0 rounded-full"
                style={{ background: "linear-gradient(90deg,#818cf8,#c4b5fd)" }}
                initial={{ width: "0%" }}
                animate={{ width: "100%" }}
                transition={{ duration: AUTO_MS / 1000, ease: "linear" }}
              />
            )}
          </button>
        ))}
      </div>

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
        className="h-full w-full object-cover select-none"
        style={{ objectPosition: "center 25%", filter: "contrast(1.04) saturate(0.95) brightness(0.92)" }}
      />
    </motion.div>
  );
}
