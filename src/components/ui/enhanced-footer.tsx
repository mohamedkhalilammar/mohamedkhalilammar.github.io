"use client";

import { profile } from "@/data/portfolio";

const SOCIALS = [
  {
    label: "GitHub",
    href: profile.socials?.github,
    icon: (
      <svg width="16" height="16" fill="currentColor" viewBox="0 0 24 24">
        <path d="M12 0C5.37 0 0 5.37 0 12c0 5.3 3.438 9.8 8.205 11.387.6.113.82-.258.82-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23A11.509 11.509 0 0 1 12 5.803c1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576C20.566 21.797 24 17.3 24 12c0-6.63-5.37-12-12-12z" />
      </svg>
    ),
  },
  {
    label: "LinkedIn",
    href: profile.socials?.linkedin,
    icon: (
      <svg width="16" height="16" fill="currentColor" viewBox="0 0 24 24">
        <path d="M20.447 20.452h-3.554v-5.569c0-1.328-.027-3.037-1.852-3.037-1.853 0-2.136 1.445-2.136 2.939v5.667H9.351V9h3.414v1.561h.046c.477-.9 1.637-1.85 3.37-1.85 3.601 0 4.267 2.37 4.267 5.455v6.286zM5.337 7.433a2.062 2.062 0 0 1-2.063-2.065 2.064 2.064 0 1 1 2.063 2.065zm1.782 13.019H3.555V9h3.564v11.452zM22.225 0H1.771C.792 0 0 .774 0 1.729v20.542C0 23.227.792 24 1.771 24h20.451C23.2 24 24 23.227 24 22.271V1.729C24 .774 23.2 0 22.222 0h.003z" />
      </svg>
    ),
  },
  {
    label: "Email",
    href: `mailto:${profile.socials?.email}`,
    icon: (
      <svg width="16" height="16" fill="none" stroke="currentColor" strokeWidth="1.5" viewBox="0 0 24 24">
        <path strokeLinecap="round" strokeLinejoin="round" d="M21.75 6.75v10.5a2.25 2.25 0 01-2.25 2.25h-15a2.25 2.25 0 01-2.25-2.25V6.75m19.5 0A2.25 2.25 0 0019.5 4.5h-15a2.25 2.25 0 00-2.25 2.25m19.5 0v.243a2.25 2.25 0 01-1.07 1.916l-7.5 4.615a2.25 2.25 0 01-2.36 0L3.32 8.91a2.25 2.25 0 01-1.07-1.916V6.75" />
      </svg>
    ),
  },
];

export function EnhancedFooter() {
  return (
    <footer className="site-footer border-t border-white/5 bg-[#050505] py-20 px-8 relative overflow-hidden">
      {/* Decorative background elements */}
      <div className="absolute top-0 left-1/2 -translate-x-1/2 w-[800px] h-[1px] bg-gradient-to-r from-transparent via-amber-500/20 to-transparent" />
      <div className="absolute bottom-0 left-0 w-64 h-64 bg-amber-500/5 blur-[100px] pointer-events-none" />

      <div className="max-w-6xl mx-auto flex flex-col items-center text-center relative z-10">
        <h2 className="brand-mark text-2xl font-black uppercase tracking-[0.2em] mb-12 text-white">
          {profile.name}
        </h2>

        <nav aria-label="Social links" className="mb-16">
          <ul className="flex flex-wrap justify-center gap-6 md:gap-8 list-none p-0 m-0">
            {SOCIALS.filter((s) => s.href).map((social) => (
              <li key={social.label}>
                <a
                  href={social.href}
                  target="_blank"
                  rel="noopener noreferrer"
                  className="group flex flex-col items-center gap-3 no-underline"
                >
                  <div className="w-12 h-12 rounded-2xl bg-white/[0.03] border border-white/10 flex items-center justify-center text-zinc-400 group-hover:text-amber-500 group-hover:border-amber-500/40 group-hover:bg-amber-500/5 transition-all duration-300">
                    {social.icon}
                  </div>
                  <span className="font-mono text-[9px] uppercase tracking-[0.3em] text-zinc-600 group-hover:text-zinc-300 transition-colors">
                    {social.label}
                  </span>
                </a>
              </li>
            ))}
          </ul>
        </nav>

        <div className="w-16 h-px bg-white/10 mb-8" />

        <p className="font-mono text-[10px] uppercase tracking-[0.4em] text-zinc-500 mb-2">
          Developed by {profile.name}
        </p>
        <p className="font-mono text-[10px] uppercase tracking-[0.2em] text-zinc-700">
          © {new Date().getFullYear()}
        </p>
      </div>
    </footer>
  );
}
