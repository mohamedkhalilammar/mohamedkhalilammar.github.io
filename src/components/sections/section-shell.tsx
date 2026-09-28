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
    <section id={id} className="portfolio-section">
      
      {/* ── SECTION DIVIDER — circuit trace on Midnight, printer's rule on light ── */}
      
      {/* Background Index Watermark */}
      <div className="section-kicker"><span className="eyebrow">{eyebrow}</span>{index && <span aria-hidden>{index}</span>}</div>

      <div className="relative mt-4">
        {title && (
          <h2 className="section-heading">{title}</h2>
        )}
        
        <div className="min-h-[80px]">
          {children}
        </div>
      </div>
    </section>
  );
}
