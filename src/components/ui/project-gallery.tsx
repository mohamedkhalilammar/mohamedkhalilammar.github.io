"use client";

import { motion, AnimatePresence } from "framer-motion";
import { useState } from "react";

interface ProjectGalleryProps {
  name: string;
  screenshots: string[];
  screenshotCaptions?: string[];
}

export function ProjectGallery({ name, screenshots, screenshotCaptions }: ProjectGalleryProps) {
  const [currentIndex, setCurrentIndex] = useState(0);

  if (!screenshots || screenshots.length === 0) return null;

  return (
    <div className="w-full flex flex-col lg:flex-row gap-4 bg-black/40 border border-white/5 rounded-2xl p-4 shadow-[0_0_40px_rgba(0,0,0,0.5)] overflow-hidden relative backdrop-blur-md">
      
      {/* Main Visual Display Container */}
      <div className="relative w-full lg:w-3/4 aspect-[16/9] sm:aspect-[16/9] max-h-[300px] md:max-h-[400px] rounded-xl overflow-hidden bg-black/90 border border-primary-500/20 group">
        
        {/* Tactical UI Elements */}
        <div className="absolute top-4 left-4 w-6 h-6 border-t-2 border-l-2 border-primary-500/50 z-20 pointer-events-none" />
        <div className="absolute top-4 right-4 w-6 h-6 border-t-2 border-r-2 border-primary-500/50 z-20 pointer-events-none" />
        <div className="absolute bottom-4 left-4 w-6 h-6 border-b-2 border-l-2 border-primary-500/50 z-20 pointer-events-none" />
        <div className="absolute bottom-4 right-4 w-6 h-6 border-b-2 border-r-2 border-primary-500/50 z-20 pointer-events-none" />

        <div className="absolute top-4 left-1/2 -translate-x-1/2 px-3 py-1 bg-black/60 border border-primary-500/30 rounded text-[9px] sm:text-[10px] font-mono text-primary-500/80 tracking-widest z-20 flex items-center gap-2 backdrop-blur-sm">
          <span className="w-1.5 h-1.5 rounded-full bg-red-500 animate-pulse shadow-[0_0_5px_red]" />
          {name.toUpperCase().substring(0, 24)} // SYS.ARCHIVE
        </div>

        {/* Hardware Scanline Animation */}
        <motion.div 
          animate={{ y: ["-100%", "3000%"] }}
          transition={{ repeat: Infinity, duration: 5, ease: "linear" }}
          className="absolute left-0 right-0 h-16 bg-gradient-to-b from-transparent via-primary-400/10 w-full to-transparent z-10 pointer-events-none"
        />

        {/* Dynamic Image Transition */}
        <AnimatePresence mode="wait">
          <motion.img
            key={currentIndex}
            src={screenshots[currentIndex]}
            alt={screenshotCaptions?.[currentIndex] || `Screenshot ${currentIndex + 1}`}
            initial={{ opacity: 0, scale: 1.03, filter: "blur(10px) brightness(1.5)" }}
            animate={{ opacity: 1, scale: 1, filter: "blur(0px) brightness(1)" }}
            exit={{ opacity: 0, scale: 0.98, filter: "blur(10px) brightness(0.5)" }}
            transition={{ duration: 0.4, ease: "easeOut" }}
            className="w-full h-full object-contain z-0 relative"
          />
        </AnimatePresence>

        {/* Caption Overlay */}
        <AnimatePresence mode="wait">
          {screenshotCaptions?.[currentIndex] && (
            <motion.div
              key={`caption-${currentIndex}`}
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: 10 }}
              transition={{ delay: 0.2, duration: 0.3 }}
              className="absolute bottom-0 left-0 right-0 p-6 md:p-8 bg-gradient-to-t from-black via-black/80 to-transparent z-20"
            >
              <p className="text-primary-50 font-mono text-[11px] md:text-sm border-l-2 border-primary-500 pl-4 py-1 leading-relaxed shadow-sm drop-shadow-lg max-w-3xl">
                <span className="text-primary-500 font-bold tracking-widest mr-2 block md:inline">DESC //</span> 
                {screenshotCaptions[currentIndex]}
              </p>
            </motion.div>
          )}
        </AnimatePresence>
        
      </div>

      {/* Thumbnail Strip / Data Index */}
      <div className="w-full lg:w-1/4 flex flex-row lg:flex-col gap-3 overflow-x-auto lg:overflow-y-auto [&::-webkit-scrollbar]:hidden [-ms-overflow-style:none] [scrollbar-width:none] py-2 lg:py-0 pr-2">
        <div className="hidden lg:flex items-center gap-2 mb-1 px-1 opacity-80">
          <span className="w-full h-px bg-primary-500/40" />
          <span className="text-[10px] font-mono text-primary-500 shrink-0 tracking-widest">THUMB_INDEX</span>
        </div>
        
        {screenshots.map((img, i) => (
          <button
            key={i}
            onClick={() => setCurrentIndex(i)}
            className={`relative shrink-0 w-32 sm:w-40 lg:w-full aspect-[4/3] lg:aspect-video rounded-md overflow-hidden transition-all duration-300 group ${
              currentIndex === i 
                ? "opacity-100 shadow-[0_0_20px_rgba(var(--primary-rgb),0.15)] z-10" 
                : "opacity-40 hover:opacity-80 grayscale hover:grayscale-[50%] scale-95 hover:scale-100"
            }`}
          >
            <img 
              src={img} 
              alt="" 
              className="w-full h-full object-cover transition-transform duration-500 group-hover:scale-110"
            />
            
            {currentIndex === i ? (
              <motion.div 
                layoutId="active-thumb-outline"
                className="absolute inset-0 border-2 border-primary-400 rounded-md z-10 bg-primary-500/10 pointer-events-none"
                transition={{ type: "spring", stiffness: 300, damping: 30 }}
              />
            ) : (
              <div className="absolute inset-0 border border-white/10 rounded-md z-10 pointer-events-none" />
            )}

            <div className="absolute top-1 left-2 text-[10px] font-mono font-bold tracking-widest drop-shadow-md z-10">
              <span className={currentIndex === i ? "text-primary-400 drop-shadow-[0_0_5px_rgba(var(--primary-rgb),0.8)]" : "text-white/80"}>
                {String(i + 1).padStart(2, '0')}
              </span>
            </div>
            {/* Overlay gradient for unselected */}
            {currentIndex !== i && <div className="absolute inset-0 bg-black/30 pointer-events-none transition-colors group-hover:bg-transparent" />}
          </button>
        ))}
      </div>
    </div>
  );
}
