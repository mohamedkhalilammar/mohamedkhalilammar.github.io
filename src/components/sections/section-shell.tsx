import { ReactNode } from "react";

type SectionShellProps = {
  id: string;
  eyebrow: string;
  title: string;
  children: ReactNode;
};

export function SectionShell({ id, eyebrow, title, children }: SectionShellProps) {
  return (
    <section id={id} className="relative py-20 md:py-28 px-6 md:px-12 max-w-[1600px] mx-auto group mt-8 md:mt-16">
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

