"use client";

import { useEffect, useRef, useState } from "react";
import {
  AnimatePresence,
  motion,
  useMotionValue,
  useReducedMotion,
  useSpring,
  useTransform,
} from "framer-motion";

const EASE = [0.16, 1, 0.3, 1] as const;

type CollagePhoto = {
  src: string;
  alt: string;
  className: string;
  delay: number;
  depth: number;
  float: number;
  /** extra photos the tile cycles through (crossfade); omit for a static tile */
  pool?: string[];
  cycleMs?: number;
};

const PHOTOS: CollagePhoto[] = [
  {
    src: "/media/photo.jpg",
    alt: "Portrait of Khalil Ammar",
    className: "row-span-2 aspect-[3/4]",
    delay: 0,
    depth: 24,
    float: 10,
  },
  {
    src: "/media/team.jpeg",
    alt: "Khalil with his CTF team",
    className: "aspect-square",
    delay: 0.12,
    depth: 40,
    float: 14,
    pool: ["/media/team.jpeg", "/media/winners.jpeg", "/media/scoreboard.jpeg"],
    cycleMs: 5600,
  },
  {
    src: "/media/teamm.jpeg",
    alt: "Khalil at a competition",
    className: "aspect-square",
    delay: 0.24,
    depth: 32,
    float: 12,
    pool: ["/media/teamm.jpeg", "/media/dup.jpeg", "/media/cybercampphoto.jpg"],
    cycleMs: 7200,
  },
];

/**
 * HeroPhotoCollage — bento photo grid with three layered motions:
 *   1. each tile gently floats forever (idle life)
 *   2. the whole cluster tilts in 3D toward the cursor (rotateX/rotateY),
 *      and each tile parallax-shifts by its own depth
 *   3. the square tiles slowly cycle through extra photos with a crossfade
 *      (paused while hovered so the current photo can be examined)
 * Reduced-motion users get a clean static grid.
 */
export function HeroPhotoCollage() {
  const reduced = useReducedMotion();
  const ref = useRef<HTMLDivElement>(null);

  // pointer position normalised to [-0.5, 0.5]
  const px = useMotionValue(0);
  const py = useMotionValue(0);
  const sx = useSpring(px, { stiffness: 120, damping: 18, mass: 0.5 });
  const sy = useSpring(py, { stiffness: 120, damping: 18, mass: 0.5 });

  // whole-cluster 3D tilt — subtle, spring-smoothed
  const rotateY = useTransform(sx, (v) => v * 9);
  const rotateX = useTransform(sy, (v) => v * -7);

  const onMove = (e: React.MouseEvent) => {
    if (reduced) return;
    const rect = ref.current?.getBoundingClientRect();
    if (!rect) return;
    px.set((e.clientX - rect.left) / rect.width - 0.5);
    py.set((e.clientY - rect.top) / rect.height - 0.5);
  };

  const onLeave = () => {
    px.set(0);
    py.set(0);
  };

  return (
    <div className="lg:col-span-4 w-full pt-8 lg:pt-0 self-center">
      <div
        ref={ref}
        onMouseMove={onMove}
        onMouseLeave={onLeave}
        className="relative max-w-[420px] mx-auto lg:ml-auto"
        style={{ perspective: 1000 }}
      >
        {/* ambient glow that follows the cursor a little */}
        <motion.div
          className="absolute -inset-8 rounded-[3rem] pointer-events-none"
          style={{
            background: "radial-gradient(60% 60% at 60% 40%, rgba(129,140,248,0.16), transparent 70%)",
            x: useTransform(sx, (v) => v * 30),
            y: useTransform(sy, (v) => v * 30),
          }}
          aria-hidden
        />

        <motion.div
          className="relative grid grid-cols-2 gap-3 md:gap-4"
          style={
            reduced
              ? { transformStyle: "preserve-3d" }
              : { transformStyle: "preserve-3d", rotateX, rotateY }
          }
        >
          {PHOTOS.map((img) => (
            <ParallaxTile key={img.src} img={img} sx={sx} sy={sy} reduced={!!reduced} />
          ))}
        </motion.div>
      </div>
    </div>
  );
}

type TileProps = {
  img: CollagePhoto;
  sx: ReturnType<typeof useSpring>;
  sy: ReturnType<typeof useSpring>;
  reduced: boolean;
};

function ParallaxTile({ img, sx, sy, reduced }: TileProps) {
  const tx = useTransform(sx, (v) => v * img.depth);
  const ty = useTransform(sy, (v) => v * img.depth);

  const pool = img.pool ?? [img.src];
  const [photoIdx, setPhotoIdx] = useState(0);
  const hoverRef = useRef(false);

  // slow crossfade cycle through the tile's photo pool; paused on hover
  useEffect(() => {
    if (reduced || pool.length < 2 || !img.cycleMs) return;
    const id = setInterval(() => {
      if (!hoverRef.current) setPhotoIdx((i) => (i + 1) % pool.length);
    }, img.cycleMs);
    return () => clearInterval(id);
  }, [reduced, pool.length, img.cycleMs]);

  return (
    <motion.div
      initial={{ opacity: 0, y: 26, scale: 0.96 }}
      whileInView={{ opacity: 1, y: 0, scale: 1 }}
      viewport={{ once: true, amount: 0.3 }}
      transition={{ delay: img.delay, duration: 0.7, ease: EASE }}
      style={reduced ? undefined : { x: tx, y: ty }}
      className={`group relative ${img.className}`}
      data-cursor="view"
      onMouseEnter={() => (hoverRef.current = true)}
      onMouseLeave={() => (hoverRef.current = false)}
    >
      {/* inner wrapper carries the endless float so it composes with parallax */}
      <motion.div
        animate={reduced ? undefined : { y: [0, -img.float, 0] }}
        transition={{ duration: 5 + img.delay * 4, repeat: Infinity, ease: "easeInOut", delay: img.delay }}
        whileHover={reduced ? undefined : { scale: 1.05, rotate: -1.5, zIndex: 30 }}
        className="relative w-full h-full overflow-hidden rounded-3xl ring-1 ring-white/10 bg-black/30 cursor-pointer"
      >
        <AnimatePresence initial={false} mode="sync">
          <motion.img
            key={pool[photoIdx]}
            src={pool[photoIdx]}
            alt={img.alt}
            initial={reduced ? { opacity: 0 } : { opacity: 0, scale: 1.12 }}
            animate={{ opacity: 1, scale: 1 }}
            exit={{ opacity: 0 }}
            transition={{ duration: 1, ease: EASE }}
            className="absolute inset-0 w-full h-full object-cover"
            draggable={false}
          />
        </AnimatePresence>
        <div className="absolute inset-0 bg-gradient-to-t from-[#080a14]/55 via-transparent to-transparent opacity-70 group-hover:opacity-25 transition-opacity duration-500 pointer-events-none" />
        <span className="card-sheen z-10" aria-hidden />
        <div className="absolute inset-0 rounded-3xl ring-1 ring-inset ring-white/10 group-hover:ring-primary-300/50 transition-colors duration-500 pointer-events-none" />
      </motion.div>
    </motion.div>
  );
}
