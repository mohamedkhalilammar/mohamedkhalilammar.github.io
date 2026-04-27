import { ctfWriteups } from "@/data/portfolio";
import { notFound } from "next/navigation";
import Link from "next/link";
import fs from "fs";
import path from "path";
import ReactMarkdown from "react-markdown";
import "@/app/globals.css";

// Generate static params so the pages can be statically generated during build
export function generateStaticParams() {
  return ctfWriteups.map((writeup) => ({
    id: writeup.id,
  }));
}

export default async function WriteupPage({
  params,
}: {
  params: Promise<{ id: string }>
}) {
  const resolvedParams = await params;
  const writeup = ctfWriteups.find((w) => w.id === resolvedParams.id);

  if (!writeup) {
    notFound();
  }

  const markdownContent = null;
  const contentBlocks = writeup.content
    .split("\n")
    .map((line) => line.trim())
    .filter(Boolean);

  return (
    <div className="min-h-screen bg-background relative selection:bg-primary/30 selection:text-primary-100">
      {/* Super Subtle Background */}
      <div className="absolute top-0 inset-x-0 h-96 bg-gradient-to-b from-primary/5 to-transparent pointer-events-none" />

      {/* Navigation */}
      <nav className="w-full max-w-3xl mx-auto py-8 px-6 relative z-10">
        <Link 
          href="/#writeups" 
          className="inline-flex items-center gap-2 text-foreground/50 hover:text-foreground font-mono text-xs uppercase tracking-widest transition-colors"
        >
          &larr; Back to Portfolio
        </Link>
      </nav>

      {/* Main Single-Column Content */}
      <main className="w-full max-w-3xl mx-auto px-6 pb-32">
        
        {/* Header / Metadata */}
        <header className="mb-14">
          <div className="flex items-center gap-3 mb-6">
            <span className="px-2.5 py-1 bg-primary/10 text-primary border border-primary/20 rounded font-mono text-[10px] uppercase tracking-widest font-bold">
              {writeup.category}
            </span>
            <span className="text-foreground/40 font-mono text-[11px] uppercase tracking-widest">
              {writeup.competition}
            </span>
          </div>

          <h1 className="text-4xl md:text-5xl lg:text-6xl font-sans font-black text-foreground tracking-tight leading-[1.1] mb-6">
            {writeup.title}
          </h1>

          <p className="text-xl md:text-2xl text-foreground/70 font-light leading-relaxed">
            {writeup.summary}
          </p>
        </header>

        <div className="w-full">
            <section className="grid grid-cols-1 md:grid-cols-2 gap-6 mb-16 w-full max-w-4xl">
              <div className="bg-gradient-to-br from-white/[0.03] to-transparent border border-white/[0.05] p-6 rounded-2xl flex flex-col h-full backdrop-blur-sm relative overflow-hidden group">
                <div className="absolute inset-0 bg-primary/0 group-hover:bg-primary/5 transition-colors duration-500 pointer-events-none" />
                <p className="font-mono text-[10px] text-primary uppercase tracking-[0.2em] mb-4 font-bold flex items-center gap-2">
                  <span className="w-1.5 h-1.5 rounded-full bg-primary" /> Competition Context
                </p>
                <div className="text-sm text-foreground/70 leading-relaxed flex-1">This challenge was part of the <strong className="text-foreground">{writeup.competition}</strong> event under the <strong className="text-foreground">{writeup.category}</strong> category.</div>
              </div>
              <div className="bg-gradient-to-br from-white/[0.03] to-transparent border border-white/[0.05] p-6 rounded-2xl flex flex-col h-full backdrop-blur-sm relative overflow-hidden group">
                <div className="absolute inset-0 bg-primary/0 group-hover:bg-primary/5 transition-colors duration-500 pointer-events-none" />
                <p className="font-mono text-[10px] text-primary uppercase tracking-[0.2em] mb-4 font-bold flex items-center gap-2">
                  <span className="w-1.5 h-1.5 rounded-full bg-primary" /> Outcome Target
                </p>
                <div className="text-sm text-foreground/70 leading-relaxed flex-1">{writeup.flag ? "Flag was successfully retrieved and validated." : "Full exploit chain was documented and tested."}</div>
              </div>
            </section>
            
            {writeup.mediaUrl && (
              <figure className="mb-16 -mx-4 md:mx-0">
                <div className="rounded-xl md:rounded-2xl overflow-hidden border border-white/10 bg-[#0a0a0c] shadow-2xl">
                  <video
                    src={writeup.mediaUrl}
                    controls
                    autoPlay
                    muted
                    loop
                    playsInline
                    className="w-full h-auto object-contain"
                  />
                </div>
                <figcaption className="mt-4 text-center font-mono text-[11px] text-foreground/40 tracking-widest uppercase">
                  Demonstration: Exploitation Proof of Concept
                </figcaption>
              </figure>
            )}

            <article className="max-w-none text-foreground/80">
              {markdownContent ? (
                <ReactMarkdown
                  components={{
                    h1: ({node, ...props}) => <h1 className="text-4xl md:text-5xl font-sans font-bold mt-12 mb-6 tracking-tighter text-foreground" {...props} />,
                    h2: ({node, ...props}) => <h2 className="text-2xl md:text-3xl font-sans font-bold mt-10 mb-5 pb-2 border-b border-white/10 text-foreground" {...props} />,
                    h3: ({node, ...props}) => <h3 className="text-xl font-sans font-bold mt-8 mb-4 text-primary-300" {...props} />,
                    p: ({node, ...props}) => <p className="leading-relaxed mb-6 font-light text-lg text-foreground/80" {...props} />,
                    a: ({node, ...props}) => <a className="text-primary hover:text-primary-300 underline underline-offset-4 decoration-primary/30 transition-colors cursor-pointer" target="_blank" rel="noopener noreferrer" {...props} />,
                    ul: ({node, ...props}) => <ul className="list-disc pl-6 mb-6 space-y-2 text-foreground/80" {...props} />,
                    ol: ({node, ...props}) => <ol className="list-decimal pl-6 mb-6 space-y-2 text-foreground/80" {...props} />,
                    li: ({node, ...props}) => <li className="pl-2" {...props} />,
                    strong: ({node, ...props}) => <strong className="font-bold text-foreground" {...props} />,
                    em: ({node, ...props}) => <em className="italic text-foreground/90" {...props} />,
                    blockquote: ({node, ...props}) => (
                      <blockquote className="border-l-4 border-primary/50 bg-primary/5 py-4 px-6 my-8 rounded-r-lg font-mono text-sm text-foreground/70" {...props} />
                    ),
                    pre: ({node, ...props}) => (
                      <div className="my-8 rounded-xl overflow-hidden bg-[#0a0a0c] border border-white/10 shadow-2xl">
                        {/* Terminal Header */}
                        <div className="flex items-center gap-2 px-4 py-3 bg-white/[0.02] border-b border-white/5">
                          <div className="w-2.5 h-2.5 rounded-full bg-red-500/80" />
                          <div className="w-2.5 h-2.5 rounded-full bg-yellow-500/80" />
                          <div className="w-2.5 h-2.5 rounded-full bg-green-500/80" />
                        </div>
                        <pre className="p-4 md:p-6 overflow-x-auto text-[13px] md:text-[14px] leading-relaxed font-mono drop-shadow-md" {...props} />
                      </div>
                    ),
                    code: ({node, inline, className, ...props}: any) => {
                      if (inline) {
                        return <code className="bg-primary/10 text-primary-300 px-1.5 py-0.5 rounded font-mono text-[0.9em]" {...props} />;
                      }
                      return <code className="text-primary-100/90 font-mono" {...props} />;
                    },
                    hr: ({node, ...props}) => <hr className="border-white/10 my-12" {...props} />
                  }}
                >
                  {markdownContent}
                </ReactMarkdown>
              ) : (
                <div className="prose prose-invert prose-lg max-w-none prose-headings:font-sans prose-headings:font-bold prose-headings:tracking-tight prose-a:text-primary prose-a:no-underline hover:prose-a:underline prose-p:text-foreground/80 prose-p:leading-relaxed prose-p:font-light prose-code:px-1.5 prose-code:py-0.5 prose-code:rounded prose-code:bg-primary/10 prose-code:text-primary-300 prose-code:before:content-none prose-code:after:content-none prose-pre:bg-[#0a0a0c] prose-pre:border prose-pre:border-white/5 prose-pre:shadow-2xl prose-pre:p-6 prose-li:text-foreground/80 prose-blockquote:border-primary/50 prose-blockquote:bg-primary/5 prose-blockquote:py-2 prose-blockquote:pl-6 prose-blockquote:not-italic prose-blockquote:rounded-r-lg prose-hr:border-white/10 text-foreground/80">
                  {contentBlocks.map((paragraph, idx) => (
                    <p 
                      key={`p-${idx}`} 
                      dangerouslySetInnerHTML={{ __html: paragraph }} 
                    />
                  ))}
                </div>
              )}
            </article>
          </div>

        <hr className="my-16 border-white/10" />

        {/* Metadata Footer */}
        <footer className="space-y-6">
          {writeup.flag && (
            <div className="bg-primary/5 border border-primary/20 rounded-xl p-6">
              <p className="font-mono text-[10px] text-primary uppercase tracking-widest font-bold mb-3">
                Recovered Flag
              </p>
              <code className="text-primary-300 font-mono text-sm break-all">
                {writeup.flag}
              </code>
            </div>
          )}

          <div className="flex flex-col md:flex-row gap-6">
            {writeup.tools && writeup.tools.length > 0 && (
              <div className="flex-1 bg-white/[0.02] border border-white/5 rounded-xl p-6">
                <p className="font-mono text-[10px] text-foreground/50 uppercase tracking-widest mb-3">Toolset</p>
                <div className="flex flex-wrap gap-2">
                  {writeup.tools.map((t) => (
                    <span key={t} className="px-2 py-1 bg-black/40 border border-white/5 rounded text-[11px] font-mono text-foreground/70">
                      {t}
                    </span>
                  ))}
                </div>
              </div>
            )}

            {writeup.isAuthored && (
              <div className="flex-1 bg-white/[0.02] border border-white/5 rounded-xl p-6">
                <p className="font-mono text-[10px] text-foreground/50 uppercase tracking-widest mb-3">Author Info</p>
                <p className="text-sm text-foreground/60 leading-relaxed">
                  This challenge was created by <span className="text-primary font-medium">Khalil Ammar</span>.
                </p>
              </div>
            )}
          </div>

          <div className="pt-8">
            <a 
              href="https://github.com/khalilammarr/CTF"
              target="_blank"
              rel="noopener noreferrer"
              className="inline-flex items-center gap-2 font-mono text-xs uppercase tracking-widest text-foreground hover:text-primary transition-colors"
            >
              <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 24 24">
                <path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z" />
              </svg>
              View Source Code on GitHub &rarr;
            </a>
          </div>
        </footer>

      </main>
    </div>
  );
}
