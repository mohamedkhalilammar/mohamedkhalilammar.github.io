"use client";

import { useState, useEffect } from "react";
import { motion, AnimatePresence } from "framer-motion";

export function CinematicVideo({ url, title }: { url: string; title: string }) {
  const [isLoaded, setIsLoaded] = useState(false);
  const [showIframe, setShowIframe] = useState(false);

  // Extract YouTube ID
  const getYoutubeId = (url: string) => {
    const regExp = /^.*(youtu.be\/|v\/|u\/\w\/|embed\/|watch\?v=|&v=)([^#&?]*).*/;
    const match = url.match(regExp);
    return match && match[2].length === 11 ? match[2] : null;
  };

  const videoId = getYoutubeId(url);
  const thumbnailUrl = videoId 
    ? `https://img.youtube.com/vi/${videoId}/maxresdefault.jpg` 
    : null;

  return (
    <div className="relative w-full aspect-video bg-zinc-900 overflow-hidden group">
      <AnimatePresence>
        {!isLoaded && (
          <motion.div
            initial={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            className="absolute inset-0 z-20 flex items-center justify-center bg-zinc-900"
          >
            {thumbnailUrl ? (
              <img 
                src={thumbnailUrl} 
                className="absolute inset-0 w-full h-full object-cover opacity-60" 
                alt=""
              />
            ) : (
              <div className="absolute inset-0 bg-gradient-to-br from-zinc-800 to-zinc-950" />
            )}
            
            <div className="relative z-30 flex flex-col items-center gap-4">
              <motion.button
                onClick={() => setShowIframe(true)}
                whileHover={{ scale: 1.1 }}
                whileTap={{ scale: 0.95 }}
                className="w-20 h-20 rounded-full bg-amber-500 flex items-center justify-center shadow-2xl shadow-amber-500/20"
              >
                <div className="w-0 h-0 border-t-[10px] border-t-transparent border-l-[18px] border-l-black border-b-[10px] border-b-transparent ml-1" />
              </motion.button>
              <span className="font-mono text-[10px] uppercase tracking-[0.3em] text-white/40">Initialize Stream</span>
            </div>
          </motion.div>
        )}
      </AnimatePresence>

      {(showIframe || !videoId) && (
        <iframe
          src={`${url}${url.includes('?') ? '&' : '?'}autoplay=1&mute=0`}
          title={title}
          frameBorder="0"
          allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
          allowFullScreen
          onLoad={() => setIsLoaded(true)}
          className={`w-full h-full absolute inset-0 z-10 transition-opacity duration-1000 ${isLoaded ? 'opacity-100' : 'opacity-0'}`}
        />
      )}
    </div>
  );
}
