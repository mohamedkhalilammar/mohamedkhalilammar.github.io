"use client";

import { useEffect, useMemo, useRef, useState } from "react";
import { useRouter } from "next/navigation";
import { AnimatePresence, motion } from "framer-motion";
import { ctfWriteups, profile, projects } from "@/data/portfolio";

type PaletteItem = {
  id: string;
  label: string;
  group: "Navigate" | "Projects" | "Writeups" | "Actions";
  hint: string;
  keywords?: string;
  action: () => void;
};

type CommandPaletteProps = {
  isOpen: boolean;
  onOpen: () => void;
  onClose: () => void;
  onArcade: () => void;
  onBreach: () => void;
};

const SECTIONS: Array<[string, string]> = [
  ["top", "Home"],
  ["projects", "Projects"],
  ["achievements", "Achievements"],
  ["ctf", "Writeups & Breakdowns"],
  ["certifications", "Certifications"],
  ["skills", "Core Skills"],
  ["contact", "Contact"],
];

const toSlug = (name: string) =>
  name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, "");

const GROUP_ICONS: Record<PaletteItem["group"], React.ReactNode> = {
  Navigate: (
    <svg viewBox="0 0 24 24" className="w-4 h-4" fill="none" stroke="currentColor" strokeWidth="1.75">
      <path strokeLinecap="round" d="M10 4L8 20M16 4l-2 16M4 9h16M3.5 15h16" />
    </svg>
  ),
  Projects: (
    <svg viewBox="0 0 24 24" className="w-4 h-4" fill="none" stroke="currentColor" strokeWidth="1.75">
      <path strokeLinecap="round" strokeLinejoin="round" d="M3 7a2 2 0 012-2h4l2 2h8a2 2 0 012 2v8a2 2 0 01-2 2H5a2 2 0 01-2-2V7z" />
    </svg>
  ),
  Writeups: (
    <svg viewBox="0 0 24 24" className="w-4 h-4" fill="none" stroke="currentColor" strokeWidth="1.75">
      <path strokeLinecap="round" strokeLinejoin="round" d="M7 3h7l5 5v13a1 1 0 01-1 1H7a1 1 0 01-1-1V4a1 1 0 011-1zM14 3v5h5M9 13h6M9 17h6" />
    </svg>
  ),
  Actions: (
    <svg viewBox="0 0 24 24" className="w-4 h-4" fill="none" stroke="currentColor" strokeWidth="1.75">
      <path strokeLinecap="round" strokeLinejoin="round" d="M13 2L4 14h6l-1 8 9-12h-6l1-8z" />
    </svg>
  ),
};

/**
 * CommandPalette — Ctrl/Cmd+K launcher with fuzzy filtering across sections,
 * projects, writeups, and actions (arcade, breach mode, socials).
 */
export function CommandPalette({ isOpen, onOpen, onClose, onArcade, onBreach }: CommandPaletteProps) {
  const router = useRouter();
  const [query, setQuery] = useState("");
  const [selected, setSelected] = useState(0);
  const inputRef = useRef<HTMLInputElement>(null);
  const listRef = useRef<HTMLUListElement>(null);

  const items = useMemo<PaletteItem[]>(() => {
    const scrollTo = (id: string) => () => {
      document.getElementById(id)?.scrollIntoView({ behavior: "smooth" });
    };

    const nav: PaletteItem[] = SECTIONS.map(([id, label]) => ({
      id: `nav-${id}`,
      label,
      group: "Navigate",
      hint: "Section",
      action: scrollTo(id),
    }));

    const proj: PaletteItem[] = projects.map((p) => ({
      id: `proj-${toSlug(p.name)}`,
      label: p.name,
      group: "Projects",
      hint: "Case Study",
      keywords: p.stack.join(" "),
      action: () => router.push(`/project/${toSlug(p.name)}`),
    }));

    const writeups: PaletteItem[] = ctfWriteups.map((w) => ({
      id: `wu-${w.id}`,
      label: w.title,
      group: "Writeups",
      hint: "Writeup",
      action: () => router.push(`/writeups/${w.id}`),
    }));

    const actions: PaletteItem[] = [
      { id: "act-arcade", label: "Open the Arcade", group: "Actions", hint: "Play", keywords: "game snake tetris pong", action: onArcade },
      { id: "act-breach", label: "Initiate Breach Protocol", group: "Actions", hint: "???", keywords: "matrix rain hack easter egg", action: onBreach },
      {
        id: "act-email",
        label: "Copy Email Address",
        group: "Actions",
        hint: "Clipboard",
        keywords: "contact mail",
        action: () => {
          if (profile.socials?.email) void navigator.clipboard?.writeText(profile.socials.email);
        },
      },
      {
        id: "act-github",
        label: "Open GitHub Profile",
        group: "Actions",
        hint: "External",
        keywords: "code source repo",
        action: () => window.open(profile.socials?.github, "_blank", "noopener,noreferrer"),
      },
      {
        id: "act-linkedin",
        label: "Open LinkedIn Profile",
        group: "Actions",
        hint: "External",
        keywords: "connect network",
        action: () => window.open(profile.socials?.linkedin, "_blank", "noopener,noreferrer"),
      },
    ];

    return [...nav, ...proj, ...writeups, ...actions];
  }, [router, onArcade, onBreach]);

  const filtered = useMemo(() => {
    const q = query.trim().toLowerCase();
    if (!q) return items;
    return items.filter((item) =>
      `${item.label} ${item.group} ${item.keywords ?? ""}`.toLowerCase().includes(q)
    );
  }, [items, query]);

  // Global Ctrl/Cmd+K toggle
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "k") {
        e.preventDefault();
        if (isOpen) onClose();
        else onOpen();
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [isOpen, onOpen, onClose]);

  // Reset + focus on open
  useEffect(() => {
    if (!isOpen) return;
    setQuery("");
    setSelected(0);
    const t = setTimeout(() => inputRef.current?.focus(), 30);
    return () => clearTimeout(t);
  }, [isOpen]);

  useEffect(() => setSelected(0), [query]);

  useEffect(() => {
    listRef.current
      ?.querySelector(`[data-index="${selected}"]`)
      ?.scrollIntoView({ block: "nearest" });
  }, [selected]);

  const run = (item: PaletteItem) => {
    onClose();
    item.action();
  };

  const onInputKey = (e: React.KeyboardEvent) => {
    if (e.key === "Escape") { onClose(); return; }
    if (e.key === "ArrowDown") {
      e.preventDefault();
      setSelected((s) => Math.min(s + 1, filtered.length - 1));
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      setSelected((s) => Math.max(s - 1, 0));
    } else if (e.key === "Enter" && filtered[selected]) {
      e.preventDefault();
      run(filtered[selected]);
    }
  };

  let lastGroup = "";

  return (
    <AnimatePresence>
      {isOpen && (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          transition={{ duration: 0.18 }}
          className="fixed inset-0 z-[10000] bg-black/60 backdrop-blur-sm flex items-start justify-center px-4"
          onMouseDown={onClose}
          role="dialog"
          aria-modal="true"
          aria-label="Command palette"
        >
          <motion.div
            initial={{ opacity: 0, y: -14, scale: 0.98 }}
            animate={{ opacity: 1, y: 0, scale: 1 }}
            exit={{ opacity: 0, y: -10, scale: 0.98 }}
            transition={{ duration: 0.22, ease: [0.16, 1, 0.3, 1] }}
            className="w-full max-w-xl mt-[12vh] rounded-2xl border border-primary-500/25 bg-[#0c0f1f]/95 backdrop-blur-xl shadow-[0_24px_70px_-20px_rgba(0,0,0,0.8),0_0_50px_-20px_rgba(129,140,248,0.45)] overflow-hidden"
            onMouseDown={(e) => e.stopPropagation()}
          >
            <div className="flex items-center gap-3 px-5 py-4 border-b border-primary-500/15">
              <span className="text-primary-400 font-mono text-sm select-none" aria-hidden>&gt;_</span>
              <input
                ref={inputRef}
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                onKeyDown={onInputKey}
                placeholder="Type a command or search…"
                aria-label="Search commands"
                className="flex-1 bg-transparent outline-none text-sm font-mono text-primary-100 placeholder-zinc-600"
              />
              <kbd className="hidden sm:inline-block text-[10px] font-mono text-zinc-500 border border-white/10 rounded px-1.5 py-0.5">ESC</kbd>
            </div>

            <ul ref={listRef} role="listbox" aria-label="Commands" className="max-h-[46vh] overflow-y-auto py-2 px-2">
              {filtered.length === 0 && (
                <li className="px-4 py-6 text-center font-mono text-xs text-zinc-500 uppercase tracking-widest">
                  No matches // try &quot;projects&quot;
                </li>
              )}
              {filtered.map((item, i) => {
                const showGroup = item.group !== lastGroup;
                lastGroup = item.group;
                return (
                  <li key={item.id}>
                    {showGroup && (
                      <p className="px-3 pt-3 pb-1.5 text-[10px] font-mono uppercase tracking-[0.25em] text-primary-400/70 select-none">
                        {item.group}
                      </p>
                    )}
                    <button
                      type="button"
                      role="option"
                      aria-selected={i === selected}
                      data-index={i}
                      onMouseEnter={() => setSelected(i)}
                      onClick={() => run(item)}
                      className={`w-full flex items-center gap-3 px-3 py-2.5 rounded-lg text-left cursor-pointer transition-colors duration-150 ${
                        i === selected
                          ? "bg-primary-500/15 text-primary-100"
                          : "text-zinc-400 hover:text-zinc-200"
                      }`}
                    >
                      <span className={i === selected ? "text-primary-300" : "text-zinc-600"}>
                        {GROUP_ICONS[item.group]}
                      </span>
                      <span className="flex-1 text-sm truncate">{item.label}</span>
                      <span className="text-[10px] font-mono uppercase tracking-wider text-zinc-600">{item.hint}</span>
                      {i === selected && (
                        <kbd className="text-[10px] font-mono text-primary-400 border border-primary-500/30 rounded px-1.5 py-0.5">↵</kbd>
                      )}
                    </button>
                  </li>
                );
              })}
            </ul>

            <div className="flex items-center gap-4 px-5 py-2.5 border-t border-primary-500/15 text-[10px] font-mono text-zinc-600 uppercase tracking-wider">
              <span><kbd className="text-zinc-500">↑↓</kbd> navigate</span>
              <span><kbd className="text-zinc-500">↵</kbd> select</span>
              <span className="ml-auto text-primary-500/60">{filtered.length} results</span>
            </div>
          </motion.div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
