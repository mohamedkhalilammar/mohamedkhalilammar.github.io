"use client";

import { Fragment, type ReactNode } from "react";

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
 * The bio as four compact, always-visible topics.
 */
export function AboutGrid() {
  return (
    <div className="about-topics">
      {CARDS.map((card) => (
        <article
          key={card.id}
          className="about-topic"
        >
          <div className="about-topic-heading">
            <span className="about-topic-icon">
              {card.icon}
            </span>
            <h3>{card.title}</h3>
          </div>
          <p>
            {richBody(card.body)}
          </p>
        </article>
      ))}
    </div>
  );
}
