import { projects } from "@/data/portfolio";
import Link from "next/link";
import { notFound } from "next/navigation";
import fs from "fs";
import path from "path";
import ReactMarkdown from "react-markdown";
import rehypeRaw from "rehype-raw";

import { ProjectGallery } from "@/components/ui/project-gallery";

export function generateStaticParams() {
  return projects.map((p) => ({
    id: p.name.toLowerCase().replace(/[^a-z0-9]+/g, "-"),
  }));
}

export default async function ProjectPage({ params }: { params: Promise<{ id: string }> }) {
  const { id } = await params;
  const project = projects.find(
    (p) => p.name.toLowerCase().replace(/[^a-z0-9]+/g, "-") === id
  );

  if (!project) {
    notFound();
  }

  const stackPreview = project.stack.slice(0, 4).join(" · ");
  const projectPulse = [
    { label: "Role", value: project.role || "Security Engineer" },
    { label: "Timeline", value: project.duration || "Continuous" },
    { label: "Stack Depth", value: `${project.stack.length} technologies` },
    { label: "Focus", value: project.context || "Applied Security Engineering" },
  ];

  let markdownContent = null;
  if (project.detailsFile) {
    try {
      const filePath = path.join(process.cwd(), "public", project.detailsFile);
      markdownContent = fs.readFileSync(filePath, "utf-8");
    } catch (e) {
      console.error(`Failed to read markdown file for ${project.name}`, e);
    }
  }

  return (
    <div className="page-shell bg-[#050505] min-h-screen">
      {/* Cinematic Sidebar Indicator */}
      <div className="fixed left-0 top-0 bottom-0 w-1 bg-gradient-to-b from-amber-500/0 via-amber-500/20 to-amber-500/0 z-50" />

      <header className="fixed top-0 left-0 right-0 z-50 backdrop-blur-md border-b border-white/[0.03] px-6 py-4 flex justify-between items-center bg-black/20">
        <Link href="/#projects" className="font-mono text-[10px] uppercase tracking-[0.3em] text-white/50 hover:text-amber-500 transition-all flex items-center gap-2 group">
          <span className="group-hover:-translate-x-1 transition-transform inline-block">&larr;</span> 
          Return to Projects
        </Link>
        <div className="flex items-center gap-4">
          <div className="w-2 h-2 rounded-full bg-amber-500" />
          <span className="font-mono text-[10px] uppercase tracking-[0.3em] text-amber-500">Project Live</span>
        </div>
      </header>
      
      <main className="max-w-[1400px] mx-auto px-6 md:px-12 lg:px-20" style={{ paddingTop: "12rem", paddingBottom: "15rem" }}>
        
        {/* TOP LEVEL INTEL: Title & Summary */}
        <div className="relative mb-24 css-stagger-item">
          {/* Decorative Corner Brackets */}
          <div className="absolute -top-12 -left-12 w-24 h-24 border-t border-l border-white/10 rounded-tl-[3rem]" />
          
          <div className="flex flex-col gap-6">
            <div className="flex items-center gap-4">
              <div className="px-3 py-1 rounded bg-amber-500/10 border border-amber-500/30 text-[10px] font-mono font-bold text-amber-500 uppercase tracking-[0.3em]">
                Project Overview
              </div>
              <div className="h-px w-24 bg-white/10" />
              <p className="font-mono text-xs uppercase tracking-[0.4em] text-white/20 font-bold">{project.context || "Engineering Student"}</p>
            </div>

            <h1 className="font-sans text-6xl md:text-8xl lg:text-9xl font-black text-white uppercase tracking-tighter leading-[0.85] mb-8 drop-shadow-2xl">
              {project.name.split(" ").map((word, i) => (
                <span key={i} className={i % 2 !== 0 ? "text-amber-500" : ""}>
                  {word}{" "}
                </span>
              ))}
            </h1>

            <p className="text-xl md:text-3xl leading-relaxed text-zinc-400 max-w-5xl font-medium">
              {project.summary}
            </p>
          </div>
        </div>



        {/* CORE CONTENT LAYOUT */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-20">
          
          <div className="lg:col-span-8 space-y-32">
            
            {/* Visual Evidence Area */}
            <div className="css-stagger-item">
              <div className="flex items-center gap-6 mb-12">
                <h2 className="font-sans text-3xl font-black uppercase tracking-tighter text-white">Visual Architecture</h2>
                <div className="h-px flex-1 bg-white/[0.05]" />
              </div>

              {/* Enhanced Media Frame */}
              {project.mediaUrl && (
                <div className="relative rounded-3xl overflow-hidden border border-white/[0.05] bg-black shadow-2xl mb-16">
                  <div className="absolute top-0 left-0 right-0 h-12 bg-white/[0.02] border-b border-white/[0.03] flex items-center justify-between px-6 z-10 backdrop-blur-sm">
                    <div className="flex gap-2">
                      <div className="w-2.5 h-2.5 rounded-full bg-red-500/20" />
                      <div className="w-2.5 h-2.5 rounded-full bg-yellow-500/20" />
                      <div className="w-2.5 h-2.5 rounded-full bg-green-500/20" />
                    </div>
                    <span className="font-mono text-[10px] uppercase text-white/20 tracking-[0.3em]">PROJECT_VIEW_01</span>
                  </div>
                  
                  <div className="pt-12">
                    {project.mediaUrl.includes("youtube.com/embed") ? (
                      <iframe
                        src={project.mediaUrl}
                        title={`${project.name} Video Demo`}
                        frameBorder="0"
                        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
                        allowFullScreen
                        className="w-full h-auto aspect-video"
                      />
                    ) : project.mediaUrl.match(/\.(mp4|webm|ogg)$/i) ? (
                      <video src={project.mediaUrl} controls autoPlay muted loop playsInline className="w-full h-auto aspect-video object-cover" />
                    ) : (
                      <img src={project.mediaUrl} alt="Preview" className="w-full h-auto aspect-video object-cover" />
                    )}
                  </div>
                </div>
              )}

              {/* Screenshot Gallery */}
              {project.screenshots && project.screenshots.length > 0 ? (
                <ProjectGallery name={project.name} screenshots={project.screenshots} screenshotCaptions={project.screenshotCaptions} />
              ) : (
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6 opacity-30 grayscale pointer-events-none">
                  {[1, 2].map((i) => (
                    <div key={i} className="aspect-video border border-dashed border-white/20 rounded-2xl flex flex-col items-center justify-center gap-4 bg-white/[0.01]">
                      <span className="text-2xl">❖</span>
                      <span className="font-mono text-[10px] tracking-[0.2em] uppercase">Visual Data Pending</span>
                    </div>
                  ))}
                </div>
              )}
            </div>

            {/* Deep Analysis (Descriptions) */}
            <div className="space-y-32 css-stagger-item">
              {project.architecture && (
                <div className="relative pl-12 border-l-2 border-amber-500/10 hover:border-amber-500/30 transition-colors">
                  <h2 className="font-sans text-4xl font-black uppercase tracking-tighter text-white mb-8">System_Architecture</h2>
                  <p className="text-xl leading-relaxed text-zinc-400 font-medium">{project.architecture}</p>
                </div>
              )}
              
              {project.challenges && (
                <div className="relative pl-12 border-l-2 border-amber-500/10 transition-colors">
                  <h2 className="font-sans text-4xl font-black uppercase tracking-tighter text-white mb-8">Challenges & Solutions</h2>
                  <p className="text-xl leading-relaxed text-zinc-400 font-medium">{project.challenges}</p>
                </div>
              )}

              {project.features && (
                <div className="p-12 rounded-[2.5rem] bg-white/[0.01] border border-white/[0.04] relative overflow-hidden group">
                  <div className="absolute top-0 right-0 w-32 h-32 bg-amber-500/5 blur-[80px] group-hover:bg-amber-500/10 transition-all" />
                  <h2 className="font-sans text-4xl font-black uppercase tracking-tighter text-white mb-10">Key Features</h2>
                  <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-8">
                    {project.features.map((feature, i) => (
                      <div key={feature} className="flex gap-4 group/item">
                        <span className="font-mono text-sm text-amber-500/40 group-hover/item:text-amber-500 font-bold">{String(i + 1).padStart(2, '0')}</span>
                        <div className="h-px flex-1 mt-3 bg-white/[0.05]" />
                        <span className="text-base text-zinc-400 font-medium group-hover/item:text-zinc-100 transition-colors">{feature}</span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>

            {/* Markdown Block */}
            {markdownContent && (
              <div className="project-markdown-block mt-32 p-12 lg:p-20 border-t border-white/5 bg-gradient-to-b from-white/[0.01] to-transparent rounded-t-[4rem]">
                <div className="max-w-4xl mx-auto prose prose-invert prose-amber prose-xl leading-relaxed">
                  <ReactMarkdown rehypePlugins={[rehypeRaw]}>{markdownContent}</ReactMarkdown>
                </div>
              </div>
            )}
          </div>

          {/* SIDEBAR: System Specs */}
          <aside className="lg:col-span-4 lg:sticky lg:top-32 self-start space-y-10 css-stagger-item">
            <div className="p-10 rounded-3xl bg-[#0a0a0a] border border-white/5 relative overflow-hidden">
               {/* Background Grid Accent */}
              <div className="absolute inset-0 opacity-[0.03] pointer-events-none" style={{ backgroundImage: "radial-gradient(var(--accent) 1px, transparent 0)", backgroundSize: "20px 20px" }} />
              
              <h3 className="font-sans text-xl font-bold text-white uppercase mb-10 pb-6 border-b border-white/5 flex items-center justify-between">
                Project Specs
              </h3>

              <div className="space-y-8">

                <div className="space-y-6 pt-6 border-t border-white/5">
                  <p className="font-mono text-[9px] uppercase tracking-[0.3em] text-white/30">Technology Array</p>
                  <ul className="grid grid-cols-1 gap-3">
                    {project.stack.map(tech => (
                      <li key={tech} className="flex items-center gap-3 group/li">
                        <div className="w-1.5 h-1.5 rounded-full bg-amber-500/20 group-hover/li:bg-amber-500 group-hover/li:scale-150 transition-all" />
                        <span className="font-mono text-xs text-white/60 group-hover/li:text-white transition-colors tracking-wide">{tech}</span>
                      </li>
                    ))}
                  </ul>
                </div>
              </div>

              {project.githubUrl && (
                <div className="mt-12">
                  <a href={project.githubUrl} target="_blank" rel="noopener noreferrer" className="btn-primary w-full text-center">
                    Access Source Code
                  </a>
                </div>
              )}
            </div>

            {/* Tactical Sub-Module */}
            <div className="p-8 rounded-3xl border border-white/[0.03] bg-gradient-to-br from-white/[0.02] to-transparent">
              <div className="flex items-center gap-4 mb-4">
                <div className="w-2 h-2 rounded-full bg-amber-500/40" />
                <span className="font-mono text-[10px] uppercase tracking-widest text-zinc-500">Project Status: Complete</span>
              </div>
              <p className="font-mono text-[9px] leading-relaxed text-zinc-600">This documentation outlines the core engineering process and technical outcomes of the project.</p>
            </div>
          </aside>

        </div>
      </main>
    </div>
  );
}
