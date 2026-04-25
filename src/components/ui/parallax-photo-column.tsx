"use client";

/**
 * ParallaxPhotoColumn — replaces the photo column JSX in your hero section.
 *
 * Your page.tsx already has:
 *   const { scrollYProgress } = useScroll({ target: heroRef, offset: ["start start", "end start"] });
 *
 * Pass that scrollYProgress into this component and it will apply a
 * subtle counter-scroll parallax to each photo independently.
 *
 * Usage in page.tsx (inside the hero grid, right column):
 *
 * import { ParallaxPhotoColumn } from "@/components/ui/parallax-photo-column";
 *
 * // Inside the hero JSX, replace the existing photo div with:
 * <ParallaxPhotoColumn scrollYProgress={scrollYProgress} />
 */

import { motion, MotionValue, useTransform } from "framer-motion";
import Image from "next/image";

interface Props {
  scrollYProgress: MotionValue<number>;
}

export function ParallaxPhotoColumn({ scrollYProgress }: Props) {
  // Each photo moves at a different rate — creates depth
  const y1 = useTransform(scrollYProgress, [0, 1], ["0%", "-18%"]); // back photo moves less
  const y2 = useTransform(scrollYProgress, [0, 1], ["0%", "-28%"]); // front photo moves more

  return (
    <div className="w-full flex justify-center lg:justify-end css-stagger-item relative">
      <div className="relative w-full max-w-[550px] aspect-[4/5] md:aspect-square">

        {/* Duo Photo — back layer, slower parallax */}
        <motion.div
          className="absolute top-0 left-0 w-[75%] md:w-[70%] cursor-pointer"
          style={{ zIndex: 0, y: y1 }}
          whileHover={{ zIndex: 50, scale: 1.05 }}
          transition={{ type: "spring", stiffness: 300, damping: 20 }}
        >
          <img
            src="/media/dup.jpeg"
            alt="CTF Event Duo"
            className="w-full h-auto object-cover rounded-3xl border border-white/5 shadow-2xl transition-all duration-500"
          />
        </motion.div>

        {/* Portrait Photo — front layer, faster parallax */}
        <motion.div
          className="absolute bottom-0 right-0 w-[75%] md:w-[70%] cursor-pointer"
          style={{ zIndex: 10, y: y2 }}
          whileHover={{ zIndex: 50, scale: 1.05 }}
          transition={{ type: "spring", stiffness: 300, damping: 20 }}
        >
          <img
            src="/media/photo.jpg"
            alt="Khalil Ammar"
            className="w-full h-auto object-cover rounded-3xl border border-white/10 shadow-[0_30px_60px_rgba(0,0,0,0.8)] transition-all duration-500"
          />
        </motion.div>

      </div>
    </div>
  );
}
