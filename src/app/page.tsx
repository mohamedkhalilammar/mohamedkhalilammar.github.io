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
import { ScatteredArtefacts } from "@/components/ui/cyber-intro";
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
  { label: "Achievements", href: "#achievements" },
  { label: "Certifications", href: "#certifications" },
  { label: "Projects", href: "#projects" },
  { label: "CTF", href: "#ctf" },
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

export default function Home() {
  const reducedMotion = useReducedMotion();
  const heroRef = useRef<HTMLElement | null>(null);
  const modalRef = useRef<HTMLDivElement | null>(null);
  const modalTriggerRef = useRef<HTMLElement | null>(null);
  const projectRectsRef = useRef<WeakMap<HTMLElement, DOMRect>>(new WeakMap());
  const pointerFrameRef = useRef<number | null>(null);
  const pointerStateRef = useRef<{ card: HTMLElement; x: number; y: number } | null>(null);
  const { scrollYProgress } = useScroll({
    target: heroRef,
    offset: ["start start", "end start"],
  });

  const allSkills = useMemo(
    () => skillGroups.flatMap((group) => group.items.map((item) => ({ ...item, group: group.title }))),
    []
  );
  const [selectedSkillName, setSelectedSkillName] = useState(allSkills[0]?.name ?? "");
  const [activeAchievementIdx, setActiveAchievementIdx] = useState(0);
  const [roleIdx, setRoleIdx] = useState(0);
  const roles = ["CTF Player", "Security Enthusiast", "Engineering Student", "Network Specialist"];
 
  useEffect(() => {
    const interval = setInterval(() => {
      setRoleIdx((prev) => (prev + 1) % roles.length);
    }, 3000);
    return () => clearInterval(interval);
  }, []);
 
  const { scrollYProgress: photoScroll } = useScroll({
    target: heroRef,
    offset: ["start start", "end start"],
  });
 
  const y1 = useTransform(photoScroll, [0, 1], [0, -100]);
  const y2 = useTransform(photoScroll, [0, 1], [0, 100]);



  const selectedSkill =
    allSkills.find((skill) => skill.name === selectedSkillName) ?? allSkills[0];
  const [isSkillAlertVisible, setIsSkillAlertVisible] = useState(false);
  
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [formSent, setFormSent] = useState(false);
 
  const handleContactSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    setIsSubmitting(true);
    setTimeout(() => {
      setIsSubmitting(false);
      setFormSent(true);
      setTimeout(() => setFormSent(false), 5000);
    }, 1500);
  };
 
  const { scrollY } = useScroll();
  const [showScrollTop, setShowScrollTop] = useState(false);
 
  useEffect(() => {
    return scrollY.onChange((latest) => {
      setShowScrollTop(latest > 1000);
    });
  }, [scrollY]);

  useEffect(() => {
    if (!isSkillAlertVisible || !modalRef.current) {
      return;
    }

    const dialog = modalRef.current;
    dialog.focus();

    const focusableSelectors = [
      "a[href]",
      "button:not([disabled])",
      "textarea:not([disabled])",
      "input:not([disabled])",
      "select:not([disabled])",
      "[tabindex]:not([tabindex='-1'])",
    ].join(",");

    const handleDialogKeyDown = (event: KeyboardEvent) => {
      if (event.key === "Escape") {
        setIsSkillAlertVisible(false);
        return;
      }

      if (event.key !== "Tab") {
        return;
      }

      const focusableElements = Array.from(dialog.querySelectorAll<HTMLElement>(focusableSelectors));
      if (focusableElements.length === 0) {
        event.preventDefault();
        return;
      }

      const first = focusableElements[0];
      const last = focusableElements[focusableElements.length - 1];
      const active = document.activeElement as HTMLElement | null;

      if (!event.shiftKey && active === last) {
        event.preventDefault();
        first.focus();
      } else if (event.shiftKey && active === first) {
        event.preventDefault();
        last.focus();
      }
    };

    document.addEventListener("keydown", handleDialogKeyDown);
    return () => {
      document.removeEventListener("keydown", handleDialogKeyDown);
      modalTriggerRef.current?.focus();
    };
  }, [isSkillAlertVisible]);

  useEffect(() => {
    return () => {
      if (pointerFrameRef.current !== null) {
        cancelAnimationFrame(pointerFrameRef.current);
      }
    };
  }, []);

  const reveal = {
    initial: reducedMotion
      ? { opacity: 1, y: 0, filter: "blur(0px)" }
      : { opacity: 0, y: 50, filter: "blur(10px)" },
    whileInView: { opacity: 1, y: 0, filter: "blur(0px)" },
    transition: reducedMotion
      ? { duration: 0 }
      : { duration: 1.2, ease: [0.16, 1, 0.3, 1], delay: 0.1 },
    viewport: { once: true, amount: 0.15 },
  };
  return (
    <MotionConfig reducedMotion="user">
      {/* <ScatteredArtefacts /> */}
      <div className="page-shell">
        <Sidebar />
        <div className="page-noise" aria-hidden />

        <header className="top-nav">
          <p className="brand-mark">{profile.name}</p>
          <ActiveNav />
        </header>

        <main>
          {/* ===== CINEMATIC ENTRANCE ===== */}
          <CreativeEntrance4D />

          {/* ===== HERO ===== */}
          <section className="hero-section relative min-h-[90vh] flex items-center overflow-hidden" id="top" ref={heroRef}>
            {/* Background Atmosphere */}
            <div className="absolute inset-0 z-0 pointer-events-none">
              <div className="absolute top-1/2 -right-20 -translate-y-1/2 w-[600px] h-full opacity-30">
                <CyberPulseScene />
              </div>
            </div>
 
            <div className="hero-content reveal-stagger w-full max-w-7xl mx-auto px-6 relative z-10 py-20">
              <div className="grid grid-cols-1 lg:grid-cols-12 gap-8 lg:gap-12 items-center w-full">

                {/* Text Column (Moved to Left for Better UX) */}
                <div className="w-full flex-col css-stagger-item text-left lg:col-span-8">


                  <div className="relative mb-8 pb-4 pl-1 mt-10">
                    <motion.div 
                      initial={{ opacity: 0, x: -40, filter: "blur(12px)" }}
                      animate={{ opacity: 1, x: 0, filter: "blur(0px)" }}
                      transition={{ duration: 1.2, ease: "easeOut", delay: 0.1 }}
                      className="absolute -top-12 md:-top-16 left-0 font-mono text-[5rem] md:text-[9rem] font-black text-white/[0.12] uppercase tracking-tighter leading-[0.8] select-none pointer-events-none"
                    >
                      MEET
                    </motion.div>
                    
                    <motion.h1 
                      initial={{ opacity: 0, scale: 0.9, y: 20 }}
                      animate={{ opacity: 1, scale: 1, y: 0 }}
                      transition={{ duration: 1, type: "spring", bounce: 0.4, delay: 0.2 }}
                      className="relative z-10 font-sans text-7xl md:text-9xl lg:text-[11rem] font-black uppercase tracking-tighter leading-[0.8] text-transparent bg-clip-text bg-gradient-to-br from-white via-amber-300 to-orange-600 filter drop-shadow-[0_10px_50px_rgba(245,158,11,0.45)]"
                    >
                      Khalil
                    </motion.h1>
                  </div>

                  <h2 className="text-xl md:text-2xl font-mono text-zinc-300 mb-8 uppercase tracking-widest border-b border-amber-500/20 pb-4 inline-block min-h-[1.5em]">
                    <AnimatePresence mode="wait">
                      <motion.span
                        key={roles[roleIdx]}
                        initial={{ opacity: 0, y: 10 }}
                        animate={{ opacity: 1, y: 0 }}
                        exit={{ opacity: 0, y: -10 }}
                        transition={{ duration: 0.5 }}
                      >
                        {roles[roleIdx]}
                      </motion.span>
                    </AnimatePresence>
                  </h2>

                  {/* Terminal-Style Profile Block */}
                  {/* Terminal-Style Profile Block */}
                  <motion.div 
                    initial={{ opacity: 0, scale: 0.98 }}
                    animate={{ opacity: 1, scale: 1 }}
                    transition={{ duration: 0.8, delay: 0.2 }}
                    className="w-full mb-12 rounded-xl overflow-hidden bg-[#09090b]/80 backdrop-blur-xl border border-white/10 shadow-[0_0_40px_rgba(0,0,0,0.5)]"
                  >
                    {/* Mac Terminal Header */}
                    <div className="flex items-center gap-2 px-4 py-3 bg-white/[0.03] border-b border-white/5">
                      <div className="w-3 h-3 rounded-full bg-red-500/90 shadow-sm" />
                      <div className="w-3 h-3 rounded-full bg-yellow-500/90 shadow-sm" />
                      <div className="w-3 h-3 rounded-full bg-green-500/90 shadow-sm" />
                    </div>
                    
                    {/* Terminal Body */}
                    <div className="p-6 md:p-8 font-mono text-sm md:text-[15px] leading-relaxed text-zinc-100 font-medium">
                      
                      {/* Authentic Bash Command Line */}
                      <motion.div 
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        transition={{ duration: 0.2 }}
                        className="mb-6 flex items-center gap-2 text-[13px] md:text-sm"
                      >
                        <span className="text-emerald-400 font-bold">root@khalil</span>
                        <span className="text-zinc-500">:</span>
                        <span className="text-blue-400 font-bold">~/system</span>
                        <span className="text-zinc-400">$</span>
                        <span className="text-zinc-200 ml-1">cat whoami.txt</span>
                      </motion.div>

                      {profile.about.split('\n\n').map((paragraph, index) => (
                        <motion.p 
                          key={index}
                          initial={{ opacity: 0, filter: "blur(4px)", y: 5 }}
                          animate={{ opacity: 1, filter: "blur(0px)", y: 0 }}
                          transition={{ duration: 0.4, delay: 0.6 + (index * 0.3) }}
                          className="mb-8 last:mb-0 drop-shadow-sm flex"
                        >
                          <span className="text-zinc-600 mr-4 select-none drop-shadow-none flex-shrink-0">
                            {String(index + 1).padStart(2, '0')}
                          </span>
                          <span 
                            dangerouslySetInnerHTML={{ __html: paragraph
                              .replace(/cybersecurity/gi, '<span class="text-amber-400 font-bold drop-shadow-[0_0_8px_rgba(251,191,36,0.4)]">cybersecurity</span>')
                              .replace(/reverse engineering/gi, '<span class="text-emerald-400 font-bold">reverse engineering</span>')
                              .replace(/binary analysis/gi, '<span class="text-emerald-400 font-bold">binary analysis</span>')
                              .replace(/Android security/gi, '<span class="text-emerald-400 font-bold">Android security</span>')
                              .replace(/Android application reversing/gi, '<span class="text-emerald-400 font-bold">Android application reversing</span>')
                              .replace(/exploitation/gi, '<span class="text-rose-400 font-bold drop-shadow-[0_0_8px_rgba(244,63,94,0.4)]">exploitation</span>')
                              .replace(/penetration testing/gi, '<span class="text-rose-400 font-bold">penetration testing</span>')
                              .replace(/CTF competitions/gi, '<span class="text-indigo-400 font-bold">CTF competitions</span>')
                            }}
                          />
                        </motion.p>
                      ))}

                      {/* Philosophy Quotes Inside Terminal */}
                      <motion.div
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        transition={{ duration: 0.5, delay: 1.5 }}
                        className="mt-8 pt-6 border-t border-white/10 space-y-4"
                      >
                        <blockquote className="group">
                          <p className="text-xs md:text-sm text-zinc-300 italic leading-relaxed group-hover:text-white transition-colors">
                            "You won't lose your job to AI — you'll lose it to{" "}
                            <span className="text-amber-500/80 not-italic font-medium">someone who uses AI.</span>"
                          </p>
                          <cite className="block not-italic font-mono text-[9px] uppercase tracking-widest text-zinc-500 mt-1.5 opacity-80">
                            — Jensen Huang, NVIDIA
                          </cite>
                        </blockquote>
    
                        <blockquote className="group">
                          <p className="text-xs md:text-sm text-zinc-400 italic leading-relaxed group-hover:text-zinc-300 transition-colors mb-2">
                            "AI is not a substitute for human intelligence — it's a{" "}
                            <span className="text-amber-500/80 not-italic font-medium">tool to amplify it.</span>"
                          </p>
                          <cite className="block not-italic font-mono text-[9px] uppercase tracking-widest text-zinc-500 mt-1.5 opacity-80">
                            — Dr. Fei-Fei Li, Stanford
                          </cite>
                        </blockquote>
                      </motion.div>
                      
                      {/* Blinking Cursor */}
                      <motion.div 
                        initial={{ opacity: 0 }} 
                        animate={{ opacity: [0, 1, 0] }} 
                        transition={{ repeat: Infinity, duration: 1, ease: "linear", delay: 2 }}
                        className="w-2.5 h-5 bg-amber-500 inline-block mt-4 shadow-[0_0_8px_rgba(245,158,11,0.6)]"
                      />
                    </div>
                  </motion.div>

                  <div className="flex flex-wrap items-center gap-4">
                    <a className="btn-primary" href="#projects">
                      {profile.cta.primary}
                    </a>
                    <a className="btn-secondary" href="#contact">
                      {profile.cta.secondary}
                    </a>
                  </div>
                </div>

                {/* Photo Column (Interactive Symmetric Overlap with Parallax) */}
                <div className="w-full flex justify-center lg:justify-end css-stagger-item relative lg:col-span-4">
                  <ParallaxPhotoColumn scrollYProgress={scrollYProgress} />
 

                </div>

              </div>
            </div>
          </section>

          {/* ===== PROJECTS ===== */}
          <motion.div {...(reveal as any)} className="section-flow">
            <SectionShell id="projects" eyebrow="Work" title="">
              <div className="mb-12">
                <h2 className="font-sans text-6xl md:text-8xl font-black uppercase tracking-tighter mb-2 leading-none">
                  Featured <span className="text-amber-500">Projects</span>
                </h2>
                <div className="h-1 w-24 bg-amber-500" />
              </div>
              <div className="w-full mx-auto">
                <ProjectCarousel3D projects={projects} />
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== CTF ===== */}
          <motion.div {...(reveal as any)} className="section-flow">
            <SectionShell id="ctf" eyebrow="Community" title="CTF Challenges">
              <div className="grid grid-cols-1 lg:grid-cols-12 gap-12 items-center">
                <div className="lg:col-span-12">
                  <p className="text-xl md:text-2xl text-zinc-400 max-w-3xl mb-12 leading-relaxed">
                    I design and solve CTF challenges across reverse engineering, forensics, and exploitation.
                    Browse selected notes and challenge breakdowns.
                  </p>
 
                  {/* CTA */}
                  <div className="mb-16">
                     <a className="btn-secondary py-4 inline-block px-10 border-2 border-amber-500/50 hover:bg-amber-500/10 transition-colors" href="#writeups">
                      View All Writeups
                    </a>
                  </div>
                </div>
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== ACHIEVEMENTS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative">
            <SectionShell id="achievements" eyebrow="Milestones" title="Key Achievements">
              <div className="grid grid-cols-1 lg:grid-cols-12 gap-12 lg:gap-8 items-start">



                {/* Mobile Photo Strip */}
                <AchievementPhotoStrip photos={ACHIEVEMENT_PHOTOS} />
 
                {/* Left side: Full-size Photo Stack */}
                <div className="lg:col-span-4 hidden lg:flex flex-col gap-12 mt-2">
                  {ACHIEVEMENT_PHOTOS.map((src, idx) => (
                    <motion.div
                      key={"photo-" + idx}
                      initial={{ opacity: 0, scale: 0.95 }}
                      whileInView={{ opacity: 1, scale: 1 }}
                      viewport={{ once: true, amount: 0.1 }}
                      transition={{ duration: 0.5 }}
                      className="relative w-full rounded-2xl overflow-hidden border-2 border-primary-500/30 shadow-[0_0_30px_rgba(var(--primary-rgb),0.15)] group aspect-[4/3] lg:aspect-video"
                    >
                      <div className="absolute inset-0 bg-primary-900/10 mix-blend-overlay group-hover:bg-transparent transition-colors duration-700 z-10 pointer-events-none" />
                      <div className="absolute inset-x-0 bottom-0 h-1/3 bg-gradient-to-t from-black via-black/40 to-transparent z-10 pointer-events-none" />
                      <img
                        src={src}
                        alt={`Strategic Milestone ${idx + 1}`}
                        className="w-full h-full object-cover object-center filter group-hover:scale-105 transition-all duration-[1s] ease-out brightness-110"
                      />
                    </motion.div>
                  ))}
                </div>

                {/* Right side: Timeline */}
                <div className="lg:col-span-8 relative border-l border-primary-500/30 ml-4 md:ml-8 pl-8 md:pl-12 space-y-12">


                  {achievements.map((achievement, idx) => (
                    <motion.div
                      key={achievement.title}
                      className="relative group perspective-1000"
                      initial={{ opacity: 0, x: -50, rotateY: 20 }}
                      whileInView={{ opacity: 1, x: 0, rotateY: 0 }}
                      viewport={{ once: true, margin: "0px", amount: 0.1 }}
                      transition={{ duration: 0.4, ease: "easeOut" }}
                    >
                      {/* Timeline Node */}
                      <div
                        className="absolute w-4 h-4 rounded-full bg-black border-2 border-primary-500 -left-[2.1rem] md:-left-[3.1rem] top-2 transition-all duration-300"
                        style={{ animation: 'pulseNode 3s infinite ease-in-out', animationDelay: `${idx * 0.2}s` }}
                      />

                      {/* Achievement Card */}
                      <div
                        className="relative bg-gradient-to-br from-primary-950/20 to-black/40 border-2 border-primary-500/20 rounded-xl p-6 md:p-8 overflow-hidden backdrop-blur-sm transform-gpu transition-all duration-500 ease-out group-hover:border-primary-500/60 group-hover:scale-[1.02] active:scale-95"
                        style={{ animation: 'achievementGlow 4s infinite ease-in-out', animationDelay: `${idx * 0.3}s` }}
                      >
                        <div className="absolute inset-0 bg-primary-500/0 group-hover:bg-primary-500/5 transition-colors duration-500" />

                        {/* Highlight Badge */}
                        <span className="inline-block px-3 py-1 mb-4 text-xs font-mono font-bold uppercase tracking-widest text-black bg-primary-400 rounded-full shadow-[0_0_15px_rgba(var(--primary-rgb),0.6)]">
                          {achievement.highlight}
                        </span>

                        <h3 className="text-xl md:text-2xl font-orbitron font-bold text-white mb-3 group-hover:text-primary-300 transition-colors drop-shadow-md">
                          {achievement.title}
                        </h3>

                        <p className="text-primary-100/70 font-mono text-sm md:text-base leading-relaxed">
                          {achievement.detail}
                        </p>

                        {/* Sci-Fi Decorative Grid */}
                        <div className="absolute top-0 right-0 w-32 h-32 bg-[radial-gradient(circle_at_top_right,rgba(var(--primary-rgb),0.1)_0%,transparent_70%)] pointer-events-none" />
                        <div className="absolute bottom-2 right-2 flex gap-1 pointer-events-none opacity-20 group-hover:opacity-50 transition-opacity">
                          {[1, 2, 3].map(i => <div key={i} className="w-1 h-1 bg-primary-400 rounded-full" />)}
                        </div>
                      </div>
                    </motion.div>
                  ))}
                </div>
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== CERTIFICATIONS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative">
            <SectionShell id="certifications" eyebrow="Validation" title="Certifications">
              <p className="projects-intro text-foreground mb-8">
                Verified training and formal credentials that back the practical offensive-security work shown in this portfolio.
              </p>

              <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 mb-8">
                {certifications.map((cert, idx) => (
                  <article
                    key={cert.name}
                    className="certification-card bg-black/40 border border-primary-500/20 rounded-xl p-6 backdrop-blur-sm hover:border-primary-500/50 transition-all duration-300"
                    style={{ animationDelay: `${idx * 0.08}s` }}
                  >
                    <p className="text-[10px] font-mono uppercase tracking-[0.2em] text-primary-400 mb-3">
                      {cert.issuer}
                    </p>
                    <h3 className="text-xl font-orbitron font-bold text-white mb-3 leading-snug">
                      {cert.name}
                    </h3>
                    <p className="text-primary-100/75 text-sm leading-relaxed mb-5">
                      {cert.description}
                    </p>
                    <a
                      href={cert.verifyUrl || cert.localFile}
                      target="_blank"
                      rel="noopener noreferrer"
                      className="inline-flex items-center gap-2 px-4 py-2 border border-primary-500/40 text-primary-300 hover:text-white hover:border-primary-400 rounded-lg text-xs font-mono uppercase tracking-widest transition-all"
                    >
                      View Certificate <span>→</span>
                    </a>
                  </article>
                ))}
              </div>

              <article className="learning-path-card bg-gradient-to-br from-primary-950/20 to-black/40 border border-primary-500/25 rounded-xl p-7 backdrop-blur-sm">
                <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-4 mb-5">
                  <div>
                    <p className="text-[10px] font-mono uppercase tracking-[0.2em] text-primary-400 mb-2">Learning Path</p>
                    <h3 className="text-2xl font-orbitron font-bold text-white">{learningPath.name}</h3>
                    <p className="text-primary-100/70 text-sm mt-1">{learningPath.provider} • {learningPath.status} • {learningPath.progress}</p>
                  </div>
                  <a
                    href={learningPath.localFile}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="inline-flex items-center justify-center gap-2 px-4 py-2 border border-primary-500/40 text-primary-300 hover:text-white hover:border-primary-400 rounded-lg text-xs font-mono uppercase tracking-widest transition-all"
                  >
                    Open Proof <span>→</span>
                  </a>
                </div>

                <p className="text-primary-100/80 mb-5 leading-relaxed">{learningPath.summary}</p>

                <div className="flex flex-wrap gap-2">
                  {learningPath.modules.map((module) => (
                    <span
                      key={module}
                      className="px-3 py-1 rounded-full border border-primary-500/35 text-xs font-mono uppercase tracking-wider text-primary-200 bg-primary-950/20"
                    >
                      {module}
                    </span>
                  ))}
                </div>
              </article>
            </SectionShell>
          </motion.div>

          {/* ===== SKILLS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative overflow-hidden rounded-3xl border border-primary-800/30">

            <SectionShell id="skills" eyebrow="Tooling" title="Core Skills">

              <div className="skills-grid relative z-10">
                {skillGroups.map((group, groupIdx) => (
                  <article
                    key={group.title}
                    className="skills-card bg-black/60 backdrop-blur-md border border-primary-500/20 hover:border-primary-500/60 transition-all duration-300 p-8 rounded-xl relative group overflow-hidden"
                    style={{ animation: `skillCardEntry 0.5s ease-out ${groupIdx * 0.08}s both` }}
                  >
                    <div className="absolute inset-0 rounded-lg bg-gradient-to-br from-primary-500/0 to-purple-500/0 group-hover:from-primary-500/5 group-hover:to-purple-500/5 transition-colors duration-300 pointer-events-none" />
                    <h3 className="font-sans text-primary-300 font-bold uppercase tracking-wider mb-4 relative z-10">{group.title}</h3>
                    <div className="mt-3 flex flex-wrap gap-2 relative z-10">
                      {group.items.map((skill, idx) => (
                        <button
                          key={skill.name}
                          type="button"
                          aria-pressed={selectedSkillName === skill.name}
                          className={`rounded-full border px-4 py-1.5 text-sm font-mono transition-all cursor-pointer hover:scale-105 active:scale-95 ${selectedSkillName === skill.name && isSkillAlertVisible
                            ? "bg-primary-500/30 text-primary-300 border-primary-400 shadow-[0_0_15px_rgba(var(--primary-rgb),0.5)]"
                            : "bg-transparent text-primary-300/80 border-primary-600/40 hover:border-primary-400 hover:text-primary-200"
                            }`}
                          style={{
                            animation: selectedSkillName === skill.name ? "glowPulse 2s ease-in-out infinite" : "none",
                            animationDelay: `${idx * 0.05}s`,
                          }}
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

          {/* ===== WRITEUPS ===== */}
          <motion.div {...(reveal as any)} className="section-flow relative">
            <SectionShell id="writeups" eyebrow="Knowledge" title="Writeups &amp; Breakdowns">
              <p className="projects-intro text-foreground">
                Technical breakdowns of CTF challenges, vulnerability research, and exploitation paths —
                featuring reverse engineering, mobile security, and binary exploitation.
              </p>
              <WriteupGrid3D writeups={ctfWriteups} />
            </SectionShell>
          </motion.div>

          {/* ===== CONTACT ===== */}
          <motion.div {...(reveal as any)} className="section-flow">
            <SectionShell id="contact" eyebrow="Connect" title="Contact">
              <p style={{ marginBottom: "3rem" }} className="text-primary-300/80">
                Open to security internships, CTF collaboration, and network engineering opportunities.
                Reach out to discuss projects via the channels below.
              </p>



              <div className="contact-grid">
                <div style={{ display: "flex", flexDirection: "column", gap: "1rem" }} className="contact-cards-col flex flex-col justify-center">
                  {[
                      {
                        href: `mailto:${profile.socials?.email}`,
                        icon: "✉",
                        label: "Email",
                        value: profile.socials?.email || "khalil.ammar@proton.me",
                        delay: 0,
                      },
                    {
                      href: profile.socials?.linkedin,
                      icon: "in",
                      label: "LinkedIn",
                      value: "Connect Professionally",
                      delay: 0.1,
                    },
                    {
                      href: profile.socials?.github,
                      icon: "</>",
                      label: "GitHub Source",
                      value: "View Repositories",
                      delay: 0.2,
                    },
                  ].map((contact, idx) => (
                    <a
                      key={contact.label}
                      className="contact-card group relative p-6 border-2 border-primary-500/30 hover:border-primary-400 bg-gradient-to-br from-primary-950/20 to-black/30 rounded-lg transition-all duration-300 hover:shadow-[0_0_20px_rgba(var(--primary-rgb),0.3)]"
                      href={contact.href}
                      target={contact.label !== "Secure Mail" ? "_blank" : undefined}
                      rel={contact.label !== "Secure Mail" ? "noopener noreferrer" : undefined}
                      style={{
                        animation: `contactCardIn 0.5s ease-out ${contact.delay}s both`,
                      }}
                    >
                      <span className="text-4xl mb-3 block opacity-70 group-hover:opacity-100 transition-opacity">{contact.icon}</span>
                      <p className="text-primary-400 font-mono text-xs uppercase tracking-widest font-bold">{contact.label}</p>
                      <p className="text-primary-300/60 text-sm mt-1 group-hover:text-primary-300 transition-colors">{contact.value}</p>
                      <div className="absolute inset-0 rounded-lg bg-gradient-to-r from-primary-500/0 via-transparent to-purple-500/0 group-hover:from-primary-500/5 group-hover:via-transparent group-hover:to-purple-500/5 pointer-events-none transition-all" />
                    </a>
                  ))}
                </div>

                <form onSubmit={handleContactSubmit} className="contact-form-ui flex flex-col gap-5 p-8 bg-gradient-to-br from-primary-950/20 to-black/30 border-2 border-primary-500/30 hover:border-primary-400/50 transition-all duration-300 rounded-lg shadow-2xl relative overflow-hidden"
                  style={{
                    animation: "contactCardIn 0.5s ease-out 0.3s both",
                  }}
                >
                  <AnimatePresence mode="wait">
                    {formSent ? (
                      <motion.div 
                        initial={{ opacity: 0, scale: 0.9 }}
                        animate={{ opacity: 1, scale: 1 }}
                        className="flex flex-col items-center justify-center py-12 text-center h-full"
                      >
                        <div className="w-16 h-16 rounded-full bg-green-500/20 border-2 border-green-500 flex items-center justify-center mb-6">
                           <span className="text-2xl">✓</span>
                        </div>
                        <h3 className="text-2xl font-bold text-white mb-2 uppercase tracking-widest">Transmission Sent</h3>
                        <p className="text-zinc-400 text-sm max-w-[250px]">Your message has been encoded and dispatched. I'll get back to you shortly.</p>
                      </motion.div>
                    ) : (
                      <motion.div exit={{ opacity: 0, scale: 0.95 }} className="flex flex-col gap-5">
                        <h3 className="text-xl font-bold uppercase text-primary-300 border-b border-primary-500/30 pb-4 font-orbitron tracking-widest">Connect Directly</h3>
                        <div className="space-y-4">
                          <input
                            type="text"
                            name="subject"
                            placeholder="Connection Subject"
                            className="w-full p-4 bg-black/40 border-2 border-primary-600/40 hover:border-primary-500 focus:border-primary-400 rounded text-primary-300 placeholder-primary-600/50 outline-none font-mono text-sm transition-all"
                            required
                          />
                          <textarea
                            name="body"
                            placeholder="Message content..."
                            rows={5}
                            className="w-full p-4 bg-black/40 border-2 border-primary-600/40 hover:border-primary-500 focus:border-primary-400 rounded text-primary-300 placeholder-primary-600/50 outline-none font-mono text-sm transition-all resize-none"
                            required
                          />
                        </div>
                        <button
                          type="submit"
                          disabled={isSubmitting}
                          className="mt-2 px-6 py-4 bg-amber-500/10 border-2 border-amber-500 hover:bg-amber-500 hover:text-black text-amber-500 font-mono uppercase tracking-[0.2em] text-sm transition-all duration-300 rounded font-black flex items-center justify-center gap-2 disabled:opacity-50"
                        >
                          {isSubmitting ? "Dispatching..." : "Send Message"} 
                          {!isSubmitting && <span className="text-lg">→</span>}
                        </button>
                      </motion.div>
                    )}
                  </AnimatePresence>
                </form>
              </div>
            </SectionShell>
          </motion.div>
        </main>
 
        <EnhancedFooter />
        <ScrollToTop />
 
        {/* ===== SKILL MODAL ===== */}
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