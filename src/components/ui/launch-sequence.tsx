"use client";

import { motion, AnimatePresence } from "framer-motion";
import { useEffect, useState } from "react";

export function LaunchSequence() {
  const [complete, setComplete] = useState(false);
  const [lines, setLines] = useState<string[]>([]);
  const [accessGranted, setAccessGranted] = useState(false);

  useEffect(() => {
    document.body.style.overflow = "hidden";
    
    const bootSequence = [
      { text: "INIT SECURE BOOT v4.2.0...", delay: 200 },
      { text: "LOADING KERNEL MODULES... [OK]", delay: 500 },
      { text: "ESTABLISHING ENCRYPTED UPLINK... [OK]", delay: 900 },
      { text: "BYPASSING MAINFRAME FIREWALLS...", delay: 1400 },
      { text: "DECRYPTING TACTICAL ASSETS... [OK]", delay: 2100 },
      { text: "INJECTING CUSTOM PAYLOADS... [OK]", delay: 2600 },
    ];

    const timeouts: NodeJS.Timeout[] = [];

    bootSequence.forEach((item) => {
      const t = setTimeout(() => {
        setLines((prev) => [...prev, item.text]);
      }, item.delay);
      timeouts.push(t);
    });

    const accessTimeout = setTimeout(() => {
      setAccessGranted(true);
    }, 3200);
    timeouts.push(accessTimeout);

    const completeTimeout = setTimeout(() => {
      setComplete(true);
      document.body.style.overflow = "auto";
      window.scrollTo(0, 0);
    }, 4500);
    timeouts.push(completeTimeout);

    return () => {
      timeouts.forEach(clearTimeout);
      document.body.style.overflow = "auto";
    };
  }, []);

  return (
    <AnimatePresence>
      {!complete && (
        <motion.div
          key="launch-sequence"
          initial={{ opacity: 1 }}
          exit={{ opacity: 0, scale: 1.1, filter: "blur(10px)" }}
          transition={{ duration: 0.8, ease: "easeInOut" }}
          className="fixed inset-0 z-[9999] flex items-center justify-center bg-black overflow-hidden"
        >
          {/* Scanline overlay */}
          <div className="absolute inset-0 z-10 pointer-events-none opacity-20" style={{
            backgroundImage: `linear-gradient(rgba(245, 158, 11, 0.2) 1px, transparent 1px)`,
            backgroundSize: "100% 4px",
          }} />

          {/* Terminal Box */}
          <div className="relative z-20 w-[90%] max-w-2xl bg-black/80 border border-amber-500/30 p-8 font-mono shadow-[0_0_50px_rgba(245,158,11,0.15)] rounded-sm">
            <div className="flex flex-col gap-2 mb-8">
              {lines.map((line, i) => (
                <motion.div
                  key={i}
                  initial={{ opacity: 0, x: -10 }}
                  animate={{ opacity: 1, x: 0 }}
                  className="text-amber-500 text-sm md:text-base tracking-widest uppercase"
                >
                  <span className="opacity-50 mr-2">{`>`}</span> {line}
                </motion.div>
              ))}
              {/* Blinking cursor */}
              {!accessGranted && (
                <motion.div
                  animate={{ opacity: [1, 0] }}
                  transition={{ repeat: Infinity, duration: 0.8 }}
                  className="w-3 h-5 bg-amber-500 inline-block mt-1"
                />
              )}
            </div>

            <AnimatePresence>
              {accessGranted && (
                <motion.div
                  initial={{ opacity: 0, scale: 0.8 }}
                  animate={{ opacity: 1, scale: 1 }}
                  className="text-center mt-12 py-8 border-t border-amber-500/30 relative"
                >
                  <motion.div 
                    animate={{ opacity: [0.5, 1, 0.5] }} 
                    transition={{ repeat: Infinity, duration: 2 }}
                    className="absolute inset-0 bg-amber-500/5 blur-xl"
                  />
                  <h2 className="text-4xl md:text-6xl font-black text-[#10b981] tracking-tighter uppercase drop-shadow-[0_0_20px_rgba(16,185,129,0.8)] m-0">
                    Access Granted
                  </h2>
                  <p className="text-[#10b981]/70 tracking-widest mt-2 uppercase text-sm">Welcome to the network, Operative.</p>
                </motion.div>
              )}
            </AnimatePresence>
          </div>

          <div className="absolute inset-0 bg-[radial-gradient(circle_at_center,transparent_0%,black_100%)] pointer-events-none z-30" />
        </motion.div>
      )}
    </AnimatePresence>
  );
}
