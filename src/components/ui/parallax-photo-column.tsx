"use client";

import { motion, MotionValue, useTransform, useScroll } from "framer-motion";
import { useEffect, useState } from "react";

interface Props {
  scrollYProgress: MotionValue<number>;
}

export function ParallaxPhotoColumn({ scrollYProgress }: Props) {
  const [rotation, setRotation] = useState(0);

  // Continuous rotation effect
  useEffect(() => {
    let frameId: number;
    const animate = () => {
      setRotation(prev => (prev + 0.05) % 360);
      frameId = requestAnimationFrame(animate);
    };
    frameId = requestAnimationFrame(animate);
    return () => cancelAnimationFrame(frameId);
  }, []);

  // Scroll influences the vertical drift and horizontal spread
  const verticalDrift = useTransform(scrollYProgress, [0, 1], [0, -100]);
  const horizontalSpread = useTransform(scrollYProgress, [0, 1], [1, 1.2]);

  const photos = [
    { src: "/media/photo.jpg", alt: "Khalil Ammar", delay: 0 },
    { src: "/media/winners.jpeg", alt: "Achievement", delay: 120 },
    { src: "/media/dup.jpeg", alt: "CTF Event", delay: 240 },
  ];

  return (
    <div className="w-full h-[500px] md:h-[650px] flex items-center justify-center perspective-[2000px] overflow-visible">
      <div className="relative w-full h-full transform-style-3d">
        {photos.map((photo, i) => {
          const angle = (rotation + photo.delay) * (Math.PI / 180);
          const radius = typeof window !== 'undefined' && window.innerWidth < 768 ? 120 : 280; 
          
          return (
            <motion.div
              key={i}
              className="absolute top-1/2 left-1/2 cursor-pointer"
              style={{
                width: i === 0 ? "min(400px, 60vw)" : "min(320px, 50vw)",
                x: "-50%",
                y: "-50%",
                zIndex: Math.round(Math.cos(angle) * 100) + 100,
              }}
              animate={{
                x: `calc(-50% + ${Math.sin(angle) * radius}px)`,
                y: `calc(-50% + ${Math.cos(angle) * 40}px)`, 
                translateZ: Math.cos(angle) * radius,
                rotateY: -Math.sin(angle) * 15,
              }}
              transition={{ type: "spring", stiffness: 30, damping: 20 }}
              whileHover={{ scale: 1.05, zIndex: 1000 }}
            >
              <div className="relative group">
                <div className="absolute inset-0 bg-amber-500/10 opacity-0 group-hover:opacity-100 blur-2xl transition-opacity duration-500 rounded-3xl" />
                <img
                  src={photo.src}
                  alt={photo.alt}
                  className={`w-full h-auto object-cover rounded-2xl md:rounded-3xl border-[0.5px] shadow-[0_20px_50px_rgba(0,0,0,0.7)] transition-all duration-700 antialiased
                    ${i === 0 ? 'border-amber-500/40' : 'border-white/20 opacity-90 group-hover:opacity-100'}
                  `}
                />
              </div>
            </motion.div>
          );
        })}
        
        {/* Central Axis Glow */}
        <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-32 h-32 bg-amber-500/5 blur-[100px] pointer-events-none" />
      </div>
    </div>
  );
}
