"use client";

import { useEffect, useState } from "react";
import { motion, AnimatePresence, useScroll, useTransform } from "framer-motion";

const PHOTOS = [
  "/media/photo.jpg",
  "/media/cybersphere.jpeg",
  "/media/darkesthours.jpg",
  "/media/cybercampphoto.jpg",
  "/media/cyber.jpeg",
  "/media/scoreboard.jpeg",
  "/media/team.jpeg",
  "/media/groupphoto.jpeg",
  "/media/dup.jpeg",
  "/media/winners.jpeg",
  "/media/finals.jpg",
];

export function PageIntro() {
  const [complete, setComplete] = useState(true);
  const [currentIndex, setCurrentIndex] = useState(0);

  useEffect(() => {
    const seen = sessionStorage.getItem("intro_seen_v1");
    if (seen) return;
    
    setComplete(false);
    
    const interval = setInterval(() => {
        setCurrentIndex(prev => {
            if (prev >= PHOTOS.length - 1) {
                clearInterval(interval);
                setTimeout(() => {
                    setComplete(true);
                    sessionStorage.setItem("intro_seen_v1", "true");
                }, 800);
                return prev;
            }
            return prev + 1;
        });
    }, 180);

    return () => clearInterval(interval);
  }, []);

  return (
    <AnimatePresence>
      {!complete && (
        <motion.div 
          className="fixed inset-0 z-[9999] flex flex-col items-center justify-center bg-black overflow-hidden"
          exit={{ opacity: 0, scale: 1.1, filter: "blur(15px)" }}
          transition={{ duration: 0.9, ease: [0.16, 1, 0.3, 1] }}
        >
            <div className="absolute inset-0 opacity-30 mix-blend-overlay bg-primary pointer-events-none z-10" />
            
            <motion.img 
                key={currentIndex}
                src={PHOTOS[currentIndex]} 
                alt="neural_flash"
                className="w-full h-full object-cover opacity-60 absolute inset-0 z-0 filter grayscale contrast-150"
                initial={{ opacity: 0, scale: 0.9 }}
                animate={{ opacity: 0.6, scale: 1.05 }}
                transition={{ duration: 0.15 }}
            />
            
            <div className="relative z-20 flex flex-col items-center text-center px-4">
                <span className="text-white font-orbitron font-bold text-5xl md:text-7xl uppercase tracking-[0.2em] mix-blend-difference drop-shadow-[0_0_20px_rgba(255,255,255,0.8)]">
                    KHALIL AMMAR
                </span>
                <span className="font-mono text-primary-300 font-bold text-xs md:text-sm tracking-widest mt-6 animate-pulse bg-black/50 px-3 py-1 rounded">
                    [ INITIALIZING PORTFOLIO: {((currentIndex / (PHOTOS.length - 1)) * 100).toFixed(0)}% ]
                </span>
            </div>
            
            <div className="pointer-events-none absolute inset-0 z-30 bg-[url('/media/noise.png')] opacity-20 bg-repeat" />
        </motion.div>
      )}
    </AnimatePresence>
  );
}

export function ScatteredArtefacts() {
    const { scrollYProgress } = useScroll();
    
    // Parallax transforms — varied speeds for depth
    const y1 = useTransform(scrollYProgress, [0, 1], ["0vh", "-80vh"]);
    const y2 = useTransform(scrollYProgress, [0, 1], ["20vh", "-120vh"]);
    const y3 = useTransform(scrollYProgress, [0, 1], ["-10vh", "60vh"]);
    const y4 = useTransform(scrollYProgress, [0, 1], ["50vh", "-50vh"]);
    const y5 = useTransform(scrollYProgress, [0, 1], ["0vh", "-140vh"]);
    const y6 = useTransform(scrollYProgress, [0, 1], ["80vh", "-30vh"]);
    const y7 = useTransform(scrollYProgress, [0, 1], ["30vh", "-90vh"]);
    const y8 = useTransform(scrollYProgress, [0, 1], ["-20vh", "40vh"]);

    const transforms = [y1, y2, y3, y4, y5, y6, y7, y8];

    // More spread positions with varied sizes and rotations for visual interest
    const spread = [
        { right: "3%", top: "8%", width: "22vw", rotate: -6, opacity: 0.04, blur: 0 },
        { left: "-3%", top: "25%", width: "28vw", rotate: 14, opacity: 0.05, blur: 1 },
        { right: "18%", top: "42%", width: "18vw", rotate: -12, opacity: 0.03, blur: 0 },
        { left: "8%", top: "58%", width: "14vw", rotate: 9, opacity: 0.06, blur: 0 },
        { right: "-4%", top: "72%", width: "32vw", rotate: 3, opacity: 0.04, blur: 2 },
        { left: "22%", top: "88%", width: "20vw", rotate: -10, opacity: 0.05, blur: 0 },
        { right: "12%", top: "105%", width: "25vw", rotate: 7, opacity: 0.035, blur: 1 },
        { left: "-2%", top: "120%", width: "30vw", rotate: -4, opacity: 0.045, blur: 0 },
    ];

    return (
        <div className="fixed inset-0 z-[-1] pointer-events-none overflow-hidden mix-blend-luminosity">
            {spread.map((pos, i) => {
                const { blur, ...rest } = pos;
                return (
                    <motion.img 
                        key={i}
                        src={PHOTOS[i % PHOTOS.length]}
                        className="absolute filter grayscale contrast-125 object-cover rounded-3xl"
                        style={{
                            ...rest,
                            y: transforms[i % transforms.length],
                            filter: `grayscale(1) contrast(1.25)${blur ? ` blur(${blur}px)` : ''}`,
                        }}
                    />
                );
            })}
        </div>
    );
}
