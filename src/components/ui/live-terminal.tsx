"use client";

import { useState, useEffect, useRef } from "react";
import { motion, AnimatePresence, useMotionValue, useSpring, useTransform } from "framer-motion";

const commands = [
  { cmd: "whoami --profile", out: "Name: Khalil Ammar\nAge: 21\nRole: Offensive Security Engineer\nEducation: INSAT ICT 3rd-Year" },
  { cmd: "cat /etc/skills.conf", out: "[ok] Malware Analysis & Reverse Engineering\n[ok] Mobile Pentesting & Hardening Defeat\n[info] SOC & Threat Hunting Protocols Active" },
  { cmd: "tail -f /var/log/syslog --lines=2", out: "Establishing secure reverse shell...\nAwaiting incoming challenges..." }
];

export function LiveTerminal() {
  const [history, setHistory] = useState<{ type: "cmd" | "out"; text: string }[]>([]);
  const [currentCmd, setCurrentCmd] = useState("");
  const [cmdIndex, setCmdIndex] = useState(0);
  const [charIndex, setCharIndex] = useState(0);
  const [phase, setPhase] = useState<"typing" | "output" | "wait">("typing");

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
    if (cmdIndex >= commands.length) return;
    const current = commands[cmdIndex];

    if (phase === "typing") {
      if (charIndex < current.cmd.length) {
        const timeout = setTimeout(() => {
          setCurrentCmd((prev) => prev + current.cmd[charIndex]);
          setCharIndex((prev) => prev + 1);
        }, 40 + Math.random() * 40);
        return () => clearTimeout(timeout);
      } else {
        const timeout = setTimeout(() => setPhase("output"), 150);
        return () => clearTimeout(timeout);
      }
    } else if (phase === "output") {
      const timeout = setTimeout(() => {
        setHistory((prev) => [...prev, { type: "cmd", text: current.cmd }]);
        
        const lines = current.out.split('\n');
        let currentLine = 0;

        const processLine = () => {
          if (currentLine < lines.length) {
            const lineToLog = lines[currentLine];
            setHistory((prev) => [...prev, { type: "out", text: lineToLog }]);
            currentLine++;
            setTimeout(processLine, 300 + Math.random() * 250);
          } else {
            setCurrentCmd("");
            setCharIndex(0);
            setPhase("wait");
          }
        };

        setTimeout(processLine, 150);
      }, 0);
      return () => clearTimeout(timeout);
    } else if (phase === "wait") {
      const timeout = setTimeout(() => {
        setCmdIndex((prev) => prev + 1);
        setPhase("typing");
      }, 1200);
      return () => clearTimeout(timeout);
    }
  }, [phase, charIndex, cmdIndex]);

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
      <aside className="hero-terminal" aria-label="Live security terminal">
        <div className="terminal-chrome">
          <div className="terminal-dots" aria-hidden>
            <span />
            <span />
            <span />
          </div>
          <p className="terminal-title">monitor@khalil:~</p>
        </div>
        <div className="terminal-lines-container">
          <ul className="terminal-lines">
            <AnimatePresence>
              {history.map((item, i) => (
                <motion.li 
                  key={i} 
                  initial={{ opacity: 0, x: -10 }}
                  animate={{ opacity: 1, x: 0 }}
                  transition={{ duration: 0.3 }}
                  className={item.type === "cmd" ? "cmd-line" : "out-line"}
                >
                  {item.type === "cmd" && <span className="prompt">$</span>}
                  {item.type === "cmd" && <span className="cmd-text">{item.text}</span>}
                  {item.type === "out" && item.text.startsWith("[ok]") ? (
                    <><span className="ok">[ok]</span>{item.text.substring(4)}</>
                  ) : item.type === "out" && item.text.startsWith("[info]") ? (
                    <><span className="warn">[info]</span>{item.text.substring(6)}</>
                  ) : item.type === "out" ? (
                    item.text
                  ) : null}
                </motion.li>
              ))}
            </AnimatePresence>
            {cmdIndex < commands.length && (
              <li className="typing-line">
                <span className="prompt">$</span> <span className="cmd-text">{currentCmd}</span>
              </li>
            )}
          </ul>
        </div>
      </aside>
    </motion.div>
  );
}
