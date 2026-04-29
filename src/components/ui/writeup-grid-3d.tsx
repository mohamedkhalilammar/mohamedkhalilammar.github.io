"use client";

import { motion } from "framer-motion";
import Link from "next/link";
import { Writeup } from "@/data/portfolio";

function WriteupCard({ writeup, index }: { writeup: Writeup; index: number }) {
  // Simple, clean entry stagger without heavy 3D math
  const entryVariants = {
    hidden: { opacity: 0, y: 30 },
    visible: {
      opacity: 1,
      y: 0,
      transition: { duration: 0.6, delay: index * 0.1, ease: [0.16, 1, 0.3, 1] as const }
    }
  };

  return (
    <motion.div
      variants={entryVariants}
      initial="hidden"
      whileInView="visible"
      viewport={{ once: true, margin: "-50px" }}
      className="group h-full"
    >
      <div className="flex flex-col h-full bg-black/40 backdrop-blur-sm border border-primary-500/20 rounded-xl overflow-hidden hover:border-primary-500/50 hover:bg-black/60 transition-all duration-300 hover:-translate-y-2 shadow-sm hover:shadow-[0_15px_30px_rgba(var(--primary-rgb),0.1)] relative">

        {/* Subtle background image header if available */}
        {writeup.mediaUrl && (
          <div className="h-32 w-full overflow-hidden relative border-b border-primary-500/10">
            <div className="absolute inset-0 bg-primary-900/40 mix-blend-multiply z-10 group-hover:bg-primary-900/20 transition-colors" />
            <img
              src={writeup.mediaUrl}
              alt={writeup.title}
              className="w-full h-full object-cover opacity-60 group-hover:opacity-80 group-hover:scale-105 transition-all duration-500"
            />
          </div>
        )}

        <div className="p-5 flex flex-col flex-1 relative z-20">
          <div className="flex items-center justify-between flex-wrap gap-2 mb-4">
            <span className="font-mono text-[10px] font-bold tracking-[0.1em] uppercase text-background bg-primary px-3 py-1 rounded shadow-sm">
              {writeup.category}
            </span>
            <span className="font-mono text-[11px] text-foreground/50 tracking-wider">
              {writeup.competition}
            </span>
          </div>

          <h3 className="font-sans text-lg font-bold text-foreground uppercase tracking-tight leading-snug mb-3 drop-shadow-sm group-hover:text-primary-300 transition-colors">
            {writeup.title}
          </h3>

          <p className="text-sm leading-relaxed text-foreground/70 mb-6 flex-1">
            {writeup.summary}
          </p>

          <Link
            href={`/writeups/${writeup.id}`}
            className="inline-flex items-center justify-between font-mono text-[0.8rem] font-bold tracking-widest uppercase text-primary mt-auto pt-5 border-t border-primary-500/10 group-hover:border-primary-500/30 transition-all no-underline"
          >
            <span>Read Full Report</span>
            <span className="transform transition-transform duration-300 group-hover:translate-x-2">{"→"}</span>
          </Link>
        </div>
      </div>
    </motion.div>
  );
}

export function WriteupGrid3D({ writeups }: { writeups: Writeup[] }) {
  return (
    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 mt-6 mb-4 w-full">
      {writeups.map((writeup, i) => (
        <WriteupCard key={writeup.id} writeup={writeup} index={i} />
      ))}
    </div>
  );
}
