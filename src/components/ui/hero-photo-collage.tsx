"use client";

import { useRef } from "react";
import { motion, useMotionValue, useReducedMotion, useSpring, useTransform } from "framer-motion";

const EASE = [0.16, 1, 0.3, 1] as const;

const PHOTOS = [
  { src: "/media/photo.jpg", alt: "Portrait of Khalil Ammar", className: "row-span-2 aspect-[3/4]", delay: 0, depth: 24, float: 10 },
  { src: "/media/team.jpeg", alt: "Khalil with his CTF team", className: "aspect-square", delay: 0.12, depth: 40, float: 14 },
  { src: "/media/teamm.jpeg", alt: "Khalil at a competition", className: "aspect-square", delay: 0.24, depth: 32, float: 12 },
];

/**
 * HeroPhotoCollage — bento photo grid with two layered motions:
 *   1. each tile gently floats forever (idle life)
 *   2. the whole cluster parallax-tilts toward the cursor, each tile by its
 *      own depth so the collage feels 3D.
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

        <div className="relative grid grid-cols-2 gap-3 md:gap-4" style={{ transformStyle: "preserve-3d" }}>
          {PHOTOS.map((img) => (
            <ParallaxTile key={img.src} img={img} sx={sx} sy={sy} reduced={!!reduced} />
          ))}
        </div>
      </div>
    </div>
  );
}

type TileProps = {
  img: (typeof PHOTOS)[number];
  sx: ReturnType<typeof useSpring>;
  sy: ReturnType<typeof useSpring>;
  reduced: boolean;
};

function ParallaxTile({ img, sx, sy, reduced }: TileProps) {
  const tx = useTransform(sx, (v) => v * img.depth);
  const ty = useTransform(sy, (v) => v * img.depth);

  return (
    <motion.div
      initial={{ opacity: 0, y: 26, scale: 0.96 }}
      whileInView={{ opacity: 1, y: 0, scale: 1 }}
      viewport={{ once: true, amount: 0.3 }}
      transition={{ delay: img.delay, duration: 0.7, ease: EASE }}
      style={reduced ? undefined : { x: tx, y: ty }}
      className={`group relative ${img.className}`}
      data-cursor="view"
    >
      {/* inner wrapper carries the endless float so it composes with parallax */}
      <motion.div
        animate={reduced ? undefined : { y: [0, -img.float, 0] }}
        transition={{ duration: 5 + img.delay * 4, repeat: Infinity, ease: "easeInOut", delay: img.delay }}
        whileHover={reduced ? undefined : { scale: 1.04, rotate: -1.5, zIndex: 30 }}
        className="relative w-full h-full overflow-hidden rounded-3xl ring-1 ring-white/10 bg-black/30 cursor-pointer"
      >
        <img
          src={img.src}
          alt={img.alt}
          className="w-full h-full object-cover transition-transform duration-700 ease-out group-hover:scale-[1.08]"
        />
        <div className="absolute inset-0 bg-gradient-to-t from-[#080a14]/55 via-transparent to-transparent opacity-70 group-hover:opacity-25 transition-opacity duration-500" />
        <div className="absolute inset-0 rounded-3xl ring-1 ring-inset ring-white/10 group-hover:ring-primary-300/50 transition-colors duration-500 pointer-events-none" />
      </motion.div>
    </motion.div>
  );
}
