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
  "/media/cybersphere.jpeg",
  "/media/finals.jpg",
  "/media/photo.jpg",
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
  const [isMobile, setIsMobile] = useState(false);

  useEffect(() => {
    const checkMobile = () => setIsMobile(window.innerWidth < 768);
    checkMobile();
    window.addEventListener("resize", checkMobile);
    return () => window.removeEventListener("resize", checkMobile);
  }, []);

  const { scrollYProgress } = useScroll({ 
    target: sectionRef,
    offset: ["start start", "end end"]
  });
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

  // Touch swipe support
  const touchStartX = useRef(0);
  const handleTouchStart = useCallback((e: React.TouchEvent) => {
    touchStartX.current = e.touches[0].clientX;
  }, []);
  const handleTouchEnd = useCallback((e: React.TouchEvent) => {
    const delta = touchStartX.current - e.changedTouches[0].clientX;
    if (Math.abs(delta) > 50) advance(delta > 0 ? 1 : -1);
  }, [advance]);

  useEffect(() => {
    const timer = setInterval(() => advance(1), 5000);
    return () => clearInterval(timer);
  }, [advance]);

  return (
    <section
      id="entrance"
      ref={sectionRef}
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
      style={{
        position: "relative",
        width: "100%",
        height: isMobile ? "70vh" : "88vh",
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

      {/* ── Gradient overlay ── */}
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

      <div style={{
        position: "absolute", inset: 0, zIndex: 5, pointerEvents: "none",
        background: "radial-gradient(ellipse at center, transparent 32%, rgba(3,7,18,0.70) 100%)",
      }} />

      {/* ── Blue accent glow ── */}
      <div style={{
        position: "absolute", top: "-10%", right: "-5%", zIndex: 4,
        width: isMobile ? "300px" : "600px", 
        height: isMobile ? "300px" : "600px", 
        borderRadius: "50%",
        background: "rgba(56,189,248,0.04)",
        filter: "blur(60px)",
        pointerEvents: "none",
      }} />

      {/* ── TEXT CONTENT ── */}
      <div
        style={{
          position: "absolute",
          zIndex: 10,
          bottom: isMobile ? "5rem" : "clamp(4rem, 15vh, 8rem)",
          left: 0,
          right: 0,
          padding: isMobile ? "0 1.5rem" : "0 clamp(1.5rem, 6vw, 6rem)",
          pointerEvents: "none",
        }}
      >
        <div style={{
          position: "absolute",
          bottom: "-2rem",
          left: isMobile ? "1.5rem" : "clamp(1.5rem, 6vw, 6rem)",
          right: isMobile ? "1.5rem" : "20%",
          top: "-3rem",
          background: "radial-gradient(ellipse at 0% 100%, rgba(3,7,18,0.85) 0%, transparent 85%)",
          pointerEvents: "none",
        }} />

        <motion.div
          initial={reducedMotion ? false : { opacity: 0, y: 12 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.7, ease: [0.16, 1, 0.3, 1], delay: 0.3 }}
          style={{ position: "relative", zIndex: 1, pointerEvents: "auto" }}
        >
          <h1 style={{
            fontFamily: "var(--font-heading, sans-serif)",
            fontSize: isMobile ? "2.8rem" : "clamp(3rem, 9vw, 7rem)",
            fontWeight: 900,
            lineHeight: 0.9,
            letterSpacing: "-0.04em",
            textTransform: "uppercase",
            margin: "0 0 1rem",
          }}>
            <motion.span
              initial={{ y: -40, opacity: 0 }}
              animate={{ y: 0, opacity: 1 }}
              transition={{ duration: 0.7, ease: [0.16, 1, 0.3, 1], delay: 0.5 }}
              style={{
                color: "#ffffff",
                textShadow: "0 2px 24px rgba(3,7,18,0.95)",
                display: "block",
              }}
            >
              Khalil
            </motion.span>
            <motion.span
              initial={{ y: -40, opacity: 0 }}
              animate={{ y: 0, opacity: 1 }}
              transition={{ duration: 0.7, ease: [0.16, 1, 0.3, 1], delay: 0.65 }}
              style={{
                color: BLUE,
                textShadow: "0 2px 24px rgba(3,7,18,0.95), 0 0 40px rgba(56,189,248,0.2)",
                display: "block",
              }}
            >
              Ammar
            </motion.span>
          </h1>

          <motion.p
            initial={{ y: -30, opacity: 0 }}
            animate={{ y: 0, opacity: 1 }}
            transition={{ duration: 0.7, ease: [0.16, 1, 0.3, 1], delay: 0.8 }}
            style={{
              fontFamily: "var(--font-mono, monospace)",
              fontSize: isMobile ? "0.75rem" : "clamp(0.85rem, 1.2vw, 1.05rem)",
              color: "rgba(255,255,255,0.75)",
              lineHeight: 1.6,
              maxWidth: isMobile ? "320px" : "400px",
              marginBottom: "2rem",
              letterSpacing: "0.02em",
              textShadow: "0 1px 12px rgba(3,7,18,0.9)",
            }}
          >
            Cybersecurity Enthusiast{" "}<br className="hidden sm:block" />
            &amp; Engineering Student at INSAT.
          </motion.p>

          <motion.div
            initial={{ y: -20, opacity: 0 }}
            animate={{ y: 0, opacity: 1 }}
            transition={{ duration: 0.7, ease: [0.16, 1, 0.3, 1], delay: 0.95 }}
            style={{ display: "flex", gap: "0.8rem", flexWrap: "wrap" }}
          >
            <a
              href="#projects"
              style={{
                fontFamily: "var(--font-mono, monospace)",
                fontSize: "0.65rem", letterSpacing: "0.15em",
                textTransform: "uppercase",
                padding: isMobile ? "10px 20px" : "12px 24px",
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
                fontSize: "0.65rem", letterSpacing: "0.15em",
                textTransform: "uppercase",
                padding: isMobile ? "10px 20px" : "12px 24px",
                background: "rgba(3,7,18,0.6)",
                color: "rgba(255,255,255,0.8)",
                fontWeight: 600, textDecoration: "none",
                border: "1px solid rgba(255,255,255,0.2)", borderRadius: "2px",
              }}
            >
              Contact
            </a>
          </motion.div>
        </motion.div>
      </div>

      {/* ── Dot indicators ── */}
      <div style={{
        position: "absolute",
        bottom: isMobile ? "1.5rem" : "2.4rem",
        left: "50%",
        transform: "translateX(-50%)",
        zIndex: 20,
        display: "flex",
        alignItems: "center",
        gap: "8px",
      }}>
        {BG_PHOTOS.map((_, i) => (
          <button
            key={i}
            onClick={() => goTo(i)}
            aria-label={`Go to ${LABELS[i]}`}
            style={{ all: "unset", cursor: "pointer", display: "flex", padding: "4px" }}
          >
            <motion.div
              animate={{
                width: i === current ? (isMobile ? "24px" : "36px") : (isMobile ? "12px" : "18px"),
                background: i === current ? BLUE : "rgba(255,255,255,0.18)",
              }}
              transition={{ duration: 0.28, ease: "easeOut" }}
              style={{ height: "2px", borderRadius: "1px" }}
            />
          </button>
        ))}
      </div>

      {/* ── Arrows ── */}
      {!isMobile && (
        <>
          <NavArrow dir="left" onClick={() => advance(-1)} />
          <NavArrow dir="right" onClick={() => advance(1)} />
        </>
      )}

      {/* ── Scroll cue ── */}
      {!isMobile && (
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
      )}

      {/* ── Bottom fade ── */}
      <div style={{
        position: "absolute", bottom: 0, left: 0, right: 0,
        height: "100px",
        background: "linear-gradient(to bottom, transparent, #0a0e14)",
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
      initial={reducedMotion ? { opacity: 1, x: 0 } : { opacity: 0, x: direction * 40 }}
      animate={{ opacity: 1, x: 0 }}
      exit={{ opacity: 0, x: -direction * 40 }}
      transition={{
        opacity: { duration: 0.65, ease: "easeInOut" },
        x: { duration: 0.75, ease: [0.22, 1, 0.36, 1] },
      }}
      style={{
        position: "absolute", inset: 0,
        overflow: "hidden",
      }}
    >
      <motion.div
        initial={{ scale: 1.05 }}
        animate={{ scale: 1.00 }}
        transition={{ duration: 8, ease: "easeOut" }}
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
            filter: "contrast(1.05) saturate(0.9) brightness(1.0)",
            userSelect: "none", display: "block",
            transform: "translateZ(0)",
          }}
        />
      </motion.div>

      <motion.div
        initial={{ opacity: 0 }}
        animate={{ opacity: 1 }}
        transition={{ duration: 1.2, delay: 0.2 }}
        style={{
          position: "absolute", inset: 0,
          background: "radial-gradient(ellipse at 70% 28%, rgba(56,189,248,0.05) 0%, transparent 60%)",
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
        transition: "all 0.2s ease",
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