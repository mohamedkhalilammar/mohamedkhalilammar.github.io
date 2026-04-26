import { ReactNode } from "react";

type SectionShellProps = {
  id: string;
  eyebrow: string;
  title: string;
  children: ReactNode;
  index?: string;
};

export function SectionShell({ id, eyebrow, title, children, index }: SectionShellProps) {
  return (
    <section id={id} className="relative py-28 md:py-36 px-6 md:px-12 max-w-[1600px] mx-auto group mt-8 md:mt-16">
      
      {/* ── BOLD TACTICAL DIVIDER ── */}
      <div className="absolute top-0 left-0 w-full h-px bg-white/10" />
      <div className="absolute top-0 left-0 w-32 h-[3px] bg-white/20" />
      
      {/* Background Index Watermark */}
      {index && (
        <div className="absolute top-20 left-6 md:left-12 font-sans text-[12rem] md:text-[20rem] font-black text-white/[0.03] leading-none select-none pointer-events-none tracking-tighter">
          {index}
        </div>
      )}

      <div className="relative mt-8">
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

