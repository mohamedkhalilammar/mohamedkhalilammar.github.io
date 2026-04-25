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

export function Sidebar() {
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
                          onClick={() => setIsOpen(false)} 
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
                    <a href="https://github.com/khalilammarr" target="_blank" rel="noopener noreferrer" className="text-[color:var(--foreground)] hover:text-[color:var(--accent)] transition-colors">GitHub</a>
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
