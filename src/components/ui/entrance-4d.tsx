"use client";

import { motion, useReducedMotion, useScroll, useTransform, AnimatePresence } from "framer-motion";
import { useEffect, useState } from "react";

const BG_PHOTOS = ["/media/finals.jpg",
  "/media/photo.jpg",
  "/media/cybersphere.jpeg",
  "/media/darkesthours.jpg",
];

const STATS = [
  { label: "CTF Rank", value: "Top 1%" },
  { label: "Competitions", value: "12+" },
  { label: "Projects", value: "8+" },
  { label: "Awards", value: "6" },
];

const containerVariants = {
  hidden: {},
  visible: { transition: { staggerChildren: 0.12, delayChildren: 0.3 } },
};

const childVariants = {
  hidden: { opacity: 0, y: 30, filter: "blur(8px)" },
  visible: {
    opacity: 1, y: 0, filter: "blur(0px)",
    transition: { duration: 0.9, ease: [0.16, 1, 0.3, 1] as const },
  },
};

export function CreativeEntrance4D() {
  const reducedMotion = useReducedMotion();
  const [bgIndex, setBgIndex] = useState(0);

  // Parallax on scroll
  const { scrollYProgress } = useScroll();
  const sectionProgress = useTransform(scrollYProgress, [0, 0.5], [0, 1]);
  const bgY = useTransform(sectionProgress, [0, 1], ["0%", "30%"]);
  const bgScale = useTransform(sectionProgress, [0, 1], [1.02, 1.15]);
  const overlayOpacity = useTransform(sectionProgress, [0, 1], [0.65, 0.88]);
  const contentY = useTransform(sectionProgress, [0, 1], ["0%", "18%"]);

  // Track mouse position to interactively switch photos without buttons
  const handleMouseMove = (e: React.MouseEvent) => {
    const { clientX } = e;
    const width = window.innerWidth;
    const sectionWidth = width / BG_PHOTOS.length;
    const index = Math.min(BG_PHOTOS.length - 1, Math.floor(clientX / sectionWidth));
    if (index !== bgIndex) {
      setBgIndex(index);
    }
  };

  return (
    <section
      id="entrance"
      onMouseMove={handleMouseMove}
      style={{
        position: "relative",
        width: "100%",
        height: "100vh",
        overflow: "hidden",
        background: "var(--background)",
      }}
    >
      {/* ── Cinematic Single Photo Background ── */}
      <motion.div
        style={{
          position: "absolute", inset: 0, zIndex: 0,
          y: bgY,
          scale: bgScale,
        }}
      >
        <AnimatePresence initial={false} mode="sync">
          <motion.img
            key={bgIndex}
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            transition={{ duration: 1.2, ease: "easeInOut" }}
            src={BG_PHOTOS[bgIndex]}
            alt=""
            draggable={false}
            style={{
              position: "absolute", inset: 0,
              width: "100%", height: "110%",
              objectFit: "cover", objectPosition: "center 20%",
              filter: "contrast(1.1) saturate(0.55) brightness(1.05)",
            }}
          />
        </AnimatePresence>
      </motion.div>

      {/* ── Dark overlay ── */}
      <motion.div
        style={{
          position: "absolute", inset: 0, zIndex: 1,
          opacity: overlayOpacity,
          background: `linear-gradient(
            180deg,
            var(--background) 0%,
            color-mix(in srgb, var(--background) 80%, transparent) 35%,
            color-mix(in srgb, var(--background) 65%, transparent) 60%,
            var(--background) 100%
          )`,
          pointerEvents: "none",
        }}
      />

      {/* ── Tactical Grid ── */}
      <div style={{
        position: "absolute", inset: "-100px", zIndex: 2, pointerEvents: "none",
        backgroundImage: `
          linear-gradient(var(--line) 1px, transparent 1px),
          linear-gradient(90deg, var(--line) 1px, transparent 1px)
        `,
        backgroundSize: "60px 60px",
        opacity: 0.12,
        animation: "global-grid-drift 40s linear infinite",
      }} />

      {/* ── Accent glow orbs ── */}
      <div style={{ position: "absolute", inset: 0, zIndex: 2, pointerEvents: "none", overflow: "hidden" }}>
        <div style={{
          position: "absolute", width: "900px", height: "900px", borderRadius: "50%",
          background: "var(--accent)", filter: "blur(200px)", opacity: 0.08,
          top: "-20%", left: "-10%",
          animation: "float-global-orb 25s ease-in-out infinite alternate",
        }} />
        <div style={{
          position: "absolute", width: "700px", height: "700px", borderRadius: "50%",
          background: "var(--accent-alt)", filter: "blur(180px)", opacity: 0.06,
          bottom: "-15%", right: "-5%",
          animation: "float-global-orb 30s ease-in-out infinite alternate-reverse",
        }} />
      </div>

      {/* ── Noise texture ── */}
      <div style={{
        position: "absolute", inset: 0, zIndex: 3, pointerEvents: "none",
        backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 200 200' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.65' numOctaves='3' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23n)'/%3E%3C/svg%3E")`,
        opacity: 0.05,
        mixBlendMode: "overlay",
      }} />

      {/* ── Progress Indicators (subtle lines) ── */}
      <div style={{
        position: "absolute", bottom: "3.5rem", left: "50%", transform: "translateX(-50%)",
        zIndex: 15, display: "flex", alignItems: "center", gap: "8px", pointerEvents: "none"
      }}>
        {BG_PHOTOS.map((_, i) => (
          <div
            key={i}
            style={{
              width: "40px",
              height: "2px",
              background: i === bgIndex ? "var(--accent)" : "rgba(255,255,255,0.15)",
              transition: "background 0.5s ease-in-out",
            }}
          />
        ))}
      </div>

      {/* ── Main Content ── */}
      <motion.div
        style={{
          position: "relative", zIndex: 10, pointerEvents: "none",
          height: "100%",
          display: "flex", flexDirection: "column",
          justifyContent: "center",
          padding: "0 clamp(2rem, 6vw, 6rem)",
          y: contentY,
        }}
      >
        <motion.div
          variants={containerVariants}
          initial={reducedMotion ? "visible" : "hidden"}
          animate="visible"
          style={{ maxWidth: "900px", pointerEvents: "auto" }}
        >


          {/* Name */}
          <motion.h1 variants={childVariants} style={{
            fontFamily: "var(--font-heading), sans-serif",
            fontSize: "clamp(3rem, 8vw, 6.5rem)",
            fontWeight: 900,
            lineHeight: 0.92,
            letterSpacing: "-0.04em",
            textTransform: "uppercase",
            color: "var(--foreground)",
            margin: "0 0 0.5rem",
          }}>
            Khalil
            <br />
            <span style={{ color: "var(--accent)", opacity: 0.85 }}>Ammar</span>
          </motion.h1>

          {/* Headline */}
          <motion.p variants={childVariants} style={{
            fontFamily: "var(--font-mono), monospace",
            fontSize: "clamp(0.85rem, 2vw, 1.1rem)",
            color: "var(--foreground)",
            opacity: 0.55,
            lineHeight: 1.7,
            maxWidth: "520px",
            marginBottom: "2.5rem",
            letterSpacing: "0.02em",
          }}>
            Cybersecurity Enthusiast &amp; 3rd Year Network and Telecommunication Engineering Student at INSAT.
          </motion.p>




        </motion.div>
      </motion.div>

      {/* ── Scroll Indicator ── */}
      <motion.div
        initial={{ opacity: 0 }}
        animate={{ opacity: 1 }}
        transition={{ delay: 1.5, duration: 1 }}
        style={{
          position: "absolute",
          bottom: "2.5rem",
          right: "clamp(2rem, 6vw, 6rem)",
          zIndex: 15,
          display: "flex", flexDirection: "column", alignItems: "center", gap: "8px",
          pointerEvents: "none",
        }}
      >
        <motion.div
          animate={{ y: [0, 6, 0], opacity: [0.3, 0.8, 0.3] }}
          transition={{ duration: 2.5, repeat: Infinity, ease: "easeInOut" }}
          style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "6px" }}
        >
          <span style={{
            fontFamily: "var(--font-mono), monospace",
            fontSize: "0.6rem",
            letterSpacing: "0.3em",
            textTransform: "uppercase",
            color: "var(--foreground)",
            opacity: 0.4,
            writingMode: "vertical-lr",
          }}>
            Scroll
          </span>
          <div style={{ width: "1px", height: "30px", background: "linear-gradient(to bottom, var(--accent), transparent)" }} />
        </motion.div>
      </motion.div>

      {/* ── Bottom gradient fade ── */}
      <div style={{
        position: "absolute", bottom: 0, left: 0, right: 0,
        height: "120px",
        background: "linear-gradient(to bottom, transparent, var(--background))",
        zIndex: 6, pointerEvents: "none",
      }} />
    </section>
  );
}