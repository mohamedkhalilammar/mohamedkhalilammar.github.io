"use client";

import { motion, AnimatePresence } from "framer-motion";
import { useEffect } from "react";
import { projects } from "@/data/portfolio";

/**
 * EnhancedSkillModal — replaces the existing skill modal at the bottom of page.tsx.
 *
 * Your skill items should have:
 * {
 *   name: string
 *   description: string
 *   use: string
 *   group: string
 *   proficiency?: number   // 0–100, optional (defaults to guessing from use length)
 *   relatedProjects?: string[]  // project names/titles (optional)
 * }
 */

interface Skill {
  name: string;
  description: string;
  use: string;
  group: string;
  proficiency?: number;
  relatedProjects?: string[];
}

interface Props {
  skill: Skill;
  isOpen: boolean;
  onClose: () => void;
  modalRef: React.RefObject<HTMLDivElement>;
}

// Derive a rough proficiency level label from 0-100
function proficiencyLabel(n: number) {
  if (n >= 90) return "Expert";
  if (n >= 70) return "Advanced";
  if (n >= 50) return "Intermediate";
  if (n >= 30) return "Familiar";
  return "Learning";
}

// Find related projects by matching skill name in project descriptions/tags
function findRelatedProjects(skillName: string) {
  if (!projects) return [];
  return projects
    .filter((p: any) => {
      const haystack = [p.name, p.summary, ...(p.stack ?? [])]
        .join(" ")
        .toLowerCase();
      return haystack.includes(skillName.toLowerCase());
    })
    .slice(0, 3);
}

export function EnhancedSkillModal({ skill, isOpen, onClose, modalRef }: Props) {
  // Proficiency: use explicit value or derive a placeholder
  const proficiency = skill.proficiency ?? Math.min(95, 50 + skill.use.length / 3);
  const relatedProjects = skill.relatedProjects
    ? projects?.filter((p: any) => skill.relatedProjects!.includes(p.name)).slice(0, 3)
    : findRelatedProjects(skill.name);

  // Focus trap
  useEffect(() => {
    if (!isOpen || !modalRef.current) return;
    const dialog = modalRef.current;
    dialog.focus();

    const focusableSelectors = [
      "a[href]",
      "button:not([disabled])",
      "[tabindex]:not([tabindex='-1'])",
    ].join(",");

    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") { onClose(); return; }
      if (e.key !== "Tab") return;
      const els = Array.from(dialog.querySelectorAll<HTMLElement>(focusableSelectors));
      if (!els.length) { e.preventDefault(); return; }
      const first = els[0], last = els[els.length - 1];
      const active = document.activeElement as HTMLElement;
      if (!e.shiftKey && active === last) { e.preventDefault(); first.focus(); }
      else if (e.shiftKey && active === first) { e.preventDefault(); last.focus(); }
    };

    document.addEventListener("keydown", handleKeyDown);
    return () => document.removeEventListener("keydown", handleKeyDown);
  }, [isOpen, onClose, modalRef]);

  return (
    <AnimatePresence>
      {isOpen && (
        <div
          className="custom-modal-overlay"
          onClick={onClose}
          style={{ zIndex: 1000 }}
        >
          <motion.div
            className="custom-modal-content"
            role="dialog"
            aria-modal="true"
            aria-labelledby="skill-modal-title"
            aria-describedby="skill-modal-description"
            tabIndex={-1}
            ref={modalRef}
            onClick={(e) => e.stopPropagation()}
            initial={{ opacity: 0, scale: 0.95, y: 12 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: 12 }}
            transition={{ duration: 0.22, ease: [0.16, 1, 0.3, 1] }}
            style={{ maxWidth: 480, width: "92vw" }}
          >
            {/* Header */}
            <div style={{ display: "flex", justifyContent: "space-between", alignItems: "flex-start", marginBottom: "1rem" }}>
              <div>
                <p className="achievement-highlight" style={{ margin: 0, fontSize: "0.65rem" }}>
                  {skill.group}
                </p>
                <h3
                  id="skill-modal-title"
                  className="font-sans"
                  style={{ margin: "0.25rem 0 0", color: "#fff", fontSize: "1.25rem", fontWeight: 700 }}
                >
                  {skill.name}
                </h3>
              </div>
              <button
                type="button"
                onClick={onClose}
                className="btn-secondary"
                style={{ padding: "0.3rem 0.7rem", fontSize: "0.75rem", flexShrink: 0, marginLeft: "1rem" }}
                aria-label="Close modal"
              >
                ✕
              </button>
            </div>

            {/* Proficiency bar */}
            <div style={{ marginBottom: "1.25rem" }}>
              <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: "0.4rem" }}>
                <span style={{ fontFamily: "monospace", fontSize: "0.7rem", textTransform: "uppercase", letterSpacing: "0.12em", color: "rgba(200,230,240,0.5)" }}>
                  Proficiency
                </span>
                <span style={{ fontFamily: "monospace", fontSize: "0.75rem", color: "var(--accent, #f59e0b)", fontWeight: 700 }}>
                  {proficiencyLabel(proficiency)}
                </span>
              </div>
              <div style={{ height: "4px", background: "rgba(255,255,255,0.07)", borderRadius: "999px", overflow: "hidden" }}>
                <motion.div
                  initial={{ width: 0 }}
                  animate={{ width: `${proficiency}%` }}
                  transition={{ duration: 0.8, ease: [0.16, 1, 0.3, 1], delay: 0.1 }}
                  style={{
                    height: "100%",
                    background: "linear-gradient(90deg, rgba(245,158,11,0.7), rgba(245,158,11,1))",
                    borderRadius: "999px",
                  }}
                />
              </div>
            </div>

            {/* Description */}
            <p
              id="skill-modal-description"
              style={{ marginBottom: "0.75rem", color: "rgba(200,230,240,0.7)", fontSize: "0.88rem", lineHeight: 1.6 }}
            >
              <strong style={{ color: "var(--accent, #f59e0b)" }}>What: </strong>
              {skill.description}
            </p>
            <p style={{ marginBottom: "1.25rem", color: "rgba(200,230,240,0.7)", fontSize: "0.88rem", lineHeight: 1.6 }}>
              <strong style={{ color: "var(--accent, #f59e0b)" }}>How I use it: </strong>
              {skill.use}
            </p>

            {/* Related projects */}
            {relatedProjects.length > 0 && (
              <div>
                <p style={{ fontFamily: "monospace", fontSize: "0.65rem", textTransform: "uppercase", letterSpacing: "0.12em", color: "rgba(200,230,240,0.4)", marginBottom: "0.5rem" }}>
                  Used in
                </p>
                <div style={{ display: "flex", flexWrap: "wrap", gap: "0.4rem" }}>
                  {relatedProjects.map((proj: any) => (
                    <span
                      key={proj.name}
                      style={{
                        padding: "0.25rem 0.65rem",
                        borderRadius: "999px",
                        border: "1px solid rgba(245,158,11,0.25)",
                        background: "rgba(245,158,11,0.07)",
                        fontFamily: "monospace",
                        fontSize: "0.75rem",
                        color: "rgba(245,158,11,0.9)",
                      }}
                    >
                      {proj.name}
                    </span>
                  ))}
                </div>
              </div>
            )}
          </motion.div>
        </div>
      )}
    </AnimatePresence>
  );
}
