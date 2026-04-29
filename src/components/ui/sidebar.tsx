"use client";

import { useState, useEffect } from "react";
import { motion, AnimatePresence } from "framer-motion";

const navigationLinks = [
  { label: "About", href: "#about" },
  { label: "Achievements", href: "#achievements" },
  { label: "Certifications", href: "#certifications" },
  { label: "Projects", href: "#projects" },
  { label: "CTF", href: "#ctf" },
  { label: "Skills", href: "#skills" },
  { label: "Arcade", href: "/arcade", target: "_self" },
  { label: "CV", href: "/media/CV.pdf", target: "_blank", rel: "noopener noreferrer" },
  { label: "Contact", href: "#contact" },
];

export function Sidebar({ onArcadeOpen }: { onArcadeOpen?: () => void }) {
  const [isOpen, setIsOpen] = useState(false);

  // Close sidebar on escape key
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") setIsOpen(false);
    };
    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, []);

  return (
    <>
      <button
        type="button"
        className={`sidebar-fab ${isOpen ? "is-active" : ""}`}
        aria-label="Toggle Navigation Sidebar"
        aria-expanded={isOpen}
        onClick={() => setIsOpen((prev) => !prev)}
      >
        <div className="burger-icon" />
      </button>

      <AnimatePresence>
        {isOpen && (
          <>
            <motion.div
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              exit={{ opacity: 0 }}
              transition={{ duration: 0.3 }}
              className="fixed inset-0 z-60 bg-black/40 backdrop-blur-sm"
              onClick={() => setIsOpen(false)}
            />
            <motion.aside
              initial={{ x: "120%", opacity: 0, scale: 0.95 }}
              animate={{ x: 0, opacity: 1, scale: 1 }}
              exit={{ x: "120%", opacity: 0, scale: 0.95 }}
              transition={{ type: "spring", damping: 25, stiffness: 200 }}
              className="sidebar-panel is-open"
              role="dialog"
              aria-modal="true"
              aria-label="Navigation Menu"
            >
              <div className="sidebar-content !p-6 flex flex-col h-full bg-[color:var(--surface)]">
                <div>
                  <p className="text-[0.7rem] uppercase tracking-[0.2em] text-[#a1a1aa] mb-6 font-mono opacity-80 pl-2">Navigation Matrix</p>
                  <ul className="flex flex-col gap-4 perspective-1000 mt-2 mr-2">
                    {navigationLinks.map((item, i) => (
                      <motion.li 
                        key={item.label}
                        initial={{ opacity: 0, rotateX: -20, y: 15 }}
                        animate={{ opacity: 1, rotateX: 0, y: 0 }}
                        transition={{ delay: 0.1 + i * 0.08, type: "spring", stiffness: 80, damping: 15 }}
                      >
                        <a 
                          href={item.href} 
                          onClick={(e) => {
                            if (item.label === "Arcade" && onArcadeOpen) {
                              e.preventDefault();
                              onArcadeOpen();
                            }
                            setIsOpen(false);
                          }} 
                          target={(item as any).target || "_self"}
                          rel={(item as any).rel || ""}
                          className="nav-smooth-btn group"
                        >
                          <div className="nav-btn-text">
                            <div className="flex items-center gap-3">
                              <span className="nav-icon-sweep font-bold">»</span>
                              <span>{item.label}</span>
                            </div>
                            <span className="opacity-0 group-hover:opacity-100 transition-opacity font-bold text-[0.7rem] text-[color:var(--accent)] tracking-widest hidden sm:block">
                              ACCESS
                            </span>
                          </div>
                        </a>
                      </motion.li>
                    ))}
                  </ul>
                </div>
                
                <div className="mt-auto pt-8 border-t border-[color:var(--line-strong)] px-2">
                  <div className="flex gap-6 font-mono text-[0.85rem]">
                    <a href="https://linkedin.com/in/khalilammarr" target="_blank" rel="noopener noreferrer" className="text-[color:var(--foreground)] hover:text-[color:var(--accent)] transition-colors">LinkedIn</a>
                    <a href="https://github.com/khalilammarr" target="_blank" rel="noopener noreferrer" className="text-[color:var(--foreground)] hover:text-[color:var(--accent)] transition-colors flex items-center gap-2">
                      <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 24 24">
                        <path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z" />
                      </svg>
                      GitHub
                    </a>
                  </div>
                </div>
              </div>
            </motion.aside>
          </>
        )}
      </AnimatePresence>
    </>
  );
}
