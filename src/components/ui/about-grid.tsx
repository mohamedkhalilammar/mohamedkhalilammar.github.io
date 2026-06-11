"use client";

import { Fragment, type ReactNode } from "react";
import { motion, useReducedMotion } from "framer-motion";

const EASE = [0.16, 1, 0.3, 1] as const;

/**
 * Renders a paragraph where any substring wrapped in *asterisks* is rendered
 * as a "hot" term — brighter weight and aurora color — so the eye lands on the
 * key skills first. Everything else stays muted body text.
 */
function richBody(text: string): ReactNode {
  return text.split(/(\*[^*]+\*)/g).map((chunk, i) => {
    if (chunk.startsWith("*") && chunk.endsWith("*")) {
      return (
        <strong key={i} className="font-semibold text-primary-200">
          {chunk.slice(1, -1)}
        </strong>
      );
    }
    return <Fragment key={i}>{chunk}</Fragment>;
  });
}

const CARDS = [
  {
    id: "01",
    title: "Low-level systems & reversing",
    body: "I’m an engineering student at *INSAT* specializing in *Networks and Telecommunications*, with a strong interest in *cybersecurity*. I enjoy exploring how systems work at a low level, especially through *reverse engineering*, *binary analysis*, and *Android application reversing*.",
    icon: (
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5" className="w-5 h-5" aria-hidden>
        <path strokeLinecap="round" strokeLinejoin="round" d="M8.25 3v1.5M4.5 8.25H3m18 0h-1.5M4.5 12H3m18 0h-1.5m-15 3.75H3m18 0h-1.5M8.25 19.5V21M12 3v1.5m0 15V21m3.75-18v1.5m0 15V21m-9-1.5h10.5a2.25 2.25 0 002.25-2.25V6.75a2.25 2.25 0 00-2.25-2.25H6.75A2.25 2.25 0 004.5 6.75v10.5a2.25 2.25 0 002.25 2.25zm.75-12h9v9h-9v-9z" />
      </svg>
    ),
  },
  {
    id: "02",
    title: "Competitions & challenge design",
    body: "Outside of my studies, I regularly take part in *CTF competitions*, work on personal projects, and occasionally *create challenges of my own*. These experiences have helped me develop practical skills in areas like *Android security*, *exploitation*, and *debugging*.",
    icon: (
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5" className="w-5 h-5" aria-hidden>
        <path strokeLinecap="round" strokeLinejoin="round" d="M16.5 18.75h-9m9 0a3 3 0 013 3h-15a3 3 0 013-3m9 0v-3.375c0-.621-.503-1.125-1.125-1.125h-.871M7.5 18.75v-3.375c0-.621.504-1.125 1.125-1.125h.872m5.007 0H9.497m5.007 0a7.454 7.454 0 01-.982-3.172M9.497 14.25a7.454 7.454 0 00.981-3.172M5.25 4.236c-.982.143-1.954.317-2.916.52A6.003 6.003 0 007.73 9.728M5.25 4.236V4.5c0 2.108.966 3.99 2.48 5.228M5.25 4.236V2.721C7.456 2.41 9.71 2.25 12 2.25c2.291 0 4.545.16 6.75.47v1.516M7.73 9.728a6.726 6.726 0 002.748 1.35m8.272-6.842V4.5c0 2.108-.966 3.99-2.48 5.228m2.48-5.492a46.32 46.32 0 012.916.52 6.003 6.003 0 01-5.395 4.972m0 0a6.726 6.726 0 01-2.749 1.35m0 0a6.772 6.772 0 01-3.044 0" />
      </svg>
    ),
  },
  {
    id: "03",
    title: "Hands-on penetration testing",
    body: "I’m currently sharpening my skills in *system and web penetration testing* through *hands-on labs* on dedicated learning platforms. My goal is to build a *solid understanding of systems and security*, and to create tools and solutions that are both useful and reliable.",
    icon: (
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5" className="w-5 h-5" aria-hidden>
        <path strokeLinecap="round" strokeLinejoin="round" d="M9 12.75L11.25 15 15 9.75m-3-7.036A11.959 11.959 0 013.598 6 11.99 11.99 0 003 9.749c0 5.592 3.824 10.29 9 11.623 5.176-1.332 9-6.03 9-11.622 0-1.31-.21-2.571-.598-3.751h-.152c-3.196 0-6.1-1.248-8.25-3.285z" />
      </svg>
    ),
  },
  {
    id: "04",
    title: "AI workflows & automation",
    body: "In parallel, I’ve been diving deeper into the effective use of AI — focusing on *prompt engineering*, *workflow optimization*, and the integration of tools such as *MCP servers* — to leverage AI as a practical, reliable component within security-focused workflows. I’m currently exploring the intersection of *AI agents and offensive security automation*.",
    icon: (
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5" className="w-5 h-5" aria-hidden>
        <path strokeLinecap="round" strokeLinejoin="round" d="M9.813 15.904L9 18.75l-.813-2.846a4.5 4.5 0 00-3.09-3.09L2.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L9 5.25l.813 2.846a4.5 4.5 0 003.09 3.09L15.75 12l-2.846.813a4.5 4.5 0 00-3.09 3.09zM18.259 8.715L18 9.75l-.259-1.035a3.375 3.375 0 00-2.455-2.456L14.25 6l1.036-.259a3.375 3.375 0 002.455-2.456L18 2.25l.259 1.035a3.375 3.375 0 002.456 2.456L21.75 6l-1.035.259a3.375 3.375 0 00-2.456 2.456zM16.894 20.567L16.5 21.75l-.394-1.183a2.25 2.25 0 00-1.423-1.423L13.5 18.75l1.183-.394a2.25 2.25 0 001.423-1.423l.394-1.183.394 1.183a2.25 2.25 0 001.423 1.423l1.183.394-1.183.394a2.25 2.25 0 00-1.423 1.423z" />
      </svg>
    ),
  },
];

/**
 * AboutGrid — the bio as four always-visible glass cards: icon, number,
 * title, short copy. No accordions, no interaction required.
 */
export function AboutGrid() {
  const reduced = useReducedMotion();

  return (
    <div className="grid md:grid-cols-2 gap-5 md:gap-6">
      {CARDS.map((card, i) => (
        <motion.article
          key={card.id}
          initial={reduced ? false : { opacity: 0, y: 28 }}
          whileInView={{ opacity: 1, y: 0 }}
          viewport={{ once: true, amount: 0.15 }}
          transition={{ duration: 0.65, delay: i * 0.08, ease: EASE }}
          className="group relative rounded-3xl bg-white/[0.03] ring-1 ring-white/10 p-7 md:p-10 backdrop-blur-sm transition-all duration-300 hover:ring-primary-400/40 hover:bg-white/[0.05] hover:-translate-y-1 hover:shadow-[0_20px_50px_-18px_rgba(129,140,248,0.3)]"
        >
          {/* top hairline accent */}
          <span
            aria-hidden
            className="absolute top-0 left-8 right-8 h-px bg-gradient-to-r from-primary-400/50 via-primary-300/20 to-transparent opacity-0 group-hover:opacity-100 transition-opacity duration-300"
          />

          <div className="flex items-center justify-between mb-6 md:mb-7">
            <span className="flex items-center justify-center w-12 h-12 md:w-14 md:h-14 rounded-2xl bg-primary-500/10 ring-1 ring-primary-500/20 text-primary-300 group-hover:bg-primary-500/15 group-hover:text-primary-200 transition-colors duration-300 [&_svg]:w-6 [&_svg]:h-6 md:[&_svg]:w-7 md:[&_svg]:h-7">
              {card.icon}
            </span>
            <span className="text-2xl md:text-3xl font-black tabular-nums text-white/[0.07] group-hover:text-primary-400/20 transition-colors duration-300 select-none">
              {card.id}
            </span>
          </div>

          <h3 className="text-white font-bold tracking-tight text-xl md:text-2xl mb-3 md:mb-4">
            {card.title}
          </h3>
          <p className="text-[15px] md:text-[17px] leading-[1.75] text-zinc-400 [text-wrap:pretty]">
            {richBody(card.body)}
          </p>
        </motion.article>
      ))}
    </div>
  );
}
