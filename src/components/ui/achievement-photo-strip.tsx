"use client";

/**
 * AchievementPhotoStrip — shows achievement photos on mobile as a horizontal scroll strip.
 * On desktop the existing vertical photo stack is still used.
 *
 * Usage in page.tsx:
 *
 * 1. Move ACHIEVEMENT_PHOTOS outside the component (see bottom of this file).
 * 2. Add <AchievementPhotoStrip photos={ACHIEVEMENT_PHOTOS} /> just ABOVE the
 *    "grid grid-cols-1 lg:grid-cols-12" div, inside your achievements SectionShell.
 *
 * The strip is hidden on lg screens (lg:hidden), so it only shows on mobile/tablet.
 */

import { motion } from "framer-motion";

interface Props {
  photos: string[];
}

export function AchievementPhotoStrip({ photos }: Props) {
  return (
    <div className="lg:hidden -mx-4 sm:-mx-6 mb-10 overflow-x-auto scroll-smooth achievement-strip-scroll relative">
      <div
        className="flex gap-3 px-4 sm:px-6 pb-3"
        style={{ width: "max-content" }}
      >
        {photos.map((src, i) => (
          <motion.div
            key={src + i}
            initial={{ opacity: 0, scale: 0.92 }}
            whileInView={{ opacity: 1, scale: 1 }}
            viewport={{ once: true }}
            transition={{ duration: 0.5, delay: i * 0.06 }}
            className="relative flex-shrink-0 w-[220px] h-[155px] rounded-xl overflow-hidden border border-primary-500/30 shadow-lg"
          >
            <img
              src={src}
              alt={`Achievement photo ${i + 1}`}
              className="w-full h-full object-cover object-center brightness-105"
              loading="lazy"
            />
            <div className="absolute inset-0 bg-gradient-to-t from-black/40 to-transparent pointer-events-none" />
          </motion.div>
        ))}
      </div>
      {/* Scroll hint fade */}
      <div className="pointer-events-none absolute right-0 top-0 h-full w-12 bg-gradient-to-l from-black to-transparent" />
    </div>
  );
}
