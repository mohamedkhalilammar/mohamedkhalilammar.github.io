"use client";

import { MotionConfig, motion, useReducedMotion, AnimatePresence } from "framer-motion";
import { useEffect, useMemo, useRef, useState } from "react";
import Link from "next/link";
import { SectionShell } from "@/components/sections/section-shell";
import { AboutGrid } from "@/components/ui/about-grid";
import { SiteHeader } from "@/components/ui/site-header";
import type { PostMeta } from "@/lib/posts";
import { ProjectCarousel3D } from "@/components/ui/project-carousel-3d";
import { CreativeEntrance4D } from "@/components/ui/entrance-4d";
import { AchievementPhotoStrip } from "@/components/ui/achievement-photo-strip";
import { ScrollToTop } from "@/components/ui/scroll-to-top";
import { ScrollProgress } from "@/components/ui/scroll-progress";
import { Magnetic } from "@/components/ui/magnetic";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";
import { EnhancedSkillModal } from "@/components/ui/enhanced-skill-modal";
import { ArcadePanel } from "@/components/ui/arcade-panel";
import { Atmosphere, PaperAtmosphere } from "@/components/ui/atmosphere";
import { PaperPlate } from "@/components/ui/paper-atmosphere";
import { CommandPalette } from "@/components/ui/command-palette";
import { MatrixRain } from "@/components/ui/matrix-rain";
import { CyberTicker } from "@/components/ui/cyber-ticker";
import { StatCounters } from "@/components/ui/stat-counters";
import { RadarSweep } from "@/components/ui/radar-sweep";
import { achievements, certifications, ctfIdentity, learningPath, profile, projects, skillGroups } from "@/data/portfolio";


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
  initial: false,
  whileInView: { opacity: 1, y: 0, filter: "blur(0px)" },
  transition: reducedMotion ? { duration: 0 } : { duration: 0.5, ease: [0.16, 1, 0.3, 1] as const },
  viewport: { once: true, amount: 0.08 },
});

export default function PortfolioHome({ posts }: { posts: PostMeta[] }) {
  const reducedMotion = useReducedMotion();
  const modalRef = useRef<HTMLDivElement | null>(null);
  const modalTriggerRef = useRef<HTMLElement | null>(null);

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
  const [formError, setFormError] = useState("");
  const [isArcadeOpen, setIsArcadeOpen] = useState(false);
  const [isPaletteOpen, setIsPaletteOpen] = useState(false);
  const [isMatrixOn, setIsMatrixOn] = useState(false);


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
      <div className="page-shell portfolio-home">
        <ScrollProgress />
        <div className="page-noise" aria-hidden />

        {/* ── Header ── */}
        <SiteHeader onSearch={() => setIsPaletteOpen(true)} />

        <main id="main-content">
          {/* ===== CINEMATIC ENTRANCE (re-keyed so its reveal replays after boot) ===== */}
          <CreativeEntrance4D />

          {/* ===== HERO ===== */}
          <section
            className="hero-section relative min-h-[75vh] flex items-center overflow-hidden"
            id="top"
          >
            <motion.div
              className="hero-content reveal-stagger w-full max-w-6xl mx-auto px-6 relative z-10 py-12 mt-4 md:mt-8"
            >
              {/* Background Index Watermark - Hero 00 */}
              <div className="absolute top-4 left-4 md:top-6 md:left-6 font-sans text-[6rem] md:text-[12rem] font-black text-[var(--text-whisper)] leading-none select-none pointer-events-none tracking-tighter">
                00
              </div>
              
              <div className="flex flex-col items-center w-full relative z-10">

                {/* ── Text column ── */}
                <div className="w-full max-w-3xl mx-auto flex flex-col items-center text-center">

                  {/* Role eyebrow */}
                  <p className="inline-flex items-center gap-3 font-mono text-[11px] md:text-xs text-[var(--text-muted)] uppercase tracking-[0.25em] mb-2 min-h-[1.6em]">
                    <span className="w-1.5 h-1.5 rounded-full bg-primary-400 animate-pulse flex-shrink-0" aria-hidden />
                    <AnimatePresence mode="wait">
                      <motion.span
                        key={roles[roleIdx]}
                        initial={{ opacity: 0, y: 8 }}
                        animate={{ opacity: 1, y: 0 }}
                        exit={{ opacity: 0, y: -8 }}
                        transition={{ duration: 0.4, ease: [0.16, 1, 0.3, 1] }}
                      >
                        {roles[roleIdx]}
                      </motion.span>
                    </AnimatePresence>
                  </p>

                  {/* Title */}
                  <h2 className="intro-heading">Security through curiosity.</h2>

                  {/* Lead line */}
                  <motion.p
                    initial={false}
                    whileInView={{ opacity: 1, y: 0 }}
                    viewport={{ once: true }}
                    transition={{ duration: 0.7, delay: 0.3, ease: [0.16, 1, 0.3, 1] }}
                    className="text-[var(--text-body)] text-base md:text-lg leading-relaxed max-w-2xl mx-auto mb-8"
                  >
                    {profile.about.split("\n\n")[0]}
                  </motion.p>

                  <div className="flex flex-wrap items-center justify-center gap-4">
                    <Magnetic strength={0.25}>
                      <a className="btn-primary focus-visible:ring-2 focus-visible:ring-primary-300 focus-visible:ring-offset-2 focus-visible:ring-offset-black active:scale-95" href="#projects">{profile.cta.primary}</a>
                    </Magnetic>
                    <Magnetic strength={0.25}>
                      <a className="btn-secondary focus-visible:ring-2 focus-visible:ring-[var(--line-border)] focus-visible:ring-offset-2 focus-visible:ring-offset-black active:scale-95" href="#contact">{profile.cta.secondary}</a>
                    </Magnetic>
                  </div>
                </div>

              </div>

              {/* ── About cards — all visible, no expanding ── */}
              <div id="about" className="relative z-10 mt-12 md:mt-16">
                <div className="mb-8 md:mb-12">
                  <h2 className="font-sans text-4xl md:text-5xl font-black uppercase tracking-tighter mb-2 leading-none text-[var(--text-ink)]">
                    About
                  </h2>
                  <div className="h-[2px] w-16 bg-gradient-to-r from-primary-400 to-primary-600" />
                </div>
                <AboutGrid />
              </div>
            </motion.div>
          </section>

          {/* ===== ACHIEVEMENT TICKER ===== */}
          <CyberTicker />

          {/* ===== ACHIEVEMENTS ===== */}
          <motion.div {...reveal} className="section-flow relative">
            <SectionShell id="achievements" eyebrow="Milestones" title="Key Achievements" index="01">
              <StatCounters
                items={[
                  { value: ctfIdentity.stats.eventsEntered, suffix: "+", label: "CTF Events Entered" },
                  { value: ctfIdentity.stats.challengesSolved, label: "Challenges Solved" },
                  { value: 2, suffix: "x", label: "First-Place Finishes" },
                  { value: 1, prefix: "Top ", suffix: "%", label: "TryHackMe Worldwide" },
                ]}
              />
              <div className="grid grid-cols-1 lg:grid-cols-12 gap-12 lg:gap-8 items-start">
                <div className="lg:col-span-12">
                  <AchievementPhotoStrip photos={ACHIEVEMENT_PHOTOS} />
                </div>
                <div className="lg:col-span-4 hidden lg:flex flex-col gap-10 mt-2">
                  {ACHIEVEMENT_PHOTOS.map((src, idx) => (
                    <motion.div key={idx} className="photo-plate relative w-full rounded-xl overflow-hidden border border-primary-400/20 shadow-lg group aspect-video"
                      initial={reducedMotion ? false : { opacity: 0, y: 18 }}
                      whileInView={{ opacity: 1, y: 0 }}
                      transition={{ delay: 0.08 * idx, duration: 0.9, ease: [0.16, 1, 0.3, 1] }}
                      viewport={{ once: true, amount: 0.25 }}>
                      <div className="absolute inset-0 bg-gradient-to-t from-[var(--photo-caption)] via-transparent to-transparent z-10 pointer-events-none" />
                      <motion.img
                        src={src}
                        alt="Achievement"
                        initial={reducedMotion ? undefined : { scale: 1.18 }}
                        whileInView={{ scale: 1 }}
                        viewport={{ once: true, amount: 0.25 }}
                        transition={{ delay: 0.08 * idx, duration: 1.2, ease: [0.16, 1, 0.3, 1] }}
                        className="w-full h-full object-cover group-hover:scale-[1.06] group-hover:brightness-110 transition-[transform,filter] duration-700 ease-out"
                      />
                      <span className="card-sheen z-20" aria-hidden />
                    </motion.div>
                  ))}
                </div>
                <div className="lg:col-span-8 relative lg:border-l-2 lg:border-primary-400/20 lg:ml-8 lg:pl-12 flex flex-row lg:flex-col overflow-x-auto lg:overflow-visible gap-6 lg:gap-12 pb-6 lg:pb-0 snap-x snap-mandatory scrollbar-hide -mx-6 px-6 lg:mx-0 w-screen lg:w-auto">
                  {achievements.map((achievement, idx) => (
                    <motion.div
                      key={idx}
                      initial={reducedMotion ? false : { opacity: 0, x: 32 }}
                      whileInView={{ opacity: 1, x: 0 }}
                      viewport={{ once: true, amount: 0.3 }}
                      transition={{ duration: 0.55, delay: idx * 0.05, ease: [0.16, 1, 0.3, 1] }}
                      className="relative group p-6 rounded-2xl border-t-[3px] border-l border-r border-b lg:border-t lg:border-t-[var(--line-border)] lg:hover:border-primary-400/30 border-t-primary-400/50 border-x-[var(--line-rule)] border-b-[var(--line-rule)] bg-[var(--surface-sunken)] hover:bg-primary-400/[0.02] hover:shadow-[0_0_30px_rgba(129,140,248,0.1)] transition-colors duration-500 backdrop-blur-sm w-[85vw] sm:w-[320px] lg:w-auto snap-center shrink-0 whitespace-normal"
                    >
                      {/* hover sheen sweep */}
                      <span className="card-sheen" aria-hidden />
                      {/* ghost rank numeral */}
                      <span className="absolute top-4 right-5 text-4xl md:text-5xl font-black tabular-nums text-[var(--text-whisper)] group-hover:text-primary-400/15 transition-colors duration-500 select-none pointer-events-none" aria-hidden>
                        {String(idx + 1).padStart(2, "0")}
                      </span>

                      {/* Desktop Timeline Dot */}
                      <div className="hidden lg:block absolute w-3 h-3 rounded-full bg-primary-400 shadow-[0_0_10px_rgba(129,140,248,0.6)] -left-[3.45rem] top-8" />
                      <div className="hidden lg:block absolute -left-[3.45rem] top-8 w-3 h-3 rounded-full bg-primary-400 animate-ping opacity-75" />

                      <span className="text-[10px] md:text-[11px] font-mono text-primary-400 uppercase tracking-[0.2em] mb-4 block font-bold">{achievement.highlight}</span>
                      <h3 className="text-xl md:text-2xl font-bold text-[var(--text-ink)] mb-3 tracking-tight group-hover:text-transparent group-hover:bg-clip-text group-hover:bg-gradient-to-r group-hover:from-white group-hover:to-zinc-400 transition-all">{achievement.title}</h3>
                      <p className="text-[var(--text-muted)] text-sm md:text-base leading-relaxed group-hover:text-[var(--text-body)] transition-colors w-full break-words">{achievement.detail}</p>
                    </motion.div>
                  ))}
                </div>
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== PROJECTS ===== */}
          <motion.div {...reveal} className="section-flow">
            <SectionShell id="projects" eyebrow="Work" title="" index="02">
              <div className="mb-8">
                <h2 className="font-sans text-4xl md:text-5xl font-black uppercase tracking-tighter mb-2 leading-none text-[var(--text-ink)]">
                  Featured{" "}
                  <span className="text-transparent bg-clip-text bg-gradient-to-br from-[var(--brand-grad-c)] via-[var(--brand-grad-b)] via-30% to-[var(--brand-grad-a)]">
                    Projects
                  </span>
                </h2>
                <div className="h-[2px] w-16 bg-gradient-to-r from-primary-400 to-primary-600" />
              </div>
              <ProjectCarousel3D projects={projects} />
            </SectionShell>
          </motion.div>

          {/* ===== WRITEUPS ===== */}
          <motion.div {...reveal} className="section-flow relative">
            <SectionShell id="blog" eyebrow="Writing" title="The Blog" index="03">
              <p className="section-description">Reverse engineering, CTF write-ups, and lessons from the projects I build.</p>
              <div className="home-writing-list">
                {posts.slice(0, 3).map((post, index) => (
                  <Link href={`/writeups/${post.id}`} key={post.id} className="home-writing-row">
                    <span className="home-writing-index" aria-hidden>{String(index + 1).padStart(2, "0")}</span>
                    <div><span className="journal-meta">{post.category}</span><h3>{post.title}</h3><p>{post.summary}</p></div>
                    <span className="home-writing-arrow" aria-hidden>↗</span>
                  </Link>
                ))}
              </div>
              <Link href="/blog" className="journal-read">Explore all {posts.length} articles <span aria-hidden>↗</span></Link>
            </SectionShell>
          </motion.div>

          {/* ===== CERTIFICATIONS ===== */}
          <motion.div {...reveal} className="section-flow relative">
            <SectionShell id="certifications" eyebrow="Validation" title="Certifications" index="04">
              <p className="text-[var(--text-muted)] text-base leading-relaxed mb-10 max-w-2xl">
                Verified training and formal credentials that back the practical offensive-security work shown in this portfolio.
              </p>

              <div className="grid grid-cols-1 lg:grid-cols-3 gap-4 md:gap-5 mb-6">
                {certifications.map((cert, idx) => (
                    <motion.article
                      key={cert.name}
                      initial={reducedMotion ? false : { opacity: 0, y: 28 }}
                      whileInView={{ opacity: 1, y: 0 }}
                      viewport={{ once: true, amount: 0.2 }}
                      whileHover={reducedMotion ? undefined : { y: -6 }}
                      transition={{ duration: 0.55, delay: idx * 0.1, ease: [0.16, 1, 0.3, 1] }}
                      className="group relative bg-[var(--surface-raised)] border border-primary-500/15 rounded-xl p-4 md:p-5 backdrop-blur-sm hover:border-primary-500/45 transition-colors duration-300 hover:bg-[var(--surface-sunken)] hover:shadow-[0_20px_50px_-18px_rgba(129,140,248,0.3)]"
                    >
                      <span className="card-sheen" aria-hidden />
                      <div className="flex items-start justify-between mb-3.5 gap-3">
                        <p className="text-[9px] md:text-[10px] font-mono uppercase tracking-[0.25em] text-primary-400 mt-0.5">
                          {cert.issuer}
                        </p>
                        {cert.logo && (
                          <img src={cert.logo} alt={cert.issuer} className="h-7 md:h-8 object-contain rounded bg-[var(--surface-sunken)] p-1 transition-transform duration-500 group-hover:scale-110" />
                        )}
                      </div>
                      <h3 className="text-base md:text-lg font-orbitron font-bold text-[var(--text-ink)] mb-2 leading-snug group-hover:text-primary-300 transition-colors">
                        {cert.name}
                      </h3>
                      <p className="text-[var(--text-muted)] text-[13px] leading-relaxed mb-4 group-hover:text-[var(--text-muted)] transition-colors duration-300">
                        {cert.description}
                      </p>
                      <a
                      href={cert.verifyUrl || cert.localFile}
                      target="_blank"
                      rel="noopener noreferrer"
                      className="group/link inline-flex items-center gap-1.5 text-[11px] font-mono uppercase tracking-wider text-primary-400 hover:text-[var(--text-ink)] transition-colors border-b border-primary-500/20 hover:border-primary-400 pb-0.5 min-h-[44px]"
                    >
                      View Certificate <span className="transition-transform duration-300 group-hover/link:translate-x-1">→</span>
                    </a>
                  </motion.article>
                ))}
              </div>

              <article className="bg-gradient-to-br from-[var(--accent-surface)] to-[var(--surface-raised)] border border-primary-500/20 rounded-xl p-5 backdrop-blur-sm hover:border-primary-500/40 transition-all duration-300">
                <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-5 mb-5">
                  <div className="flex items-center gap-5">
                    {learningPath.logo && (
                      <img src={learningPath.logo} alt={learningPath.provider} className="h-11 object-contain rounded bg-[var(--surface-sunken)] p-1.5" />
                    )}
                    <div>
                      <p className="text-[9px] font-mono uppercase tracking-[0.25em] text-primary-400 mb-2">Learning Path</p>
                      <h3 className="text-xl font-orbitron font-bold text-[var(--text-ink)]">{learningPath.name}</h3>
                      <p className="text-[var(--text-muted)] text-sm mt-1">{learningPath.provider} · {learningPath.status} · {learningPath.progress}</p>
                    </div>
                  </div>
                  <a
                    href={learningPath.localFile}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="inline-flex items-center gap-2 px-4 py-2.5 border border-primary-500/35 text-primary-300 hover:text-[var(--text-ink)] hover:border-primary-400 rounded-lg text-[11px] font-mono uppercase tracking-wider transition-all whitespace-nowrap"
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
          <motion.div {...reveal} className="section-flow relative overflow-hidden rounded-2xl md:rounded-3xl border border-primary-800/20">
            <SectionShell id="skills" eyebrow="Tooling" title="Core Skills" index="05">
              <div className="skills-grid relative z-10 flex flex-row lg:grid overflow-x-auto lg:overflow-visible gap-4 md:gap-6 pb-6 lg:pb-0 snap-x snap-mandatory scrollbar-hide w-[calc(100vw-3rem)] -mx-6 px-6 lg:w-auto lg:mx-0 lg:px-0">
                {skillGroups.map((group, groupIdx) => (
                  <article
                    key={group.title}
                    className="skills-card spotlight-card bg-[var(--surface-raised)] backdrop-blur-md border border-primary-500/15 hover:border-primary-500/50 transition-all duration-400 p-4 md:p-5 rounded-xl relative group overflow-hidden min-w-[240px] sm:min-w-[280px] lg:min-w-0 snap-center shrink-0"
                    style={{ animation: `skillCardEntry 0.5s ease-out ${groupIdx * 0.07}s both` }}
                    onMouseMove={(e) => {
                      const rect = e.currentTarget.getBoundingClientRect();
                      e.currentTarget.style.setProperty("--mx", `${e.clientX - rect.left}px`);
                      e.currentTarget.style.setProperty("--my", `${e.clientY - rect.top}px`);
                    }}
                  >
                    <div className="absolute inset-0 rounded-xl opacity-0 group-hover:opacity-100 transition-opacity duration-400 bg-gradient-to-br from-primary-500/4 to-purple-500/4 pointer-events-none" />
                    <h3 className="font-mono text-primary-300/90 font-semibold uppercase tracking-[0.15em] text-[11px] mb-4 relative z-10">
                      {group.title}
                    </h3>
                    <div className="mt-2 flex flex-wrap gap-2 relative z-10">
                      {group.items.map((skill, idx) => (
                        <motion.button
                          key={skill.name}
                          initial={reducedMotion ? false : { opacity: 0, scale: 0.85 }}
                          whileInView={{ opacity: 1, scale: 1 }}
                          viewport={{ once: true, amount: 0.4 }}
                          transition={{ duration: 0.3, delay: idx * 0.03, ease: [0.16, 1, 0.3, 1] }}
                          type="button"
                          aria-pressed={selectedSkillName === skill.name && isSkillAlertVisible}
                          className={`rounded-full border px-3.5 py-1.5 text-[12px] font-mono transition-all cursor-pointer hover:scale-[1.03] active:scale-95 min-h-[36px] flex items-center ${selectedSkillName === skill.name && isSkillAlertVisible
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
                        </motion.button>
                      ))}
                    </div>
                  </article>
                ))}
              </div>
            </SectionShell>
          </motion.div>

          {/* ===== CONTACT ===== */}
          <motion.div {...reveal} className="section-flow">
            <SectionShell id="contact" eyebrow="Connect" title="Contact" index="06">
              <p className="text-[var(--text-muted)] text-base leading-relaxed mb-10 max-w-xl">
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
                      value: "View Source Code",
                    },
                  ].map((contact, idx) => (
                    <a
                      key={contact.label}
                      href={contact.href}
                      target={contact.label !== "Email" ? "_blank" : undefined}
                      rel={contact.label !== "Email" ? "noopener noreferrer" : undefined}
                      className="contact-card group flex items-center gap-5 p-5 border border-primary-500/20 hover:border-primary-400/60 bg-[var(--surface-raised)] hover:bg-[var(--surface-raised)] rounded-xl transition-all duration-300"
                      style={{ animation: `contactCardIn 0.5s ease-out ${idx * 0.08}s both` }}
                    >
                      <span className="text-primary-400 opacity-70 group-hover:opacity-100 transition-opacity flex-shrink-0">
                        {contact.icon}
                      </span>
                      <div>
                        <p className="text-primary-400 font-mono text-[10px] uppercase tracking-[0.2em] font-bold">{contact.label}</p>
                        <p className="text-[var(--text-muted)] text-sm mt-0.5 group-hover:text-[var(--text-body)] transition-colors">{contact.value}</p>
                      </div>
                      <span className="ml-auto text-[var(--text-faint)] group-hover:text-primary-400 group-hover:translate-x-1.5 transition-all duration-300 ease-out text-sm">→</span>
                    </a>
                  ))}
                  <Atmosphere>
                    <RadarSweep />
                  </Atmosphere>
                  <PaperAtmosphere>
                    <PaperPlate />
                  </PaperAtmosphere>
                </div>

                {/* Contact form */}
                <form
                  onSubmit={async (e) => {
                    e.preventDefault();
                    if (isSubmitting) return;
                    setIsSubmitting(true);
                    setFormError("");

                    const FORMSPREE_ENDPOINT = "https://formspree.io/f/mnjwqeyv";

                    const formData = new FormData(e.currentTarget);
                    
                    try {
                      const response = await fetch(FORMSPREE_ENDPOINT, {
                        method: "POST",
                        body: formData,
                        headers: {
                          'Accept': 'application/json'
                        }
                      });

                      if (response.ok) {
                        setIsSubmitting(false);
                        setFormSent(true);
                        (e.target as HTMLFormElement).reset();
                      } else {
                        throw new Error("Form submission failed");
                      }
                    } catch (error) {
                      setIsSubmitting(false);
                      setFormError("Your message couldn't be sent. Please try again, or use the email link.");
                      console.error("Formspree Error:", error);
                    }
                  }}
                  aria-busy={isSubmitting}
                  className="contact-form-ui flex flex-col gap-5 p-7 bg-[var(--surface-sunken)] border border-[var(--line-border)] rounded-lg relative overflow-hidden"
                  style={{ animation: "contactCardIn 0.5s ease-out 0.24s both" }}
                >
                  <AnimatePresence mode="wait">
                    {formSent ? (
                      <motion.div
                        initial={{ opacity: 0, scale: 0.92 }}
                        animate={{ opacity: 1, scale: 1 }}
                        className="flex flex-col items-center justify-center py-12 text-center"
                        role="status"
                      >
                        <div className="w-14 h-14 rounded-full bg-green-500/15 border-2 border-green-500/60 flex items-center justify-center mb-6">
                          <svg viewBox="0 0 24 24" className="w-6 h-6 text-green-400" fill="none" stroke="currentColor" strokeWidth="2.5" aria-hidden>
                            <motion.path
                              d="M5 13l4 4L19 7"
                              strokeLinecap="round"
                              strokeLinejoin="round"
                              initial={{ pathLength: 0 }}
                              animate={{ pathLength: 1 }}
                              transition={{ duration: 0.5, ease: "easeOut", delay: 0.15 }}
                            />
                          </svg>
                        </div>
                        <h3 className="text-xl font-bold text-[var(--text-ink)] mb-2 uppercase tracking-widest font-orbitron">Sent</h3>
                        <p className="text-[var(--text-muted)] text-sm max-w-[260px]">Thanks for reaching out. I&apos;ll get back to you shortly.</p>
                        <button type="button" className="journal-read" onClick={() => setFormSent(false)}>Send another message</button>
                      </motion.div>
                    ) : (
                      <motion.div exit={{ opacity: 0, scale: 0.96 }} className="flex flex-col gap-5">
                        <h3 className="text-base font-bold uppercase text-primary-300 border-b border-primary-500/20 pb-4 font-orbitron tracking-[0.15em]">
                          Send a Message
                        </h3>
                        <label htmlFor="contact-email" className="contact-label">Email address</label>
                        <input
                          id="contact-email"
                          autoComplete="email"
                          type="email"
                          name="email"
                          placeholder="you@example.com"
                          className="w-full p-4 bg-[var(--surface-raised)] border border-primary-600/30 hover:border-primary-500/60 focus:border-primary-400/80 focus:ring-1 focus:ring-primary-400/40 rounded-lg text-primary-200 placeholder-zinc-600 outline-none font-mono text-sm transition-all"
                          required
                        />
                        <label htmlFor="contact-message" className="contact-label">Message</label>
                        <textarea
                          id="contact-message"
                          name="message"
                          placeholder="Tell me what you're working on…"
                          rows={4}
                          className="w-full p-4 bg-[var(--surface-raised)] border border-primary-600/30 hover:border-primary-500/60 focus:border-primary-400/80 focus:ring-1 focus:ring-primary-400/40 rounded-lg text-primary-200 placeholder-zinc-600 outline-none font-mono text-sm transition-all resize-none"
                          required
                        />
                        {formError && <p role="alert" className="form-error">{formError}</p>}
                        <button
                          type="submit"
                          disabled={isSubmitting}
                          className="mt-1 px-6 py-4 bg-primary-400/8 border-2 border-primary-400/80 hover:bg-primary-400 hover:text-black hover:border-primary-400 text-primary-300 font-mono uppercase tracking-[0.18em] text-sm transition-all duration-300 rounded-lg font-bold flex items-center justify-center gap-2 disabled:opacity-40 active:scale-95 focus-visible:ring-2 focus-visible:ring-primary-300 focus-visible:ring-offset-2 focus-visible:ring-offset-black"
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
          <motion.div {...reveal} className="section-flow relative overflow-hidden mb-32">
             <div className="max-w-[1200px] mx-auto px-6">
                <div className="bg-[var(--surface-sunken)] border border-[var(--line-rule)] rounded-xl p-8 md:p-12 flex flex-col md:flex-row items-center justify-between gap-8 relative group hover:bg-[var(--surface-sunken)] transition-colors">
                   <div className="relative z-10">
                      <h2 className="text-4xl font-sans font-black text-[var(--text-ink)] uppercase tracking-tighter mb-2 italic">
                         TAKE A <span className="text-primary-300">BREAK</span>
                      </h2>
                      <p className="text-[var(--text-muted)] font-mono text-xs uppercase tracking-widest">
                         Visit the arcade for a quick game.
                      </p>
                   </div>

                   <div className="flex items-center gap-6">
                      <div className="hidden lg:flex gap-4">
                        {[
                          { id: "SNAKE", label: "Snake", color: "text-primary-300", border: "border-primary-500/15", bg: "bg-primary-500/5" },
                          { id: "RUNNER", label: "Runner", color: "text-indigo-300", border: "border-indigo-500/15", bg: "bg-indigo-500/5" },
                          { id: "MINESWEEPER", label: "Minesweeper", color: "text-violet-300", border: "border-violet-500/15", bg: "bg-violet-500/5" }
                        ].map((game, i) => (
                          <motion.div
                            key={i}
                            whileHover={{ y: -2, scale: 1.02, borderColor: "rgba(255,255,255,0.1)" }}
                            className={`w-28 h-12 ${game.bg} border ${game.border} rounded-lg flex items-center justify-center relative overflow-hidden transition-all duration-300`}
                          >
                            <p className={`font-mono text-[10px] font-bold uppercase tracking-widest ${game.color} relative z-10`}>{game.label}</p>
                            <div className={`absolute left-0 top-0 bottom-0 w-0.5 ${game.color.replace('text-', 'bg-')} opacity-30`} />
                          </motion.div>
                        ))}
                      </div>
                      <button 
                        onClick={() => setIsArcadeOpen(true)}
                        className="btn-primary py-4 px-10 text-[10px] tracking-widest font-black uppercase cursor-pointer"
                      >
                         PLAY NOW →
                      </button>
                   </div>
                </div>
             </div>
          </motion.div>
        </main>

        <EnhancedFooter />
        <ScrollToTop />

        {selectedSkill && (
          <EnhancedSkillModal
            skill={selectedSkill}
            isOpen={isSkillAlertVisible}
            onClose={() => setIsSkillAlertVisible(false)}
            modalRef={modalRef}
          />
        )}
        
        <ArcadePanel
          isOpen={isArcadeOpen}
          onClose={() => setIsArcadeOpen(false)}
        />

        <CommandPalette
          isOpen={isPaletteOpen}
          onOpen={() => setIsPaletteOpen(true)}
          onClose={() => setIsPaletteOpen(false)}
          onArcade={() => setIsArcadeOpen(true)}
          onBreach={() => setIsMatrixOn(true)}
        />
        <MatrixRain active={isMatrixOn} onExit={() => setIsMatrixOn(false)} />
      </div>
    </MotionConfig>
  );
}
