"use client";

import {
  motion,
  AnimatePresence,
  useReducedMotion,
  useScroll,
  useTransform,
} from "framer-motion";
import { useCallback, useEffect, useRef, useState } from "react";

const BG_PHOTOS = [
  "/media/finals.jpg",
  "/media/photo.jpg",
  "/media/cybersphere.jpeg",
];

const LABELS = ["Finals", "Portrait", "Cyber Lab"];

const BLUE = "rgba(56, 189, 248, 0.95)";
const BLUE_DIM = "rgba(56, 189, 248, 0.55)";
const BLUE_FAINT = "rgba(56, 189, 248, 0.10)";

export function CreativeEntrance4D() {
  const reducedMotion = useReducedMotion();
  const [current, setCurrent] = useState(0);
  const [direction, setDirection] = useState<1 | -1>(1);
  const sectionRef = useRef<HTMLElement | null>(null);

  // Only darken the photo overlay on scroll — nothing else moves
  const { scrollYProgress } = useScroll({ target: sectionRef });
  const overlayOpacity = useTransform(scrollYProgress, [0, 0.6], [0.50, 0.90]);

  const goTo = useCallback((idx: number) => {
    if (idx === current) return;
    setDirection(idx > current ? 1 : -1);
    setCurrent(idx);
  }, [current]);

  const advance = useCallback((dir: 1 | -1) => {
    setDirection(dir);
    setCurrent(c => (c + dir + BG_PHOTOS.length) % BG_PHOTOS.length);
  }, []);

  // Automatically cycle photos every 6 seconds
  useEffect(() => {
    const timer = setInterval(() => advance(1), 6000);
    return () => clearInterval(timer);
  }, [advance]);

  return (
    <section
      id="entrance"
      ref={sectionRef}
      style={{
        position: "relative",
        width: "100%",
        height: "100vh",
        overflow: "hidden",
        background: "#030712",
      }}
    >
      {/* ── Photo layer ── */}
      <div style={{ position: "absolute", inset: 0, zIndex: 0 }}>
        <AnimatePresence initial={false} custom={direction} mode="sync">
          <SlidePhoto
            key={current}
            src={BG_PHOTOS[current]}
            direction={direction}
            reducedMotion={!!reducedMotion}
          />
        </AnimatePresence>
      </div>

      {/* ── Gradient overlay (darkens on scroll) ── */}
      <motion.div
        style={{
          position: "absolute", inset: 0, zIndex: 5, pointerEvents: "none",
          opacity: overlayOpacity,
          background: `linear-gradient(165deg,
            rgba(3,7,18,0.80) 0%,
            rgba(3,7,18,0.32) 38%,
            rgba(3,7,18,0.26) 60%,
            rgba(3,7,18,0.85) 100%
          )`,
        }}
      />

      {/* ── Edge vignette ── */}
      <div style={{
        position: "absolute", inset: 0, zIndex: 5, pointerEvents: "none",
        background: "radial-gradient(ellipse at center, transparent 32%, rgba(3,7,18,0.70) 100%)",
      }} />

      {/* ── Blue accent glow (top-right atmosphere) ── */}
      <div style={{
        position: "absolute", top: "-15%", right: "-5%", zIndex: 4,
        width: "480px", height: "480px", borderRadius: "50%",
        background: "rgba(56,189,248,0.07)",
        filter: "blur(90px)",
        pointerEvents: "none",
      }} />

      {/* ── TEXT CONTENT — position: absolute, NOT inside a motion wrapper ──
           This means it NEVER moves, no scroll transform can touch it.        ── */}
      <div
        style={{
          position: "absolute",
          zIndex: 10,
          bottom: "13.5rem",
          left: 0,
          right: 0,
          padding: "0 clamp(2rem, 6vw, 6rem)",
          pointerEvents: "none",
        }}
      >
        {/* Dark halo behind the text block so it stays readable on any photo */}
        <div style={{
          position: "absolute",
          bottom: "-2rem",
          left: "clamp(2rem, 6vw, 6rem)",
          right: "40%",
          top: "-3rem",
          background: "radial-gradient(ellipse at 0% 100%, rgba(3,7,18,0.72) 0%, transparent 80%)",
          pointerEvents: "none",
        }} />

        <motion.div
          initial={reducedMotion ? false : { opacity: 0, y: 18 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 1.0, ease: [0.16, 1, 0.3, 1], delay: 0.4 }}
          style={{ position: "relative", zIndex: 1, pointerEvents: "auto" }}
        >

          {/* Name — solid fill + strong shadow for crisp legibility on any bg */}
          <h1 style={{
            fontFamily: "var(--font-heading, sans-serif)",
            fontSize: "clamp(3.5rem, 9vw, 7.5rem)",
            fontWeight: 900,
            lineHeight: 0.88,
            letterSpacing: "-0.04em",
            textTransform: "uppercase",
            margin: "0 0 1rem",
          }}>
            <span style={{
              color: "#ffffff",
              textShadow: "0 2px 32px rgba(3,7,18,0.95), 0 0 80px rgba(3,7,18,0.7)",
              display: "block",
            }}>
              Khalil
            </span>
            <span style={{
              color: BLUE,
              textShadow: "0 2px 32px rgba(3,7,18,0.95), 0 0 50px rgba(56,189,248,0.3)",
              display: "block",
            }}>
              Ammar
            </span>
          </h1>

          {/* Subtitle — readable opacity with shadow */}
          <p style={{
            fontFamily: "var(--font-mono, monospace)",
            fontSize: "clamp(0.8rem, 1.6vw, 0.93rem)",
            color: "rgba(255,255,255,0.70)",
            lineHeight: 1.75,
            maxWidth: "400px",
            marginBottom: "2rem",
            letterSpacing: "0.02em",
            textShadow: "0 1px 14px rgba(3,7,18,0.9)",
          }}>
            Cybersecurity Enthusiast &amp; 3rd Year<br />
            Network &amp; Telecom Engineering Student at INSAT.
          </p>

          {/* CTAs */}
          <div style={{ display: "flex", gap: "0.8rem", flexWrap: "wrap" }}>
            <a
              href="#projects"
              style={{
                fontFamily: "var(--font-mono, monospace)",
                fontSize: "0.67rem", letterSpacing: "0.18em",
                textTransform: "uppercase",
                padding: "13px 26px",
                background: BLUE_FAINT,
                border: `1px solid ${BLUE_DIM}`,
                color: BLUE,
                fontWeight: 700, textDecoration: "none", borderRadius: "2px",
              }}
            >
              View Work
            </a>
            <a
              href="#contact"
              style={{
                fontFamily: "var(--font-mono, monospace)",
                fontSize: "0.67rem", letterSpacing: "0.18em",
                textTransform: "uppercase",
                padding: "12px 26px",
                background: "rgba(3,7,18,0.45)",
                color: "rgba(255,255,255,0.72)",
                fontWeight: 600, textDecoration: "none",
                border: "1px solid rgba(255,255,255,0.15)", borderRadius: "2px",
              }}
            >
              Contact
            </a>
          </div>
        </motion.div>
      </div>

      {/* ── Dot indicators ── */}
      <div style={{
        position: "absolute",
        bottom: "2.4rem",
        left: "50%",
        transform: "translateX(-50%)",
        zIndex: 20,
        display: "flex",
        alignItems: "center",
        gap: "10px",
      }}>
        {BG_PHOTOS.map((_, i) => (
          <button
            key={i}
            onClick={() => goTo(i)}
            aria-label={`Go to ${LABELS[i]}`}
            style={{ all: "unset", cursor: "pointer", display: "flex", flexDirection: "column", alignItems: "center", gap: "5px" }}
          >

            <motion.div
              animate={{
                width: i === current ? "36px" : "18px",
                background: i === current ? BLUE : "rgba(255,255,255,0.18)",
              }}
              transition={{ duration: 0.28, ease: "easeOut" }}
              style={{ height: "2px", borderRadius: "1px" }}
            />
          </button>
        ))}
      </div>

      {/* ── Arrows ── */}
      <NavArrow dir="left" onClick={() => advance(-1)} />
      <NavArrow dir="right" onClick={() => advance(1)} />

      {/* ── Scroll cue ── */}
      <motion.div
        initial={{ opacity: 0 }}
        animate={{ opacity: 1 }}
        transition={{ delay: 1.8, duration: 1 }}
        style={{
          position: "absolute", bottom: "2.4rem",
          right: "clamp(2rem, 6vw, 6rem)",
          zIndex: 20, pointerEvents: "none",
          display: "flex", flexDirection: "column", alignItems: "center", gap: "5px",
        }}
      >
        <motion.div
          animate={{ y: [0, 6, 0], opacity: [0.14, 0.45, 0.14] }}
          transition={{ duration: 2.8, repeat: Infinity, ease: "easeInOut" }}
          style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "5px" }}
        >
          <span style={{
            fontFamily: "var(--font-mono, monospace)", fontSize: "0.46rem",
            letterSpacing: "0.34em", textTransform: "uppercase",
            color: "rgba(255,255,255,0.28)", writingMode: "vertical-lr",
          }}>
            Scroll
          </span>
          <div style={{
            width: "1px", height: "26px",
            background: `linear-gradient(to bottom, ${BLUE_DIM}, transparent)`,
          }} />
        </motion.div>
      </motion.div>

      {/* ── Bottom fade ── */}
      <div style={{
        position: "absolute", bottom: 0, left: 0, right: 0,
        height: "120px",
        background: "linear-gradient(to bottom, transparent, var(--background, #030712))",
        zIndex: 8, pointerEvents: "none",
      }} />
    </section>
  );
}

function SlidePhoto({
  src, direction, reducedMotion,
}: {
  src: string; direction: 1 | -1; reducedMotion: boolean;
}) {
  return (
    <motion.div
      custom={direction}
      initial={reducedMotion ? { opacity: 1, x: 0 } : { opacity: 0, x: direction * 55 }}
      animate={{ opacity: 1, x: 0 }}
      exit={{ opacity: 0, x: -direction * 55 }}
      transition={{
        opacity: { duration: 0.65, ease: "easeInOut" },
        x: { duration: 0.75, ease: [0.22, 1, 0.36, 1] },
      }}
      style={{
        position: "absolute", inset: 0,
        overflow: "hidden",    // contains the Ken Burns scale
      }}
    >
      {/* Ken Burns — contained, no outer scale */}
      <motion.div
        initial={{ scale: 1.06 }}
        animate={{ scale: 1.00 }}
        transition={{ duration: 9, ease: [0.0, 0.0, 0.15, 1] }}
        style={{
          position: "absolute", inset: 0,
          transformOrigin: "center center",
        }}
      >
        <img
          src={src}
          alt=""
          draggable={false}
          style={{
            width: "100%", height: "100%",
            objectFit: "cover", objectPosition: "center 20%",
            filter: "contrast(1.05) saturate(0.38) brightness(0.98)",
            userSelect: "none", display: "block",
          }}
        />
      </motion.div>

      {/* Blue tint fades in */}
      <motion.div
        initial={{ opacity: 0 }}
        animate={{ opacity: 1 }}
        transition={{ duration: 1.4, delay: 0.3 }}
        style={{
          position: "absolute", inset: 0,
          background: "radial-gradient(ellipse at 70% 28%, rgba(56,189,248,0.07) 0%, transparent 62%)",
          mixBlendMode: "screen", pointerEvents: "none",
        }}
      />
    </motion.div>
  );
}

function NavArrow({ dir, onClick }: { dir: "left" | "right"; onClick: () => void }) {
  const isLeft = dir === "left";
  return (
    <button
      onClick={onClick}
      aria-label={isLeft ? "Previous photo" : "Next photo"}
      style={{
        all: "unset", cursor: "pointer",
        position: "absolute", top: "50%", transform: "translateY(-50%)",
        [isLeft ? "left" : "right"]: "clamp(1rem, 2.5vw, 2rem)",
        zIndex: 20,
        width: "40px", height: "40px",
        display: "flex", alignItems: "center", justifyContent: "center",
        border: "1px solid rgba(56,189,248,0.15)", borderRadius: "50%",
        background: "rgba(3,7,18,0.40)", backdropFilter: "blur(12px)",
        color: "rgba(255,255,255,0.50)",
        transition: "border-color 0.2s, color 0.2s, background 0.2s",
      }}
      onMouseEnter={e => {
        const b = e.currentTarget as HTMLButtonElement;
        b.style.borderColor = "rgba(56,189,248,0.50)";
        b.style.color = BLUE;
        b.style.background = "rgba(56,189,248,0.10)";
      }}
      onMouseLeave={e => {
        const b = e.currentTarget as HTMLButtonElement;
        b.style.borderColor = "rgba(56,189,248,0.15)";
        b.style.color = "rgba(255,255,255,0.50)";
        b.style.background = "rgba(3,7,18,0.40)";
      }}
    >
      <svg
        width="13" height="13" viewBox="0 0 14 14"
        fill="none" stroke="currentColor"
        strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"
      >
        {isLeft
          ? <polyline points="9,2 4,7 9,12" />
          : <polyline points="5,2 10,7 5,12" />
        }
      </svg>
    </button>
  );
}