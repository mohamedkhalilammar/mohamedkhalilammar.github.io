"use client";

import { useState, useEffect, useRef } from "react";
import { motion, AnimatePresence, useMotionValue, useSpring, useTransform } from "framer-motion";

const commands = [
  { 
    cmd: "cat /bio/overview.md", 
    out: "01 I’m an engineering student at INSAT specializing in Networks and Telecommunications, with a strong interest in cybersecurity. I enjoy exploring how systems work at a low level, especially through reverse engineering, binary analysis, and Android application reversing.\n\n02 Outside of my studies, I regularly take part in CTF competitions, work on personal projects, and occasionally create challenges of my own. These experiences have helped me develop practical skills in areas like Android security, exploitation, and debugging.\n\n03 I’m currently working on improving my skills in system and web penetration testing through hands-on labs on dedicated learning platforms. My goal is to build a solid understanding of systems and security, and to create tools and solutions that are both useful and reliable.\n\n04 In parallel, I’ve been diving deeper into the effective use of AI—focusing on prompt engineering, workflow optimization, and the integration of tools such as MCP servers—to better understand how to systematically leverage AI as a practical, reliable component within technical and security-focused workflows. I am currently exploring the intersection of AI agents and offensive security automation." 
  }
];

export function LiveTerminal() {
  const [history, setHistory] = useState<{ type: "out"; text: string }[]>([]);
  const [lineIndex, setLineIndex] = useState(0);
  const [phase, setPhase] = useState<"output" | "done">("output");

  const ref = useRef<HTMLDivElement>(null);
  
  const x = useMotionValue(0);
  const y = useMotionValue(0);

  const mouseXSpring = useSpring(x, { stiffness: 300, damping: 30 });
  const mouseYSpring = useSpring(y, { stiffness: 300, damping: 30 });

  const rotateX = useTransform(mouseYSpring, [-0.5, 0.5], ["10deg", "-10deg"]);
  const rotateY = useTransform(mouseXSpring, [-0.5, 0.5], ["-10deg", "10deg"]);

  const handleMouseMove = (e: React.MouseEvent<HTMLDivElement>) => {
    if (!ref.current) return;
    
    const rect = ref.current.getBoundingClientRect();
    
    const width = rect.width;
    const height = rect.height;
    
    const mouseX = e.clientX - rect.left;
    const mouseY = e.clientY - rect.top;
    
    const xPct = mouseX / width - 0.5;
    const yPct = mouseY / height - 0.5;
    
    x.set(xPct);
    y.set(yPct);
  };

  const handleMouseLeave = () => {
    x.set(0);
    y.set(0);
  };

  useEffect(() => {
    const lines = commands[0].out.split('\n\n');
    
    if (phase === "output" && lineIndex < lines.length) {
      const timeout = setTimeout(() => {
        setHistory((prev) => [...prev, { type: "out", text: lines[lineIndex] }]);
        setLineIndex((prev) => prev + 1);
      }, 800 + Math.random() * 400);
      return () => clearTimeout(timeout);
    } else if (lineIndex >= lines.length) {
      setPhase("done");
    }
  }, [phase, lineIndex]);

  return (
    <motion.div 
      className="terminal-perspective-wrapper" 
      ref={ref}
      onMouseMove={handleMouseMove}
      onMouseLeave={handleMouseLeave}
      initial={{ opacity: 0, scale: 0.9, y: 30 }}
      animate={{ opacity: 1, scale: 1, y: 0 }}
      transition={{ duration: 0.8, delay: 0.2, type: 'spring', bounce: 0.4 }}
      style={{
        rotateX,
        rotateY,
        transformStyle: "preserve-3d",
        perspective: 1200
      }}
    >
      <aside 
        className="hero-terminal border-[0.5px] border-white/20 bg-[#07090e]/95 backdrop-blur-2xl shadow-2xl relative overflow-hidden flex flex-col h-[600px] md:h-[800px] rounded-2xl" 
        aria-label="Live security terminal"
      >
        <div className="bg-white/5 border-b-[0.5px] border-white/10 px-5 py-3 flex items-center justify-between">
          <div className="flex gap-2" aria-hidden>
            <div className="w-2.5 h-2.5 rounded-full bg-red-500/50" />
            <div className="w-2.5 h-2.5 rounded-full bg-amber-500/50" />
            <div className="w-2.5 h-2.5 rounded-full bg-green-500/50" />
          </div>
          <p className="font-mono text-[9px] uppercase tracking-widest text-zinc-500 font-bold">session://root@khalil-dossier</p>
        </div>

        <div className="flex-1 overflow-y-auto p-10 font-mono text-sm md:text-base antialiased scrollbar-hide">
          <ul className="space-y-12">
            <AnimatePresence mode="popLayout">
              {history.map((item, i) => (
                <motion.li 
                  key={i} 
                  initial={{ opacity: 0, y: 20, filter: "blur(4px)" }}
                  animate={{ opacity: 1, y: 0, filter: "blur(0px)" }}
                  transition={{ duration: 0.8, ease: [0.16, 1, 0.3, 1] }}
                  className="relative group pb-4"
                >
                  <p className="text-zinc-300 leading-relaxed font-light tracking-wide text-base md:text-lg">
                    {item.text.replace(/^\d+\s/, '')}
                  </p>
                </motion.li>
              ))}
            </AnimatePresence>
            {phase === "output" && (
              <li className="flex items-center gap-4 opacity-30 mt-8">
                 <div className="w-1.5 h-1.5 rounded-full bg-amber-500 animate-pulse" />
                 <span className="text-[9px] uppercase tracking-widest font-bold">Injesting_Data...</span>
              </li>
            )}
          </ul>
        </div>
      </aside>
    </motion.div>
  );
}
