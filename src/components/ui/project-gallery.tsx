"use client";

import { motion, AnimatePresence } from "framer-motion";
import { useState } from "react";

interface ProjectGalleryProps {
  name: string;
  screenshots: string[];
  screenshotCaptions?: string[];
}

export function ProjectGallery({ name, screenshots, screenshotCaptions }: ProjectGalleryProps) {
  const [currentIndex, setCurrentIndex] = useState<number | null>(null);

  if (!screenshots || screenshots.length === 0) return null;

  const nextImage = (e?: React.MouseEvent) => {
    e?.stopPropagation();
    if (currentIndex !== null) {
      setCurrentIndex((currentIndex + 1) % screenshots.length);
    }
  };

  const prevImage = (e?: React.MouseEvent) => {
    e?.stopPropagation();
    if (currentIndex !== null) {
      setCurrentIndex((currentIndex - 1 + screenshots.length) % screenshots.length);
    }
  };

  return (
    <>
      <div className="w-full">
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-8 w-full">
          {screenshots.map((img, i) => (
            <motion.div
              key={i}
              initial={{ opacity: 0, y: 20 }}
              whileInView={{ opacity: 1, y: 0 }}
              viewport={{ once: true }}
              transition={{ delay: i * 0.05 }}
              className="flex flex-col gap-4 group"
            >
              {/* Photo Display */}
              <div 
                onClick={() => setCurrentIndex(i)}
                className="relative w-full rounded-[2rem] overflow-hidden bg-black/40 border border-white/10 cursor-zoom-in hover:border-amber-500/40 transition-colors shadow-2xl min-h-[200px]"
              >
                <img src={img} alt={screenshotCaptions?.[i] || ""} className="w-full h-auto block group-hover:scale-[1.03] transition-transform duration-700" />
                
                {/* Subtle Hover Overlay */}
                <div className="absolute inset-0 bg-amber-500/0 group-hover:bg-amber-500/5 transition-colors pointer-events-none" />
                
                {/* Zoom Icon */}
                <div className="absolute top-6 right-6 p-2 rounded-full bg-black/60 backdrop-blur-md border border-white/10 opacity-0 group-hover:opacity-100 transition-opacity">
                  <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M15 3h6v6M9 21H3v-6M21 3l-7 7M3 21l7-7"/>
                  </svg>
                </div>
              </div>

              {/* Caption */}
              {screenshotCaptions?.[i] && (
                <p className="text-zinc-400 font-sans text-sm font-medium leading-relaxed px-4 border-l border-amber-500/30 group-hover:text-zinc-200 transition-colors tracking-tight">
                  {screenshotCaptions[i]}
                </p>
              )}
            </motion.div>
          ))}
        </div>
      </div>

      {/* ENHANCED FULLSCREEN LIGHTBOX */}
      <AnimatePresence>
        {currentIndex !== null && (
          <motion.div
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            className="fixed inset-0 z-[100] bg-black/95 backdrop-blur-2xl flex flex-col items-center justify-center p-4 md:p-8"
            onClick={() => setCurrentIndex(null)}
          >
            {/* Close Button */}
            <button 
              className="absolute top-8 right-8 text-white/40 hover:text-white transition-colors z-50 p-2"
              onClick={() => setCurrentIndex(null)}
            >
              <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><path d="M18 6L6 18M6 6l12 12"/></svg>
            </button>

            {/* Navigation Buttons */}
            {screenshots.length > 1 && (
              <>
                <button 
                  className="absolute left-8 top-1/2 -translate-y-1/2 text-white/20 hover:text-white transition-all bg-white/5 hover:bg-white/10 p-4 rounded-full border border-white/5 z-50"
                  onClick={prevImage}
                >
                  <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5"><path d="M15 18l-6-6 6-6"/></svg>
                </button>
                <button 
                  className="absolute right-8 top-1/2 -translate-y-1/2 text-white/20 hover:text-white transition-all bg-white/5 hover:bg-white/10 p-4 rounded-full border border-white/5 z-50"
                  onClick={nextImage}
                >
                  <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5"><path d="M9 18l6-6-6-6"/></svg>
                </button>
              </>
            )}

            {/* Main Image View */}
            <div className="relative max-w-[95vw] max-h-[80vh] flex flex-col items-center gap-8">
              <motion.img
                key={currentIndex}
                initial={{ scale: 0.9, opacity: 0, filter: "blur(10px)" }}
                animate={{ scale: 1, opacity: 1, filter: "blur(0px)" }}
                exit={{ scale: 0.9, opacity: 0, filter: "blur(10px)" }}
                src={screenshots[currentIndex]}
                className="w-full h-full max-h-[75vh] object-contain rounded-2xl shadow-[0_0_100px_rgba(0,0,0,0.8)] border border-white/10"
                onClick={(e) => e.stopPropagation()}
              />
              
              {/* Lightbox Caption */}
              {screenshotCaptions?.[currentIndex] && (
                <motion.p
                  initial={{ opacity: 0, y: 10 }}
                  animate={{ opacity: 1, y: 0 }}
                  className="text-white text-lg md:text-xl font-medium text-center max-w-3xl leading-relaxed"
                >
                  {screenshotCaptions[currentIndex]}
                </motion.p>
              )}

              {/* Counter */}
              <div className="absolute -bottom-16 left-1/2 -translate-x-1/2 px-4 py-1.5 rounded-full bg-white/5 border border-white/10 text-[10px] font-mono text-white/40 tracking-widest uppercase">
                Image {currentIndex + 1} / {screenshots.length}
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </>
  );
}
