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
    <section id={id} className="relative py-12 md:py-20 px-6 md:px-12 max-w-[1300px] mx-auto group mt-4 md:mt-12">
      
      {/* ── BOLD TACTICAL DIVIDER ── */}
      <div className="absolute top-0 left-0 w-full h-px bg-white/10" />
      <div className="absolute top-0 left-0 w-24 h-[2px] bg-white/20" />
      
      {/* Background Index Watermark */}
      {index && (
        <div className="absolute top-12 left-6 md:top-16 md:left-12 font-sans text-[6rem] md:text-[12rem] font-black text-white/[0.03] leading-none select-none pointer-events-none tracking-tighter">
          {index}
        </div>
      )}

      <div className="relative mt-6">
        <h2 className="text-3xl md:text-5xl font-black mb-8 tracking-tighter uppercase drop-shadow-sm">
          {title}
        </h2>
        
        <div className="min-h-[100px]">
          {children}
        </div>
      </div>
    </section>
  );
}

