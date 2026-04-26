"use client";

import { MotionConfig, motion, useReducedMotion, useScroll, useTransform, AnimatePresence } from "framer-motion";
import { useEffect, useMemo, useRef, useState } from "react";
import Link from "next/link";
import Image from "next/image";
import { SectionShell } from "@/components/sections/section-shell";
import { CyberPulseScene } from "@/components/three/cyber-pulse-scene";
import { LiveTerminal } from "@/components/ui/live-terminal";
import { Sidebar } from "@/components/ui/sidebar";
import { ProjectCarousel3D } from "@/components/ui/project-carousel-3d";
import { WriteupGrid3D } from "@/components/ui/writeup-grid-3d";
import { CreativeEntrance4D } from "@/components/ui/entrance-4d";
import { ActiveNav } from "@/components/ui/active-nav";
import { AchievementPhotoStrip } from "@/components/ui/achievement-photo-strip";
import { ParallaxPhotoColumn } from "@/components/ui/parallax-photo-column";
import { ScrollToTop } from "@/components/ui/scroll-to-top";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";
import { EnhancedSkillModal } from "@/components/ui/enhanced-skill-modal";
import { achievements, certifications, ctfWriteups, ctfIdentity, learningPath, profile, projects, skillGroups } from "@/data/portfolio";

const navigationLinks = [
  { label: "About", href: "#about" },
  { label: "Projects", href: "#projects" },
  { label: "Achievements", href: "#achievements" },
  { label: "Writeups", href: "#writeups" },
  { label: "Certifications", href: "#certifications" },
  { label: "Skills", href: "#skills" },
  { label: "Arcade", href: "/arcade" },
  { label: "Contact", href: "#contact" },
];

const ACHIEVEMENT_PHOTOS = [
  "/media/team.jpeg",
  "/media/winners.jpeg",
  "/media/dup.jpeg",
  "/media/finals.jpg",
  "/media/scoreboard.jpeg",
  "/media/cybercampphoto.jpg",
  "/media/teamm.jpeg"
];

// Refined reveal config — tighter, more intentional
const makeReveal = (reducedMotion: boolean) => ({
  initial: reducedMotion ? { opacity: 1, y: 0 } : { opacity: 0, y: 36, filter: "blur(8px)" },
  whileInView: { opacity: 1, y: 0, filter: "blur(0px)" },
  transition: reducedMotion ? { duration: 0 } : { duration: 1.0, ease: [0.16, 1, 0.3, 1], delay: 0.08 },
  viewport: { once: true, amount: 0.12 },
});

export default function Home() {
  const reducedMotion = useReducedMotion();
  const heroRef = useRef<HTMLElement | null>(null);
  const modalRef = useRef<HTMLDivElement | null>(null);
  const modalTriggerRef = useRef<HTMLElement | null>(null);
  const { scrollYProgress } = useScroll({ target: heroRef, offset: ["start start", "end start"] });

  const allSkills = useMemo(
    () => skillGroups.flatMap((group) => group.items.map((item) => ({ ...item, group: group.title }))),
    []
  );
  const [selectedSkillName, setSelectedSkillName] = useState(allSkills[0]?.name ?? "");
  const [roleIdx, setRoleIdx] = useState(0);
  const roles = ["CTF Player", "Cyber Security Enthusiast", "Engineering Student"];

  useEffect(() => {
    const interval = setInterval(() => setRoleIdx(p => (p + 1) % roles.length), 3200);
    return () => clearInterval(interval);
  }, []);

  const selectedSkill = allSkills.find((s) => s.name === selectedSkillName) ?? allSkills[0];
  const [isSkillAlertVisible, setIsSkillAlertVisible] = useState(false);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [formSent, setFormSent] = useState(false);
  const { scrollY } = useScroll();
  const [showScrollTop, setShowScrollTop] = useState(false);

  useEffect(() => scrollY.onChange(v => setShowScrollTop(v > 1000)), [scrollY]);

  useEffect(() => {
    if (!isSkillAlertVisible || !modalRef.current) return;
    const dialog = modalRef.current;
    dialog.focus();
    const focusable = [
      "a[href]", "button:not([disabled])", "textarea:not([disabled])",
      "input:not([disabled])", "select:not([disabled])", "[tabindex]:not([tabindex='-1'])",
    ].join(",");

    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") { setIsSkillAlertVisible(false); return; }
      if (e.key !== "Tab") return;
      const els = Array.from(dialog.querySelectorAll<HTMLElement>(focusable));
      if (!els.length) { e.preventDefault(); return; }
      const active = document.activeElement as HTMLElement | null;
      if (!e.shiftKey && active === els[els.length - 1]) { e.preventDefault(); els[0].focus(); }
      else if (e.shiftKey && active === els[0]) { e.preventDefault(); els[els.length - 1].focus(); }
    };
    document.addEventListener("keydown", onKey);
    return () => { document.removeEventListener("keydown", onKey); modalTriggerRef.current?.focus(); };
  }, [isSkillAlertVisible]);

  const reveal = makeReveal(!!reducedMotion);

  return (
    <MotionConfig reducedMotion="user">
      <div className="page-shell">
        <Sidebar />
        <div className="page-noise" aria-hidden />

        {/* ── Header ── */}
        <header className="top-nav" style={{ backdropFilter: "blur(20px)", borderBottom: "1px solid rgba(255,255,255,0.05)" }}>
          <p className="brand-mark" style={{ letterSpacing: "0.05em", fontWeight: 700 }}>{profile.name}</p>
          <ActiveNav />
        </header>

        <main>
          {/* ===== CINEMATIC ENTRANCE ===== */}
          <CreativeEntrance4D />

          {/* ===== HERO ===== */}
          <section
            className="hero-section relative min-h-[90vh] flex items-center overflow-hidden"
            id="top"
            ref={heroRef}
          >
            {/* Atmosphere */}
            <div className="absolute inset-0 z-0 pointer-events-none">
              <div className="absolute top-1/2 -right-20 -translate-y-1/2 w-[600px] h-full opacity-25">
                <CyberPulseScene />
              </div>
            </div>

            {/* ── BOLD TACTICAL DIVIDER (Synchronized with SectionShell) ── */}
            <div className="absolute top-0 left-0 w-full h-px bg-white/10" />
            <div className="absolute top-0 left-0 w-32 h-[3px] bg-white/20" />

            <div className="hero-content reveal-stagger w-full max-w-7xl mx-auto px-6 relative z-10 py-20 mt-12">
              {/* Background Index Watermark - Hero 00 */}
              <div className="absolute top-10 left-6 font-sans text-[12rem] md:text-[20rem] font-black text-white/[0.03] leading-none select-none pointer-events-none tracking-tighter">
                00
              </div>
              
              <div className="grid grid-cols-1 lg:grid-cols-12 gap-8 lg:gap-12 items-center w-full relative z-10">

                {/* ── Text column ── */}
                <div className="w-full flex-col css-stagger-item text-left lg:col-span-8">

                  {/* Title Area */}
                  <div className="relative mb-12 pb-4 pl-1 mt-10">
                    <h1 className="font-sans text-5xl md:text-9xl lg:text-[10rem] font-black uppercase tracking-tighter leading-none mb-4 text-white">
                      MEET KHALIL
                    </h1>
                    <div className="h-[2px] w-24 bg-gradient-to-r from-amber-500 to-orange-600" />
                  </div>

                  {/* Role rotator */}
                  <h2 className="text-lg md:text-xl font-mono text-zinc-400 mb-10 uppercase tracking-[0.2em] pb-4 inline-flex items-center gap-3 min-h-[1.5em]">
                    <span className="w-1.5 h-1.5 rounded-full bg-amber-500 animate-pulse flex-shrink-0" />
                    <AnimatePresence mode="wait">
                      <motion.span
                        key={roles[roleIdx]}
                        initial={{ opacity: 0, y: 8, filter: "blur(4px)" }}
                        animate={{ opacity: 1, y: 0, filter: "blur(0px)" }}
                        exit={{ opacity: 0, y: -8, filter: "blur(4px)" }}
                        transition={{ duration: 0.45, ease: "easeOut" }}
                      >
                        {roles[roleIdx]}
                      </motion.span>
                    </AnimatePresence>
                  </h2>

                  {/* Terminal block */}
                  <motion.div
                    initial={{ opacity: 0, scale: 0.985 }}
                    animate={{ opacity: 1, scale: 1 }}
                    transition={{ duration: 0.9, delay: 0.25 }}
                    className="w-full mb-12 rounded-2xl overflow-hidden bg-[#08080a]/90 backdrop-blur-2xl border border-white/[0.07] shadow-[0_0_60px_rgba(0,0,0,0.6),inset_0_1px_0_rgba(255,255,255,0.04)]"
                  >
                    {/* Traffic lights */}
                    <div className="flex items-center gap-2 px-5 py-3.5 bg-white/[0.02] border-b border-white/[0.04]">
                      <div className="w-3 h-3 rounded-full bg-red-500/80" />
                      <div className="w-3 h-3 rounded-full bg-yellow-400/80" />
                      <div className="w-3 h-3 rounded-full bg-green-500/80" />
                      <span className="ml-3 font-mono text-[10px] text-zinc-600 tracking-widest uppercase">khalil.sh — zsh</span>
                    </div>

                    <div className="p-7 md:p-9 font-mono text-[14px] md:text-[15px] leading-[1.9] text-zinc-100">
                      {profile.about.split('\n\n').map((paragraph, index) => (
                        <motion.p
                          key={index}
                          initial={{ opacity: 0, filter: "blur(3px)", y: 4 }}
                          animate={{ opacity: 1, filter: "blur(0px)", y: 0 }}
                          transition={{ duration: 0.5, delay: 0.45 + index * 0.18 }}
                          className="mb-6 last:mb-0 flex items-start"
                        >
                          <span className="text-zinc-700 mr-5 select-none font-bold w-5 text-right flex-shrink-0 text-[11px] mt-1.5">
                            {String(index + 1).padStart(2, "0")}
                          </span>
                          <span className="flex-1 text-zinc-200/90 leading-[1.85]">{paragraph}</span>
                        </motion.p>
                      ))}

                      {/* Quotes */}
                      <motion.div
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        transition={{ duration: 0.6, delay: 1.4 }}
                        className="mt-8 pt-6 border-t border-white/[0.06] space-y-5"
                      >
                        {[
                          {
                            text: <>AI is not a substitute for human intelligence — it's a{" "}<span className="text-amber-400/90 not-italic font-medium">tool to amplify it.</span></>,
                            cite: "Dr. Fei-Fei Li, Stanford",
                          },
                        ].map(({ text, cite }, i) => (
                          <blockquote key={i} className="group pl-4 transition-colors duration-500">
                            <p className="text-[13px] text-zinc-400 italic leading-relaxed group-hover:text-zinc-300 transition-colors duration-400">
                              "{text}"
                            </p>
                            <cite className="block not-italic font-mono text-[9px] uppercase tracking-[0.25em] text-zinc-600 mt-1.5">
                              — {cite}
                            </cite>
                          </blockquote>
                        ))}
                      </motion.div>

                      {/* Blinking cursor */}
                      <motion.span
                        animate={{ opacity: [0, 1, 0] }}
                        transition={{ repeat: Infinity, duration: 1.1, ease: "linear", delay: 2 }}
                        className="inline-block w-2.5 h-[1.1em] bg-amber-500 ml-1 mt-3 align-middle shadow-[0_0_10px_rgba(245,158,11,0.5)]"
                      />
                    </div>
                  </motion.div>

                  <div className="flex flex-wrap items-center gap-4">
                    <a className="btn-primary" href="#projects">{profile.cta.primary}</a>
                    <a className="btn-secondary" href="#contact">{profile.cta.secondary}</a>
                  </div>
                </div>

                {/* ── Photo column ── */}
                <div className="w-full flex justify-center lg:justify-end css-stagger-item relative lg:col-span-4">
                  <ParallaxPhotoColumn scrollYProgress={scrollYProgress} />
                </div>
              </div>
            </div>
          </section>

          {/* ===== PROJECTS ===== */}
          <motion.div {...(reveal as any)} className="section-flow">
            <SectionShell id="projects" eyebrow="Work" title="" index="01">
              <div className="mb-12">
                <h2 className="font-sans text-5xl md:text-7xl font-black uppercase tracking-tighter mb-3 leading-none text-white">
                  Featured Projects
                </h2>
                <div className="h-[2px] w-20 bg-gradient-to-r from-amber-500 to-orange-600" />
              </div>
              <ProjectCarousel3D projects={projects} />
            </SectionShell>
          </motion.div>

          {/* ===== ACHIEVEMENTS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative">
            <SectionShell id="achievements" eyebrow="Milestones" title="Key Achievements" index="02">
              <div className="grid grid-cols-1 lg:grid-cols-12 gap-12 lg:gap-8 items-start">
                <AchievementPhotoStrip photos={ACHIEVEMENT_PHOTOS} />
                <div className="lg:col-span-4 hidden lg:flex flex-col gap-10 mt-2">
                  {ACHIEVEMENT_PHOTOS.map((src, idx) => (
                    <motion.div key={idx} className="relative w-full rounded-xl overflow-hidden border border-primary-500/25 shadow-lg group aspect-video">
                      <div className="absolute inset-0 bg-gradient-to-t from-black/50 via-transparent to-transparent z-10 pointer-events-none" />
                      <img src={src} alt="Achievement" className="w-full h-full object-cover group-hover:scale-[1.04] transition-transform duration-700" />
                    </motion.div>
                  ))}
                </div>
                <div className="lg:col-span-8 relative border-l border-primary-500/25 ml-4 md:ml-8 pl-8 md:pl-12 space-y-10">
                  {achievements.map((achievement, idx) => (
                    <div key={idx} className="relative group p-6 rounded-xl border border-white/5 bg-white/[0.02]">
                      <div className="absolute w-3 h-3 rounded-full bg-amber-500 -left-[2.35rem] md:-left-[3.35rem] top-8" />
                      <span className="text-[10px] font-mono text-amber-500 uppercase tracking-widest mb-4 block">{achievement.highlight}</span>
                      <h3 className="text-xl font-bold text-white mb-2">{achievement.title}</h3>
                      <p className="text-zinc-400 text-sm">{achievement.detail}</p>
                    </div>
                  ))}
                </div>
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== WRITEUPS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative">
            <SectionShell id="writeups" eyebrow="Knowledge" title="Writeups & Breakdowns" index="03">
              <p className="text-zinc-400 text-base leading-relaxed mb-10 max-w-2xl">
                I design and solve CTF challenges across reverse engineering, mobile security, and exploitation. 
                Browse selected technical notes and systemic challenge breakdowns.
              </p>
              <WriteupGrid3D writeups={ctfWriteups} />
            </SectionShell>
          </motion.div>

          {/* ===== CERTIFICATIONS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative">
            <SectionShell id="certifications" eyebrow="Validation" title="Certifications" index="04">
              <p className="text-zinc-400 text-base leading-relaxed mb-10 max-w-2xl">
                Verified training and formal credentials that back the practical offensive-security work shown in this portfolio.
              </p>

              <div className="grid grid-cols-1 lg:grid-cols-3 gap-5 mb-6">
                {certifications.map((cert, idx) => (
                  <article
                    key={cert.name}
                    className="group bg-black/50 border border-primary-500/15 rounded-xl p-6 backdrop-blur-sm hover:border-primary-500/45 transition-all duration-300 hover:bg-black/60"
                    style={{ animationDelay: `${idx * 0.08}s` }}
                  >
                    <div className="flex items-start justify-between mb-4 gap-3">
                      <p className="text-[9px] font-mono uppercase tracking-[0.25em] text-amber-500 mt-0.5">
                        {cert.issuer}
                      </p>
                      {cert.logo && (
                        <img src={cert.logo} alt={cert.issuer} className="h-8 object-contain rounded bg-white/5 p-1" />
                      )}
                    </div>
                    <h3 className="text-lg font-orbitron font-bold text-white mb-3 leading-snug group-hover:text-primary-300 transition-colors">
                      {cert.name}
                    </h3>
                    <p className="text-zinc-500 text-[13px] leading-relaxed mb-5">
                      {cert.description}
                    </p>
                    <a
                      href={cert.verifyUrl || cert.localFile}
                      target="_blank"
                      rel="noopener noreferrer"
                      className="inline-flex items-center gap-1.5 text-[11px] font-mono uppercase tracking-wider text-primary-400 hover:text-white transition-colors border-b border-primary-500/20 hover:border-primary-400 pb-0.5"
                    >
                      View Certificate <span>→</span>
                    </a>
                  </article>
                ))}
              </div>

              <article className="bg-gradient-to-br from-primary-950/15 to-black/50 border border-primary-500/20 rounded-xl p-7 backdrop-blur-sm hover:border-primary-500/40 transition-all duration-300">
                <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-5 mb-5">
                  <div className="flex items-center gap-5">
                    {learningPath.logo && (
                      <img src={learningPath.logo} alt={learningPath.provider} className="h-14 object-contain rounded bg-white/5 p-1.5" />
                    )}
                    <div>
                      <p className="text-[9px] font-mono uppercase tracking-[0.25em] text-amber-500 mb-2">Learning Path</p>
                      <h3 className="text-xl font-orbitron font-bold text-white">{learningPath.name}</h3>
                      <p className="text-zinc-500 text-sm mt-1">{learningPath.provider} · {learningPath.status} · {learningPath.progress}</p>
                    </div>
                  </div>
                  <a
                    href={learningPath.localFile}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="inline-flex items-center gap-2 px-4 py-2.5 border border-primary-500/35 text-primary-300 hover:text-white hover:border-primary-400 rounded-lg text-[11px] font-mono uppercase tracking-wider transition-all whitespace-nowrap"
                  >
                    Open Proof →
                  </a>
                </div>
                <p className="text-primary-100/75 mb-5 leading-relaxed text-sm">{learningPath.summary}</p>
                <div className="flex flex-wrap gap-2">
                  {learningPath.modules.map((module) => (
                    <span
                      key={module}
                      className="px-3 py-1 rounded-full border border-primary-500/30 text-[11px] font-mono uppercase tracking-wider text-primary-200/80 bg-primary-950/15"
                    >
                      {module}
                    </span>
                  ))}
                </div>
              </article>
            </SectionShell>
          </motion.div>

          {/* ===== SKILLS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative overflow-hidden rounded-3xl border border-primary-800/20">
            <SectionShell id="skills" eyebrow="Tooling" title="Core Skills" index="05">
              <div className="skills-grid relative z-10">
                {skillGroups.map((group, groupIdx) => (
                  <article
                    key={group.title}
                    className="skills-card bg-black/55 backdrop-blur-md border border-primary-500/15 hover:border-primary-500/50 transition-all duration-400 p-7 rounded-xl relative group overflow-hidden"
                    style={{ animation: `skillCardEntry 0.5s ease-out ${groupIdx * 0.07}s both` }}
                  >
                    <div className="absolute inset-0 rounded-xl opacity-0 group-hover:opacity-100 transition-opacity duration-400 bg-gradient-to-br from-primary-500/4 to-purple-500/4 pointer-events-none" />
                    <h3 className="font-mono text-primary-300/90 font-semibold uppercase tracking-[0.15em] text-[11px] mb-4 relative z-10">
                      {group.title}
                    </h3>
                    <div className="mt-2 flex flex-wrap gap-2 relative z-10">
                      {group.items.map((skill, idx) => (
                        <button
                          key={skill.name}
                          type="button"
                          aria-pressed={selectedSkillName === skill.name && isSkillAlertVisible}
                          className={`rounded-full border px-3.5 py-1.5 text-[12px] font-mono transition-all cursor-pointer hover:scale-[1.03] active:scale-95 ${selectedSkillName === skill.name && isSkillAlertVisible
                              ? "bg-primary-500/25 text-primary-200 border-primary-400/80 shadow-[0_0_12px_rgba(var(--primary-rgb),0.4)]"
                              : "bg-transparent text-primary-300/70 border-primary-600/30 hover:border-primary-400/70 hover:text-primary-200"
                            }`}
                          onClick={(event) => {
                            modalTriggerRef.current = event.currentTarget;
                            setSelectedSkillName(skill.name);
                            setIsSkillAlertVisible(true);
                          }}
                        >
                          {skill.name}
                        </button>
                      ))}
                    </div>
                  </article>
                ))}
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== CONTACT ===== */}
          <motion.div {...(reveal as any)} className="section-flow">
            <SectionShell id="contact" eyebrow="Connect" title="Contact" index="06">
              <p className="text-zinc-400 text-base leading-relaxed mb-10 max-w-xl">
                Open to security internships, CTF collaboration, and network engineering opportunities.
              </p>

              <div className="contact-grid">
                {/* Contact cards */}
                <div className="contact-cards-col flex flex-col gap-3.5">
                  {[
                    {
                      href: `mailto:${profile.socials?.email}`,
                      icon: (
                        <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="1.5">
                          <path strokeLinecap="round" strokeLinejoin="round" d="M21.75 6.75v10.5a2.25 2.25 0 01-2.25 2.25h-15a2.25 2.25 0 01-2.25-2.25V6.75m19.5 0A2.25 2.25 0 0019.5 4.5h-15a2.25 2.25 0 00-2.25 2.25m19.5 0v.243a2.25 2.25 0 01-1.07 1.916l-7.5 4.615a2.25 2.25 0 01-2.36 0L3.32 8.91a2.25 2.25 0 01-1.07-1.916V6.75" />
                        </svg>
                      ),
                      label: "Email",
                      value: profile.socials?.email || "khalil.ammar@proton.me",
                    },
                    {
                      href: profile.socials?.linkedin,
                      icon: (
                        <svg className="w-6 h-6" fill="currentColor" viewBox="0 0 24 24">
                          <path d="M19 0h-14c-2.761 0-5 2.239-5 5v14c0 2.761 2.239 5 5 5h14c2.762 0 5-2.239 5-5v-14c0-2.761-2.238-5-5-5zm-11 19h-3v-11h3v11zm-1.5-12.268c-.966 0-1.75-.79-1.75-1.764s.784-1.764 1.75-1.764 1.75.79 1.75 1.764-.783 1.764-1.75 1.764zm13.5 12.268h-3v-5.604c0-3.368-4-3.113-4 0v5.604h-3v-11h3v1.765c1.396-2.586 7-2.777 7 2.476v6.759z" />
                        </svg>
                      ),
                      label: "LinkedIn",
                      value: "Connect Professionally",
                    },
                    {
                      href: profile.socials?.github,
                      icon: (
                        <svg className="w-6 h-6" fill="currentColor" viewBox="0 0 24 24">
                          <path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z" />
                        </svg>
                      ),
                      label: "GitHub",
                      value: "View Repositories",
                    },
                  ].map((contact, idx) => (
                    <a
                      key={contact.label}
                      href={contact.href}
                      target={contact.label !== "Email" ? "_blank" : undefined}
                      rel={contact.label !== "Email" ? "noopener noreferrer" : undefined}
                      className="contact-card group flex items-center gap-5 p-5 border border-primary-500/20 hover:border-primary-400/60 bg-black/35 hover:bg-black/55 rounded-xl transition-all duration-300"
                      style={{ animation: `contactCardIn 0.5s ease-out ${idx * 0.08}s both` }}
                    >
                      <span className="text-amber-500 opacity-70 group-hover:opacity-100 transition-opacity flex-shrink-0">
                        {contact.icon}
                      </span>
                      <div>
                        <p className="text-primary-400 font-mono text-[10px] uppercase tracking-[0.2em] font-bold">{contact.label}</p>
                        <p className="text-zinc-400 text-sm mt-0.5 group-hover:text-zinc-300 transition-colors">{contact.value}</p>
                      </div>
                      <span className="ml-auto text-zinc-600 group-hover:text-amber-500 group-hover:translate-x-1 transition-all duration-300 text-sm">→</span>
                    </a>
                  ))}
                </div>

                {/* Contact form */}
                <form
                  onSubmit={(e) => {
                    e.preventDefault();
                    setIsSubmitting(true);
                    setTimeout(() => { setIsSubmitting(false); setFormSent(true); setTimeout(() => setFormSent(false), 5000); }, 1500);
                  }}
                  className="contact-form-ui flex flex-col gap-5 p-7 bg-gradient-to-br from-primary-950/15 to-black/40 border border-primary-500/20 hover:border-primary-400/40 transition-all duration-300 rounded-xl shadow-xl relative overflow-hidden"
                  style={{ animation: "contactCardIn 0.5s ease-out 0.24s both" }}
                >
                  <AnimatePresence mode="wait">
                    {formSent ? (
                      <motion.div
                        initial={{ opacity: 0, scale: 0.92 }}
                        animate={{ opacity: 1, scale: 1 }}
                        className="flex flex-col items-center justify-center py-12 text-center"
                      >
                        <div className="w-14 h-14 rounded-full bg-green-500/15 border-2 border-green-500/60 flex items-center justify-center mb-6">
                          <span className="text-green-400 text-xl">✓</span>
                        </div>
                        <h3 className="text-xl font-bold text-white mb-2 uppercase tracking-widest font-orbitron">Sent</h3>
                        <p className="text-zinc-500 text-sm max-w-[220px]">I'll get back to you shortly.</p>
                      </motion.div>
                    ) : (
                      <motion.div exit={{ opacity: 0, scale: 0.96 }} className="flex flex-col gap-5">
                        <h3 className="text-base font-bold uppercase text-primary-300 border-b border-primary-500/20 pb-4 font-orbitron tracking-[0.15em]">
                          Send a Message
                        </h3>
                        <input
                          type="text"
                          name="subject"
                          placeholder="Subject"
                          className="w-full p-4 bg-black/50 border border-primary-600/30 hover:border-primary-500/60 focus:border-primary-400/80 rounded-lg text-primary-200 placeholder-zinc-600 outline-none font-mono text-sm transition-all"
                          required
                        />
                        <textarea
                          name="body"
                          placeholder="Your message..."
                          rows={5}
                          className="w-full p-4 bg-black/50 border border-primary-600/30 hover:border-primary-500/60 focus:border-primary-400/80 rounded-lg text-primary-200 placeholder-zinc-600 outline-none font-mono text-sm transition-all resize-none"
                          required
                        />
                        <button
                          type="submit"
                          disabled={isSubmitting}
                          className="mt-1 px-6 py-4 bg-amber-500/8 border-2 border-amber-500/80 hover:bg-amber-500 hover:text-black hover:border-amber-500 text-amber-400 font-mono uppercase tracking-[0.18em] text-sm transition-all duration-300 rounded-lg font-bold flex items-center justify-center gap-2 disabled:opacity-40"
                        >
                          {isSubmitting ? "Sending..." : <>Send Message <span>→</span></>}
                        </button>
                      </motion.div>
                    )}
                  </AnimatePresence>
                </form>
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== ARCADE CTA ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative overflow-hidden mb-32">
             <div className="max-w-[1200px] mx-auto px-6">
                <div className="bg-white/[0.02] border border-white/5 rounded-xl p-8 md:p-12 flex flex-col md:flex-row items-center justify-between gap-8 relative group hover:bg-white/[0.04] transition-colors">
                   <div className="relative z-10">
                      <h2 className="text-4xl font-sans font-black text-white uppercase tracking-tighter mb-2 italic">
                         TAKE A <span className="text-amber-500">BREAK</span>
                      </h2>
                      <p className="text-zinc-500 font-mono text-xs uppercase tracking-widest">
                         Visit the arcade for a quick game.
                      </p>
                   </div>

                   <div className="flex items-center gap-6">
                      <div className="hidden sm:flex gap-3">
                         {["🐍", "🏃", "💣"].map((icon, i) => (
                           <div key={i} className="w-12 h-12 bg-white/5 border border-white/10 rounded-lg flex items-center justify-center text-xl">
                              {icon}
                           </div>
                         ))}
                      </div>
                      <Link href="/arcade" className="btn-primary py-3 px-8 text-xs">
                         PLAY NOW →
                      </Link>
                   </div>
                </div>
             </div>
          </motion.div>
        </main>

        <EnhancedFooter />
        <ScrollToTop />

        {selectedSkill && (
          <EnhancedSkillModal
            skill={selectedSkill as any}
            isOpen={isSkillAlertVisible}
            onClose={() => setIsSkillAlertVisible(false)}
            modalRef={modalRef as any}
          />
        )}
      </div>
    </MotionConfig>
  );
}