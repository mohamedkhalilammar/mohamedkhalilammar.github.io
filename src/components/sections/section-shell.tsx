import { ReactNode } from "react";

type SectionShellProps = {
  id: string;
  eyebrow: string;
  title: string;
  children: ReactNode;
};

export function SectionShell({ id, eyebrow, title, children }: SectionShellProps) {
  return (
    <section id={id} className="relative py-28 md:py-36 px-6 md:px-12 max-w-[1600px] mx-auto group mt-8 md:mt-16">
      
      {/* ── MASSIVE GLOWING SECTION DIVIDER ── */}
      <div className="absolute top-0 left-1/2 -translate-x-1/2 w-screen h-px bg-gradient-to-r from-transparent via-amber-500/30 to-transparent" />
      <div className="absolute top-0 left-1/2 -translate-x-1/2 w-1/3 md:w-96 h-[2px] bg-amber-500/80 shadow-[0_0_30px_rgba(245,158,11,0.8)]" />
      
      {/* Centered Anchor Node */}
      <div className="absolute -top-[5px] left-1/2 -translate-x-1/2 w-3 h-3 bg-[#09090b] border-2 border-amber-500 rotate-45 shadow-[0_0_15px_rgba(245,158,11,0.8)] z-10" />

      {/* Tactical Accents */}
      <div className="absolute top-0 left-0 w-12 h-12 border-t-2 border-l-2 border-amber-500/20 group-hover:border-amber-500/80 transition-colors duration-700" />
      <div className="absolute top-0 right-0 w-12 h-12 border-t-2 border-r-2 border-amber-500/20 group-hover:border-amber-500/80 transition-colors duration-700" />
      <div className="absolute bottom-0 left-0 w-12 h-12 border-b-2 border-l-2 border-amber-500/20 group-hover:border-amber-500/80 transition-colors duration-700" />
      <div className="absolute bottom-0 right-0 w-12 h-12 border-b-2 border-r-2 border-amber-500/20 group-hover:border-amber-500/80 transition-colors duration-700" />

      <div className="relative mt-8">
        <div className="flex items-center gap-4 mb-6">
          <div className="h-[1px] w-12 bg-amber-500/80 shadow-[0_0_10px_rgba(245,158,11,0.6)]" />
          <p className="font-mono text-sm md:text-md uppercase tracking-[0.4em] text-amber-400 font-bold">{eyebrow}</p>
        </div>
        
        <h2 className="text-5xl md:text-7xl font-black mb-16 tracking-tighter uppercase drop-shadow-sm">
          {title}
        </h2>
        
        <div className="min-h-[100px]">
          {children}
        </div>
      </div>
    </section>
  );
}

