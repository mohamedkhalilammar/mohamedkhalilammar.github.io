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
    <section id={id} className="relative py-10 md:py-20 px-5 md:px-10 lg:px-12 max-w-[1400px] mx-auto group mt-2 md:mt-12">
      
      {/* ── TACTICAL DIVIDER ── */}
      <div className="absolute top-0 left-0 w-full h-px bg-white/10" />
      <div className="absolute top-0 left-0 w-20 h-[2px] bg-white/20" />
      
      {/* Background Index Watermark */}
      {index && (
        <div className="absolute top-8 left-5 md:top-14 md:left-10 font-sans text-[5rem] md:text-[10rem] font-black text-white/[0.025] leading-none select-none pointer-events-none tracking-tighter">
          {index}
        </div>
      )}

      <div className="relative mt-4">
        {title && (
          <h2 className="text-2xl md:text-5xl font-black mb-6 md:mb-10 tracking-tighter uppercase drop-shadow-sm">
            {title}
          </h2>
        )}
        
        <div className="min-h-[80px]">
          {children}
        </div>
      </div>
    </section>
  );
}
