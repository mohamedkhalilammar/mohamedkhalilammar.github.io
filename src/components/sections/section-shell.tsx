import { ReactNode } from "react";

type SectionShellProps = {
  id: string;
  eyebrow: string;
  title: string;
  children: ReactNode;
};

export function SectionShell({ id, eyebrow, title, children }: SectionShellProps) {
  return (
    <section id={id} className="relative py-24 px-6 md:px-12 max-w-[1600px] mx-auto group">
      {/* Tactical Accents */}
      <div className="absolute top-0 left-0 w-8 h-8 border-t-2 border-l-2 border-primary-500/30 group-hover:border-primary-500 transition-colors" />
      <div className="absolute top-0 right-0 w-8 h-8 border-t-2 border-r-2 border-primary-500/30 group-hover:border-primary-500 transition-colors" />
      <div className="absolute bottom-0 left-0 w-8 h-8 border-b-2 border-l-2 border-primary-500/30 group-hover:border-primary-500 transition-colors" />
      <div className="absolute bottom-0 right-0 w-8 h-8 border-b-2 border-r-2 border-primary-500/30 group-hover:border-primary-500 transition-colors" />

      <div className="relative">
        <div className="flex items-center gap-4 mb-6">
          <div className="h-[1px] w-12 bg-primary-500/50" />
          <p className="font-mono text-xs uppercase tracking-[0.3em] text-primary-400">{eyebrow}</p>
        </div>
        
        <h2 className="text-4xl md:text-6xl font-black mb-12 tracking-tight uppercase">
          {title}
        </h2>
        
        <div className="min-h-[100px]">
          {children}
        </div>
      </div>
    </section>
  );
}

